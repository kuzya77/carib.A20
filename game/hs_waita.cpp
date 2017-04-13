#include "head.h"

namespace Head
{
	void hsWaitA()
	{
		if(previousState!=hsw.state)
		{
		    animShow("dmp");
		    previousState=hsw.state;
		}

        for(;!t_keys.empty();)// прием входящих сообщений
        {
            auto key=t_keys.front();
            t_keys.pop();
            int index=key.from-1;
            switch(key.key)
            {
            	case SDLK_3:   // Change ante
                	if(hsw.console[index].paid==0)
                    {
                    	int j;
                        for(j=0;AnteValues[j];j++)
                        	if(AnteValues[j]==hsw.console[index].ante)  // нашли текущую позицию
                            	break;
                        if(!AnteValues[j])  // Значение не найдено? ставим минимальное
                        	j=0;
                        // Проверяем на корректность позицию j
                        for(int k=j+1;k!=j;k++)
                        {
                        	if(!AnteValues[k])
                            	k=0;
                            hsw.console[index].ante=AnteValues[k];
                            BlackBoxWriteState();
                            break;
                        }
                    }
                    break;
				case SDLK_8:   // Start
                	if(hsw.console[index].credit>=hsw.console[index].ante && hsw.console[index].paid==0)
                    {   // Ставка сделана
                    	hsw.console[index].paid=0;
                        hsw.console[index].game=0;
                        Pay(index,hsw.console[index].ante);
                        BlackBoxWriteState();
                        t_keys = TerminalKeyBuff();	// clear queue and go to the next stage
                    }
            }
        }
        UpdateTerminalStates(); // Обновление состояния терминалов

        // Проверяем состояние терминалов:
        // если есть хоть один терминал с запросом игры
        // начать анимацию тусни карт
        int availTerminals=0;
        int ingameTerminals=0;

        // В данной версии будет только 1 терминал
        for(int i=0;i<TERMINAL_COUNT;i++)
            if(TerminalOnLine(i) && (hsw.console[i].credit || hsw.console[i].paid))
            {
                availTerminals++;
                if(hsw.console[i].paid)     // Ставка сделана?
                    ingameTerminals++;
            }

        // Если все доступные терминалы приняли устойчивое состояние
        if(availTerminals==ingameTerminals && availTerminals)
        {                                   // начать игру
            hsw.state=HS_BEGIN_GAME;
            BlackBoxWriteState();
            animHide("dmp");
        }
	}
}