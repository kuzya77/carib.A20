#include "head.h"

// Поведение хоста в состоянии "BEGIN_GAME"
// прокручивается эффект экрана (камин+свечи)
// Происходит генерация первого набора карт (игроки (все 5 открытых)+ диллер (1 открытая))
// анимация: тусня -> выдача левому -> выдача среднему -> выдача правому -> себе, озвучка
// запуск фразы 1 на местах, 
// автовыбор карт
// В этой фазе все нажимаемые клавиши пропускаются и игнорируются, фаза завершается сама
// выходом

namespace Head
{
	static int stage=0;

	void hsGameB()
	{
		if(previousState!=hsw.state)
		{
		    previousState=hsw.state;
		    stage = 0;
		}
		switch(stage)
		{
			case 0:	// Start animation
				CurrentAnimDone=false;
				animPlay("trick");
		        UpdateTerminalStates(0);    // Обновление состояния терминалов...
		        stage++;
		        break;
			case 1: // Wait for end of the animation
				if(CurrentAnimDone)
				{
    	    		MakeFirstComby();
				    BlackBoxWriteState();
					stage++;
				}
				break;
			// выдача по столам
			case 2:	// Левый игрок
				if(hsw.console[0].paid)
				{
					animPlay(drop_in[0]);
					CurrentAnimDone = false;
					UpdateTerminalStates(0);
					stage++;
				}
				else
					stage += 8;	// Переход всразу к центру
			    break;
			case 3:
				if(CurrentAnimDone)
				{
					animPlay(drop_part[0]);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 4:
			case 5:
			case 6:
			case 7:
				if(CurrentAnimDone)
				{
					animPlay(drop_part[0]);
					UpdateTerminalStates(stage-3);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 8:
				if(CurrentAnimDone)
				{
					animPlay(drop_out[0]);
					UpdateTerminalStates(5);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 9:
				if(CurrentAnimDone)
				{
					CurrentAnimDone = false;
					animHide(drop_out[0]);
					stage++;
				}
				break;
			case 10:	// Центральный игрок
				if(hsw.console[1].paid)
				{
					animPlay(drop_in[1]);
					CurrentAnimDone = false;
					UpdateTerminalStates(5);
					stage++;
				}
				else
					stage += 8;	// Переход всразу к правому
			    break;
			case 11:
				if(CurrentAnimDone)
				{
					animPlay(drop_part[1]);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 12:
			case 13:
			case 14:
			case 15:
				if(CurrentAnimDone)
				{
					animPlay(drop_part[1]);
					UpdateTerminalStates(stage-6);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 16:
				if(CurrentAnimDone)
				{
					animPlay(drop_out[1]);
					UpdateTerminalStates(10);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 17:
				if(CurrentAnimDone)
				{
					CurrentAnimDone = false;
					animHide(drop_out[1]);
					stage++;
				}
				break;
			case 18:	// Игрок справа
				if(hsw.console[2].paid)
				{
					animPlay(drop_in[2]);
					CurrentAnimDone = false;
					UpdateTerminalStates(10);
					stage++;
				}
				else
					stage += 8;	// Переход всразу к выдаче дилеру
			    break;
			case 19:
				if(CurrentAnimDone)
				{
					animPlay(drop_part[2]);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 20:
			case 21:
			case 22:
			case 23:
				if(CurrentAnimDone)
				{
					animPlay(drop_part[2]);
					UpdateTerminalStates(stage-9);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 24:
				if(CurrentAnimDone)
				{
					animPlay(drop_out[2]);
					UpdateTerminalStates(15);
					CurrentAnimDone = false;
					stage++;
				}
				break;
			case 25:
				if(CurrentAnimDone)
				{
					CurrentAnimDone = false;
					animHide(drop_out[2]);
					stage++;
				}
				break;
			case 26:	    // Выдача себе
				animPlay("drop_self");
				stage++;
				break;
			case 27:
				if(CurrentAnimDone)
				{
					CurrentAnimDone = false;
					animShow("card4", hsw.cards[4]-1);
					stage++;
				}
				break;
			case 28:
			default:	// Just in case
				if(CurrentAnimDone)	// go to a game stage
				{
					CurrentAnimDone = false;
					hsw.state = HS_GAME;
					t_keys = TerminalKeyBuff();	// clear queue and go to the next stage
					logMessage("GAME_BEGIN -> HS_GAME");
				}
				break;
		}
	}
}
