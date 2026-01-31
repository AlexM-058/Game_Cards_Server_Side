#ifndef PlayerH
#define PlayerH

#include <System.hpp>
#include <IdContext.hpp>
#include <vector>
// =======================
// Starile jucatorului
// =======================
enum class PlayerStatus {
	Free,
	Connected,
	Ready,
	Jumping,
	MyTurn,
	Waiting,
	InGame ,
    Finished



};
// =======================
// Clasa Player
// =======================
class Player {
public:
	int slotID;
    int points;
	TIdContext *context;
	TDateTime connectTime;
	 PlayerStatus status;
	Player(int pid, TIdContext* ctx);
};

// =======================
// Declaratii globale
// =======================
extern std::vector<Player> players;
extern int nextPlayerID;
extern TCriticalSection *playersLock;
extern std::vector<int> Cards;

// =======================
// Functii
// =======================
int getRandomCard();
void SendCards(TIdContext *AContext);
 void SendTheNextCard(TIdContext *AContext);


#endif

