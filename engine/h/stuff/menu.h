#ifndef __USER_MENU__
#define __USER_MENU__

#include <gfx.h>

class Menu;
class Console;

///////////////////////////////////////////////////////////////////////////
///Клавиши меню
///////////////////////////////////////////////////////////////////////////
enum MenuKey 
{
  MENU_KEY_UP,       //клавиша вверх
  MENU_KEY_DOWN,     //клавиша вниз
  MENU_KEY_LEFT,     //клавиша влево
  MENU_KEY_RIGHT,    //клавиша вправо
  MENU_KEY_APPLY,    //клавиша "подтвердить"
  MENU_KEY_SELECT,   //клавиша выбора
  MENU_KEY_ESC,      //клавиша отмены
  MENU_KEY_BACKSPACE,    //клавиша возврата на один символ
};

/*
    Common
*/

void   uiInit     ();
void   uiShutdown ();
bool   uiLoad     (const char* file_name);

/*
    Console
*/

void uiClear (Console*);

void uiPrintf  (Console*, color_t color,const char* message,...);
void uiPrintf  (Console*,const char* message,...);
void uiVPrintf (Console*, color_t color,const char* message,va_list);
void uiVPrintf (Console*,const char* message,va_list);

void uiSeek (Console*,int top_line);
int  uiTell (Console*); //return top line number

/*
    Menu  
*/

Menu* uiLoadMenu (const char*);
void  uiDestroy  (Menu*);

//is frame active
bool uiIsActive (Menu*,const char*);

//redraw menu
void  uiDraw (Menu*); 
void uiClick(Menu*, int x, int y);
//actions
void uiPutKey (Menu*,MenuKey);

//consoles
Console* uiFindConsole (Menu*,const char* name);

//--------------------------------------------------------------------------
// text console support

bool conInit(int width,int height);
void conDone();

int  conWidth();
int  conHeight();

void conClear();

void conPutch(char ch);
void conPuts(const char* s);
void conPrintf(const char* message,...);
void conVPrintf(const char* message,va_list);

void conSetColor(color_t color);
void conDraw(int x=0,int y=0,cRect* rect=NULL);

#endif
