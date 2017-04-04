#ifndef __BLITZ_BINFILE_H__
#define __BLITZ_BINFILE_H__

#include <stddef.h>

#include "types.h"

// Режимы доступа к файлам
enum 
{ 
    OPEN_RO=0,      // только чтение
    OPEN_RW,        // чтение/запись
    OPEN_CR         // создание файла, если такой файл уже есть, файл обнуляется
};


class BinFile;

typedef BinFile* BINFILE;

bool bfInit(int MaxFileCount=32,int MaxPathsCount=32);
void bfDone();

bool bfAddPath(const char* name,const char* password=NULL);     
                                        // Добавляет указанный путь в пути поиска файла, добавляется в конец списка путей.
bool bfAddZip(BINFILE zipFile,const char* password=NULL);       
                                        // механизм применения: если файл открывается в режиме на запись, пути игнорируются
                                        // Перебор начинается с первого каталога, простым добавлением пути к имени файла.
                                        // возможно применение .zip архивов в качестве пути.
                                        // отказ возвращается если путь не может быть добавлен (заполнена вся таблица), 
                                        // либо (в случае .zip) неправильный или отсутствующий архив

BINFILE bfOpen(const char* name, int mode);
BINFILE bfOpenMem(void* ptr, int mode, int size);

inline BINFILE bfOpenMem(int size) {    return bfOpenMem(NULL,OPEN_RW,size);   }

void bfClose(BINFILE bf);       // закрытие файла

size_t  bfRead(BINFILE bf, void* buf, size_t len);
size_t  bfWrite(BINFILE bf, const void* buf, size_t len);

// basic types io
int     bfGetc(BINFILE bf);     // byte     return EOF if eof
int     bfGetw(BINFILE bf);     // u16_t    return EOF if eof
s32_t   bfGetl(BINFILE bf);

bool    bfPutc(BINFILE bf,char c);
bool    bfPutw(BINFILE bf,s16_t c);
bool    bfPutl(BINFILE bf,s32_t c);

char* 	bfGets(BINFILE file,char* s, size_t size);

void    bfRewind(BINFILE bf);

size_t  bfSeek(BINFILE bf,size_t pos);
size_t  bfSeekEnd(BINFILE bf,size_t pos);
size_t  bfSeekCur(BINFILE bf,size_t pos);
size_t  bfTell(BINFILE bf);


size_t  bfSize(BINFILE bf);
size_t  bfResize(BINFILE bf,size_t size);


bool    bfEof(BINFILE bf);

// Проверка наличия файла
bool    bfFileExist(const char* fname);

#endif
