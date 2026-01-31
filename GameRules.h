#ifndef GAMERULES_H
#define GAMERULES_H

#include <vector>
#include <string>
#include <functional>
#include <IdContext.hpp>

// Tip definit pentru funcția de broadcast (pentru a trimite mesaje către UI/Clienți)
typedef std::function<void(std::string)> BroadcastFn;

// Structura care menține starea fiecărui jucător în motorul de joc
struct PlayerState {
    TIdContext* context;
    int slotID;          // ID-ul de slot (1-4) pentru consistență
    bool connected;
    bool ready;
    bool hasPlayed;
    std::vector<int> hand;

    PlayerState() : context(nullptr), slotID(0), connected(false),
                    ready(false), hasPlayed(false) {}
};

class GameRules {
private:
    std::vector<PlayerState> players;
    bool gameStarting;
    bool gameInProgress;
    int countdown;
    int maxPlayers;
    int currentPlayerIndex;
    int viewingTimer;
public:
    GameRules(int maxPlayers);
     std::function<void(int, int)> OnStateChanged;
    // Managementul conexiunilor
    void PlayerConnected(TIdContext* ctx);
    void PlayerDisconnected(TIdContext* ctx);

    // Logica de Ready și Start
    void PlayerReady(TIdContext* ctx); // Folosim contextul pentru a găsi jucătorul
    void StartCountdown();
	void Tick();
	  std::string DebugStatus = " test test \n" ; // <--- Adaugă asta

    // Distribuire
    void SendCardsToReadyPlayers();
    void NextPlayerTurn();

	// Getters pentru Backend/UI
    bool IsStarting() const { return gameStarting; }
    bool IsInProgress() const { return gameInProgress; }
    int GetCountdown() const { return countdown; }
    int GetPlayerCount() const { return players.size(); }

    // Metodă pentru a evita eroarea de nullptr în Form4
    PlayerState* GetPlayerBySlot(int slotID) {
		for (auto &p : players) {
			if (p.slotID == slotID) return &p;
        }
        return nullptr;
    }

    // Returnează un pointer la PlayerState după index (0-3)
    PlayerState* GetPlayer(int index) {
        if (index >= 0 && (size_t)index < players.size()) return &players[index];
        return nullptr;
    }
};

#endif
