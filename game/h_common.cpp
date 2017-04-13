#include "head.h"

namespace Head
{
	const char* cards[]={"card0", "card1", "card2", "card3", "card4"};
	const char* CardPointer[5]	= {"pointer0", "pointer1", "pointer2", "pointer3", "pointer4"};

	void CreditAppend(int i,int crVal)
	{
        hsw.console[i].credit+=crVal;

        hsw.mLocal.money_in+=crVal;
        hsw.mTotal.money_in+=crVal;

        hsw.cMoney[i].money_in+=crVal;
        hsw.cMoneyTotal[i].money_in+=crVal;
	}

	// Выигрыш игрока
	void PlayerWin(int i,int value)
	{
        hsw.console[i].credit+=value;

        hsw.mLocal.turn_out+=value;
        hsw.mTotal.turn_out+=value;

        hsw.cMoney[i].turn_out+=value;
        hsw.cMoneyTotal[i].turn_out+=value;
	}

	// Оплата действия (игра, покупка диллеру, обмен и т.д.)
	bool Pay(int i, int value, bool save)
	{
        if(value>hsw.console[i].credit)
			return false;
        hsw.console[i].credit-=value;
        hsw.console[i].paid+=value;
        hsw.mLocal.turn_in+=value;
        hsw.mTotal.turn_in+=value;

        hsw.cMoney[i].turn_in+=value;
        hsw.cMoneyTotal[i].turn_in+=value;

        if(save)
			BlackBoxWriteState();
        return true;
	}


	bool BackPaid(int i)
	{
        if(i<0 || i>=TERMINAL_COUNT || !hsw.console[i].paid)
        	return false;
        hsw.console[i].credit+=hsw.console[i].paid;
        hsw.mLocal.turn_in-=hsw.console[i].paid;
        hsw.mTotal.turn_in-=hsw.console[i].paid;
        hsw.cMoney[i].turn_in-=hsw.console[i].paid;
        hsw.cMoneyTotal[i].turn_in-=hsw.console[i].paid;
        hsw.console[i].paid=0;          // Бабульки тю-тю
        return true;
	}

	// Обновление состояния холдов
	bool ChangeHolds(int index, SDLKey key)
	{
		int ikey;
		switch(key)
		{
			case SDLK_3:	ikey=0;	break;
			case SDLK_4:	ikey=1;	break;
			case SDLK_5:	ikey=2;	break;
			case SDLK_6:	ikey=3;	break;
			case SDLK_7:	ikey=4;	break;
			default:		return false;
		}

    	if(hsw.console[index].game<3 && hsw.console[index].paid)
	    {
        	// Обновление холдов
	        if(hsw.console[index].cards[ikey]>=100)
    	    {
        	    hsw.console[index].cards[ikey]-=100;
            	cmdSoundPlay("unhold");
	        }
    	    else
        	{
            	hsw.console[index].cards[ikey]+=100;
	            cmdSoundPlay("hold");
    	    }
        	return true;
	    }
    	return false;
	}


	// Обработка изменения кредита по кнопкам гамепада, 
	// Возвращает true, если обработка имела место
	bool CreditKeyUpdate(SDLKey key, int i)
	{       
        int crVal=1;
        switch(key)
        {
                case SDLK_e: // +100
                        crVal*=10;
                case SDLK_w: // +10
                        crVal*=10;              
                case SDLK_q:     // +1
                        CreditAppend(i,crVal);
                        break;
                case SDLK_r: // reset credit

                        hsw.mLocal.money_out+=hsw.console[i].credit;
                        hsw.mTotal.money_out+=hsw.console[i].credit;

                        hsw.cMoney[i].money_out+=hsw.console[i].credit;
                        hsw.cMoneyTotal[i].money_out+=hsw.console[i].credit;

                        hsw.console[i].credit=0;

                        break;
                default:
                        return false;
        }
        BlackBoxWriteState();
        return true;
	}
}