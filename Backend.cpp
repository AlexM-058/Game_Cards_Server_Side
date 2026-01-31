#include <vcl.h>
#include <System.SysUtils.hpp>
#pragma hdrstop
#include <vector>
#include <random>
#include <memory>

#include "Backend.h"
#include "GameRules.h"
#include "Player.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

using namespace std;

TForm4 *Form4;
GameRules* game = new GameRules(4);

//---------------------------------------------------------------------------
// 1. CONSTRUCTOR ȘI DESTRUCTOR
//---------------------------------------------------------------------------

__fastcall TForm4::TForm4(TComponent* Owner) : TForm(Owner)
{
	playersLock = new TCriticalSection(); //obiect de sincronizare
    attackConfirmations = 0;

    // Resetare vizuală elemente UI la pornire
    Play1->Text = "Liber"; Play1->Color = clSilver;
    Play2->Text = "Liber"; Play2->Color = clSilver;
    Play3->Text = "Liber"; Play3->Color = clSilver;
    Play4->Text = "Liber"; Play4->Color = clSilver;

    // Mapare evenimente Indy
    IdTCPServer1->OnConnect    = IdTCPServer1Connect;
    IdTCPServer1->OnDisconnect = IdTCPServer1Disconnect;
    IdTCPServer1->OnExecute    = IdTCPServer1Execute;
}

void __fastcall TForm4::FormDestroy(TObject *Sender)
{
    if (playersLock) delete playersLock;
    if (game) delete game;
}

//---------------------------------------------------------------------------
// 2. MANAGEMENTUL CONEXIUNILOR (CONNECT / DISCONNECT)
//---------------------------------------------------------------------------

void __fastcall TForm4::IdTCPServer1Connect(TIdContext *AContext)
{
    playersLock->Enter();
    try {
        int slot = GetFreeSlot();

        if (slot == -1) {
            AContext->Connection->Socket->WriteLn("Full");
            AContext->Connection->Disconnect();
            playersLock->Leave();
            return;
        }

        // Adăugare în lista pentru Interfață
        players.push_back(Player(slot, AContext));

        // Adăugare în Motorul de Joc (Logică)
        if (game) {
            game->PlayerConnected(AContext);
        }

        AContext->Connection->Socket->Write(slot);
        UpdatePlayerView(slot, PlayerStatus::Connected);
    }
    catch (...) {
        playersLock->Leave();
        throw;
    }
    playersLock->Leave();
}

void __fastcall TForm4::IdTCPServer1Disconnect(TIdContext *AContext)
{
    int slotEliberat = -1;
    playersLock->Enter();
	__try {
        auto it = std::find_if(players.begin(), players.end(),
            [AContext](const Player &p){ return p.context == AContext; });

        if (it != players.end()) {
            slotEliberat = it->slotID;
            it->status = PlayerStatus::Free;
            UpdatePlayerView(slotEliberat, PlayerStatus::Free);
            players.erase(it);
		}
    }
    __finally {
        playersLock->Leave();
    }

    if (slotEliberat != -1) {
		String mesajDeconectare = "Jucatorul " + IntToStr(slotEliberat) + " s-a deconectat.";
		BroadcastMessage(mesajDeconectare);
	}
}

void TForm4::DisconnectByID(int id)
{
    playersLock->Enter();
	Player* playerToKick = nullptr;
    try {
        for (auto &p : players) {
            if (p.slotID == id) {
                playerToKick = &p;
				break;
            }
        }
        if (playerToKick) {
			playerToKick->status = PlayerStatus::Free;
            UpdatePlayerView(playerToKick->slotID, PlayerStatus::Free);
		}
	}
	catch (...) {}
	playersLock->Leave();

	if (playerToKick && playerToKick->context) {
        try { playerToKick->context->Connection->Disconnect(); } catch (...) {}
    }

	playersLock->Enter();
	try {
        for (auto it = players.begin(); it != players.end(); ++it) {
			if (it->slotID == id) {
                players.erase(it);
				break;
			}
		}
    }
	catch (...) {}
	playersLock->Leave();
}

//---------------------------------------------------------------------------
// 3. LOGICA PRINCIPALĂ DE REȚEA (ON EXECUTE)
//---------------------------------------------------------------------------

