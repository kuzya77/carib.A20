#include <stuff/parser.h>

const AttrList default_attr_list;

template <class T> inline T get_value (ParseNode* p,const char* tag,const T& def_value)
{
  T t;

  return read (p,tag,t) ? t : def_value;
}

int attri (ParseNode* p,const char* tag, int def_value)
{
  return get_value (p,tag,def_value);
}

float attrf (ParseNode* p,const char* tag,float def_value)
{
  return get_value (p,tag,def_value);
}

double attrd (ParseNode* p,const char* tag,double def_value)
{
  return get_value (p,tag,def_value);
}

const char* attrs (ParseNode* p,const char* tag, const char* def_value)
{
  if (!p || !tag)
    return NULL;

  p = p->first (tag);

  return p ? ::attrs (p->list (),0,def_value) : def_value;
}

bool test  (ParseNode* p, const char* tag)
{
  return p && p->first(tag);
}

const AttrList& attrlist (ParseNode* p,const char* tag, const AttrList& list)
{
    if (!p || !tag)
        return list;

    p=p->first(tag);

    return p?p->list():list;
}
