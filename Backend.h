//---------------------------------------------------------------------------

#ifndef BackendH
#define BackendH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
// Includerile pentru Indy (Componenta TCP Server)
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdTCPServer.hpp>
#include <IdContext.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <utility>
#include "Player.h"
// === ATENȚIE: Aceasta este linia CRITICĂ pentru a scăpa de erori ===
#include <syncobjs.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------

class TForm4 : public TForm
{
__published:	// Componente gestionate de IDE (puse pe formă)
	TIdTCPServer *IdTCPServer1;
	TEdit *Play1;
	TEdit *Play2;
	TEdit *Play3;
	TEdit *Play4;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *CountDown;
	TTimer *Timer1;
	TButton *Reset;

	// Declarațiile evenimentelor (trebuie să corespundă cu ce e în .cpp)
	void __fastcall IdTCPServer1Connect(TIdContext *AContext);
    	void __fastcall IdTCPServer1Disconnect(TIdContext *AContext);
	void __fastcall IdTCPServer1Execute(TIdContext *AContext);

	// Evenimentul de distrugere a formei (pentru a șterge Lock-ul)
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall ResetClick(TObject *Sender);

private:	// Declarații private

	TCriticalSection *playersLock;
    int attackConfirmations;
	int GetFreeSlot();
	 void BroadcastMessage(const String &mesaj);
    void DisconnectByID(int id);
	 void ProcessPlayerScore(int slotID, int score);
	 void AnnounceWinner();

    int GetPlayerID(TIdContext *AContext);
	void UpdateGUI(int slot, bool connected);
	 std::pair<int, int> currentWinner;
public:
	__fastcall TForm4(TComponent* Owner);
	 void UpdatePlayerView(int slot, PlayerStatus status);
	 void __fastcall CountDownClick();
		TIdContext* GetContextForPlayer(int slot);
		void __fastcall BroadcastNumber(int numar);
};

//---------------------------------------------------------------------------
extern PACKAGE TForm4 *Form4;
//---------------------------------------------------------------------------
#endif
