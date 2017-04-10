#ifndef __STUFF_CMD_ARGS__
#define __STUFF_CMD_ARGS__

#include <mem.h>
#include <stdarg.h>
#include <stuff/attr.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///Аргументы, передаваемые функции-обработчику
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class CmdArgs: public AttrList
{
  public:
    CmdArgs ();
    CmdArgs (const char*,...);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///Парсинг / сброс
//////////////////////////////////////////////////////////////////////////////////////////////////////////
    void parse  (const char*,...);
    void vparse (const char*,va_list);
    void reset  ();

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///Сдвиг
//////////////////////////////////////////////////////////////////////////////////////////////////////////
    void shift (int offset);

  private:
    enum { MAX_ARG_COUNT = 32, BUF_SIZE = 1024 };

    char  buf  [BUF_SIZE];
    char* argv [MAX_ARG_COUNT];
    int   argc, pos;
};

#endif