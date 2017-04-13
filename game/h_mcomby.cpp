// Оценка комбинаций карт. вынесено из модуля математики

#include "head.h"
#include <stdlib.h>


//-------------- COEFFICIENTS
#define         jack_pot                        65535           // no such comby => care
#define         royal_flush                     204            
#define         straight_flush                  104            
#define         care                            44             
#define         full_house                      18             
#define         flush                           14
#define         straight                        12
#define         three_kind                      10             
#define         two_pairs                       8              
#define         pair                            6              
#define         AK                              1
#define         nocomby                         0              


const int SuitPrice[4]={2,3,1,0};
const int OverSuitPrice=4;

bool    IsJocker(int c) { return c==53; }
int     Suit(int c)     { return SuitPrice[(c-1)/13]; }
int     Price(int c)    { return ((c-1)%13)+2; }


unsigned int getcomby(const int* p);

// Проверка на ...
// RoyalFlush: (0x900)
// Все карты одной масти (кроме жокера)
// Все карты >9
// Возвращает 0 если карта не опознана,
// иначе стоимость комбинации+стоимостьмасти
int CheckRF(const int c5[5])
{
        int st=-1,i;
        for(i=0;i<5;i++)
                if(!IsJocker(c5[i]))    // Жокер не проверяется
                {       // Все карты одной масти
                        if(st==-1)
                                st=Suit(c5[i]);
                        else
                                if(st!=Suit(c5[i]))
                                        return 0;
                        // Все карты >9
                        if(Price(c5[i])<10)
                                return 0;
                }
        for(i=0;i<5;i++)
                MaskComby[i]=true;
        return 0x900+st;
}

// Straight Flush(0x800)
// Все карты одной масти (кроме жокера)
// Все карты лежат в диапазоне 5(высшая-низшая)<5 (кроме жокера)
// Возвращает 0 если карта не опознана,
// иначе стоимость комбинации+стоимостьмасти+стоимость старшей карты 
// (!! фокус с пересчетом: если есть жокер, учесть его возможное старшинство)
int CheckSF(const int c5[5])
{
        int st=-1;
        int smax=0,smin=15;
        for(int i=0;i<5;i++)
                if(!IsJocker(c5[i]))    // Жокер не проверяется
                {       // Все карты одной масти
                        if(st==-1)
                                st=Suit(c5[i]);
                        else
                                if(st!=Suit(c5[i]))
                                        return 0;                       
                        int p=Price(c5[i]);
                        if(p>smax)
                                smax=p;
                        if(p<smin)
                                smin=p;
                }       
/*
// Все карты лежат в диапазоне 5(высшая-низшая)<5
        if(smax-smin>=5)
                return 0;
        if(smax-smin<4) // Занимается не весь диапазон, жокер считается старшим
                smax++;
*/
	int code=getcomby(c5);
	if((code>>16)!=straight_flush)
		return 0;
        for(int i=0;i<5;i++)
                MaskComby[i]=true;
//        return 0x800+st+OverSuitPrice*smax;
	return 0x800+OverSuitPrice*(code&0xFFFF)+st;
}

// Care                 (0x700)
// Должно накопится 4 одинаковых, или 3+Jocker
int CheckCare(const int c5[5])
{
        // Считаем карты :)
        int pCount[13];
        int jIndex=-1,i;
        CLEAR(pCount);
        for(i=0;i<5;i++)
                if(IsJocker(c5[i]))
                        jIndex=i;
        for(i=0;i<5;i++)
                if(!IsJocker(c5[i]))
                {
                        int j=Price(c5[i])-2;
                        pCount[j]++;
                        if(pCount[j]==4 || (pCount[j]==3 && jIndex!=-1))
                        {   // все ок, комбинация есть
                                // Отмечаем маску
                                j+=2;
                                for(i=0;i<5;i++)
                                        MaskComby[i]=Price(c5[i])==j || IsJocker(c5[i]);
                                return 0x700+OverSuitPrice*j;
                        }
                }
        return 0;
}

