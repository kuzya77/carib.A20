#include "head.h"
//#include "h_math.h"

bool MathStorage::read(ParseNode* node)
{
    if(node)
        node=node->first("math");       
    if(!node)
    {
        reset();            
        logError("Error: can't read math data (h_box.cpp::ReadMath)");
        round=1;
        mRoundRange[0]=2;
        mRoundRange[1]=5;
        prevp=float(hsw.procent)/100;
        procent=float(hsw.procent)/100;
        for(int i=0;i<CHOISE;i++)
        for(int i=0;i<TERMINAL_COUNT;i++)
                lastpaid[i] = 0;
        midsum=CHOISE;
        index=0;                     // индекс для выбора комбинации
        return false;
    }
    lowbound =attri(node,"lowbound",0);
    interval =attri(node,"interval",CHOISE);
    money_in =attri(node,"money_in",0);
    money_out=attri(node,"money_out",0);
    round    =attri(node,"mathround",1);
    midsum   =attri(node,"midsum", CHOISE);
    gametype =attri(node,"gametype",0);
    index    =attri(node,"index",0);
    bought   =attri(node,"bought",0);
    dcomby   =attri(node,"dcomby",0);
    read_array(node,"RoundRange",mRoundRange,2);
    read_array(node,"dcards",dcards,5);

    if(!read_array(node,"midint",midint,CHOISE))
    {
            for(int i=0;i<CHOISE;i++)
                    midint[i] = 1;
    }

    cCount   =attri(node,"cCount",CARD_COUNT);

    if(!read_array(node,"cards",cards,CARD_COUNT))
        cInit();

    procent  =attrf(node,"procent",float(hsw.procent)/100);
    prevp    =attrf(node,"prevpercent",float(hsw.procent)/100); 

    lastrand =attri(node,"lastrand");

    return true;
}

void MathStorage::write(WXF& wxf)
{
    wxf.begin("math");
    wxf.printf("lowbound      %d\n",lowbound);
    wxf.printf("interval      %d\n",interval);
    wxf.printf("mathround     %d\n",round);
    wxf.printf("money_in      %d\n",money_in);
    wxf.printf("money_out     %d\n",money_out);
    wxf.printf("midsum        %d\n",midsum);
    wxf.printf("mathround     %d\n",round);
    wxf.printf("gametype      %d\n",gametype);
    wxf.printf("prevpercent   %f\n",prevp);
    wxf.printf("procent       %f\n",procent);
    wxf.printf("cCount        %d\n",cCount);
    wxf.printf("index         %d\n",index);
    wxf.printf("dcomby        %d\n",dcomby);
    wxf.printf("bought        %d\n",bought);
    
    print(wxf,"dcards",    dcards,5);
    print(wxf,"RoundRange",mRoundRange,2);
    print(wxf,"cards",     cards,CARD_COUNT);
    print(wxf,"midint",    midint,CHOISE);

    wxf.printf("lastrand      %d\n",lastrand);
    
    wxf.end();
}

void MathStorage::cCutCard(int card)
{
    if(card>100)
        card-=100;
    if(card>0 && card<54)                                                   // не Рубашка или пусто
    {
        for(int i=0;i<cCount;i++)
            if(cards[i]==card)
            {
                for(cCount--;i<cCount;i++)                      // Сместить оставшиеся карты
                    cards[i]=cards[i+1];
                break;
            }                                       
    }       
}

void MathStorage::reset()
{
    lowbound=0;
    interval=CHOISE-1;
    money_in=0; 
    money_out=0;
}
