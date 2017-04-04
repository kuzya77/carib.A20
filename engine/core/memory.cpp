#include "log.h"
#include "mem.h"

#include "istuff.h"

#include <malloc.h>

static MemStat default_stat = {0,0,0,0,0,0};

inline void* align_ptr (void* p,size_t align)
{
  return (void*)(((int)p+(align-1))&~(align-1));
}

inline void init_stat (MemStat& stat)
{
  stat.mem_used         = stat.mem_reserved = 0;
  stat.alloc_call_count = stat.free_call_count = 0;
  stat.max_size         = 0;  
  stat.min_size         = (size_t)-1;
}

/*
    Глобальный менеджер памяти
*/

struct DefMemStat: public MemStat
{
  DefMemStat () { init_stat (*this); }
};

static DefMemStat default_space_stat;

void* memAlloc(size_t size)
{
  if (!size)
    return NULL;
    
  void* p = malloc(size);

  if (!p)
  {
        logError("MemManager: No enough memory.");
// !!!!! см. комментарии и мысли по крит. выходу
//      abort ();
        return NULL;  
  }

  default_space_stat.update_alloc(malloc_usable_size(p));
  
  return p;
}

void* memAlloc(size_t size, size_t align)
{
  if (!size)
    return NULL;   
    
  void* p = malloc(size);

  if (!p)
  {
    if (align&(align-1))
    {
            logError("MemManager: No enough memory.");
// !!!!! см. комментарии и мысли по крит. выходу
//          abort ();
    }   
    return NULL;  
  }

  default_space_stat.update_alloc(malloc_usable_size(p));
  
  return p;
}

void memFree (void* p)
{
	if (!p)
    		return;
    
  	default_space_stat.update_free(malloc_usable_size(p));

	free(p);
} 

const MemStat memStat ()
{  
	default_space_stat.mem_reserved = 0;
	return default_space_stat;
}

void memDestroyAll ()
{
}

void memInit()
{
}

void memDone ()
{
  memDestroyAll (); 
}
