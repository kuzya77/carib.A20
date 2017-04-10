#include <picture.h>	// def color_t
#include "console.h"

ConsoleCtrl::ConsoleCtrl (Console* con)
  : Control (con,"__noname")
{
  owner = con; 
}

void ConsoleCtrl::up ()
{
  owner->seek (owner->tell ()-1);
}

void ConsoleCtrl::down ()
{
  owner->seek (owner->tell ()+1);
}

Console::Console(Menu* owner,const char* name):Frame(owner,name),ctrl(this)
{
  clear ();
  select (&ctrl);
}

void Console::clear ()
{
  pos            = buf;
  end        = buf + CONSOLE_BUF_SIZE;
  cur_line       = line;
  end_line   = line + CONSOLE_LINES_NUM;
  line_start_pos = buf;  
  color          = 0;
  top_line   = 0;

  *buf = 0;
}

void Console::resize(int width,int height)
{
  Frame::resize (width,height);

  con_height = Frame::height () / vstep (); //recalc console height  
}

void Console::putline ()
{
  char* s; //for NIOS

  for (s=pos;*s;s++)
    switch (*s)
    {
      case '\n':
        if (cur_line == end_line)
          return;

        cur_line->msg   = line_start_pos;
        cur_line->color = color;
        line_start_pos  = s+1;
        cur_line++;
        *s = 0;

    if (cur_line-line-top_line-con_height == 1)
      seek (tell () + 1);     

        break;
    }

  pos = s;
}

void Console::printf(color_t color,const char* message,...)
{
    va_list list;
    va_start (list,message);
    vprintf (color,message,list);
}

void Console::printf(const char* message,...)
{
    va_list list;
    va_start (list,message);
    vprintf (message,list);
}

void Console::vprintf(color_t _color, const char* message, va_list list)
{
  color = _color;  

  if (end == pos)
    return;

  vsnprintf (pos,end-pos-1,message,list);

  putline ();
}

void Console::vprintf (const char* message,va_list list)
{
  vprintf (255,message,list);
}

void Console::seek (int top)
{
  top_line = top;

  int lines_count = cur_line - line;

  if (top_line + con_height > lines_count) top_line = lines_count - con_height;
  if (top_line < 0) top_line = 0;
}

void Console::_draw ()
{
  int count = con_height, lines_count = cur_line - line;

  if (top_line + count > lines_count) count = lines_count - top_line;
  if (count < 0) count = 0;

  for (int y=0;y<count;y++)
    drawtext (0,y*vstep(),line [y+top_line].msg,line [y+top_line].color,INDENT_FULL);
}

void uiClear (Console* con)
{
  if (con)
    con->clear ();
}

void uiPrintf(Console* con, color_t color, const char* message,...)
{
  va_list list;

  va_start (list,message);
  
  uiVPrintf (con,color,message,list);
}

void uiPrintf  (Console* con,const char* message,...)
{
  va_list list;

  va_start (list,message);
  
  uiVPrintf (con,message,list);
}

void uiVPrintf (Console* con, color_t color, const char* message, va_list list)
{
  if (con)
    con->vprintf (color,message,list);
}

void uiVPrintf (Console* con, const char* message, va_list list)
{
  if (con)
    con->vprintf (message,list);
}

void uiSeek (Console* con,int top_line)
{
  if (con)
    con->seek (top_line);
}

int uiTell (Console* con)
{
  return con ? con->tell () : 0;
}
