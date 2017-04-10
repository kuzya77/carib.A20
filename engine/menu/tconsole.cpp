#include <mem.h>
#include <log.h>
#include <gfx.h>

#include <string.h>

#include <stuff/menu.h>

#include "tconsole.h"

static tConsole* console=NULL;

void tConsole::release_int_data()
{
    delete[] colors;
    delete[] sbuff;
    colors=NULL;
    sbuff=NULL;
    cWidth=cHeight=0;
}

void tConsole::endLine()
{
    sCurLine[cPos]='\0'; // Завершение строки
    color_t c=colors[cLine];
    if(++cLine>=cHeight)
        cLine=0;
    colors[cLine]=c;
    sCurLine=sbuff+cLine*(cWidth+1);
    sCurLine[cPos=0]='\0';
}

bool tConsole::SetSize(int w,int h)
{
    if(w>0 && h>0)
    {
        try
        {
            release_int_data();
            colors=new color_t[h];
            size_t sz=(w+1)*h;
            sbuff=new char[sz];

            memset(colors,0,sizeof(color_t)*h);
            memset(sbuff,0,sz);
            cWidth=w;
            cHeight=h;
            cLine=0;
            cPos=0;
            sCurLine=sbuff;
            return true;
        }
        catch(std::bad_alloc& ba)
        {
            logError("tConsole::SetSize(%d,%d) error: %s",w,h,ba.what());
        }
        release_int_data();
    }
    return false;
}

void tConsole::put(char ch)
{
    switch(ch)
    {
        default:
            sCurLine[cPos]=ch;
            if(++cPos<cWidth)
                break;
        case '\n':
            endLine();
            break;
        case '\r':
            cPos=0;
            break;
        case '\b':
            if(cPos>0)
                cPos--;
            break;
        case '\t':  // выравнивание по 8 (имитация символа табубяции)
            {           
                int len=8-(cPos&0x7);
                if(cPos+len<cWidth)
                {
                    memset(sCurLine+cPos,' ',len);
                    cPos+=len;
                }
                else
                    endLine();
            }
            break;
    }
}

void tConsole::puts(const char* s)
{
    if(sCurLine)
    {
        for(int i=0;s[i];i++)
            put(s[i]);
        sCurLine[cPos]='\0';
    }
}

void tConsole::draw(int x,int y,cRect* rect)
{
    if(!sCurLine)
        return;

    cRect rprev=gfxViewPort();
    if(rect)
        gfxViewPort(*rect);
    else
        rect=&rprev;

    for(int i=cLine;y<rect->bottom;y+=GFX_FONT_HEIGHT)
    {
        if(++i>=cHeight)
            i=0;
        gfxOutText(x,y,sbuff+i*(cWidth+1),colors[i]);
                
        if(i==cLine)
            break;
    }

    if(rect!=&rprev)
        gfxViewPort(rprev);
}

void conDone()
{
    logMessage("conDone()");
    delete console; 
    console=NULL;
}

bool conInit(int w,int h)
{
    logMessage("conInit(w=%d,h=%d)",w,h);
    if(console)
        conDone();
    
    console = new tConsole();
    
    if(!console)
    {   
        logError("conInit(w=%d,h=%d): we haven't memory",w,h);
        return false;
    }

    if(console->SetSize(w,h))        
        return true;
    conDone();
    return false;
}

int  conWidth()
{
    return console?console->width():0;
}

int  conHeight()
{
    return console?console->height():0;
}

void conPutch(char ch)
{
    if(console)
        console->putch(ch);
}

void conClear()
{
    for(int i=conHeight();--i>=0;conPutch('\n'));
}

void conPuts(const char* s)
{
    if(console)
        console->puts(s);
}

void conVPrintf(const char* msg,va_list list)
{
    if(console)
    {
        char s[1024];
        vsnprintf(s,sizeof(s),msg,list);
        console->puts(s);
    }    
}

void conPrintf(const char* msg,...)
{
    va_list list;
    va_start(list,msg);
    conVPrintf(msg,list);
    va_end(list);
}

void conSetColor(color_t color)
{
    if(console) console->SetColor(color);
}

void conDraw(int x,int y,cRect* rect)
{
    if(console) console->draw(x,y,rect);
}