void __fastcall TForm4::IdTCPServer1Execute(TIdContext *AContext)
{
	try {
		int slot = GetPlayerID(AContext);
		String receivedData = AContext->Connection->Socket->ReadLn();

		// --- MECANISM ATAC (BARIERĂ SINCRONIZARE) ---
        if (receivedData == "STARTPLAYINGTHERIGHTCARD")
        {
			int cardID = AContext->Connection->Socket->ReadInt32();
			playersLock->Enter();
			attackConfirmations = 0;
            playersLock->Leave();

            BroadcastMessage("ATTACK");
            BroadcastNumber(cardID);
		}

        if (receivedData == "DONEAT")
		{
            bool toataLumeaEGata = false;
            playersLock->Enter();
            try {
				attackConfirmations++;
                if (attackConfirmations >= (int)players.size()) {
					toataLumeaEGata = true;
					attackConfirmations = 0;
                }
			}
			catch (...) { playersLock->Leave(); throw; }
			playersLock->Leave();

            if (toataLumeaEGata && game && game->IsInProgress()) {
				game->NextPlayerTurn();
            }
        }

        // --- COMENZI STATUS ȘI FLUX JOC ---
        if (receivedData == "READY") {
			if (game) game->PlayerReady(AContext);
			int s = GetPlayerID(AContext);
			if (s != -1) UpdatePlayerView(s, PlayerStatus::Ready);
		}

        if (receivedData == "DONE") {
			if (game && game->IsInProgress()) game->NextPlayerTurn();
		}

		if (receivedData == "PRIMITCARTI") {
			SendCards(AContext);
		}

		if (receivedData == "SEND") {
			SendTheNextCard(AContext);
		}

		// --- SCOR ȘI FINALIZARE ---
        if (receivedData == "MYPOINTS") {
			int incomingScore = AContext->Connection->Socket->ReadInt32();
			ProcessPlayerScore(slot, incomingScore);
        }

        if (receivedData == "PLAYER_CALLED_FINISH") {
            BroadcastMessage("GAME_FINISHED_DECK_EMPTY");
		}

        // --- UTILITARE ---
		if (receivedData == "CALCULEAZA") {
            int FirstN = AContext->Connection->Socket->ReadInt32();
			int SecondN = AContext->Connection->Socket->ReadInt32();
			AContext->Connection->Socket->Write(FirstN + SecondN);
		}

		if (receivedData == "EXIT") {
			int idJucator = GetPlayerID(AContext);
			if (idJucator != -1) DisconnectByID(idJucator);
		}
	}
	catch (const Exception &e) {
        AContext->Connection->Disconnect();
	}
}

//---------------------------------------------------------------------------
// 4. FUNCȚII DE BROADCAST (TRIMITERE MESAJE)
//---------------------------------------------------------------------------

void __fastcall TForm4::BroadcastMessage(const String &mesaj)
{
	playersLock->Enter();
    __try {
        for (auto &p : players) {
			if (p.context && p.context->Connection->Connected()) {
                p.context->Connection->Socket->WriteLn(mesaj);
            }
		}
    }
    __finally { playersLock->Leave(); }
}

void __fastcall TForm4::BroadcastNumber(int numar)
{
    playersLock->Enter();
    __try {
        for (auto &p : players) {
            if (p.context && p.context->Connection->Connected()) {
				p.context->Connection->Socket->Write(numar);
            }
        }
    }
    __finally { playersLock->Leave(); }
}

//---------------------------------------------------------------------------
// 5. GESTIUNE SCOR ȘI CÂȘTIGĂTOR
//---------------------------------------------------------------------------

void TForm4::ProcessPlayerScore(int slotID, int score)
{
	bool isGameOver = false;
	playersLock->Enter();
	try {
		for (auto &p : players) {
			if (p.slotID == slotID) {
				p.points = score;
				p.status = PlayerStatus::Finished;
				break;
			}
		}

		isGameOver = true;
        for (const auto &p : players) {
            if (p.status != PlayerStatus::Finished) {
				isGameOver = false;
                break;
            }
        }
	} catch (...) {}
	playersLock->Leave();

	if (isGameOver) AnnounceWinner();
}

