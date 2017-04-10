#ifndef __TCONSOLE_H__
#define __TCONSOLE_H__

#include <mem.h>
#include <picture.h>

//////////////////////////////////////////////////////////////////////////
// Текстовая консоль
class tConsole
{
    int cWidth,cHeight;
    color_t* colors;     // Массив цветов строк
    char* sbuff;       // собственно строки

    char* sCurLine;

    int cLine;
    int cPos;

    void release_int_data();
    void endLine();

    void put(char ch);
public:
    tConsole():cWidth(0),cHeight(0),colors(NULL),sbuff(NULL),sCurLine(NULL) {}
    ~tConsole()                 { release_int_data(); }

    bool SetSize(int w,int h);      // размеры консоли
    int  width()    const       {    return cWidth; }
    int  height()   const       {    return cHeight; }
  
    void putch(char ch)         { if(sCurLine) { put(ch); sCurLine[cPos]='\0'; } }
    void puts(const char* s);

    void SetColor(color_t c)    { if(colors) colors[cLine]=c; }
    void draw(int x,int y,cRect* rect);
};

#endif
