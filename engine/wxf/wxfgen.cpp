#include <mem.h>
#include <stuff/wxf.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

const size_t BUF_SIZE = 1024;

static char tab_buf [BUF_SIZE];

WXF& operator << (WXF& os,char x)        { return os.printf ("%c",x), os;  }
WXF& operator << (WXF& os,unsigned char x)       { return os.printf ("%c",x), os;  }
WXF& operator << (WXF& os,int x)         { return os.printf ("%d",x), os;  }
WXF& operator << (WXF& os,unsigned int x){ return os.printf ("%u",x), os;  }
WXF& operator << (WXF& os,float x)       { return os.printf ("%f",x), os;  }
WXF& operator << (WXF& os,double x)      { return os.printf ("%lf",x), os; }
WXF& operator << (WXF& os,const char* x) { return os.print (x), os; }

void WXF::begin ()
{ 
  print("\n{");
  level++;
  print("\n");
}

void WXF::end ()
{
  if (level)
  {
    level--;
    print ("}\n");
  }
}

void WXF::begin (const char* fmt,...)
{
  va_list list;

  va_start (list,fmt);
  vprintf  (fmt,list);
  va_end   (list);

  begin ();
}

void WXF::comment (const char* text)
{
  print ("/*");
  print (text);
  print ("*/\n");
}

void WXF::printf  (const char* fmt,...)
{
	va_list list;

  	va_start (list,fmt);
  	vprintf  (fmt,list);
	va_end   (list);
}

void WXF::vprintf (const char* fmt,va_list list)
{
	char default_buf[BUF_SIZE], *buf = default_buf;
	vsprintf(buf,fmt,list);
  	print (buf);
}

void WXF::puts (const char* s)
{
  printf ("\"%s\"",s);
}

void WXF::puts (const char* tag,const char* fmt,...)
{
  va_list list;

  va_start (list,fmt);
  vputs    (tag,fmt,list);
  va_end   (list);
}

void WXF::vputs (const char* tag,const char* fmt,va_list list)
{
  printf  ("%s\t\"",tag);
  vprintf (fmt,list);
  print   ("\"\n");
}

static void write_tabs(BINFILE file, size_t len)
{
  while (len)
  {     
    size_t wlen = len<BUF_SIZE ? len : BUF_SIZE;

	bfWrite(file,tab_buf,wlen);

    len -= wlen;
  }
}

static void tab_buf_init ()
{
  static bool init = false;

  if (!init)
  {
    memset (tab_buf,'\t',BUF_SIZE);
    init = true;
  }
} 

void WXF::print (const char* s)
{
	const char* first;
  	for(first=s;*s;s++)
  {
    if (!pos)      
      write_tabs (file,pos=level);

    switch (*s)
    {
      case '\n':
      {
      	bfWrite(file,first,s-first+1);
        first = s+1;
        pos   = 0;
        break;
      } 
      default:
        pos++;
        break;
    }
  }

  	bfWrite(file,first,s-first);
}

WXF::WXF (BINFILE _file)
{
	assert(_file);

	tab_buf_init();

	file=_file;

	level = 0;
	pos   = 0;
}

WXF::~WXF ()
{
}
