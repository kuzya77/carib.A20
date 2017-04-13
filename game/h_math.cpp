// Модуль, отвечающий за мат модель игры, учет
// бабла и статистики

#include "head.h"

#include <stuff/cmd.h>

#include <stdlib.h>


float midsearch_f(float* p, int size, uint32 k); 
void randomize();
uint32 rand32(uint32 max);
uint32 rand32();

const char* cards_str[54]=
                {
                        "(none)",
                        "2:B","3:B","4:B","5:B","6:B","7:B","8:B","9:B","10:B","J:B","D:B","K:B","A:B",
                        "2:C","3:C","4:C","5:C","6:C","7:C","8:C","9:C","10:C","J:C","D:C","K:C","A:C",
                        "2:K","3:K","4:K","5:K","6:K","7:K","8:K","9:K","10:K","J:K","D:K","K:K","A:K",
                        "2:P","3:P","4:P","5:P","6:P","7:P","8:P","9:P","10:P","J:P","D:P","K:P","A:P",
                        "jock"
                };

const char* cardstr[54] = 
                {
                        " ---- ",
                        "  2\4  ","  3\4  ","  4\4  ","  5\4  ","  6\4  ","  7\4  ","  8\4  ","  9\4  "," 10\4  ","  B\4  ","  D\4  ","  K\4  ","  T\4  ",
                        "  2\3  ","  3\3  ","  4\3  ","  5\3  ","  6\3  ","  7\3  ","  8\3  ","  9\3  "," 10\3  ","  B\3  ","  D\3  ","  K\3  ","  T\3  ",
                        "  2\5  ","  3\5  ","  4\5  ","  5\5  ","  6\5  ","  7\5  ","  8\5  ","  9\5  "," 10\5  ","  B\5  ","  D\5  ","  K\5  ","  T\5  ",
                        "  2\6  ","  3\6  ","  4\6  ","  5\6  ","  6\6  ","  7\6  ","  8\6  ","  9\6  "," 10\6  ","  B\6  ","  D\6  ","  K\6  ","  T\6  ",
                        "joker "
                };

void printcards (const char* text, const int* p)
{
        logDebug("%s\t%s%s%s%s%s", text, cardstr[p[0]], cardstr[p[1]], cardstr[p[2]], cardstr[p[3]], cardstr[p[4]]);
}


//-------------------------------- CARD EXCHANGE --------------------------------------------------------------------------
#define         EXCHANGE_VAR_COUNT              27                      // Кол-во вариантов обмена (трехкарточные обмены считаются неразумными)
#define         ATTEMPT_COUNT                   128                     // Кол-во проб при оценке матожидания выигрыша комбинации
#define         lo_limit                        32                      // [lo_limit..CHOISE]     - the best  for player
#define         hi_limit                        32                      // [0..CHOISE - hi_limit] - the worst for player
#define         percent                         float(hsw.procent)*1e-2 // Процент в виде 0,...

#define         HONEST_GAME                     0                       // Тип игры - честная
#define         STRATEGY_GAME                   1                       // стратегическая
#define         UP_GAME                         2                       // подъем игрочков
#define         VIPCOMBY                        6553600

//-------------------------------------------------------------------------------------------------------------------------

const int price[] = {0,6,8,10,12,14,18,44,104,204};

//--------------------------------------------------------------------
// Вынесено в статик из опасения невмещения в стек.
// массивы для хранения очков комбинации
static float scoretable[CHOISE];                                                // unsorted array of score
static float sc[CHOISE];                                                        // sorted array of score


static void cCutCard(int card)
{
    hsw.math.cCutCard(card);
}

// Извлечение карты из колоды
int GetCard()
{
        return hsw.math.cCount?hsw.math.cards[--hsw.math.cCount]:0;
}
// Возврат карты в колоду
void PutCard(int card)
{
        hsw.math.cards[hsw.math.cCount++] = card;
}

// Перетасовка оставшихся карт
void UnsortCards()
{
        for(int i=0;i<hsw.math.cCount;i++)       // меняем i-ю карту с любой из остальных
                std::swap(hsw.math.cards[i],hsw.math.cards[rand32(hsw.math.cCount)]);
}

//-------------------------------------------------------------------------------------------------
void cInit()
{
        hsw.math.cCount=CARD_COUNT;
        // Перетасовка 1й раз
        for(int i=0;i<CARD_COUNT;i++)
                hsw.math.cards[i]=i+1;
        UnsortCards();
}

// маскирует карты, участвующие в комбинации
// используется для отображения hold'oв
bool MaskComby[5];

