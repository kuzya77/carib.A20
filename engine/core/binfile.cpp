#include "types.h"
#include "mem.h"
#include "stuff.h"

#include "ibinfile.h"

#include <stdlib.h>

BinFile::~BinFile()
{
    node.remove();  // remove link from FileList's
    bfSys.free(buffer);    
}

long BinFile::read(void*,long)
{
    return 0;
}

long BinFile::write(const void*, long)
{
    return 0;
}

bool BinFile::reset()
{
    return seek(0)==0;
}

long BinFile::seek(long pos)
{
    if(canseek())
    {
        if(pos<0)
                pos=0;
        if(pos>filelen)
                pos=filelen;
        filepos=pos;
    }
    return filepos;
}

long BinFile::resize(long)
{
    return filelen;
}

int BinFile::getc()
{
    char c;
    return read(&c,sizeof(c))?c:EOF;
}

int BinFile::gets()
{
    s16_t s;
    return read(&s,sizeof(s))?s:EOF;
}

BinFile &BinFile::putc(char c)
{
    write(&c, 1);
    return *this;
}

BinFile &BinFile::puts(s16_t s)
{
    write(&s, 2);
    return *this;
}
