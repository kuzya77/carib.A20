#include "istl.h"
#include <assert.h>

void ListNodePro::insert_after(ListNodePro* node)
{
	assert(node);
	n_next=node->n_next;
	n_prev=node;
	if(n_next)
		n_next->n_prev=this;
	node->n_next=this;
}

void ListNodePro::remove()
{
	if(n_prev)
	{
		n_prev->n_next=n_next;		
		if(n_next)
			n_next->n_prev=n_prev;
		n_prev=n_next=NULL;
	}
}
