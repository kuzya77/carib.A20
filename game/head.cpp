#include "head.h"
#include "terminal.h"

#include <stuff/cmd.h>

#include <queue>

namespace Head
{
	const char* drop_in[TERMINAL_COUNT]   = {"drop_in_l", "drop_in_c", "drop_in_r"};
	const char* drop_part[TERMINAL_COUNT]  = {"drop_part_l", "drop_part_c", "drop_part_r"};
	const char* drop_out[TERMINAL_COUNT]   = {"drop_out_l", "drop_out_c", "drop_out_r"};

	std::queue<NetworkMsg> msgQueue;
	TerminalKeyBuff t_keys;

	bool CurrentAnimDone=false;

	/*HOST_STATE*/int previousState=HS_NOSTATE;	// Служит для переходов анимации при переключении состояний

	static void gamebTrickDone(const AttrList&)
	{
		CurrentAnimDone=true;
	}

	bool Init()
	{
	    logTitle("Init");
		cmdAddCmd("gamebTrickDone", gamebTrickDone);

    	int i;

    // Чтение состояния игры
    	print(WHITE,"BlackBox...");
	    if(BlackBoxInit())
	    	print(GREEN, "Ok\n");
    	else
	    {
    		print(RED, ". Failed\n");
    		return false;
	    }
	    mathInit();
    	//  сделана, возвращаем деньги:
	    switch(hsw.state)
    	{
        	default:
	        case HS_NOSTATE:
    	        for(i=0;i<TERMINAL_COUNT;i++)
        	    {
            		for(int j=0;j<5;j++)
                    	hsw.console[i].cards[j]=0;
                    hsw.console[i].paid=0;
                    hsw.console[i].win=0;
            	}
            	for(i=0;i<5;i++)
                    hsw.cards[i]=0;
            	hsw.state=HS_WAIT_ANTE;
        	case HS_WAIT_ANTE: // если до перезагрузки сидели в ожидании, и часть ставок была сделана, возвращаем деньги
            	for(i=0;i<TERMINAL_COUNT;i++)
            	{
                    hsw.console[i].win=0;
                    BackPaid(i);
                    hsw.console[i].game=0;
            	}
            	break;
        	case HS_BEGIN_GAME:
        	case HS_GAME:
	        case HS_GAME_OVER:
            	break;
        	case HS_GAME_EWAIT:
            	hsw.state=HS_WAIT_ANTE;
            	for(int i=0;i<TERMINAL_COUNT;i++)
	            {
                    PlayerWin(i,hsw.console[i].win);
                    hsw.console[i].LastWin=hsw.console[i].win;
                    hsw.console[i].win=0;
                    hsw.console[i].paid=0;
                    hsw.console[i].game=0;
                    for(int j=0;j<5;j++)
                            hsw.console[i].cards[j]=0;
            	}
            	BlackBoxWriteState();
            	break;
    	}
		return true;
	}

	void send(const NetworkMsg& msg)
	{
		msgQueue.push(msg);
	}

	void SendTerminalState(int index, const TerminalState* state)
	{
	    NetworkMsg msg;   
	    msg.state.from=0;
    	msg.state.to=index;
	    msg.state.cmd=NCMD_T_UPDATE_STATE;
    	msg.state.tstate=*state;
	    Terminal::send(msg);
	}

	//-------------------------------------------------------------------------------
	// Обновление (периодическое и по требованию) состояний рабочих мест
	void UpdateTerminalState(int i, const TerminalState* state)
	{
    	if(TerminalOnLine(i))
        	SendTerminalState(i+1,state);
	}

	bool TerminalOnLine(int i)
	{
    	return Terminal::TerminalID==i+1;
	}

	void UpdateTerminalStates(int card_max)
	{
    	TerminalState tstate;
	    // Обновление состояния терминалов...
    	for(int i=0;i<TERMINAL_COUNT;i++)
	    {
    	    if(TerminalOnLine(i))
	        {
        	    tstate=hsw.console[i];
    	        if(card_max<5)
	            {               
            	    if(card_max>0)
    	                for(int j=card_max;j<5;j++)
        	                tstate.cards[j]=0;
	                else
                	    for(int j=0;j<5;j++)
            	            tstate.cards[j]=0;
        	    }
    	        SendTerminalState(i+1,&tstate);
	        }
        	card_max-=5;
    	}
	}
	
	void process()
	{
		while(!msgQueue.empty())
		{
			NetworkMsg msg = msgQueue.front();
			msgQueue.pop();

        	if(msg.common.from<1 || msg.common.from>TERMINAL_COUNT)
            	continue;

	        switch(msg.common.cmd)
    	    {
        	    case NCMD_TERMINAL_KEY: // на терминале нажали кнопку...
            	    if(msg.key.key) // кнопка!= NULL
            	    {
            	    	if(CreditKeyUpdate(msg.key.key, msg.common.from-1))	// credit updated, update terminal state
            	    		UpdateTerminalStates();	// TODO: can show uproper value when cards are flying
                	 	else
                	 		t_keys.push(RemoteKey(msg.common.from, msg.key.key));	// push key into key queue
                	}
	                break;
    	    }
		}
		
		switch(hsw.state)
        {
            case HS_WAIT_ANTE:
                hsWaitA();
                break;
            case HS_BEGIN_GAME:
                hsGameB();
                break;
            case HS_GAME:
//                hsGame();
//                break;
            case HS_GAME_OVER:
//                hsGameO();
//                break;
            case HS_GAME_EWAIT:
                hsGameEW();
                break;
        }

	}
}
