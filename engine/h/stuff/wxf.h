#ifndef __WXF_OUT_H__
#define __WXF_OUT_H__

#include <mem.h>
#include <binfile.h>
#include <stdarg.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
///Утилита генерации WXF файлов
/////////////////////////////////////////////////////////////////////////////////////////////////////
class WXF
{
  public:
    WXF(BINFILE);
    ~WXF ();

/////////////////////////////////////////////////////////////////////////////////////////////////////
///Начало / конец блока
/////////////////////////////////////////////////////////////////////////////////////////////////////
    void begin ();
    void end   ();
    void begin (const char*,...);

/////////////////////////////////////////////////////////////////////////////////////////////////////
///Вставка блочного коментария
/////////////////////////////////////////////////////////////////////////////////////////////////////
    void comment (const char*);

/////////////////////////////////////////////////////////////////////////////////////////////////////
///Базовый вывод
/////////////////////////////////////////////////////////////////////////////////////////////////////
    void print   (const char*);
    void printf  (const char*,...);
    void vprintf (const char*,va_list);

/////////////////////////////////////////////////////////////////////////////////////////////////////
///Вывод строк (в кавычках)
/////////////////////////////////////////////////////////////////////////////////////////////////////
    void puts  (const char*);
    void puts  (const char*,const char*,...);
    void vputs (const char*,const char*,va_list);

/////////////////////////////////////////////////////////////////////////////////////////////////////
///Форматированный вывод
/////////////////////////////////////////////////////////////////////////////////////////////////////
  private:
    BINFILE file;
    int   level,pos;
};

/*
        Форматированная запись базовых типов
*/

WXF& operator << (WXF&,char);
WXF& operator << (WXF&,unsigned char);
WXF& operator << (WXF&,int);
WXF& operator << (WXF&,unsigned int);
WXF& operator << (WXF&,float);
WXF& operator << (WXF&,double);
WXF& operator << (WXF&,const char*);

template <class T> void print(WXF& wxf,const char* tag,const T& x,const char* comment)
{
    wxf<<tag<<'\t'<<x;
    if(comment)
        wxf<<"\t//"<<comment;
    wxf<<'\n';
}

template <class T> void print(WXF& wxf,const char* tag,const T& x)
{
    print(wxf,tag,x,NULL);
}

template <class T> void print(WXF& wxf, const T* array, size_t count, size_t stride)
{
  if (count)
  {
    if(!stride)
    {
      wxf<<'(';

      while (--count) wxf<<*array++<<' ';

      wxf<<*array++<<')';
    }
    else
    {
      wxf<<"\n(\n";

      while (count)
      {
        wxf<<'\t';

        for(size_t j=0; --count && j<stride-1; j++)
			wxf<<*array++<<'\t';

        wxf<<*array++<<'\n';
      }     

      wxf<<')';
    }
  }
}

template <class T> void print(WXF& wxf,const T* array,size_t count)
{
    print(wxf,array,count,0);
}

template <class T> void print(WXF& wxf,const char* tag,const T* array,size_t count,size_t stride,const char* comment)
{
  wxf<<tag<<'\t';
  print(wxf,array,count,stride);
  if (comment)
    wxf<<"\t//"<<comment;
  wxf<<'\n';
}
template <class T> void print(WXF& wxf,const char* tag,const T* array,size_t count,size_t stride)
{
    print(wxf,tag,array,count,stride,NULL);
}

template <class T> void print(WXF& wxf,const char* tag,const T* array,size_t count,const char* comment)
{
    print(wxf,tag,array,count,0,comment);  
}

template <class T> void print(WXF& wxf,const char* tag,const T* array,size_t count)
{
    print(wxf,tag,array,count,0,NULL);  
}

#endif
