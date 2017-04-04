#include "types.h"
#include "mem.h"
#include "log.h"

#include "ibinfile.h"
#include "istuff.h"

bool bfSystem::RegisterFile(BinFile* f)
{
    return Register(f);
}

bfPathInfo::~bfPathInfo()
{
    if(uf)
    {
        unzClose(uf);
        delete zip_index;
    }
}

bfSystem bfSys;

bool bfInit(int MaxFileCount,int MaxPathsCount)
{
    logMessage("bfInit(MaxFileCount=%d,MaxPathsCount=%d)", MaxFileCount, MaxPathsCount);
    if(bfSys.IsInit)   // инициализация уже проведена, требуется bfDone;
        bfDone();
    bfSys.IsInit=true;
    return true;
}

void bfDone()   // Удаление всех открытых файлов
{       
    logMessage("bfDone (FileCount=%d)",bfSys.Count);
// Убираем пути и закрываем файлы
    bfSys.paths.clear();

    while(bfSys.first())
        bfClose(bfSys.first()->owner);  
    bfSys.IsInit=false;
}

void CreateZipIndex(bfPathInfo& pi)
{
    if(!pi.uf)     // Это zip-файл?
        return;
    logDebug("BinFile::CreateZipIndex begin");
    // Заказываем память под записи
    unz_global_info pglobal_info;
    if(unzGetGlobalInfo(pi.uf,&pglobal_info)!=UNZ_OK)
        return;
    if(unzGoToFirstFile(pi.uf)!=UNZ_OK)
        return;
    int entryCount=(int)pglobal_info.number_entry;    // Получили кол-во файлов

    pi.zip_index=NULL;
    
    try
    {
        pi.zip_index=new zipIndex(entryCount);
        zipIndex& zi=*pi.zip_index;
        // Читаем записи каталога
        for(int i=0;i<entryCount;i++)
        {
            char name[512];
            if(unzGetCurrentFileInfo(pi.uf,NULL,name,512,NULL,0,NULL,0)!=UNZ_OK)
                logError("BinFile::CreateZipIndex: ERROR at #%d Entry",i);
            zi[i].crc=FileNameCrc(name);
            unzGetFilePos(pi.uf,&zi[i].pos);
/*        logDebug("CreateZipIndex: Entry# %d(%d; offs=%d) name='%s'(%08X)",i,zi->file[i].pos.num_of_file,
                                                                     zi->file[i].pos.pos_in_zip_directory,
                                                                     name,zi->file[i].crc);
*/
            unzGoToNextFile(pi.uf);
        }
        // Сортируем...
        sort(zi.begin(),zi.end());
        logDebug("BinFile::CreateZipIndex end");
    }
    catch(std::bad_alloc& ba)
    {
        logDebug("BinFile::CreateZipIndex: error: %s",ba.what());
        delete pi.zip_index;
    }
}

//---------------------------------------------------------------------------
// name!=NULL
bool LocateInZip(bfPathInfo& pi,const char* name)
{
    if(pi.uf)
    {                       
        if(pi.zip_index) // Поиск в таблице
        {
            zipIndex& zi=*pi.zip_index;
            IndexEntry value;
            value.crc=FileNameCrc(name);

            pair<IndexEntry*,IndexEntry*> pr=equal_range(zi.begin(),zi.end(),value);            
            if(pr.first!=pr.second)
            {
                if(distance(pr.first,pr.second)>1)
                    logWarning("LocateInZip: undetermined state for file \"%s\"(crc=%08X), count=%d",
                                        name,value.crc,distance(pr.first,pr.second));
                unzGoToFilePos(pi.uf,&pr.first->pos);
                return true;
            }
        }
        return unzLocateFile(pi.uf,name,0)==UNZ_OK;
    }
    return false;
}               

class memZipBinStream
{
public:
    static voidpf ZCALLBACK fopen_func(voidpf opaque, const char* filename, int mode);
    static uLong  ZCALLBACK fread_func(voidpf opaque, voidpf stream, void* buf, uLong size);
    static uLong  ZCALLBACK fwrite_func(voidpf opaque, voidpf stream, const void* buf, uLong size);
    static long   ZCALLBACK ftell_func(voidpf opaque, voidpf stream);
    static long   ZCALLBACK fseek_func(voidpf opaque, voidpf stream, uLong offset, int origin);
    static int    ZCALLBACK fclose_func(voidpf opaque, voidpf stream);
    static int    ZCALLBACK ftesterror_func(voidpf opaque, voidpf stream);
};

voidpf ZCALLBACK memZipBinStream::fopen_func(voidpf opaque, const char* filename, int mode)
{
    return opaque;
}

uLong  ZCALLBACK memZipBinStream::fread_func(voidpf opaque, voidpf stream, void* buf, uLong size)
{
    return bfRead((BINFILE)stream,buf,size);
}

uLong  ZCALLBACK memZipBinStream::fwrite_func(voidpf opaque, voidpf stream, const void* buf, uLong size)
{
    return 0;//bfWrite(stream,buf,size);
}

