#ifndef __BLITZ_INTERNAL_STL_H__
#define __BLITZ_INTERNAL_STL_H__

#include "mem.h"

#ifdef _MSC_VER
#pragma warning (disable:4355) // 'this' used in base member initializer list
#endif

class ListNodePro
{
protected:
    ListNodePro* n_next,*n_prev;
public:
    ListNodePro(ListNodePro* n,ListNodePro* p):n_next(n),n_prev(p) {}
    ListNodePro():n_next(NULL),n_prev(NULL) {}

    void insert_after(ListNodePro* node);   // insert after (node)
    void remove();
};

template <class T>
class ListNode:public ListNodePro
{
public:
    T* owner;
    ListNode(T* o):owner(o) { }

    void init(T* o) { n_next=n_prev=NULL; owner=o; }

    ListNode<T>* next() const { return (ListNode<T>*)n_next; }
    ListNode<T>* prev() const { return (ListNode<T>*)n_prev; }
};

template <class T>
class baseSystem
{
public:
    typedef ListNode<T> tListNode;

    bool    IsInit;
    int     Count;      // Кол-во подключенных ресурсов

    tListNode* first()      { return root.next(); }
    bool Register(T* b);
    bool IsRegistered(const T* b);  // Проверка на принадлежность объекта к списку ресурсов

    void*   alloc(size_t size)  { return memAlloc(size);   }
    void    free(void* ptr)     { memFree(ptr); }

protected:
    tListNode root;
    baseSystem():IsInit(false),Count(0),root(NULL) {}
};

template <class T>
inline bool baseSystem<T>::Register(T* b)
{
    if(!b)
        return false;
    b->node.insert_after(&root);
    Count++;
    return true;
}

template <class T>
inline bool baseSystem<T>::IsRegistered(const T* b)
{
    for(tListNode* node=first();node;node=node->next())
        if(node->owner==b)
            return true;
    return false;
}

// Менеджер фиксированных кусков со статическим определением размера
template <class T>
class MemBlock 
{
public:
    union
    {
        MemBlock<T> *Next;
        T Element;
    };
};

template <class T,int n>
class BlockManager 
{
    MemBlock<T> BlockArray[n];
    MemBlock<T> *FreeBlocks;
public:
    BlockManager()
        {
            FreeBlocks=BlockArray;
            for(int i=1;i<n;i++)
                BlockArray[i-1].Next=&BlockArray[i];
            BlockArray[n-1].Next=NULL;
        }

    T* alloc()
        {
            MemBlock<T>* block=FreeBlocks;
            if(FreeBlocks)
                FreeBlocks=block->Next;
            return &(block->Element);
        }   

    void free(T* t)
        {
            MemBlock<T>* b=(MemBlock<T>*)t;
            b->Next  = FreeBlocks;
            FreeBlocks=b;
        }
};

#endif