// FullHouse    (0x600)
// 3+2, стоимость оценивается по 3ке
// Способ поиска: 3+2 или 2+2+j (Не катит 3+j => Care)
int CheckFH(const int c5[5])
{
        // Считаем карты :)
        int pCount[13];
        int jIndex=-1,i;
        memset(pCount,0,sizeof(pCount));
        int triple=-1;
        for(i=0;i<5;i++)
                if(IsJocker(c5[i]))
                        jIndex=i;
                else
                {
                        int j=Price(c5[i])-2;
                        pCount[j]++;
                        if(pCount[j]==3)
                                triple=j;                                                                               
                }
        if(jIndex!=-1)
        {
                // Ищем 2 двойки:
                int d[2],dcount;
                for(dcount=i=0;i<13 && dcount<2;i++)
                        if(pCount[i]==2)
                                d[dcount++]=i;
                if(dcount<2)
                        return 0;       // Таких нет.
                // Маскируем все                                        
                for(i=0;i<5;i++)
                        MaskComby[i]=true;
                return (std::max(d[0],d[1])+2)*OverSuitPrice+0x600;
        }
        else
        {
                if(triple!=-1)  // Ищем двойку
                        for(i=0;i<13;i++)
                                if(pCount[i]==2)
                                {
                                        for(i=0;i<5;i++)
                                                MaskComby[i]=true;                                      
                                        return (triple+2)*OverSuitPrice+0x600;
                                }
                return 0;
        }
}

// Flash                (0x500)
// 5 карт одной масти (кроме джокера), стоимость - по высшей
int CheckF(const int c5[5])
{
        int st=-1;
        int smax=0,i;
        for(i=0;i<5;i++)
                if(IsJocker(c5[i]))     // Жокер не проверяется
                        smax=15;
                else
                {       // Все карты одной масти
                        if(st==-1)
                                st=Suit(c5[i]);
                        else
                                if(st!=Suit(c5[i]))
                                        return 0;                       
                        int p=Price(c5[i]);
                        if (p>smax)
                                smax=p;
                }
        // Все проверены,
        for(i=0;i<5;i++)
                MaskComby[i]=true;
        return 0x500+st+OverSuitPrice*smax;
}

// Straight       (0x400)       
// Все карты лежат в диапазоне 5(высшая-низшая)<5 (кроме жокера)
// Возвращает 0 если карта не опознана,
// иначе стоимость комбинации+стоимостьмасти+стоимость старшей карты 
// (!! фокус с пересчетом: если есть жокер, учесть его возможное старшинство)
int CheckS(const int c5[5])
{
        // Проверка
        bool cExist[13];
        CLEAR(cExist);
        int smax=0,smin=15,i;
        for(i=0;i<5;i++)
                if(!IsJocker(c5[i]))    // Жокер не проверяется
                {       
                        int p=Price(c5[i]);
                        if(cExist[p-2])
                                return 0;               // 2 одинаковых карты
                        cExist[p-2]=true;
                        if(p>smax)
                                smax=p;
                        if(p<smin)
                                smin=p;
                }       
// Все карты лежат в диапазоне 5(высшая-низшая)<5
/*
        if(smax-smin>=5)
                return 0;
        if(smax-smin<4) // Занимается не весь диапазон, жокер считается старшим
                smax++;
*/

	int code=getcomby(c5);
	if((code>>16)!=straight)
		return 0;

        for(i=0;i<5;i++)
                MaskComby[i]=true;
//        return 0x400+OverSuitPrice*smax;
	return 0x400+(code&0xFFFF);
}

// 3of a Kind (0x300)
// Ищем 3 или 2+j
int Check3K(const int c5[5])
{
        // Считаем карты :)
        int pCount[13];
        int jIndex=-1;
        memset(pCount,0,sizeof(pCount));
        int triple=-1;
        int deus=-1,i;
        for(i=0;i<5;i++)
                if(IsJocker(c5[i]))
                        jIndex=i;
                else
                {
                        int j=Price(c5[i])-2;
                        switch(++pCount[j])
                        {
                                case 3:
                                        triple=j;
                                        break;
                                case 2:
                                        deus=j;
                                        break;
                        }
                }
        if(jIndex!=-1)
        {
                // Проверяем 2:
                if(deus!=-1)
                {       // Маскируем 2+j
                        deus+=2;
                        for(i=0;i<5;i++)
                                MaskComby[i]=Price(c5[i])==deus || i==jIndex;
                        return deus*OverSuitPrice+0x300;
                }
        }
        else
                if(triple!=-1)  // Проверяем 3:
                {
                        triple+=2;
                        for(i=0;i<5;i++)
                                MaskComby[i]=Price(c5[i])==triple;
                        return triple*OverSuitPrice+0x300;
                }
        return 0;
}