// Отмечает (+100) карты, участвовавшие в пред. рассчитаной комбинации
void MaskLastComby(int32 c5[5])
{
        for(int i=0;i<5;i++)
        {
                if(c5[i]>=100)
                        c5[i]-=100;
                if(MaskComby[i])
                        c5[i]+=100;
        }
}

// выбор более лучшего интервала для выдачи раскладов
void do_better(int n)
{
        for(int i=0;i<n;i++)
        {
                if(hsw.math.lowbound)
                {
                        if(hsw.math.interval>hi_limit) 
                        {
                                hsw.math.lowbound++; 
                                hsw.math.interval--;
                        }
                        else 
                        {
                                hsw.math.lowbound=0; 
                                hsw.math.interval=CHOISE;
                        }
                }
                else
                {
                        if(hsw.math.interval<CHOISE)
                                hsw.math.interval++;
                        else 
                        {
                                hsw.math.interval--; 
                                hsw.math.lowbound++;
                        }
                }
        }
}

// выбор более плохого интервала для выдачи раскладов
void do_worse(int n)
{
        for(int i=0;i<n;i++)
        {
                if(hsw.math.lowbound)
                {
                        hsw.math.interval++;
                        hsw.math.lowbound--;
                }
                else
                {
                        if(hsw.math.interval>lo_limit) 
                                hsw.math.interval--;
                        else 
                        {
                                hsw.math.interval=CHOISE; 
                                hsw.math.lowbound=0;
                        }
                }
        }
}

// локальный процент (денежный : деньги считаются с последнего хорошего момента)
// c учетом текущего кредита
float GetProcent()
{
        if(hsw.mTotal.money_in==hsw.math.money_in)
                return hsw.procent;

        int s=hsw.mTotal.money_out-hsw.math.money_out;
        for(int i=0;i<TERMINAL_COUNT;i++)
                s+=hsw.console[i].credit;
        float m_in=hsw.mTotal.money_in-hsw.math.money_in;

        return float(s)/m_in;
}

// реальный процент (денежный)
// c учетом кредита
float GetTotalProcent()
{
        if(!hsw.mTotal.money_in)
                return hsw.procent;
        int s=hsw.mTotal.money_out;
        for(int i=0;i<TERMINAL_COUNT;i++)
                s+=hsw.console[i].credit;
        return float(s)/float(hsw.mTotal.money_in);
}

//
void mathRoundInit()
{
    logDebug("mathRoundInit() begin");
        for(int i=0;i<TERMINAL_COUNT;i++)       // Credit != 0 => get out
                if(hsw.console[i].credit)
                {
                    logDebug("mathRoundInit() end");
                        return;
        }
        float l_percent=GetProcent();
        if((l_percent<=hsw.math.procent && hsw.math.procent<percent) ||               //понизили достаточно
           (l_percent>hsw.math.procent) && (hsw.math.procent>percent))                //повысили достаточно
        {
                // Достигли локального процента 
                // сохраняем это достижение
                hsw.math.money_in  = hsw.mTotal.money_in;
                hsw.math.money_out = hsw.mTotal.money_out;
                // .... и начинаем игру на новый локальный процент
                hsw.math.procent=percent+(GetTotalProcent()<percent)?0.02:-0.05;
                hsw.math.lowbound=0;
                hsw.math.interval=CHOISE;
        }
        logDebug("mathRoundInit() end");
}

unsigned int getcomby(const int* p);

