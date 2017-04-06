#ifndef __BLITZ_STUFF_H__
#define __BLITZ_STUFF_H__

#include <types.h>
#include <mem.h>

#include <algorithm>

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
	u16_t 	year;
    u8_t    month;
    u8_t    day;
    u8_t    hour;
    u8_t    minute;
    u8_t    seconds;
    u8_t    reserved;
} tDateTime;
#pragma pack()

void GetTime(tDateTime* t);
void SetTime(tDateTime* t);

inline long tag2long(const char* s) { return *reinterpret_cast<const long*>(s); }

inline char toupper(char c) { return c>='a' && c<='z'?c&0xDF:c; }

//------------------------------------------------------
// Действующие спецификаторы:
// b - byte
// w - слово
// d - dword (4 байта)
// -/+ - порядок байт, - обратный (мотороловский), действует на последующие символы:
// bbw-dw - выгрузить 2 байта прямого, слово прямого, двойное слово обратного порядка
extern "C" size_t bnprintf(void* p,size_t b_size, const char* format,...);
extern "C" size_t bnscanf(const void* p,size_t b_size, const char* format,...);

//------------------------------------------------------
// CRC32

u32_t crc32(const void* data, size_t size, u32_t crc=0xFFFFFFFF);
u32_t crc32(u8_t data, u32_t crc);

u32_t strhash(const char* s);
u32_t istrhash(const char* s);

//------------------------------------------------------
// CRC16
// Polynom X^16+X^12+X^5+1
u16_t crc16(const void* data, size_t size, u16_t crc=0xFFFF);
u16_t crc16(u8_t data, u16_t crc);

// stuff
inline int random(int x)   { return rand()%x; }

inline void memsetw(u16_t* ptr,u16_t data,int count)  { for(;count--;*ptr++=data); }
inline void memsetd(u32_t* ptr,u32_t data,int count)  { for(;count--;*ptr++=data); }

#endif
