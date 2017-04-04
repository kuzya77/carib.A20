#include "types.h"

#include "ibinfile.h"

memBinFile::memBinFile(void* ptr,size_t size,bool ro)
{
    buffer=(u8_t*)ptr;
    if(buffer)
        owner=false;
    else
    {
        owner=true;
        if(ro)
            return;
        if((buffer=(u8_t*)bfSys.alloc(size))==NULL)
            throw std::bad_alloc();
    }
    filelen=size;
    mode=(ro?can_read:can_rw)|can_seek;
}

memBinFile::~memBinFile()   
{ 
    if(mode && !owner)
        buffer=NULL;        
}
    
long memBinFile::read(void *buf, long len)
{
    if((filepos+len)>filelen && (len=filelen-filepos)<=0)
        return 0;
    memcpy(buf,buffer+filepos,len);
    filepos+=len;
    return len;
}

long memBinFile::write(const void *buf, long len)
{
    if(!canwrite())
        return BinFile::write(buf,len);
    if(len>(filelen-filepos))
    {
        if(canresize())
            resize(filepos+len);
        if(len>(filelen-filepos) && (len=filelen-filepos)<=0)
            return 0;
    }
    memcpy(buffer+filepos,buf,len);
    filepos+=len;
    return len;
}

int memBinFile::getc()
{
    return filepos<filelen?buffer[filepos++]:EOF;
}

// Методы getl, gets выброшены из-за проблем выравнивания в NIOS2 (если адрес некратный размеру слова, читаецца всяка хня

BinFile& memBinFile::putc(char c)
{
	if(canwrite())
	{
    	if(eof())
        	resize(filelen+1);
	    if(filepos<filelen)
    	    buffer[filepos++]=c;
	}
    return *this;
}

BinFile& memBinFile::puts(s16_t s)
{
    return putc(LOBYTE(s)).putc(HIBYTE(s));
}

BinFile& memBinFile::putl(long l)
{
	write(&l, sizeof(l));
	return *this;
}
