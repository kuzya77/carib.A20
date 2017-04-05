#ifndef __BLITZ_INTERNAL_STUFF_H__
#define __BLITZ_INTERNAL_STUFF_H__

#include <types.h>
#include <stuff.h>
#include <assert.h>
#include <string.h>

template<class Type>
inline void LowTo(Type& _Left,const Type& _Right)
    { if(_Left>_Right) _Left=_Right; }

// Смещение для указанного типа в байтах
template<class T>
inline T* offset_of(T* p,int offset) { return (T*)(((u8_t*)p)+offset); }

/////////////////////////////////////////////////////////////////////////////////////////////////

int GetShiftBit(u32_t bit);

u32_t FileNameCrc(const char* s); // Вычисляет crc32 со следующими параметрами: '\' меняет на '/', маленькие буквы на большие

#endif
