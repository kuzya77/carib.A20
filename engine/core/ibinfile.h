#ifndef __BLITZ_INTERNAL_BINFILE_H__
#define __BLITZ_INTERNAL_BINFILE_H__

#include "types.h"
#include "mem.h"

#include "binfile.h"
#include "stuff.h"

#include <string>
#include <list>
#include <vector>

#include "istl.h"

#include <stdexcept>

#ifdef getc
  #undef getc
#endif
#ifdef putc
  #undef putc
#endif

class std_binfile_error: public std::runtime_error
{
public:
    std_binfile_error(const char* msg):runtime_error(msg) {}
};

#include "unzip.h"  

class IndexEntry
{
public:
//  char name[512];
	u32_t  crc;    // crc-name, must be first
        unz_file_pos pos;
        bool operator <  (const IndexEntry& ie) const { return crc<ie.crc; }
};

typedef std::vector<IndexEntry> zipIndex;

class BinFile;

// Информация о пути доступа.
// Содержит имя каталога поиска (строка) и (если это архив)
// состояние открытого архива
class bfPathInfo
{
public:
    std::string name;            // имя архива или каталога
    std::string password;       
    unzFile uf;             // ID открытого архива, для NIOSа - файл архива
    zipIndex*   zip_index;  // Индекс для ускорения поиска файла (если есть)
    bfPathInfo():zip_index(NULL) { uf=NULL; }
    ~bfPathInfo();
};


class bfSystem:public baseSystem<BinFile>
{
public:
    std::list<bfPathInfo> paths;           // Ссылка на следующий путь
    bfSystem()                       {}
    bool RegisterFile(BinFile* b);
};

extern bfSystem bfSys;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BinFile
{   
public:
    bfSystem::tListNode node;   // инфа узла файлов
    enum 
    { 
        can_read  =0x01, 
        can_write =0x02, 
        can_seek  =0x04, 
        can_chsize=0x08,
        can_resize=can_chsize,
        can_rw    =can_read|can_write,
        can_all   =can_rw|can_resize|can_seek
    };
 
    virtual ~BinFile();
    

// abstract functions
    virtual long read(void *buf, long len);
    virtual long write(const void *buf, long len);
    virtual bool reset();
    virtual long seek(long pos);
    virtual long resize(long pos);

// basic types io
    virtual int getc();
    virtual int gets();

    virtual BinFile &putc(char c);
    virtual BinFile &puts(s16_t s);

    BinFile& operator[](long pos)       { seek(pos); return *this; }

    int      getmode()      const       { return mode; }
    long     size()         const       { return filelen; }
    long     length()       const       { return filelen; }
    long     size(long pos)             { return resize(pos); }
    long     seekcur(long pos)          { return seek(filepos+pos); } 
    long     seekend(long pos)          { return seek(filelen+pos); }
    long     tell()         const       { return filepos;           }       
    long     getpos()       const       { return filepos;           }
    bool     eof()          const       { return filepos==filelen;  }
    bool     canread()      const       { return !!(mode&can_read); }
    bool     canwrite()     const       { return !!(mode&can_write);}
    bool     canseek()      const       { return !!(mode&can_seek); }
    bool     canresize()    const       { return !!(mode&can_resize);}
protected:  
    int mode;
    long filepos,
         filelen;

    u8_t* buffer;
             
    BinFile():node(this),mode(0),filepos(0),filelen(0),buffer(NULL) {}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class stdBinFile: public BinFile
{
protected:
    FILE* f;
    std::string fname;
public:
    stdBinFile(const char *name,int type);
    virtual ~stdBinFile();

    virtual long read(void *buf, long len);
    virtual long write(const void *buf, long len);
    virtual long seek(long pos);
    virtual long resize(long pos);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class memBinFile : public BinFile
{
protected:
    bool owner;
public:
    memBinFile(void* ptr,size_t size,bool ro=false);
    virtual ~memBinFile();
    
    void* GetBuffer()  { return buffer; }
       
    virtual long read(void *buf,long len);
    virtual long write(const void *buf, long len);

// basic types io
    virtual int getc();
    virtual BinFile &putc(char c);
    virtual BinFile &puts(s16_t s);
    virtual BinFile &putl(long l);
};

class aBinFileRO : public BinFile
{
protected:
    BinFile* f;
    long fofs;
public:
    aBinFileRO(BinFile* bf,long pos,long size):f(bf),fofs(pos) { filelen=size; mode=can_read|can_seek; }
    virtual long read(void *buf, long len);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MakeDir(const char* _dir);

#endif
