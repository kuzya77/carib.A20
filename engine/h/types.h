#ifndef __BLITZ_TYPES_H__
#define __BLITZ_TYPES_H__

#include "common.h"

typedef int                 int32, s32_t;
typedef unsigned int        uint32, u32_t;
typedef unsigned long       ulong;
typedef short               int16, s16_t;
typedef unsigned short      uint16, u16_t, WORD;
typedef char                int8;
typedef unsigned char       uint8, byte, uchar, u8_t;
typedef long long           int64;
typedef unsigned long long  uint64;

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

inline void memsetw(uint16* ptr,uint16 data,int count)  { for(;count--;*ptr++=data); }
inline void memsetd(uint32* ptr,uint32 data,int count)  { for(;count--;*ptr++=data); }

#define lobyte(w) LOBYTE(w)
#define hibyte(w) HIBYTE(w)
#define loword(l) LOWORD(l)
#define hiword(l) HIWORD(l)

#include <log.h>

#endif
