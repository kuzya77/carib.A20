#include <types.h>
#include <mem.h>
#include <log.h>

#include "ibinfile.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stuff.h"

static long FdGetFileSize(int fd)
{
    struct stat stat_buf;
    int rc = fstat(fd, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

stdBinFile::stdBinFile(const char *name,int type):f(NULL),fname(name)
{ 
    int fmode=0,i;  
    const char PATH_DELIMITER='/';
    for(i=0;name[i];i++)
        if(fname[i]=='\\')
            fname[i]=PATH_DELIMITER;    
    switch(type)
    {
        case OPEN_RO:
            fmode=can_read|can_seek;
            f=fopen(fname.c_str(),"rb");
            break;
        case OPEN_RW:
            f=fopen(fname.c_str(),"r+b");
            if(f)   // Файл открыт?
            {
                fmode=can_read|can_seek|can_write|can_chsize;
                break;
            }
            // нет - пробуем создать
        case OPEN_CR:
            // check path
            {
                char* ptr=(char*)strrchr(fname.c_str(),PATH_DELIMITER);
                if(ptr)
                {
                    *ptr=0;
                    if(!MakeDir(fname.c_str()))
                    { 
                        logDebug("Can't create dir \"%s\"",fname.c_str());
                        *ptr=PATH_DELIMITER;
                        break;
                    }
                    *ptr=PATH_DELIMITER;
                }
            }
            fmode=can_read|can_seek|can_write|can_chsize;
            f=fopen(fname.c_str(),"w+b");
            break;
        default:
            logError("Invalid mode for stdBinFile::open");
            break;
    }
    if(f)
    {       
        // Вычисление длины файла
        fseek(f,0,SEEK_END);
        filelen=ftell(f);
        rewind(f);
        mode=fmode;
    }
    else
        throw std_binfile_error(fname.c_str());
}

stdBinFile::~stdBinFile()
{
    NULL_DESTROY(fclose,f);
}

long stdBinFile::read(void *buf,long len)
{
    if(!canread())
            return BinFile::read(buf,len);
    if((len=fread(buf,1,len,f))==-1)
        return -1;
    filepos+=len;
    return len;
}

long stdBinFile::write(const void *buf,long len)
{
    if(!canread())
        return BinFile::write(buf,len);
    if(!canresize())
        if(len>(filelen-filepos))
            len=filelen-filepos;
    if((len=fwrite(buf,1,len,f))!=-1)
    {
        if((filepos+=len)>filelen)
            filelen=filepos;
    }
    return len;
}

long stdBinFile::seek(long pos)
{
    if(pos==filepos)
            return filepos;
    BinFile::seek(pos);
    if(fseek(f,filepos,SEEK_SET)==-1)
        return -1;
    return filepos;
}

long stdBinFile::resize(long pos)
{
    if(!canresize())
        return BinFile::resize(pos);
    if(ftruncate(fileno(f),pos)!=0)
        return -1;
    filepos=ftell(f);
    return filelen=FdGetFileSize(fileno(f));
}
