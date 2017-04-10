#include <stuff/attr.h>
#include <string.h>

AttrList::AttrList ()
  : attrs (NULL), attrs_count (0)
{ }

AttrList::AttrList (int argc,const char* const* argv)
  : attrs ((const Attr*)argv), attrs_count (argc>=0?argc:0)
{ }

AttrList::AttrList (int argc,const Attr* argv)
  : attrs (argv), attrs_count (argc>=0?argc:0)
{ }

void AttrList::set (int argc,const char* const* argv)
{
  attrs_count = argc>=0?argc:0;
  attrs       = (const Attr*)argv;
}

template <class T> inline bool read_scalar (const AttrList& list,int index,T& x)
{
  if (index < list.count ())
  {
    x = list [index];
    return true;
  }  
  else return false;
}

template <class T> inline T get_value (const AttrList& list,int index,const T& def_value)
{
  T t;

  return read (list,index,t) ? t : def_value;
}

bool read (const AttrList& list,int index,char& x)   { return read_scalar (list,index,x); }
bool read (const AttrList& list,int index,unsigned char& x)  { return read_scalar (list,index,x); }
bool read (const AttrList& list,int index,short& x)  { return read_scalar (list,index,x); }
bool read (const AttrList& list,int index,unsigned short& x) { return read_scalar (list,index,x); }
bool read (const AttrList& list,int index,int& x)    { return read_scalar (list,index,x); }
bool read (const AttrList& list,int index,unsigned int& x)   { return read_scalar (list,index,x); }
bool read (const AttrList& list,int index,long& x)   { return read_scalar (list,index,x); }
//bool read (const AttrList& list,int index,ulong& x)  { return read_scalar (list,index,x); }
bool read (const AttrList& list,int index,float& x)  { return read_scalar (list,index,x); }
bool read (const AttrList& list,int index,double& x) { return read_scalar (list,index,x); }

bool read (const AttrList& list,int index,char* x)
{
  if (index < list.count ())
  {
    strcpy (x,list [index]);
    return true;
  }  
  else return false;  
}

int attri (const AttrList& list,int index,int def_value)    
{ 
  return get_value (list,index,def_value); 
}

float attrf (const AttrList& list,int index,float def_value)  
{ 
  return get_value (list,index,def_value); 
}

double attrd (const AttrList& list,int index,double def_value) 
{ 
  return get_value (list,index,def_value); 
}

const char* attrs (const AttrList& list,int index,const char* def_value)
{
  return index < list.count () ? (const char*)list [index] : def_value;
}

bool test (const AttrList& list,int index,const char* s)
{
  return s && index < list.count () ? !strcmp (s,attrs (list,index)) : false;
}

bool test (const AttrList& list,int index,int i)
{
  return index < list.count () ? attri (list,index) == i : false;
}

bool test (const AttrList& list,int index,float x)
{
  return index < list.count () ? attrf (list,index) == x : false;
}
