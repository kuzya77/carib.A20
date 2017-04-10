#ifndef __WXF_PARSER_H__
#define __WXF_PARSER_H__

#include <mem.h>
#include <stuff/attr.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///Узел синтаксического дерева
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ParseNode
{
  public:
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///Итерация
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    virtual ParseNode* parent () = 0;
    virtual ParseNode* first  () = 0;
    virtual ParseNode* first  (const char*) = 0;
    virtual ParseNode* next   () = 0;
    virtual ParseNode* next   (const char*) = 0;
    virtual ParseNode* prev   () = 0;
    virtual ParseNode* prev   (const char*) = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///Тэг и список атрибутов
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    virtual const char* tag   () = 0;
    virtual const AttrList&   list  () = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///Освобождение
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
    virtual void release () = 0;

  protected:
    virtual ~ParseNode () {}
};

extern const AttrList default_attr_list;

/*
        Функции чтения атрибутов
*/
/*
template <class T> bool read (ParseNode*,const char* tag,T&);
template <class T> bool read (ParseNode*,T&);
template <class T> bool read_array (ParseNode*,T* array,unsigned int count,unsigned int stride=1);
template <class T> bool read_array (ParseNode*,const char* tag,T* array,unsigned int count,unsigned int stride=1);
*/
int         attri (ParseNode*,const char* tag,int = 0);
float       attrf (ParseNode*,const char* tag,float = 0);
double      attrd (ParseNode*,const char* tag,double = 0);
const char* attrs (ParseNode*,const char* tag,const char* = NULL);
const AttrList& attrlist (ParseNode*,const char* tag,const AttrList& list = default_attr_list);

/*
        Парсинг
*/

ParseNode* parLoadWXF  (const char* name, size_t max_nodes, size_t max_attributes);
ParseNode* parParseWXF (const char* text, int len);

//ParseNode*  parClone (ParseNode*,bool copy=false);
void        parFree  (ParseNode*);

template <class T> 
bool read (ParseNode* p,T& x)
    { return p?read(p->list(),0,x) : false; }

template <class T> bool read (ParseNode* p,const char* tag,T& x)    
                                { return tag ? read (p->first (tag),x) : false; }
template <class T> bool read_array (ParseNode* p,const char* tag,T* array, size_t count, size_t stride)
                                {  return tag ? read_array (attrlist(p,tag), 0, array,count, stride) : false; }
template <class T> bool read_array (ParseNode* p,const char* tag,T* array,size_t count)
                                {  return read_array(attrlist(p,tag), 0, array, count, 1);  }

#endif
