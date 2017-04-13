#include "head.h"
#include <stuff/wxf.h>

namespace Head
{

bool BlackBoxInit()
{   
        return BlackBoxReadIni("blackbox.ini") || BlackBoxReadIni("blackdef.ini");
}

void ReadMoney(TurnMoney* money,ParseNode* node,const char* name)
{
        money->reset();
        if(node)
                node=node->first(name); 
        if(!node)
                return;
        money->money_in =attri(node,"money_in");
        money->money_out=attri(node,"money_out");
        money->turn_in  =attri(node,"turn_in");
        money->turn_out =attri(node,"turn_out");
}

bool BlackBoxReadIni(const char* name)
{
    assert(name);
    ParseNode* root = parLoadWXF(name,512,2048);
    if(!root)
    {
        logError("BlackBoxReadIni error: can't read file %s",name);
        return false;
    }

    memset(&hsw,0,sizeof(hsw));

    hsw.procent=attri(root,"Procent");
    hsw.SndVol=attri(root,"SoundVol");
    hsw.state=attri(root,"State");

    ReadMoney(&hsw.mLocal,root,"Money");
    ReadMoney(&hsw.mTotal,root,"TotalMoney");

    read_array(root,"cards",(int*)hsw.cards,5);

    hsw.math.read(root); 

    for(int i=0;i<TERMINAL_COUNT;i++)
    {
        char s[50];
        sprintf(s,"terminal_%d",i+1);
        ParseNode* tnode=root->first(s);
        if(tnode)
        {
            hsw.console[i].game=attri(tnode,"Game");
            hsw.console[i].credit=attri(tnode,"Credit");

            ReadMoney(&hsw.cMoney[i],tnode,"Money");
            ReadMoney(&hsw.cMoneyTotal[i],tnode,"TotalMoney");

            hsw.console[i].win  =attri(tnode,"Win");
            hsw.console[i].LastWin=attri(tnode,"LastWin");
            hsw.console[i].paid =attri(tnode,"Paid");
            hsw.console[i].ante =attri(tnode,"Ante",2);
            read_array(tnode,"cards",hsw.console[i].cards,5);
        }
        else
            logWarning("BlackBoxReadIni warning: can't read info about %d player",i+1);
    }

    parFree(root);

    return true;
}

void WriteMoney(WXF& wxf,const TurnMoney* money,const char* name)
{
        wxf.begin(name);
        wxf.printf("money_in\t%lu\nmoney_out\t%lu\nturn_in\t%lu\nturn_out\t%lu\n",
                money->money_in,money->money_out,money->turn_in,money->turn_out);       
        wxf.end();
}

bool BlackBoxWriteState()
{
	logDebug("BlackBoxWriteState() begin");
    BINFILE f=bfOpen("blackbox.ini",OPEN_CR);
    if(!f)
    	return false;

    logDebug("BlackBoxWriteState() ???");
    WXF wxf(f);
    wxf.printf("Procent\t%ld\n"
              "SoundVol\t%d\n"
              "State\t%lu\n",
              hsw.procent,hsw.SndVol,hsw.state);
        WriteMoney(wxf,&hsw.mLocal,"Money");
        WriteMoney(wxf,&hsw.mTotal,"TotalMoney");

        hsw.math.write(wxf);

        print(wxf,"cards",hsw.cards,5);

    for(int i=0;i<TERMINAL_COUNT;i++)
    {
        char s[50];
        sprintf(s,"terminal_%d",i+1);
        wxf.begin(s);
        wxf.printf("Game\t%d\n"
                  "Credit\t%ld\n"
                  "Win\t%ld\n"
                  "LastWin\t%ld\n"
                  "Paid\t%ld\n"
                  "Ante\t%ld\n",
                                hsw.console[i].game,
                                hsw.console[i].credit,
                                hsw.console[i].win,
                                hsw.console[i].LastWin,
                                hsw.console[i].paid,
                                hsw.console[i].ante
                );
        WriteMoney(wxf,&hsw.cMoney[i],"Money");
        WriteMoney(wxf,&hsw.cMoneyTotal[i],"TotalMoney");

        print(wxf,"cards",hsw.console[i].cards,5);      
        wxf.end();
    }
    bfClose(f);

    return true;
}

}