void TForm4::AnnounceWinner()
{
    int winnerID = -1;
    int minPoints = 10000;

    playersLock->Enter();
    try {
        for (const auto& p : players) {
            if (p.status == PlayerStatus::Finished) {
                if (p.points < minPoints) {
                    minPoints = p.points;
                    winnerID = p.slotID;
                }
            }
        }
    }
    catch (...) {}
    playersLock->Leave();

    String winnerMsg = (winnerID != -1) ?
        "Jucatorul " + IntToStr(winnerID) + " a castigat cu " + IntToStr(minPoints) + " puncte!" :
        "Nu s-a putut determina un castigator.";

    BroadcastMessage("WINNERIS");
    BroadcastMessage(winnerMsg);

    TThread::CreateAnonymousThread([this]() {
        Sleep(2000);
        BroadcastMessage("RESET_GAME");
        playersLock->Enter();
        for (auto &p : players) {
            p.status = PlayerStatus::Connected;
            p.points = 0;
            UpdatePlayerView(p.slotID, PlayerStatus::Connected);
        }
        playersLock->Leave();
    })->Start();
}

//---------------------------------------------------------------------------
// 6. UTILITARE INTERFAȚĂ (UI) ȘI CĂUTARE
//---------------------------------------------------------------------------

void TForm4::UpdatePlayerView(int slot, PlayerStatus status)
{
	TThread::Synchronize(nullptr, [=, this]() {
		TEdit* targetEdit = nullptr;
        switch (slot) {
			case 1: targetEdit = Play1; break;
            case 2: targetEdit = Play2; break;
            case 3: targetEdit = Play3; break;
			case 4: targetEdit = Play4; break;
        }
		if (!targetEdit) return;

		switch (status) {
			case PlayerStatus::Free:      targetEdit->Text = "LIBER"; targetEdit->Color = clSilver; break;
			case PlayerStatus::Connected: targetEdit->Text = "CONECTAT"; targetEdit->Color = clSkyBlue; break;
			case PlayerStatus::Ready:     targetEdit->Text = "READY"; targetEdit->Color = clLime; break;
			case PlayerStatus::Jumping:   targetEdit->Text = "JUMPING (Carti)"; targetEdit->Color = clFuchsia; break;
            case PlayerStatus::InGame:    targetEdit->Text = "MEMORARE (10s)"; targetEdit->Color = clWebLightGreen; break;
            case PlayerStatus::MyTurn:    targetEdit->Text = ">>> LA RAND <<<"; targetEdit->Color = clYellow; break;
			case PlayerStatus::Waiting:   targetEdit->Text = "Asteapta..."; targetEdit->Color = clWebLightBlue; break;
            case PlayerStatus::Finished:  targetEdit->Text = "SCOR TRIMIS"; targetEdit->Color = clMedGray; break;
		}
	});
}

int TForm4::GetFreeSlot()
{
    for (int i = 1; i <= 4; i++) {
        bool occupied = false;
        for (const auto& p : players) { if (p.slotID == i) { occupied = true; break; } }
        if (!occupied) return i;
    }
    return -1;
}

int TForm4::GetPlayerID(TIdContext *AContext)
{
    for (const auto &p : players) { if (p.context == AContext) return p.slotID; }
    return -1;
}

TIdContext* TForm4::GetContextForPlayer(int slot)
{
    for (const auto& p : players) { if (p.slotID == slot) return p.context; }
    return nullptr;
}

//---------------------------------------------------------------------------
// 7. TIMERE ȘI BUTOANE ADMIN
//---------------------------------------------------------------------------

void __fastcall TForm4::Timer1Timer(TObject *Sender)
{
    if (game) {
        game->Tick();
        CountDown->Caption = game->DebugStatus.c_str();
    }
}

void __fastcall TForm4::ResetClick(TObject *Sender)
{
    Timer1->Enabled = false;
    playersLock->Enter();
    try {
        for (unsigned int i = 0; i < players.size(); i++) {
            if (players[i].context && players[i].context->Connection->Connected()) {
                try {
                    players[i].context->Connection->Socket->WriteLn("RESET");
                    players[i].context->Connection->Disconnect();
                } catch (...) {}
            }
        }
        players.clear();

        if (game) { delete game; game = nullptr; }
        game = new GameRules(4);
        std::fill(Cards.begin(), Cards.end(), 0);

        Play1->Text = "Liber"; Play1->Color = clSilver;
        Play2->Text = "Liber"; Play2->Color = clSilver;
        Play3->Text = "Liber"; Play3->Color = clSilver;
        Play4->Text = "Liber"; Play4->Color = clSilver;
        CountDown->Caption = "Gata";
    }
    catch (...) {
        playersLock->Leave();
        Timer1->Enabled = true;
        throw;
    }
    playersLock->Leave();
    Timer1->Enabled = true;
}

void __fastcall TForm4::CountDownClick()
{
    CountDown->Caption ="5";
}