long ZCALLBACK memZipBinStream::ftell_func(voidpf opaque, voidpf stream)
{
    return bfTell((BINFILE)stream);
}

long ZCALLBACK memZipBinStream::fseek_func(voidpf opaque, voidpf stream, uLong offset, int origin)
{
    switch(origin)
    {
        case SEEK_SET: bfSeek((BINFILE)stream,offset);      break;
        case SEEK_CUR: bfSeekCur((BINFILE)stream,offset);   break;
        case SEEK_END: bfSeekEnd((BINFILE)stream,offset);   break;
    }
    return 0;
}

int ZCALLBACK memZipBinStream::fclose_func(voidpf opaque, voidpf stream)
{
    return 0;
}

int ZCALLBACK memZipBinStream::ftesterror_func(voidpf opaque, voidpf stream)
{
    return 0;
}

// Добавляет указанный путь в пути поиска файла, добавляется в конец списка путей.
// BINFILE - заполнение структуры на хандлер файла, перенаправление потока открытия для указанного архива
bool bfAddZip(BINFILE zipFile,const char* password)
{
    logDebug("BinFile::bfAddZip(zipFile=%p,password=%s)",zipFile,password);
    if(!zipFile)
        return false;
    // Описываем доступ к файлу
    zlib_filefunc_def ZipBin={ 
                               memZipBinStream::fopen_func,
                               memZipBinStream::fread_func,
                               memZipBinStream::fwrite_func,
                               memZipBinStream::ftell_func,
                               memZipBinStream::fseek_func,
                               memZipBinStream::fclose_func,
                               memZipBinStream::ftesterror_func,
                               (voidpf)zipFile
                             };
    
    // Открываем архив...
    unzFile uf=unzOpen2("BINFILE", &ZipBin);     // если это не zip, вернут NULL
    if(!uf)
        return false;
    // Создаем фрагмент описания пути
    try
    {
        bfSys.paths.push_front();
        bfPathInfo& pi=bfSys.paths.front();
        pi.uf=uf;
        if(password)
            pi.password=password;
        CreateZipIndex(pi);
        return true;
    }
    catch(std::bad_alloc& ba)
    {
        logError("BinFile::bfAddZip failed: %s",ba.what());
    }
    catch(...)
    {
        logError("BinFile::bfAddZip failed: Facked shit (...)");
    }
    unzClose(uf);
    return false;
}

BinFile* bfOpenSimple(const char* name,int mode)
{
    BinFile* bf=NULL;
    try
    {
        bf=new stdBinFile(name,mode);    // Открытие нового файла
        bfSys.RegisterFile(bf);
        return bf;
    }
    catch(std::bad_alloc&)
    {
        delete bf;
        throw;
    }
    catch(std_binfile_error& /*stde*/)
    {
//        logDebug("bfOpenSimple excp: std_open error: %s",stde.what());
    }
    delete bf;
    return NULL;
}


/*bool bfAddPath(const char* name);     // Добавляет указанный путь в пути поиска файла, добавляется в конец списка путей.
                                        // механизм применения: если файл открывается в режиме на запись, пути игнорируются
                                        // Перебор начинается с первого каталога, простым добавлением пути к имени файла.
                                        // возможно применение .zip архивов в качестве пути.
                                        // отказ возвращается если путь не может быть добавлен (заполнена вся таблица), 
                                        // либо (в случае .zip) неправильный или отсутствующий архив*/

bool bfAddPath(const char* name,const char* password)
{
        logMessage("BinFile::bfAddPath(name=%s;pass=%s)",name,password);
        if(!bfSys.IsInit)
            bfInit();
        // Создаем фрагмент описания
        zipIndex* zi=NULL;
        try
        {
            bfSys.paths.push_front();
            bfPathInfo& pi=bfSys.paths.front();
            pi.name=name;
            if(password)
                pi.password=password;

            // Проверяем, что нам передали: файл zip или каталог
            pi.uf=unzOpen(name);       // если это не zip, вернут NULL
            CreateZipIndex(pi); 
            return true;
        }
        catch(std::bad_alloc& ba)
        {
            logError("BinFile::bfAddPath failed: %s",ba.what());
        }
        catch(...)
        {
            logError("BinFile::bfAddPath failed: Facked shit (...)");
        }     
        delete zi;
        return true;
}

