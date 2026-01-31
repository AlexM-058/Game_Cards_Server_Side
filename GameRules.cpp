#include "GameRules.h"
#include "Player.h"
#include "Backend.h"
#include <algorithm>
#include <ctime>
#include <random>
#include <sstream>
#include <iostream>
#include <IdIOHandler.hpp>

extern void SendCards(TIdContext *AContext);

//---------------------------------------------------------------------------
// 1. CONSTRUCTOR
//---------------------------------------------------------------------------

GameRules::GameRules(int maxPlayers)
	: gameStarting(false), countdown(0), maxPlayers(maxPlayers),
	  currentPlayerIndex(0), gameInProgress(false), viewingTimer(0)
{
	for(int i = 0; i < maxPlayers; i++) {
		PlayerState ps;
		ps.slotID = i + 1;
		ps.connected = false;
		ps.ready = false;
		ps.context = nullptr;
		players.push_back(ps);
	}
}

//---------------------------------------------------------------------------
// 2. GESTIUNEA JUCĂTORILOR (CONECTARE, DECONECTARE, READY)
//---------------------------------------------------------------------------

void GameRules::PlayerConnected(TIdContext* ctx)
{
	for (auto &p : players) {
		if (!p.connected) {
			p.connected = true;
			p.context = ctx;
			break;
		}
	}
}

void GameRules::PlayerDisconnected(TIdContext* ctx)
{
	for (auto &p : players) {
		if (p.context == ctx) {
			p.connected = false;
			p.ready = false;
			p.context = nullptr;
			p.hand.clear();
			break;
		}
	}
}

void GameRules::PlayerReady(TIdContext* ctx)
{
	this->DebugStatus = "Căutare jucător...";

	for (auto &p : players) {
		if (p.context == ctx) {
			p.ready = true;
			this->DebugStatus = "Jucător pregătit (READY)";
			break;
		}
	}

	int readyCount = 0;
	for (auto &p : players) {
		if (p.ready) readyCount++;
	}

	if (readyCount >= 2 && !gameStarting && !gameInProgress) {
		this->StartCountdown();
		this->DebugStatus = "Countdown activat: " + std::to_string(countdown);
	}
}

//---------------------------------------------------------------------------
// 3. LOGICA DE TIMP ȘI SINCRONIZARE (TICK)
//---------------------------------------------------------------------------

void GameRules::StartCountdown()
{
	if (!gameStarting) {
		gameStarting = true;
		countdown = 5;
	}
}

void GameRules::Tick()
{
    if (gameStarting) {
        if (countdown > 0) {
            countdown--;
        } else {
            gameStarting = false;
            viewingTimer = 10;

            for (auto &p : players) {
                if (p.ready && p.context) {
                    if (OnStateChanged) OnStateChanged(p.slotID, (int)PlayerStatus::InGame);

                    p.context->Connection->Socket->WriteLn("COMINGDECK");
                    SendCards(p.context);
                }
            }
        }
        return;
    }

    if (viewingTimer > 0) {
        viewingTimer--;
        this->DebugStatus = "Timp memorare: " + std::to_string(viewingTimer);

        if (viewingTimer == 0) {
            gameInProgress = true;
            currentPlayerIndex = -1;
            NextPlayerTurn();
        }
    }
}

//---------------------------------------------------------------------------
// 4. CONTROLUL TURNURILOR ȘI DISTRIBUIRE CĂRȚI
//---------------------------------------------------------------------------

void GameRules::SendCardsToReadyPlayers()
{
    for (auto &p : players) {
        if (p.ready && p.context && p.context->Connection->Connected()) {
            p.context->Connection->Socket->WriteLn("COMINGDECK");
        }
    }

    for (auto &p : players) {
        if (p.ready && p.context && p.context->Connection->Connected()) {
            SendCards(p.context);
        }
    }
}

void GameRules::NextPlayerTurn()
{
    if (currentPlayerIndex >= 0 && (size_t)currentPlayerIndex < players.size()) {
        PlayerState &prevPlayer = players[currentPlayerIndex];
        if (OnStateChanged) OnStateChanged(prevPlayer.slotID, (int)PlayerStatus::Waiting);

        if (prevPlayer.context && prevPlayer.connected) {
             prevPlayer.context->Connection->Socket->WriteLn("WAIT");
        }
    }

    size_t nextIndex = currentPlayerIndex;
    int safetyCount = 0;
    do {
        nextIndex = (nextIndex + 1) % players.size();
        safetyCount++;
    } while ((!players[nextIndex].connected || !players[nextIndex].ready) && safetyCount < maxPlayers * 2);

    currentPlayerIndex = nextIndex;
    PlayerState &nextPlayer = players[currentPlayerIndex];

    if (nextPlayer.context && nextPlayer.connected) {
        if (OnStateChanged) OnStateChanged(nextPlayer.slotID, (int)PlayerStatus::MyTurn);

        int randomCardIndex = getRandomCard();

        try {
            nextPlayer.context->Connection->Socket->WriteLn("YOURTURN");
            nextPlayer.context->Connection->Socket->Write(randomCardIndex);

            this->DebugStatus = "Jucător " + std::to_string(nextPlayer.slotID) +
                                " -> Carte primită: " + std::to_string(randomCardIndex);
        }
        catch (...) {
            NextPlayerTurn();
        }
    }
}