// Генерация карт 1й раздачи
void MakeFirstComby()
{
        logDebug("MakeFirstComby :: entry :: gametype = %d", hsw.math.gametype);
        logDebug("LocalIn = %d. LocalOut = %d. Local percent = %f.",hsw.math.money_in,hsw.math.money_out,GetProcent());
        logDebug("TotalIn = %d. TotalOut = %d. Total percent = %f.",hsw.mTotal.money_in,hsw.mTotal.money_out,GetTotalProcent());

        if(hsw.math.round)      // время пересчитывать серию
                hsw.math.round--;
        else
        {
                hsw.math.round=hsw.math.mRoundRange[0]+rand32(hsw.math.mRoundRange[1]-hsw.math.mRoundRange[0]+1);
                float p=GetProcent();
                // Корректировка стратегии
                bool better=false, worse=false;
                if(hsw.math.prevp>percent)
                {
                if(p>=hsw.math.prevp) 
                        worse=true;
                        else 
                                if(p<=percent) 
                                better=true;
                }
                else
                {
                        if(p<=hsw.math.prevp) 
                        better=true;
                        else 
                                if(p>=percent) 
                                        worse=true;
        }

                if (better) do_better(1 + rand32(8));
                if (worse) do_worse(1 + rand32(8));
                hsw.math.prevp=p;
        }

        int s, sum;
  
        logTitle("gametype & index selecting ... start");

        int c=rand32()&0xff;
        if(c<192)   // probability = 0.75000
        {
                hsw.math.gametype=STRATEGY_GAME; 
                logDebug("STRATEGY GAME");
        }
        else
                if(c<248) // probability = 0.18750
                {
                        hsw.math.gametype=HONEST_GAME;   
                        logDebug("HONEST   GAME");
                }  
                else
                        {       // probability = 0.03125  
                                hsw.math.gametype=UP_GAME;       
                                logDebug("UP_GAME  GAME");
                        }  

        switch (hsw.math.gametype)
        {
                case HONEST_GAME:
                        s=rand32()%hsw.math.midsum;
                        sum=0;
                        hsw.math.index=0;
                        while (sum<s)
                        {
                                sum = sum + hsw.math.midint[hsw.math.index];
                                hsw.math.index++; 
                        };
                break;
                case STRATEGY_GAME:
                        for(s = hsw.math.lowbound; s < hsw.math.lowbound + hsw.math.interval; s++) hsw.math.midint[s]++;
                        hsw.math.midsum = hsw.math.midsum + hsw.math.interval;
                        hsw.math.index  = hsw.math.lowbound + rand32()%hsw.math.interval;
                break;
                case UP_GAME:
                        hsw.math.index = CHOISE - 1 - (rand32()%(CHOISE/4));
                break;

        }
        logDebug("gametype & index selecting ... index = %u. end", hsw.math.index);

        static int pcards[CHOISE][TERMINAL_COUNT][5];                      // 15 cards for all player
        static int dcards[CHOISE][5];                                      // one card for dealer
        int i;

        for(i=0;i<CHOISE;i++)
        {

                int pcomby[TERMINAL_COUNT];
                int pscore[TERMINAL_COUNT];

                start:
                cInit();
                
                bool VIPcomby = false;
                
                for(int j = 0; j < TERMINAL_COUNT; j++)
                        if(hsw.console[j].paid)                                         //терминал в игре
                        {
                                for(int k = 0; k < 5; k++)
                                        pcards[i][j][k] = GetCard();
                                pcomby[j] = getcomby(pcards[i][j]);

                                VIPcomby = VIPcomby || (pcomby[j] > VIPCOMBY);

                        }
                for(int k = 0; k < 5; k++)
                        dcards[i][k] = GetCard();
                
                hsw.math.dcomby = getcomby(dcards[i]);

                if ((VIPcomby) && (!hsw.math.dcomby)) goto start;

                sc[i] = 0.0;
                for(int j = 0; j < TERMINAL_COUNT; j++)
                        if(hsw.console[j].paid)                                         //терминал в игре
                        {
                                if (!hsw.math.dcomby)
                                        pscore[j] = 4;
                                else
                                        if (pcomby[j] > hsw.math.dcomby)
                                                pscore[j] = (pcomby[j]>>16);
                                        else
                                                pscore[j] = 0;                                        

                                float t = float(hsw.cMoneyTotal[j].money_out + hsw.console[j].credit + (hsw.console[j].ante*pscore[j]))
                                                /float(hsw.cMoneyTotal[j].money_in) - percent;
                                t*=rand32()&1?t*2:t;
                                sc[i]+=t;
                        }
                scoretable[i] = sc[i];
        }
        
        float fl = midsearch_f(sc,CHOISE,rand32(CHOISE/3));

        int l;
        for(l=0;scoretable[l]!=fl;l++);             //..... теперь находим индекс соответствующей ему сдачи

        // Перенос карт в комбинацию
        cInit();

        logDebug("cCount = %d", hsw.math.cCount);

        for(int j=0;j<TERMINAL_COUNT;j++)
                if(hsw.console[j].paid)                                                 //терминал в игре
                {
                        
                        for (int k = 0; k < 5; k++)
                        {
                                hsw.console[j].cards[k] = pcards[l][j][k];
                                cCutCard(pcards[l][j][k]);
                        }
                        logDebug("MakeFirstComby:: j=%d",j);
                        printcards("MakeFirstComby :: give cards: ", hsw.console[j].cards);
                        if(GetComby(hsw.console[j].cards)>=0x100)
                                MaskLastComby(hsw.console[j].cards);                        

                        logDebug("cCount = %d", hsw.math.cCount);
                }
                else
                        memset(hsw.console[j].cards,0,sizeof(hsw.console[j].cards));

        // Диллер:

        logDebug("cCount = %d", hsw.math.cCount);

        for (int j = 0; j < 5; j++)
        {
                hsw.math.dcards[j] = dcards[l][j];
                cCutCard(dcards[l][j]);
        }

        for(i = 0; i < 4; i++)
                hsw.cards[i] = 54;
        hsw.cards[4] = hsw.math.dcards[4];
        hsw.math.dcomby = getcomby(hsw.math.dcards);        
        logDebug("MakeFirstComby() end");
}


