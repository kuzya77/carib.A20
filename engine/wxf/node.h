#ifndef __BLITZ_I_WXF_NODE_H__
#define __BLITZ_I_WXF_NODE_H__

#include <types.h>
#include <mem.h>

#include <stuff/parser.h>

//узел парсинга при WXF разборе
class WXFNode: public ParseNode
{
  public:    
    WXFNode (const char* tag_name = NULL);
    WXFNode (const char* tag_name,const Attr* first_attr, size_t attrs_count, WXFNode* parent, WXFNode* last);         

    ParseNode* parent () { return wxf_parent; }
    ParseNode* first  () { return wxf_first; }
    ParseNode* next   () { return wxf_next; }
    ParseNode* prev   () { return wxf_prev; }

    ParseNode* first  (const char*);
    ParseNode* next   (const char*);
    ParseNode* prev   (const char*);

    const char* tag   () { return tag_name; }
    const AttrList&   list  () { return attr_list; }

    void release ();

    void* operator new (size_t,void* x) { return x; }
  protected:
    WXFNode     *wxf_first,*wxf_next,*wxf_prev,*wxf_parent;
    AttrList    attr_list;
    const char* tag_name;
    u32_t      	hash;
};

//корень парсера
class WXFRoot: public WXFNode
{
  friend class WXFParser;
  public:
    enum Error { 
      FILE_NOT_OPENED, //файл не найден
      NO_MEMORY,       //не хватает памяти
    };

    WXFRoot  (const char* file_name, size_t max_nodes, size_t max_attrs) throw (Error);
    ~WXFRoot ();

    const char* tag () { return name; }
    void release () { delete this; }

    void* operator new (size_t size) { return ::operator new (size); }
    void  operator delete (void* p) { ::operator delete (p); }
  
  private:
    char*     	name;
    char*     	text_buf;
    Attr*     	attr_pool;
    WXFNode* 	node_pool;
    size_t     	attr_pool_size;
	size_t		node_pool_size;
};

#endif
