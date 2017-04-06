#ifndef __CONTROL_MENU_IH__
#define __CONTROL_MENU_IH__

#include <mem.h>
#include <picture.h>

#include "menuimpl.h"

class Frame;
class Menu;

//////////////////////////////////////////////////////////////////////////////
///Контрол
//////////////////////////////////////////////////////////////////////////////
class Control:public UIObj
{
  friend class Frame;
public:
    Control(Frame* _owner,const char* name);
    virtual ~Control();                                

//////////////////////////////////////////////////////////////////////////////
///Владельцы
//////////////////////////////////////////////////////////////////////////////
    Frame* frame () const { return owner; }
    Menu*  menu  () const;

//////////////////////////////////////////////////////////////////////////////
///Активен ли контрол
//////////////////////////////////////////////////////////////////////////////
    bool IsActive () const;

//////////////////////////////////////////////////////////////////////////////
///Имя / заголовок / подсказка
//////////////////////////////////////////////////////////////////////////////
    void SetTitle (const char* t)               { ctrl_title=t?t:""; }
    void SetTip   (const char* tip)             { ctrl_tip=tip?tip:""; }

    const char* title () const                  { return ctrl_title.c_str(); }
    const char* tip   () const                  { return ctrl_tip.c_str(); }

//////////////////////////////////////////////////////////////////////////////
///Команда
//////////////////////////////////////////////////////////////////////////////
	// command!=NULL
    void    SetCommand (CmdType,const char* command);
    const char* command(CmdType type) const { return type >= 0 && type < CMD_TYPES_NUM ? ctrl_cmd[type].c_str():""; }

//////////////////////////////////////////////////////////////////////////////
///Перерисовка
//////////////////////////////////////////////////////////////////////////////
    virtual void draw (int x,int y, color_t clr) {}
    virtual cRect size(int x, int y) const { return cRect(0, 0, 0, 0); }

//////////////////////////////////////////////////////////////////////////////
///Основные команды
//////////////////////////////////////////////////////////////////////////////
    virtual void enter     (); //call command
    virtual void backspace () {}
    virtual void apply     () {}
    virtual void escape    () {}
    virtual void up        ();
    virtual void down      ();
    virtual void left      () {}
    virtual void right     () {}
    virtual void click(int x, int y) { enter(); }

//code BlackKite
    void enable(bool);
////////////////


  protected:
    void drawtext (int x,int y,const char* text, color_t color, IndentType = INDENT_FULL);
    void updatetip ();

    static void format(char* dst,const char* src,int size);

private:
    Frame* owner;

    Control*  next,*prev;

    std::string  ctrl_title,
            ctrl_tip;

    std::string  ctrl_cmd[CMD_TYPES_NUM];

    //code BlackKite
    bool enableFlag;                            //флаг enable 
    ////////////////
};

#endif
