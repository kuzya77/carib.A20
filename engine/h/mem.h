#ifndef __XEN_MEM_MAMANGER__
#define __XEN_MEM_MAMANGER__

#include "types.h"
#include <stddef.h>

#include <new>

struct MemStat
{
  size_t mem_reserved;
  size_t mem_used;  
  size_t min_size;
  size_t max_size;
  uint   alloc_call_count;
  uint   free_call_count;

  void update_alloc(size_t size)
  {
  	mem_used += size;
  	alloc_call_count++;
   
  	if (size<min_size) min_size = size;
  	if (size>max_size) max_size = size;  
  }

  void update_free(size_t size)
  {
    mem_used -= size;
    free_call_count++;
  }
};

/*
        Инициализация / завершение
*/

void memInit ();
void memDone ();

/*
        Global allocator
*/

void*   memAlloc (size_t size);
void*   memAlloc (size_t size,size_t align);
void    memFree  (void*);
size_t  memSize  (void*); //размер блока

const MemStat memStat (); //статистика выделений для дефолтного аллокатора

inline void* operator new(size_t size)   throw (std::bad_alloc) 
{ 
    void* p=memAlloc(size);
    if(!p && size)
        throw std::bad_alloc();
    return p; 
}

inline void* operator new[](size_t size) throw (std::bad_alloc) 
{ 
    void* p=memAlloc(size);
    if(!p && size)
        throw std::bad_alloc();
    return p; 
}

inline void operator delete(void* ptr)      { memFree(ptr); }
inline void operator delete[](void* ptr)    { memFree(ptr); }

#endif
