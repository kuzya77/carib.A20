#ifndef __CARIB_MATH_H__
#define __CARIB_MATH_H__

#include <stuff.h>
#include <stuff/wxf.h>
#include <stuff/parser.h>

const int TERMINAL_COUNT=3;

const int CARD_COUNT=53;
const int CHOISE=128;

void randomize();
uint32 rand32(uint32 max);
uint32 rand32();

void cInit();

class MathStorage
{
public:
    int             lowbound,                  // start with absolutely
                    interval,                  // random cards (lowbound = 0, interval = choise)
                    round,                     // длина одной серии
                    money_in,                  // Последний хороший вход и выход денег
                    money_out,                 // Необходимы для мягкости игры
                    midsum;                    // Общий вес
    float           prevp,                     // Процент предыдущей стратегии
                    procent;                   // Локальный процент
    int             mRoundRange[2];                 // Интервал длины стратегии
    int             gametype;                  // Тип игры - честная(HONEST_GAME = 0), стратегическая(STRATEGY_GAME = 1) или подъем игрочков(UP_GAME = 2)
    unsigned int 
                    midint[CHOISE];            // Веса для среднего интервала
    unsigned int    lastrand;
    unsigned int    lastpaid[TERMINAL_COUNT];
    int             cards[CARD_COUNT];         // Переменные для хранения колоды
    int             cCount;                    // число карт в колоде : 52 - без джокера
    int             index;                     // индекс для выбора комбинации
    int             dcards[5];
    int             bought;
    int             dcomby;
            
    bool read(ParseNode* node);
    void write(WXF& wxf);
    void cCutCard(int card);

    void reset();
};

// From h_math.cpp
extern const char* cards_str[54];

void MakeFirstComby();
int  MakeNextComby(int player,int32 c5[5]);
int  GetCard();
int  GetComby(const int32 c5[5]);
void MaskLastComby(int32 c5[5]);
void PayCardForDealer(int mci);
void mathInit();

#endif
