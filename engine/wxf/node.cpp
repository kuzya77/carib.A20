#include <types.h>
#include <mem.h>
#include <stuff.h>
#include <binfile.h>

#include "node.h"
#include "wxfpar.h"

#include <string.h>

/*
        WXFNode
*/

WXFNode::WXFNode  (const char* _name)
  : tag_name (_name)
{
  wxf_parent = wxf_next = wxf_prev = wxf_first = NULL;
  hash       = strhash(tag_name);
}

WXFNode::WXFNode (const char* _name, const Attr* first_attr, size_t attrs_count, WXFNode* _parent,WXFNode* last)
  : attr_list (attrs_count,first_attr)
{
  tag_name   = _name;
  wxf_parent = _parent;
  wxf_prev   = last;
  wxf_next   = NULL;
  wxf_first  = NULL;

  if (last)             last->wxf_next = this;
  else if (wxf_parent)  wxf_parent->wxf_first = this;

  hash = strhash(tag_name);
}

void WXFNode::release ()
{
  if (wxf_prev) wxf_prev->wxf_next = wxf_next;
  else if (wxf_parent) wxf_parent->wxf_first = wxf_next;

  if (wxf_next) wxf_next->wxf_prev = wxf_prev;  

  while (wxf_first) wxf_first->release ();
}

ParseNode* WXFNode::first(const char* tag)
{
  u32_t htag = strhash (tag);

  for (WXFNode* i=wxf_first;i;i=i->wxf_next)
    if (htag == i->hash)
      return i;

  return NULL;
}

ParseNode* WXFNode::next (const char* tag)
{
  u32_t htag = strhash (tag);

  for (WXFNode* i=wxf_next;i;i=i->wxf_next)
    if (htag == i->hash)
      return i;

  return NULL;  
}

ParseNode* WXFNode::prev (const char* tag)
{
  u32_t htag = strhash (tag);

  for (WXFNode* i=wxf_prev;i;i=i->wxf_next)
    if (htag == i->hash)
      return i;

  return NULL;  
}

/*
        WXFRoot
*/

WXFRoot::WXFRoot (const char* file_name, size_t max_nodes, size_t max_attrs) throw (Error)
  : WXFNode (file_name), attr_pool (NULL), text_buf (NULL), node_pool (NULL)
{
  try
  {
    BINFILE bf = bfOpen (file_name,OPEN_RO);

    if (!bf)
      throw FILE_NOT_OPENED;

    size_t size = bfSize(bf);
    text_buf    = (char*)memAlloc(sizeof(char)*size+1);

    if (!text_buf)
    {
      bfClose (bf);
      throw NO_MEMORY;     
    }  

    size_t res = bfRead  (bf, text_buf, size);

    bfClose (bf);
     
    text_buf [res] = 0;

    node_pool      = (WXFNode*)memAlloc (sizeof (WXFNode) * max_nodes);
    attr_pool      = (Attr*)memAlloc (sizeof (Attr) * max_attrs);

    attr_pool_size = max_attrs;
    node_pool_size = max_nodes;

    if (!attr_pool || !node_pool)
      throw NO_MEMORY;

    if(file_name)
    {
    	name=(char*) memAlloc(strlen(file_name)+1);
    	if(name)
    		strcpy(name,file_name);
    }
    else
    	name=NULL;

    WXFParser (this);
  }
  catch (...)
  {  	
    if (attr_pool) memFree (attr_pool);
    if (node_pool) memFree (node_pool);
    if (text_buf)  memFree (text_buf);

    throw;
  }
}

WXFRoot::~WXFRoot ()
{
  if (text_buf)  memFree (text_buf);
  if (attr_pool) memFree (attr_pool);
  if (node_pool) memFree (node_pool);  
  if (name)	 memFree (name);
}
