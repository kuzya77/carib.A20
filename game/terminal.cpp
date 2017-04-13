#include "terminal.h"
#include "head.h"

#include <stuff/anim.h>
#include <stuff/cmd.h>

#include <queue>

namespace Terminal
{
	#define WAVE_MSG_COUNT  3

	int TerminalID=2;

	TerminalState tms={0,0,0,0,0,0,0,0,0,0,0};

	const char* movecards[5] = 	{ "move0", "move1", "move2", "move3", "move4" };
	const char* holds[5] = 		{ "hold0", "hold1", "hold2", "hold3", "hold4" };
	const char* cards[5] = 		{ "card_s0", "card_s1", "card_s2", "card_s3", "card_s4" };

	std::queue<NetworkMsg> msgQueue;

	static void cmdFlyDoneEvent(const AttrList& list)
	{
		if(list.count()!=1)
			return;
		int i=list[0];
		if(i<0 || i>4)
			return;
	   	animShow(cards[i], (tms.cards[i]%100)-1);
  		if(i==4)
	   	for(int j=0; j<5; j++)
    		if(tms.cards[j]>=100)
        		animShow(holds[j], (tms.cards[j]%100)-1);	
	}

	bool Init()
	{
	    cmdAddCmd("FlyDone", cmdFlyDoneEvent);
	    return true;
	}

	int wavemsgCurrent=0;

	const char* wavemsg[WAVE_MSG_COUNT+1]={"bkground", "wave1", "wave2", "wave3"};

	static void wavemsgSet(int mode)
	{   
		if(mode<0 || mode>WAVE_MSG_COUNT)
			mode=0;
		if(wavemsgCurrent!=mode)
		{
			if(mode)
				animMerge(wavemsg[mode], wavemsg[wavemsgCurrent]);
			if(wavemsgCurrent)
				animHide(wavemsg[wavemsgCurrent]);
	    	wavemsgCurrent=mode;
    	}
	}

	static void SetValue(const char* name, int oldv, int newv)
	{
		if(oldv != newv)
	    {
    		char s[50];
			if(newv>0)
	      		sprintf(s, "%d", newv);
    	    else
        		s[0]='\0';
	    	animSetIndicator(name, s);
		}
	}	

	void send(const NetworkMsg& msg)	// msg: * -> terminal
	{
		msgQueue.push(msg);
	}

	void process()
	{
		while(!msgQueue.empty())
		{
			const NetworkMsg& msg = msgQueue.front();
			if(msg.common.from==0 && msg.common.to==TerminalID)	// compatibility check
			{
                switch(msg.common.cmd)
                {
                    case NCMD_T_UPDATE_STATE:
                        SetValue("credit", tms.credit, msg.state.tstate.credit);
                        tms.credit = msg.state.tstate.credit;

                        SetValue("lastwin", tms.LastWin, msg.state.tstate.LastWin);
                        tms.LastWin =msg.state.tstate.LastWin;

                        if(tms.win!=msg.state.tstate.win)
                        {
                            if((msg.state.tstate.win==0 && tms.win==msg.state.tstate.ante*4) || 
                                (tms.win==0 && msg.state.tstate.win==msg.state.tstate.ante*4))
                                animHide("win_show");
                            else
                            	animPlay(msg.state.tstate.win==0?"win_hide":"win_show");
                            SetValue("win", tms.win, msg.state.tstate.win);
                            tms.win=msg.state.tstate.win;
                        }

                        if(tms.ante!=msg.state.tstate.ante)
    					{
	    					tms.ante  = msg.state.tstate.ante;
    						char s[100];
    						sprintf(s, "ante%d", tms.ante);
        					animPlay(s);
    					}

                        if(tms.paid==0 && msg.state.tstate.paid) // запуск анимации полета фишки к полю анте
                        	animPlay("antemove");

                        SetValue("paid", tms.paid, msg.state.tstate.paid);
                        tms.paid=msg.state.tstate.paid;

                        if(tms.paid==0)
                        	animHide("antemove");
                        // Если терминалу передали карты, то запускаем анимацию
                        for(int i=0;i<5;i++)
                        	if(tms.cards[i]!=msg.state.tstate.cards[i])
                            {
                                	if(tms.cards[i])
                                	{
	                                	animHide(cards[i]);
	                                	animHide(holds[i]);

	                                	tms.cards[i]=msg.state.tstate.cards[i];
	                                	if(tms.cards[i])
		                                	animShow(cards[i], (tms.cards[i]%100)-1);
                						if(tms.cards[i]>=100)
                							animShow(holds[i], (tms.cards[i]%100)-1);
	                                }
									else 	// start animation
									{
										tms.cards[i]=msg.state.tstate.cards[i];
                                		animPlay(movecards[i]);
                                	}
                            }

                        if(tms.game!=msg.state.tstate.game)
                        {   
                            switch(msg.state.tstate.game)
                            {
                                case 1:
                                	animHide("betmove");
                                    wavemsgSet(1);
                                    break;
                                case 2:
                                	animHide("betmove");
                                    wavemsgSet(2);
                                    break;
                                case 4:
                                	animPlay("betmove");
                                case 3:
                                    wavemsgSet(0);
                                    break;
                                case 5:
                                	if(!tms.game)
                                		animPlay("betmove");
                                    wavemsgSet(3);
                                    break;
                                default:
                                	animHide("betmove");
                                    wavemsgSet(0);
                            }
                            tms.game=msg.state.tstate.game;
                        }
                }
			}
			msgQueue.pop();
		}
	}

	// temporary hack to avoid double press react (in fact, there is no double press)
	static u32_t prev_call_time = 0;

	void keyPress(SDLKey key)
	{
		if(clock()-prev_call_time<CLOCKS_PER_SEC/2)
		{
			return;
		}

		prev_call_time =clock();

    	switch(key)
        {
        	case SDLK_z:
	            TerminalID=1;            
				break;
            case SDLK_x:
                TerminalID=2;
	            break;
            case SDLK_c:
                TerminalID=3;
                break;
		}
        NetworkMsg msg;
        msg.key.from= TerminalID;
        msg.key.to  = 0;
        msg.key.cmd = NCMD_TERMINAL_KEY; // код команды
        msg.key.key = key;
        Head::send(msg);
	}
}