// РеГенерация набора карт для расклада, меняются карты < 100
int MakeNextComby(int player,int32 c5[5])
{    
        logDebug("MakeNextComby: begin");

        bool holded[5];
        int i;
        for(i = 0; i < 5; i++)
        {
                holded[i] = c5[i] > 100;
                if(holded[i])
                        c5[i] -= 100;
        }

        static int pcards[CHOISE][5];

        for(int i = 0; i < CHOISE; i++)
        {
                start:
                
                UnsortCards();
                for(int j = 0; j < 5; j++)
                        pcards[i][j]=holded[j]?c5[j]:GetCard();

                int pcomby = getcomby(pcards[i]);
                int pscore;

                for(int j = 0; j < 5; j++)
                        if(!holded[j])
                                PutCard (pcards[i][j]);

                if ((!hsw.math.dcomby) && (pcomby > VIPCOMBY)) goto start;
                
                if (!hsw.math.dcomby)
                        pscore = 4;
                else
                        if (pcomby > hsw.math.dcomby)
                                pscore = (pcomby>>16);
                        else
                                pscore = 0;                                        

                float t = float((hsw.cMoneyTotal[player].money_out + hsw.console[player].credit + (hsw.console[player].ante*pscore))
                          /float(hsw.cMoneyTotal[player].money_in)) - percent;

                scoretable[i] = sc[i] = fabs(t);

        }
        
        float fl = midsearch_f (sc, CHOISE, rand32(CHOISE/2));                                       //ищем l-й по величине элемент массива

        int l;
        for(l = 0; scoretable[l] != fl; l++);

        for(int i = 0; i<5; i++)
                if(!holded[i])
                {
                        c5[i]=pcards[l][i];
                        cCutCard(c5[i]);
                }
        int comby = GetComby(c5);

        if(comby>=0x100)
                MaskLastComby(c5);
        logDebug("MakeNextComby :: end : l = %d", l);
        return comby;
}

// Создание раздачи диллера
void MakeFinalDealerCards()
{
        for(int i = 0; i < 4; i++)
                hsw.cards[i] = hsw.math.dcards[i];
}

void PayCardForDealer(int mci)
{
    logDebug("PayCardForDealer() begin");
        int bcards[CHOISE];
        int dcards[5];
        memcpy(dcards, hsw.cards, sizeof(hsw.cards));
        
        int pcomby[TERMINAL_COUNT];
        for(int j = 0; j < TERMINAL_COUNT; j++)
                if(hsw.console[j].paid)                                         //терминал в игре
                        pcomby[j] = getcomby(hsw.console[j].cards);

        for(int i = 0; i < CHOISE; i++)
        {                
                dcards[mci] = bcards[i] = GetCard();
                int dcomby = getcomby(dcards);

                sc[i] = 0.0;
                for(int j = 0; j < TERMINAL_COUNT; j++)
                        if(hsw.console[j].paid)                                         //терминал в игре
                        {
                                int pscore;
                                if (!dcomby)
                                        pscore = 4;
                                else
                                        if (pcomby[j] > dcomby)
                                                pscore = (pcomby[j]>>16);
                                        else
                                                pscore = 0;                                        

                                float t = (float(hsw.cMoneyTotal[j].money_out + hsw.console[j].credit + (hsw.console[j].ante * pscore))
                                          /float(hsw.cMoneyTotal[j].money_in)) - percent;
                                sc[i] = sc[i] + float(1 + rand32()%2) * t * t;
                        }
                scoretable[i] = sc[i];
                PutCard(bcards[i]);
        }

        float fl = midsearch_f (sc, CHOISE, rand32(CHOISE/4));//hsw.math.index);//);  //ищем l-й по величине элемент массива

        int l;
        for(l=0;scoretable[l]!=fl;l++);                                         //..... теперь находим индекс соответствующей ему сдачи

        hsw.cards[mci] = bcards[l];
    logDebug("PayCardForDealer() end");
}

void mathInit()
{
        randomize();
}
