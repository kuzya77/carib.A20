#ifndef __HEAD_H__
#define __HEAD_H__

#include "carib.h"

#include "h_math.h"

#include <queue>

extern bool MaskComby[5];

namespace Head
{

	//--------------------------------------------------------------------------
	// Анимация начала игры
	// 
	extern const char* drop_in[TERMINAL_COUNT];
	extern const char* drop_part[TERMINAL_COUNT];
	extern const char* drop_out[TERMINAL_COUNT];

	enum HOST_STATE
    {
                HS_NOSTATE      =0,
                HS_WAIT_ANTE,   // Ждем ставку. В этом режиме регистрируются рабочие терминалы,
                                                // воспроизводятся эффекты анимации и возможен переход в режим настройки
                                                // или переход в режим начала игры
                HS_BEGIN_GAME,  // 1я раздача карт
                HS_GAME,                // Собственно игра
                HS_GAME_OVER,
                HS_GAME_EWAIT   // Ожидание завершения (нажатия любой клавиши и перелив бабла)
    };

    const int AnteValues[]={2,5,10,20,30,50,70,100,0};

    extern const char* cards[];
    extern const char* CardPointer[5];

	// Оценка комбинаций, начиная RoyalFlush и до пары
	const int CombyPrice[]= {204,104,44,18,14,12,10,8,6};
	extern /*HOST_STATE*/int previousState;
	extern bool CurrentAnimDone;

	bool Init();

	void send(const NetworkMsg& msg);
	void process();

	//--------------------------------------------------------------------------
	// BlackBox support
	bool BlackBoxInit();
	// чтение/запись настроек
	bool BlackBoxWriteState();
	bool BlackBoxReadIni(const char* name);

	void SendTerminalState(int index, const TerminalState* state);
	void UpdateTerminalState(int i, const TerminalState* state);
	bool TerminalOnLine(int i);
	void UpdateTerminalStates(int card_max=15);

	void PlayerWin(int i,int value);
	bool BackPaid(int i);
	bool Pay(int i, int value, bool save=true);
// Обработка изменения кредита по кнопкам гамепада, 
// Возвращает true, если обработка имела место
	bool CreditKeyUpdate(SDLKey key, int i);

	bool ChangeHolds(int index, SDLKey key);

	// Буфер кнопок
	class RemoteKey
	{
	public:
        int  	from;
        SDLKey  key;
        RemoteKey(int _f, SDLKey _k):from(_f),key(_k) {}
        RemoteKey() {}
	};

	typedef std::queue<RemoteKey> TerminalKeyBuff;

	extern TerminalKeyBuff t_keys;

	// Обработчик состояния Wait
	void hsWaitA();
	void hsGameB();
	void hsGame();
//	void hsGameO();
	void hsGameEW();
}

#endif