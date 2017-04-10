#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "utils.h"
#include "cmdargs.h"

CmdArgs::CmdArgs ()
{
  reset ();
}

CmdArgs::CmdArgs (const char* fmt,...)
{
  va_list list;

  va_start (list,fmt);
  vparse   (fmt,list);
  va_end   (list);
}

void CmdArgs::reset ()
{
  *buf = 0;
  argc = 0;
  pos  = 0;

  set (0,NULL);
}

void CmdArgs::shift (int offset)
{
  pos += offset;

  if (pos >= argc) pos = argc;
  if (pos < 0)     pos = 0;

  set (argc-pos,argv+pos);
}

static char* nextword (char*& s)
{
  s += strspn (s," \t\r");

  char* first = s;
  const char *t = " \t\r";

  switch (*s)
  {
    case '\'': 
      first = ++s; 
      t     = "'";
      break;
    case '"':  
      first = ++s;
      t     = "\"";
      break;
    case '/':
      if (s [1] == '/')
        return NULL;
      break;
    case '\0':
      return NULL;
  }

  char* last = strpbrk (s,t);

  if (last)
  {
    s    = last;
    *s++ = 0;
  }  
  else s = s + strlen (s);

  return first;
}

void CmdArgs::vparse (const char* fmt,va_list list)
{
  reset ();

  if (!fmt)
    return ;

#ifdef _MSC_VER
  _vsnprintf (buf,BUF_SIZE,fmt,list);
#else
  vsnprintf (buf,BUF_SIZE,fmt,list);
#endif 

  char *s, **i, **end; 

  for (s=buf,i=argv,end=argv+MAX_ARG_COUNT;i != end && (*i=nextword(s));i++);

  set (argc=i-argv,argv);
}

void CmdArgs::parse (const char* fmt,...)
{
  va_list list;

  va_start (list,fmt);
  vparse   (fmt,list);
  va_end   (list);
}                                
