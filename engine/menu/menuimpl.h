#ifndef __USER_MENU_IMPL__
#define __USER_MENU_IMPL__

#include <types.h>
#include <mem.h>

#include <stuff.h>
#include <stuff/menu.h>
#include <stuff/cmd.h>

class Control;
class Frame;
class Menu;
class EditBox;

///////////////////////////////////////////////////////////////////////////
///Константы
///////////////////////////////////////////////////////////////////////////
const int  EDITBOX_BUF_SIZE  = 128; //максимальное количество символов для поля ввода
const int  SELECT_LINE_STEP  = 10;  //растояние между символами  в алфавите(для поля ввода)
const int  BUF_SIZE          = 100;  //макс. длинна статического текста
const char MARKER_CHAR       = '%'; //символ подстановки (для стат. текста)

const int  MAX_FRAMES_COUNT  = 32; //максимальное число фреймов            /////!!!!!!!!!!!!!!!!!!!!!!!!!!

const int  CONSOLE_BUF_SIZE  = 64*1024; //размер консольного буфера       //расчитывать колво символов в строке взависимости
                                                                        //от разрешения экрана

const int  CONSOLE_LINES_NUM = 1024;    //максимальное количество линий консоли

///////////////////////////////////////////////////////////////////////////
///Команды
///////////////////////////////////////////////////////////////////////////
enum CmdType 
{
  CMD_APPLY,     //подтверждение
  CMD_FOCUS_IN,  //получение фокуса
  CMD_FOCUS_OUT, //потеря фокуса
  CMD_TYPES_NUM
};

///////////////////////////////////////////////////////////////////////////
///Отступ 
///////////////////////////////////////////////////////////////////////////
enum IndentType 
{
  INDENT_FULL,  //без оступа (на весь фрейм)
  INDENT_LEFT,  //левый отступ (до границы разделения)
  INDENT_RIGHT  //правый отступ (от границы разделения)
};

///////////////////////////////////////////////////////////////////////////
///Метод выравнивания
///////////////////////////////////////////////////////////////////////////
enum Justify
{
  JUST_LEFT,    //выравнивание по левому краю 
  JUST_RIGHT,   //выравнивание по правому краю
  JUST_CENTER,  //выравнивание по центру
  JUST_TOP,     //выравнивание по верхнему краю
  JUST_BOTTOM   //выравнивание по нижнему краю
};

class UIObj    // именованый интерфейсный объект
{
protected:
    std::string obj_name;
    u32_t name_hash;
public:
    const char* name() const        { return obj_name.c_str(); }
    void rename(const char* name);
};

//////////////////////////////////////////////////////////////////////////////
///Меню
//////////////////////////////////////////////////////////////////////////////
class Menu:public UIObj
{
  friend class Frame;
public:
    Menu();
    ~Menu();   

//////////////////////////////////////////////////////////////////////////////
///Имя
//////////////////////////////////////////////////////////////////////////////
    static Menu* findmenu(const char* name);

//////////////////////////////////////////////////////////////////////////////
///Работа с фреймами
//////////////////////////////////////////////////////////////////////////////
    void     select      (const char* frame_name);
    void     select      (Frame* frame);
    Frame*   findframe   (const char* name) const;

    int      framescount () const { return frames_count; }
    Frame*   curframe    () const; 
    Control* curctrl     () const; 
    bool     IsActive    (const Frame*) const;

    void     nextframe   ();
    void     prevframe   ();

//////////////////////////////////////////////////////////////////////////////
///Указание фрейма подсказки
//////////////////////////////////////////////////////////////////////////////
    void   SetTipFrame (const char*);
    Frame* GetTipFrame () const { return tip_frame; }

//////////////////////////////////////////////////////////////////////////////
///Указание картинки из которой берётся палитра
//////////////////////////////////////////////////////////////////////////////
    void            SetPalette (const char*);
    const RGBQUAD*  palette    () const;

//////////////////////////////////////////////////////////////////////////////
///Перерисовка
//////////////////////////////////////////////////////////////////////////////
    void draw ();

//////////////////////////////////////////////////////////////////////////////
///Основные команды
//////////////////////////////////////////////////////////////////////////////
    void enter     ();
    void backspace ();
    void apply     ();
    void escape    ();
    void up        ();
    void down      ();
    void left      ();
    void right     ();

    void click(int x, int y);

    Menu    *next,*prev;  //!!!!!!!!!!!!!!!!!!!!!

private:
    Frame*  frame[MAX_FRAMES_COUNT];
    int     frames_count, active_frame;
    Frame*  tip_frame;
    //Menu    *next,*prev;
    IMAGE   picture;
};

/*
    Frame
*/

Frame* uiCreateFrame (Menu* owner,const char* name=NULL);
Frame* uiCreateFrame (const char* menu_owner,const char* name=NULL);

#endif
