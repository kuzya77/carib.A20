#ifndef __WXF_ATTR_H__
#define __WXF_ATTR_H__

#include <types.h>
#include <mem.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
///Атрибут
////////////////////////////////////////////////////////////////////////////////
class Attr
{
  public:
    Attr () {}
    Attr (const char* _data) : data (_data) { }

    operator const char* () const { return data; }
    operator int         () const { return strtol (data,NULL,0); }
    operator unsigned int() const { return operator int (); }
    operator long        () const { return operator int (); }
    operator unsigned long() const { return operator int (); }
    operator char        () const { return operator int (); }
    operator unsigned char() const { return operator int (); }
    operator short       () const { return operator int (); }
    operator unsigned short      () const { return operator int (); }
    operator bool        () const { return operator int (); }
    operator float       () const { return atof (data); }
    operator double      () const { char* end; return strtod (data,&end); }
    
  private:
    const char* data;
};

////////////////////////////////////////////////////////////////////////////////
///Список атрибутов
////////////////////////////////////////////////////////////////////////////////
class AttrList
{
  public:
    AttrList ();
    AttrList (int argc,const char* const* argv);
    AttrList (int argc,const Attr*);

    int count () const { return attrs_count; }

    operator const Attr* () const { return attrs; }

    void set (int argc,const char* const* argv);

  private:
    int         attrs_count;
    const Attr* attrs;
};

bool read (const AttrList&,int index,char&);
bool read (const AttrList&,int index,unsigned char&);
bool read (const AttrList&,int index,short&);
bool read (const AttrList&,int index,unsigned short&);
bool read (const AttrList&,int index,int&);
bool read (const AttrList&,int index,unsigned int&);
bool read (const AttrList&,int index,long&);
bool read (const AttrList&,int index,unsigned long&);
bool read (const AttrList&,int index,float&);
bool read (const AttrList&,int index,double&);
bool read (const AttrList&,int index,char*);

int         attri (const AttrList&,int index,int = 0);
float       attrf (const AttrList&,int index,float = 0);
double      attrd (const AttrList&,int index,double = 0);
const char* attrs (const AttrList&,int index,const char* = NULL);

template <class T>
bool read_array(const AttrList& list,int index,T* array,int count,int stride)
{
  if (index + count*stride <= list.count ())
  {
    for(int i=0;i<count;i++,index+=stride,array++)
      if(!read (list,index,*array))
        return false;

    return true;
  }

  return false;
}

template <class T>
bool read_array(const AttrList& list,int index,T* array,int count)
{
  if (index + count <= list.count ())
  {
   const Attr* attr = list+index;

    for (int i=0;i<count;i++,attr++,array++)
      *array = *attr;

    return true;
  }

  return false;
}

#endif
