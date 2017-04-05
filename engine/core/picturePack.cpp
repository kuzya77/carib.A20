#include <types.h>
#include <mem.h>
#include <log.h>

#include "ipicture.h"
#include "istuff.h"
#include <stuff.h>

const s32_t PikPakInfoTag=tag2long("pikp"); // PICture Package

struct pikpakHeader
{
    s32_t   tag;
    u32_t  imgCount;
    u32_t  imgSize;
    u32_t  palCount;   // palSize is palCount*1024
};

struct ImageRecord
{
    u32_t id;          // crc32 from filename
    u32_t offset;      // start image offset (from beginning of all loaded image data)
    u32_t palette_idx; // palette index. if absent, set to -1;
};

extern imgSystem* imgSys;
extern tPicture* imgCreate(u32_t id, int flags, int pf, tImage* image, int palsize, RGBQUAD* palette);

tImagePack::~tImagePack()
{
    for(size_t i=0; i<imglist.size(); i++)
        imgFree(imglist[i]);
    memFree(pictures);
}

tImagePack::tImagePack(pikpakHeader* ph, ImageRecord* irs, u8_t* image_data)
{
    RGBQUAD* palettes = (RGBQUAD*)(image_data+ph->imgSize);
    int count = ph->imgCount;

    imglist.reserve(count);
    pictures = image_data;

    for(int i=0; i<count; i++)
    {
        tPicture* p = imgCreate(irs[i].id, 
                                IMG_SHARED|IMG_PACK, BPP_8bit,
                                (tImage*)(image_data+irs[i].offset), 
                                256, &palettes[256*irs[i].palette_idx]);
        imgSys->imap.insert(std::pair<u32_t,tPicture*>(p->id,p));
        imglist.push_back(p);
    }
}

bool imgLoadPack(const char* name)
{   
    logMessage("imgLoadPack(%s)", name);
    if(!imgInit())
    {
        logMessage("imgInit failed");
    	return false;
    }


    u32_t id_crc=crc32(name,strlen(name));

    std::pair<imgSystem::pack_map::iterator,imgSystem::pack_map::iterator> pr=imgSys->pmap.equal_range(id_crc);
    if(pr.first!=pr.second)
    {
        logMessage("imgLoadPack(%s) already loaded. Skipped", name);
        return true;
    }

    BINFILE fpack=bfOpen(name, OPEN_RO);

    if(!fpack)
    {   
        logError("imgLoadPack(%s): can't open file", name);
        return false;
    }

    pikpakHeader ph;
    if(bfRead(fpack, &ph, sizeof(ph))!=sizeof(ph) || 
        ph.tag!=PikPakInfoTag || 
        ph.imgSize+sizeof(ph)+ph.palCount*1024+sizeof(ImageRecord)*ph.imgCount!=bfSize(fpack))
    {
        logError("imgLoadPack(%s): wrong data", name);
        bfClose(fpack);
        return false;
    }

    ImageRecord* irs = (ImageRecord*)memAlloc(sizeof(ImageRecord)*ph.imgCount);
    u8_t* images = (u8_t*)memAlloc(ph.imgSize+sizeof(RGBQUAD)*256*ph.palCount);
    
    if(irs==NULL || images==NULL)
    {
        logError("imgLoadPack(%s): no memory", name);
        memFree(irs);
        memFree(images);
        bfClose(fpack);
        return false;
    }

    bfRead(fpack, irs, sizeof(ImageRecord)*ph.imgCount);
    bfRead(fpack, images, ph.imgSize+sizeof(RGBQUAD)*256*ph.palCount);

    tImagePack* pack=new tImagePack(&ph, irs, images);
    memFree(irs);
    imgSys->pmap.insert(std::pair<u32_t,tImagePack*>(id_crc,pack));

    bfClose(fpack);
    // load pack
    // register
    return true;
}

void imgReleasePack(const char* name)
{
    logMessage("imgReleasePack(%s)", name);
    u32_t id_crc=crc32(name,strlen(name));
    std::pair<imgSystem::pack_map::iterator,imgSystem::pack_map::iterator> pr=imgSys->pmap.equal_range(id_crc);
    if(pr.first!=pr.second)
    {
        tImagePack* p=pr.first->second;
        imgSys->pmap.erase(pr.first);
        delete p;
    }
}
