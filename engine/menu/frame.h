#ifndef __FRAME_MENU_IH__
#define __FRAME_MENU_IH__

#include <mem.h>
#include <picture.h>

#include "menuimpl.h"

//////////////////////////////////////////////////////////////////////////////
///Фрейм
//////////////////////////////////////////////////////////////////////////////
class Frame:public UIObj
{
  friend class Control;
  friend class Menu;
  public:
    Frame(Menu* owner,const char* name=NULL);
    virtual ~Frame();

//////////////////////////////////////////////////////////////////////////////
///Владелец
//////////////////////////////////////////////////////////////////////////////
    Menu* menu () const { return owner; }

//////////////////////////////////////////////////////////////////////////////
///Положение и размер
//////////////////////////////////////////////////////////////////////////////
            void SetPosition (int x,int y);
            void move        (int dx,int dy);
    virtual void resize      (int width,int height); //for autocalc console height
            int  width       () const { return frame_width; }
            int  height      () const { return frame_height; }
            int  getx        () const { return x; }
            int  gety        () const { return y; }

//////////////////////////////////////////////////////////////////////////////
///Выравнивание / отступ / вертикальный шаг
//////////////////////////////////////////////////////////////////////////////
    void    SetIndent  (int indent);
    void    SetJustify (Justify hor,Justify vert);
    void    SetVStep   (int item_height);
    int     indent     () const { return frame_indent; }
    Justify hjust      () const { return h_just; }
    Justify vjust      () const { return v_just; }
    int     vstep      () const { return v_step; }

//////////////////////////////////////////////////////////////////////////////
///Работа с контролами
//////////////////////////////////////////////////////////////////////////////
    Control* curctrl  () const { return cur_ctrl; }
    Control* find     (const char* name) const;
    void     select   (const char* name);
    void     select   (Control*);

    bool     IsActive (const Control* ctrl) const { return IsActive () && ctrl == cur_ctrl; }
    bool     IsActive () const { return owner->IsActive (this); }

    bool	click(int x, int y);

//////////////////////////////////////////////////////////////////////////////
///Задний фон
//////////////////////////////////////////////////////////////////////////////
    void  SetPicture (const char*);
    IMAGE picture    () const { return frame_picture; }

//////////////////////////////////////////////////////////////////////////////
///Видимость фрейма
//////////////////////////////////////////////////////////////////////////////
    void show (bool state=true);
    void hide () { show (false); }
    bool visible () const { return frame_visible; }

//////////////////////////////////////////////////////////////////////////////
///Перерисовка
//////////////////////////////////////////////////////////////////////////////
    void draw  ();
    cRect textsize(int x, int y, const char* text, IndentType = INDENT_FULL) const;

//////////////////////////////////////////////////////////////////////////////
///Цвет
//////////////////////////////////////////////////////////////////////////////
    void SetColor     	(color_t passive_color, color_t active_color, color_t _disable, color_t back_color);
    color_t activecolor  () const { return active_color; }
    color_t passivecolor () const { return passive_color; }    
    color_t backcolor    () const { return back_color; }
    color_t disablecolor () const { return disable_color;}
    ///////////////////////////////////////////////////

  protected:
    void drawtext (int x,int y,const char* text, color_t color,IndentType = INDENT_FULL);
    virtual void _draw ();

  private:
    Menu*     owner;
    Control*  first;
    Control*  cur_ctrl;

    int       count;
    int       x,y,frame_width,frame_height,frame_indent,v_step;
    bool      frame_visible;
    Justify   h_just,v_just;
    IMAGE     frame_picture;
    color_t   active_color,passive_color,disable_color,back_color;
    cRect     clip [3];
};

#endif