// two pair (0x200)
// Именно 2+2 (2+j+? превращаются в 3)
int Check2x2(const int c5[5])
{
        // Считаем карты :)
        int pCount[13];
        int jIndex=-1;
        memset(pCount,0,sizeof(pCount));
        int triple=-1;
        int deus=-1;
        for(int i=0;i<5;i++)
                if(IsJocker(c5[i]))
                        jIndex=i;
                else
                        pCount[Price(c5[i])-2]++;
        if(jIndex==-1)
        {
                // Ищем 2 двойки:
                int d[2],i,dcount;
                for(dcount=i=0;i<13 && dcount<2;i++)
                        if(pCount[i]==2)
                                d[dcount++]=i;
                if(dcount==2)
                {
                        int md=std::max(d[0],d[1]);
                        int suit=0;
                        // Маскируем 2ки
                        for(i=0;i<5;i++)
                        {
                                int p=Price(c5[i])-2;
                                if(p==md)       // Если это макс. пара...
                                        suit=std::max(Suit(c5[i]),suit);
                                MaskComby[i]=(p==d[0]) || (p==d[1]);
                        }
                        return (md+2)*OverSuitPrice+0x200+suit;
                }
        }
        return 0;
}

// pair     (0x100)
int Check2(const int c5[5])
{
        // Считаем карты :)
        int pCount[13],i;
        CLEAR(pCount);
        for(i=0;i<5;i++)
                if(IsJocker(c5[i]))
                {
                        // Ищем любую карту, чем больше достоинство, тем лучше
                        int smax=0;
                        int iMax=0,j;
                        for(j=0;j<5;j++)
                                if(i!=j)
                                {
                                        int p=Price(c5[j]);
                                        if(p>smax)
                                        {
                                                smax=p;
                                                iMax=j;
                                        }
                                }
                        for(j=0;j<5;j++)
                                MaskComby[j]=j==iMax || j==i;
                        return smax*OverSuitPrice+0x100+3; // +3 - макс. величина масти
                }
                else
                {
                        int j=Price(c5[i]);
                        if(++pCount[j-2]==2)
                        {
                                int suit=0;
                                for(i=0;i<5;i++)
                                        if((MaskComby[i]=Price(c5[i])==j))
                                                suit=std::max(suit,Suit(c5[i]));
                                return j*OverSuitPrice+0x100+suit;
                        }
                }
        return 0;
}

// Ace/King (0)
int CheckAK(const int c5[5])
{
        int AceIndex=-1,
                KingIndex=-1,i;
        for(i=0;i<5;i++)
                if(!IsJocker(c5[i]))
                        switch(Price(c5[i]))
                        {
                                case 13:        // King
                                        KingIndex=i;
                                        break;
                                case 14:        // Ace
                                        AceIndex=i;
                                        break;
                        }
        if(AceIndex!=-1 && KingIndex!=-1)
        {
                for(i=0;i<5;i++)
                        MaskComby[i]=i==AceIndex || i==KingIndex;
                return OverSuitPrice*(14);
        }
        return 0;
}

typedef struct
{
        int (*fn)(const int c5[5]);
        const char* rem;
} CheckFnRec;

CheckFnRec checks[]=
        {
                {CheckRF,    "RoyalFlash"},
                {CheckSF,    "StraightFlush"},
                {CheckCare,  "Care"},
                {CheckFH,    "FullHouse"},
                {CheckF,     "Flush"},
                {CheckS,     "Straight"},
                {Check3K,    "3 of a Kind"},
                {Check2x2,   "two pair"},
                {Check2,     "pair"},
                {CheckAK,    "Ace/King"},
                {NULL,       NULL}
        };

int GetComby(const int32 c5[5])
{
        logDebug("GetComby(p=%p) begin",c5);
        int c[5],i;

        // debug - check player

        int player=-1; // undef 
        for(i=0;i<TERMINAL_COUNT;i++)
                if(hsw.console[i].cards==c5)
                {
                        player=i+1;
                        break;
                }
        if(hsw.cards==c5)
                player=0;

        logDebug("GetComby(p=%p,player=%d) xxx",c5,player);

        int comby=0;
        for(i=0;i<5;i++)                
        {
                if(!c5[i])
                {
                        logDebug("GetComby(p=%p,player=%d) warning: Uncompleted data",c5,player);
                        return 0;
                }
                c[i]=c5[i]<100?c5[i]:c5[i]-100;
                MaskComby[i]=false;
        }

        logDebug("GetComby(p=%p,player=%d) run test...",c5,player);

        for(i=0;checks[i].fn;i++)
        {
                logDebug("\t#%d %s",i,checks[i].rem);
                comby=checks[i].fn(c);
                if(comby)
                        break;
        }

        // debug...
        extern const char* cards_str[54];
        char s[256],*sp=s;
        for(i=0;i<5;i++)
        {                                         
                sprintf(sp,MaskComby[i]?"[%s]":" %s ",cards_str[c[i]]);
                sp+=strlen(sp);
        }
        logDebug("GetComby(p=%p,player=%d) end: %s",c5,player,s);
        return comby;
}

