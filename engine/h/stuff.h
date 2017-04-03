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

extern uint32 crc_32_table[];

uint32 crc32(const void* data,size_t size,uint32 crc=0xFFFFFFFF);
inline uint32 crc32(byte data,uint32 crc) { return crc_32_table[(crc^data)&0xff]^(crc>>8); }

//------------------------------------------------------
// CRC16
// Polynom X^16+X^12+X^5+1
extern uint16 crc_16_table[];

uint16 crc16(const void* data,size_t size,uint16 crc=0xFFFF);
inline uint16 crc16(byte data,uint16 crc) { return crc_16_table[(crc^data)&0xff]^(crc>>8); }

// stuff
#define random(x)   (rand()%x)

#endif
