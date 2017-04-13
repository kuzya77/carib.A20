#include "head.h"

// Обработка финала 2:
// Выход по "Start" с любого терминала

namespace Head
{
	static int stage=0;

	void hsGameEW()
	{
		if(previousState!=hsw.state)
		{
		    previousState=hsw.state;
		    stage = 0;
		}

		switch(stage)
		{
			case 0:
				{
		    		int dComby=GetComby(hsw.cards);
    				for(int i=0;i<5;i++)
				    {
    					if(MaskComby[i])
		    				animPlay(CardPointer[i]);
    					if(hsw.cards[i])
			    			animShow(cards[i], hsw.cards[i]-1);
				    }
				    animShow("combolist", dComby?10-(dComby>>8):0);
				    t_keys = TerminalKeyBuff();
				    stage++;
			    }
			    break;
			case 1:	    // Ждем любую клавишу для перехода в режим HS_WAIT_ANTE
				if(!t_keys.empty())
				{
					stage++;
					animPlay("hide_all");
					CurrentAnimDone=false;
				}
				break;
			case 2:
				if(CurrentAnimDone)
				{
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
				    for(int i=0;i<5;i++)
				        hsw.cards[i]=0;
            		BlackBoxWriteState();
       		        UpdateTerminalStates(); // Обновление состояния терминалов...
				}
				break;
		}
	}
}
