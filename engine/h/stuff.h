#ifndef __BLITZ_STUFF_H__
#define __BLITZ_STUFF_H__

#include "types.h"
#include "mem.h"

#include <stl/algorithm>
using namespace stl;

// всякие полезности
#define SAFE_DESTROY(fn,ptr)    { fn(ptr); ptr=NULL; }
#define NULL_DESTROY(fn,ptr)    { if(ptr) SAFE_DESTROY(fn,ptr); }

//  Работа со временем
// время в тыс. сек.
uint32 MilliSecs();
void Delay(uint32 t);

#pragma pack(1)
typedef struct
{
        uint16  year;
        byte    month;
        byte    day;
        byte    hour;
        byte    minute;
        byte    seconds;
	byte    reserved;   // В ниосе проблемы с выравниванием
} tDateTime;
#pragma pack()

void GetTime(tDateTime* t);
void SetTime(tDateTime* t);

inline long tag2long(const char* s) { return *((const long*)s); }

inline char toupper(char c) { return c>='a' && c<='z'?c&0xDF:c; }

//------------------------------------------------------
// Действующие спецификаторы:
// b - byte
// w - слово
// d - dword (4 байта)
// -/+ - порядок байт, - обратный (мотороловский), действует на последующие символы:
// bbw-dw - выгрузить 2 байта прямого, слово прямого, двойное слово обратного порядка
size_t bnprintf(void* p,size_t b_size,const char* format,...);
size_t bnscanf(const void* p,size_t b_size,const char* format,...);

//------------------------------------------------------
// CRC32

u32_t crc32(const void* data,size_t size,u32_t crc=0xFFFFFFFF);
u32_t crc32(byte data, u32_t crc);

//------------------------------------------------------
// CRC16
// Polynom X^16+X^12+X^5+1

u16_t crc16(const void* data, size_t size, u16_t crc=0xFFFF);
u16_t crc16(byte data, u16_t crc);

// stuff
#define random(x)   (rand()%x)

#endif