BINFILE bfOpen(const char* name,int mode)
{
// Проверка корректности аргументов
    if(!name)
        return NULL;
/////////////////////////////////////
    if(!bfSys.IsInit)
        bfInit();
    try
    {
    // Попытка прямого открытия
    BinFile* bf=bfOpenSimple(name,mode);
    if(!bf && mode==OPEN_RO)        // Попытка не удалась, пробуем искать в архивах или каталогах
    {
        for(list<bfPathInfo>::iterator pi=bfSys.paths.begin();pi!=bfSys.paths.end();pi++)
            if(pi->uf)      // Ищем в zip'е
            {
                if(LocateInZip(*pi,name))    // файл найден
                {
                    int err=UNZ_OK;
                    unz_file_info file_info;
                    char filename_inzip[256];
                    err=unzGetCurrentFileInfo(pi->uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
                    if(err!=UNZ_OK)
                    {
                        logError("BinFile::bfOpen error %d with zipfile in unzGetCurrentFileInfo",err);
                        continue;
                    }                    
                    // Создаем файл в памяти соответствующего(распакованного) размера
                    // Доступ - только чтение
                    memBinFile* mf=new memBinFile(NULL,file_info.uncompressed_size);
                    // Попытка открыть...
                    err=unzOpenCurrentFilePassword(pi->uf,pi->password.empty()?NULL:pi->password.c_str());
                    if(err!=UNZ_OK)
                    {
                        logError("BinFile::bfOpen error %d with zipfile in unzOpenCurrentFilePassword",err);
                        unzCloseCurrentFile(pi->uf);
                        delete mf;
                        continue;
                    }
                    unzReadCurrentFile(pi->uf,mf->GetBuffer(),file_info.uncompressed_size);
                    unzCloseCurrentFile(pi->uf);
                    bfSys.RegisterFile(bf=mf);
                    break;
                }
            }
            else
            {   // Добавляем путь
                bf=bfOpenSimple(string(pi->name).append("/").append(name).c_str(),mode);
                if(bf)
                    break;
            }
    }
    return bf;
    }
    catch(std::bad_alloc& ba)
    {
        logError("BinFile::bfOpen failed: %s",ba.what());
    }

    catch(...)
    {
        logError("BinFile::bfOpen failed: Facked shit (...)");
    }
    return NULL;
}

BINFILE bfOpenMem(void* ptr,int mode,int size)
{
    if(!bfSys.IsInit)
        bfInit();
    if(mode==OPEN_RO && ptr==NULL)
        return NULL;
    BinFile* bf=NULL;
    try
    {
        bf=new memBinFile(ptr,size,mode==OPEN_RO);
        bfSys.RegisterFile(bf);
        return bf;
    }
    catch(std::bad_alloc& ba)
    {
        logError("BinFile::bfOpenMem failed: %s",ba.what());
    }

    catch(...)
    {
        logError("BinFile::bfOpenMem failed: Facked shit (...)");
    }
    delete bf;
    return NULL;
}

void bfClose(BINFILE bf)        // закрытие файла
{
    if(!bf)
        return;
    if(bfSys.IsRegistered(bf))
    {
        delete bf;
        bfSys.Count--;
    }
    else
        logError("bfClose error: unregistered handler %08X",bf);
}

size_t  bfRead(BINFILE bf,void* buf,size_t len)         { return bf?bf->read(buf,len):0; }
size_t  bfWrite(BINFILE bf,const void* buf,size_t len)  { return bf?bf->write(buf,len):0; }

// basic types io
int bfGetc(BINFILE bf)      // byte     return EOF if eof
{
    return bf->getc();
}

int bfGetw(BINFILE bf)      // s16_t    return EOF if eof
{
    return bf->gets();
}

s32_t bfGetl(BINFILE bf)
{
    if(bf)
    {
        s32_t lv;
        if(bfRead(bf,&lv,4)==4)
            return lv;
    }
    return -1;
}

bool bfPutc(BINFILE bf,char c)          { return bf?bf->write(&c,sizeof(c))==sizeof(c):false; }
bool bfPutw(BINFILE bf,s16_t c)         { return bf?bf->write(&c,sizeof(c))==sizeof(c):false; }
bool bfPutl(BINFILE bf,s32_t c)         { return bf?bf->write(&c,sizeof(c))==sizeof(c):false; }

void    bfRewind(BINFILE bf)                { if(bf) bf->seek(0); }
size_t  bfSeek(BINFILE bf,size_t pos)       { return bf?bf->seek(pos):0; }
size_t  bfSeekEnd(BINFILE bf,size_t pos)    { return bf?bf->seekend(pos):0; }
size_t  bfSeekCur(BINFILE bf,size_t pos)    { return bf?bf->seekcur(pos):0; }
size_t  bfTell(BINFILE bf)                  { return bf?bf->tell():0; }
size_t  bfSize(BINFILE bf)                  { return bf?bf->size():0; }
size_t  bfResize(BINFILE bf,size_t size)    { return bf?bf->resize(size):0; }
bool    bfEof(BINFILE bf)                   { return bf?bf->eof():true; }

bool bfFileExist(const char* fname)
{
    BinFile* f=bfOpen(fname,OPEN_RO);
    if(f)
    {
        bfClose(f);
        return true;
    }
    return false;
}

char* bfGets(BINFILE file, char* s, size_t size)
{
	if(!file || !s || bfEof(file) || !size)
    	return NULL;

	for(size_t i=0; i<size; i++)
	{
		int c = bfGetc(file);
		if(c==EOF || c=='\n')
		{
			s[i]='\0';
			return s;
		}
		if(c=='\r')
			c = ' ';
		s[i] = c;
  	}

  	s[size-1] = '\0';
	return s;
}
