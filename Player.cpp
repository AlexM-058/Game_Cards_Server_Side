#include "Player.h"
#include <random>
#include <System.SyncObjs.hpp>
#include <Windows.hpp>

//---------------------------------------------------------------------------
// 1. VARIABILE GLOBALE ȘI RESURSE PARTAJATE
//---------------------------------------------------------------------------

std::vector<Player> players;
int nextPlayerID = 1;
TCriticalSection *playersLock = new TCriticalSection();
std::vector<int> Cards(53, 0);

//---------------------------------------------------------------------------
// 2. IMPLEMENTARE CLASA PLAYER
//---------------------------------------------------------------------------

Player::Player(int pid, TIdContext* ctx)
{
	slotID = pid;
	context = ctx;
	connectTime = Now();
	status = PlayerStatus::Connected;
	points = 0;
}

//---------------------------------------------------------------------------
// 3. LOGICA DE GENERARE ȘI DISTRIBUIRE CĂRȚI
//---------------------------------------------------------------------------

int getRandomCard()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 52);
	return dis(gen);
}

void SendCards(TIdContext *AContext)
{
	String cardList = "";

	for (int i = 0; i < 4; i++) {
		int s = -1;
		int attempts = 0;

		while (attempts < 100) {
			int candidate = getRandomCard();
			if (Cards[candidate] == 0) {
				s = candidate;
				break;
			}
			attempts++;
		}

		if (s == -1) {
			for (int j = 1; j <= 52; j++) {
				if (Cards[j] == 0) { s = j; break; }
			}
		}

		if (s != -1) {
			Cards[s] = 1;

			if (cardList != "") cardList += ",";
			cardList += IntToStr(s);
		}
	}

	if (cardList != "") {
		AContext->Connection->IOHandler->WriteLn(cardList);
	}
}

void SendTheNextCard(TIdContext *AContext)
{
	int s = -1;
	bool deckIsEmpty = true;

	for (int i = 1; i <= 52; i++) {
		if (Cards[i] == 0) {
			deckIsEmpty = false;
			break;
		}
	}

	if (deckIsEmpty) {
		AContext->Connection->IOHandler->WriteLn("GAME_FINISHED_DECK_EMPTY");
		return;
	}

	int attempts = 0;
	while (attempts < 50) {
		int candidate = getRandomCard();
		if (Cards[candidate] == 0) {
			s = candidate;
			break;
		}
		attempts++;
	}

	if (s == -1) {
		for (int i = 1; i <= 52; i++) {
			if (Cards[i] == 0) { s = i; break; }
		}
	}

	if (s != -1) {
		Cards[s] = 1;
		AContext->Connection->IOHandler->WriteLn("YOURTURN");
		AContext->Connection->IOHandler->Write(s);
	}
}