//-------------------------------- COMBINATIONS -------------------------------------------------------------------------------------
//-------------- TYPE
#define         jack_pot_t                      11              // 0
#define         royal_flush_t                   10              // 0
#define         straight_flush_t                9               // <= 13
#define         care_t                          8               // <= 13
#define         full_house_t                    7               // <= 13
#define         flush_t                         6               // <= 13
#define         straight_t                      5               // <= 13
#define         three_kind_t                    4
#define         two_pairs_t                     3
#define         pair_t                          2
#define         AK_t                            1
#define         nocomby_t                       0       

unsigned int getcomby(const int* p)
{
        int
                kind[14]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                mast[4]   = {0,0,0,0},
                comb[5]   = {0,0,0,0,0},
                flag      = 0,
                straight1 = 0;
        int i, j1, j2, k, maxsame;
        bool joker = false;
        for (i = 0; i < 5; i++)
        {
                if (p[i] == 53) 
                        joker = true;
                else
                {
                        if (p[i]<14)
                        {
                                kind [p[i]]++;
                                mast [0]++;
                        }
                        else
                                if(p[i]<27)
                                {
                                        kind[p[i]-13]++;
                                        mast[1]++;
                                }
                                else
                                        if(p[i]<40)
                                        {
                                                kind[p[i]-26]++;
                                                mast[2]++;
                                        }
                                        else
                                        {
                                                kind[p[i]-39]++;
                                                mast[3]++;
                                        }
                };
        };

        maxsame = 0;
        for (i = 1; i < 14; i++)
        {
                comb [kind[i]]++;
                if (kind[i] >= maxsame)
                {
                        maxsame = kind[i];
                        j1 = i;
                }
        };
        j2 = 0;
        for (i = 1; i < j1; i++) 
                if (kind[i] >= kind[j2]) 
                        j2 = i;
        if(maxsame!= 1) 
        {
                if (joker)
                {
                        switch (maxsame)
                        {
                                case 2 : 
                                        if (comb[2] == 1) 
                                                return (three_kind<<16) + j1;
                                        else
                                                return (full_house<<16) + (j1<<4) + j2;
                                case 3 : return (care<<16) + j1;
                                case 4 : return (jack_pot<<16);
                        }
                }
                else
                {
                        switch (maxsame)
                        {
                                case 2 : 
                                        if (comb[2] == 1) 
                                                return (pair<<16) + j1;
                                        else 
                                                return (two_pairs<<16) + (j1<<4) + j2;
                                case 3 : 
                                        if (comb[2] == 1)
                                                return (full_house<<16) + (j1<<4) + j2;
                                        else
                                                return (three_kind<<16) + j1;
                                case 4 : return (care<<16) + j1;
                        }
                }
        }
        kind[0] = kind[13]; 
        straight1 = 0;
        for (i = 0; i < 10; i++)
        {
                k = (kind[i] + kind[i + 1] + kind[i + 2] + kind[i + 3] + kind[i + 4]);
                if (k >= straight1)
                {
                        straight1 = k;
                        j2 = i;
                }
        };
        if (joker)
        {
                if ((mast[0] == 4)||(mast[1]==4)||(mast[2]==4)||(mast[3]==4))
                {
                        if (straight1 == 4)
                        {
                                if (j2 == 9)
                                        return royal_flush<<16;
                                else
                                        return (straight_flush<<16) + j2;
                        }
                        else
                                return (flush<<16) + j1;
                }
                else
                {
                        if (straight1 == 4)
                                return (straight<<16) + j1;
                        else
                                return (pair<<16) + j1;
                }
        }
        else
        {
                if ((mast[0] == 5)||(mast[1] == 5)||(mast[2] == 5)||(mast[3] == 5))
                {
                        if (straight1 == 5)
                        {
                                if (j2 == 9)
                                        return royal_flush<<16;
                                else
                                        return (straight_flush<<16) + j2;
                        }
                        else
                                return (flush<<16) + j1;
                }
                else
                {
                        if (straight1 == 5)
                                return (straight<<16) + j1;
                        else
                        {
                                if ((kind[12] == 1) && (kind[13] == 1))
                                        return 1;
                                else
                                        return 0;
                        }
                }
        }
}
