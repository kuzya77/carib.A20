#ifndef __BLITZ_TYPES_H__
#define __BLITZ_TYPES_H__

#include "common.h"

typedef int                 int32, s32_t;
typedef unsigned int        uint32, u32_t;
typedef unsigned long       ulong;
typedef short               int16, s16_t;
typedef unsigned short      uint16, u16_t;
typedef char                int8;
typedef unsigned char       uint8, byte, uchar, u8_t;
typedef long long           int64;
typedef unsigned long long  uint64;

#define MAKEWORD(a, b)      ((u16_t)(((u8_t)(a)) | ((u16_t)((u8_t)(b))) << 8))
#define MAKELONG(a, b)      ((s32_t)(((u16_t)(a)) | ((u32_t)((u16_t)(b))) << 16))
#define LOWORD(l)           ((u16_t)(l))
#define HIWORD(l)           ((u16_t)(((u32_t)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((u8_t)(w))
#define HIBYTE(w)           ((u8_t)(((u16_t)(w) >> 8) & 0xFF))

#define lobyte(w) LOBYTE(w)
#define hibyte(w) HIBYTE(w)
#define loword(l) LOWORD(l)
#define hiword(l) HIWORD(l)

#include <log.h>

#endif
