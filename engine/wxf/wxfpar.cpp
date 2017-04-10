#include <mem.h>
#include <log.h>

#include "wxfpar.h"

#include <stdarg.h>
#include <stdio.h>
#include <common.h>

/*
        AttrPool
*/

inline AttrPool::AttrPool (Attr* attr_pool, size_t pool_size)
  : pos (attr_pool), end (attr_pool+pool_size)
{ }

inline void AttrPool::push_attr (const char* attr) throw (Error)
{
  if (pos == end)
    throw POOL_OVERFLOW;

  *pos++ = attr;
}

/*
        NodePool
*/

inline NodePool::NodePool (WXFNode* nodes_pool, size_t pool_size)
  : pos (nodes_pool), end (nodes_pool+pool_size)
{ }

WXFNode* NodePool::alloc () throw (Error)
{
  if (pos == end)
    throw POOL_OVERFLOW;

  return pos++;
}

/*
        WXFParser
*/

WXFParser::WXFParser (WXFRoot* _root)
  : lex   (_root->text_buf), 
    attrs (_root->attr_pool,_root->attr_pool_size),
    nodes (_root->node_pool,_root->node_pool_size),
    root  (_root)
{  
  parse_root (root);
}

void WXFParser::error (const char* fmt,...)
{
  va_list list;
  char buf [1024*10];

  va_start (list,fmt);
  vsprintf (buf,fmt,list);
  va_end   (list);

    //переделать!

  logError("%s (%d,%d) error: %s",root->tag (),lex.line(),lex.offs(),buf);
}

void WXFParser::process_lex_error ()
{
  switch (lex.error ())
  {
    case Lexer::WRONG_CHAR:       error ("Wrong char '%c'",lex.curchar ()); break;
    case Lexer::WRONG_IDENTIFIER: error ("Wrong identifier"); break;
    case Lexer::WRONG_VALUE:      error ("Wrong value"); break;
    case Lexer::UNCLOSED_COMMENT: error ("Unclosed comment"); break;
    case Lexer::UNCLOSED_STRING:  error ("Unclosed string"); break;
	default:;
  }  
}

bool WXFParser::parse_nextline ()
{
  for (;;)
  {
    switch (lex.next ())
    {
      case Lexer::NEW_LINE:     break;
      case Lexer::BEGIN_BRACET: return true;
      default:                  return false;
    }
  }
}

void WXFParser::parse_attributes ()
{
  int bracet_level = 0;

  for (;;)
  {
    switch (lex.lexem ())
    {      
      case Lexer::VALUE:
      case Lexer::IDENTIFIER:
      case Lexer::STRING:
        attrs.push_attr (lex.token ());
        break;
      case Lexer::BEGIN_BRACET:
        bracet_level++;
        break;
      case Lexer::END_BRACET:
        if (bracet_level) bracet_level--;
        else
          error ("Syntax error: Unexpected bracet");
        break;
      case Lexer::BEGIN_FRAME:
        if (bracet_level)
          error ("Unexpected '{'. Unclosed attribute list");
        else return;
        break;
      case Lexer::END_FRAME:
        if (bracet_level)
          error ("Unexpected '}'. Unclosed attribute list");
        else return;
        break;
      case Lexer::NEW_LINE:
        if (!bracet_level)
        {
          if (parse_nextline ()) 
            continue;

          return;             
        }
        break;       
      case Lexer::END_OF_FILE: 
        if (bracet_level)
          error ("Unexpected EOF: Unclosed attribute list");
        else return;          
        break;
      default:
        process_lex_error ();
        break;
    }

	lex.next ();
  }
}

WXFNode* WXFParser::parse_node (WXFNode* parent,WXFNode* last)
{
  char* tag_name   = lex.token ();
  const Attr* first_attr = attrs.tell ();

  lex.next ();

  parse_attributes ();

  WXFNode* node = new (nodes.alloc ()) WXFNode (tag_name,first_attr,attrs.tell()-first_attr,parent,last);

  for (;;lex.next ())
    switch (lex.lexem ())
    {
      case Lexer::NEW_LINE: break;
      case Lexer::BEGIN_FRAME:
        parse_frame (node);

        switch (lex.lexem ())
        {
          case Lexer::END_FRAME:   lex.next ();  break;
          case Lexer::END_OF_FILE: error ("Unexpected EOF. Unclosed frame"); break;
          default:                 
            error ("Unprocessed error at parse_node");
            process_lex_error ();
            break;
        }
      default:
        return node;
    }          
}

void WXFParser::parse_frame (WXFNode* parent)
{
  WXFNode* last = NULL;

  lex.next ();

  for (;;)
  {
    switch (lex.lexem ())
    {
      case Lexer::IDENTIFIER:   last = parse_node (parent,last);  continue;
      case Lexer::VALUE:        error ("Syntax error: unexpected value %s",lex.token ());   break;
      case Lexer::STRING:       error ("Syntax error: unexpected string %s",lex.token ());  break;      
      case Lexer::BEGIN_FRAME:  error ("Syntax error. Unexpected '{'");  break;
      case Lexer::BEGIN_BRACET: error ("Syntax error. Unexpected '('");  break;
      case Lexer::END_BRACET:   error ("Syntax error. Unexpected ')'");  break;
      case Lexer::NEW_LINE:     break;
      case Lexer::END_FRAME:
      case Lexer::END_OF_FILE:  return;
      default:                  process_lex_error ();  break;
    }

    lex.next ();
  }
}

void WXFParser::parse_root (WXFNode* root)
{
  parse_frame (root);

  switch (lex.lexem ())
  {
    case Lexer::END_OF_FILE: break;
    case Lexer::END_FRAME:   error ("Unexpected '}'. No frame opened"); break;
	default: break;
  }
}

ParseNode* parLoadWXF (const char* fname, size_t max_nodes, size_t max_attrs)
{
  try
  {
    return fname ? new WXFRoot (fname,max_nodes,max_attrs) : NULL;
  }
  catch (WXFRoot::Error err)
  {
    switch (err)
    {
      case WXFRoot::FILE_NOT_OPENED:    
        logError("Error at open file '%s'",fname); 
        break;
      case WXFRoot::NO_MEMORY:
        logError("No memory for parsing '%s' (max_nodes=%u, max_attributes=%u)", fname, max_nodes, max_attrs); 
        break;
    }    
  }
  catch (AttrPool::Error err)
  {
    switch (err)
    {
      case AttrPool::POOL_OVERFLOW:
        logError("Too many attributes '%s' (max_atributes=%u)",fname,max_attrs);
        break;
    }
  }  
  catch (NodePool::Error err)
  {
    switch (err)
    {
      case NodePool::POOL_OVERFLOW:
        logError("Too many nodes '%s' (max_nodes=%u)",fname,max_nodes);
        break;
    }
  }  

  return NULL;
}

void parFree(ParseNode* p)
{
	if(!p)
    		return;
  	p->release ();
}
