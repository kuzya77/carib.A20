#ifndef __BLITZ_INTERNAL_PICTURE_H__
#define __BLITZ_INTERNAL_PICTURE_H__

#include <types.h>
#include <mem.h>

#include <picture.h>
#include <stuff.h>
#include "istl.h"

#include <unordered_map>
#include <vector>

class tPicture;
class tImagePack;

class imgSystem:public baseSystem<tPicture>
{
public:     
    typedef std::unordered_multimap<u32_t,tPicture*> image_map;
    typedef image_map::iterator             iterator;

    typedef std::unordered_multimap<u32_t,tImagePack*> pack_map;


    image_map imap; // Карта расшареных рисунков
	pack_map  pmap;

    int RefCount;
    imgSystem();
    ~imgSystem();
};

enum
{
    IMG_PACK=4		// Рисунок из пакета, не удалять
};

struct pikpakHeader;
struct ImageRecord;

class tImagePack
{
private:
	std::vector<IMAGE> imglist;
	u8_t*		  pictures;

public:
	tImagePack(pikpakHeader* ph, ImageRecord*, u8_t* image_data);
	~tImagePack();
};

///////////////////////////////////////////////////////////////////////////
// Image

struct tImage                   // Данные имиджа хранятся с учетом выравнивания
{
public:
    int   	width,height;
    int   	line_width;     // Реальная длина строки (width*sizeof(Pixel)+выравнивание до dword
    u8_t   	data[];

    static size_t size(int w,int h,int bpp);
    static size_t line_size(int w,int bpp); // вычисляет выровненную длину строки
    size_t size() const  		{ return sizeof(tImage)+line_width*height; }
};


class tPicture
{
public:
    imgSystem::tListNode node;  // инфа узла    

    int     flags;              // rc-flags: shared or not  
    u32_t  	id;                 // идентификатор ресурса, crc32 от имени
    int     count;              // кол-во копий данного рисунка

    int     bpp;                // Формат пикселя
    int     palSize,palBegin;   // Размер и начало палитры
    tImage* image;
        
    static size_t size(int w,int h,int bpp,int PalCount);

    static tPicture* create(int x0,int y0,int w,int h,int bpp,int PalCount=0);

    void init(int w,int h,int Bpp,int PalCount)
                            { bpp=Bpp; palSize=PalCount; image->width=w; image->height=h; }
    size_t size() const     { return size(image->width,image->height,bpp,palSize); }


    RGBQUAD* GetPalette()	{ return _palette; }
//private:

	RGBQUAD* _palette;
};

#endif
