#ifndef __WXF_I_WXFPAR_H__
#define __WXF_I_WXFPAR_H__

#include <mem.h>

#include "lexer.h"
#include "node.h"

//пул атрибутов
class AttrPool
{
  public:
    enum Error { POOL_OVERFLOW };

    AttrPool (Attr* attr_pool, size_t pool_size);

      //позиция/добавление атрибута
    const Attr* tell      () const { return pos; }
    void        push_attr (const char*) throw (Error);

  private:
    Attr *pos, *end;
};

//пул узлов
class NodePool
{
  public:
    enum Error { POOL_OVERFLOW };

    NodePool (WXFNode* nodes_pool, size_t pool_size);

      //выделение памяти под узел
    WXFNode* alloc () throw (Error);

  private:
    WXFNode *pos, *end;
};

//парсер WXF ресурсов
class WXFParser
{
  public:
    WXFParser (WXFRoot* root);

  private:
    void     parse_root       (WXFNode*);
    void     parse_frame      (WXFNode*);
    WXFNode* parse_node       (WXFNode*,WXFNode*);
    void     parse_attributes ();
    bool     parse_nextline   ();

    void error   (const char*,...);
    void process_lex_error ();

  private:
    Lexer    lex;
    WXFRoot* root;
    NodePool nodes;
    AttrPool attrs;
};

#endif
