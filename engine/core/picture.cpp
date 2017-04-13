#include <types.h>
#include <mem.h>
#include <log.h>

#include "ipicture.h"
#include "istuff.h"
#include <stuff.h>

#include <assert.h>

imgSystem* imgSys=NULL;

imgSystem::imgSystem():RefCount(0)     
{
	FreeImage_Initialise();
	logMessage("imgSystem: FreeImage %s", FreeImage_GetVersion());
}

imgSystem::~imgSystem()
{
    logMessage("~imgSystem");
    logMessage("\tCount=%d",Count);
    logMessage("\t In imap %d items",imap.size());

    for(pack_map::iterator i=pmap.begin();i!=pmap.end();i++)
    	delete i->second;

    pmap.clear();

    while(first())
        imgFree(first()->owner);
    if(imap.size())
    {
        logMessage("\t In imap %d items after remove all",imap.size());
        for(image_map::iterator i=imap.begin();i!=imap.end();i++)
            logMessage("\tpict=%08X,key=%08X",i->second,i->first);
//            logMessage("\tpict=%08X,key=%08X,id=%08X,flags=%08X",i->second,i->first,i->second->id,i->second->flags);
    }
    IsInit=false;
    FreeImage_DeInitialise();
}

bool imgInit()
{
    if(imgSys)
        return true;
    try
    {
        imgSys=new imgSystem;
        imgSys->IsInit=true;
//        imgSys->imap.resize(10000);
        return true;
    }
    catch(std::bad_alloc& ba)
    {
        logError("imgInit error: %s",ba.what());
    }
    catch(...)
    {
        logError("imgInit error: ...");
    }
    delete imgSys;
    imgSys=NULL;
    return false;
}

void imgDone()
{
    logMessage("imgDone()");
    delete imgSys;
    imgSys=NULL;
}

size_t tImage::line_size(int w,int bpp) // вычисляет выровненную длину строки
{ 
    switch(bpp)
    {
        case BPP_8bit:  // STD_COLOR
            return (w+3)&0xFFFFFFFC;
        case BPP_15bit: // HI_COLOR
        case BPP_16bit:
            return (w*2+3)&0xFFFFFFFC;
        case BPP_24bit: // TRUE_COLOR
            return (w*3+3)&0xFFFFFFFC;
        case BPP_32bit: //TRUE32_COLOR
            return w*4;
    }
    return 0; 
}


size_t tImage::size(int w,int h,int bpp)
{ 
    size_t line_width=line_size(w,bpp);
    return line_width?sizeof(tImage)+line_width*h:0;
}


size_t tPicture::size(int w,int h,int bpp,int PalCount)
{
    size_t size=tImage::size(w,h,bpp);
    if(size)
        return size-sizeof(tImage)+sizeof(tPicture)+(bpp==BPP_8bit?PalCount*sizeof(RGBQUAD):0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//extern blitz_log_stream dlogout;

IMAGE imgLoad(const char* fname,int pf,int flags)
{
    if(!fname || !imgInit())  // no image - no load
        return NULL;
    tPicture* p=NULL;

    try
    {
        u32_t id_crc=FileNameCrc(fname);
        if(flags&IMG_SHARED)    // search image...
        {
            // Начинаем искать...
            for(auto pr=imgSys->imap.equal_range(id_crc);pr.first!=pr.second;pr.first++)
                if(pr.first->second->bpp==pf || pf==BPP_DEFAULT) // нашли, все ок, возвращаем расшаренную копию
                {
     //              debug("picture::imgLoad(fname=\""<<fname<<"("<<(void*)id_crc<<")) shared from "<<p->owner<<"\n");
                    pr.first->second->count++;
                    imgSys->RefCount++;
                    return pr.first->second;
                }
        }
        // картинки нет, загружаем...
        BINFILE f=bfOpen(fname,OPEN_RO);    // Открываем файл...
        if(!f)
            return 0;

        std::vector<byte> file_data(bfSize(f));
        bfRead(f, &file_data[0], bfSize(f));
        bfClose(f);

        // Do not cut off palette if we need convert picture
	    if(pf>BPP_8bit)
    		flags &=~IMG_NOPALETTE;

        FIMEMORY *hmem = FreeImage_OpenMemory(&file_data[0], file_data.size());
        if(!hmem)
        {
        	logError("imgLoad: couldn't create FIMEMORY obj to load \"%s\"(%d bytes)", fname, file_data.size());
        	return 0;
        }

        // get the file type
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
		if(fif==FIF_UNKNOWN)
		{
			logError("imgLoad: unknown picture type \"%s\"(%d bytes)", fname, file_data.size());
			FreeImage_CloseMemory(hmem);
			return 0;
		}
//		logMessage("imgLoad: detected type '%s' in \"%s\" (%d bytes)"
//				, FreeImage_GetFormatFromFIF(fif)
//				, fname, file_data.size());
		// load an image from the memory stream
		FIBITMAP *dib = FreeImage_LoadFromMemory(fif, hmem, 0);
		FreeImage_CloseMemory(hmem);

		if(!dib)
		{
			logMessage("imgLoad: can't load type '%s' in \"%s\" (%d bytes)"
				, FreeImage_GetFormatFromFIF(fif)
				, fname, file_data.size());
			return 0;
		}

		// check acceptable types:
		FREE_IMAGE_COLOR_TYPE color_type = FreeImage_GetColorType(dib);
		int w = FreeImage_GetWidth(dib);
		int h = FreeImage_GetHeight(dib);
		int bpp = FreeImage_GetBPP(dib);

		switch(color_type)
		{
			case FIC_PALETTE:	// Only 8 bits are supported!
				if(bpp==8)
				{					
					// create image
					int palSize = flags&IMG_NOPALETTE?0:FreeImage_GetColorsUsed(dib);
					p=imgCreate(w, h, BPP_8bit, palSize);
					if(p)
					{
						// copy palette
						if(palSize>0)
							memcpy(imgPalette(p), FreeImage_GetPalette(dib), palSize*sizeof(RGBQUAD));
						// copy image itself
						FreeImage_ConvertToRawBits((byte*)imgBuffer(p), dib, p->image->line_width, 8, 0, 0, 0, TRUE);
					}
					else
						logMessage("imgLoad: can't load type '%s' in \"%s\" (%d bytes): out of memory"
							, FreeImage_GetFormatFromFIF(fif)
							, fname, file_data.size());
				}
				else
				{
					logMessage("imgLoad: can't load type '%s' in \"%s\" (%d bytes): supported only 8bit palette"
						, FreeImage_GetFormatFromFIF(fif)
						, fname, file_data.size());
				}
				break;
			case FIC_RGB:
			case FIC_RGBALPHA:
				switch(bpp)
				{
					case 16:
						p=imgCreate(w, h, bpp, 0);
						if(p)
							FreeImage_ConvertToRawBits((byte*)imgBuffer(p), dib, p->image->line_width, bpp, 
								FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK, TRUE);
						break;
					case 24:
					case 32:
						p=imgCreate(w, h, bpp, 0);
						if(p)
							FreeImage_ConvertToRawBits((byte*)imgBuffer(p), dib, p->image->line_width, bpp, 
								FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
						break;
					default:
						logMessage("imgLoad: unsupported bpp(%d) '%s' in \"%s\" (%d bytes)"
							, bpp
							, FreeImage_GetFormatFromFIF(fif)
							, fname, file_data.size());					
				}
				break;
			default:
				logError("this colortype is not supported yet!");
		}
		
		FreeImage_Unload(dib);

        // go to convert
        if(p)
        {
            p->id=id_crc;
            if(pf!=BPP_DEFAULT && p->bpp!=pf)
            {
//                logDebug("picture::imgLoad(fname=\"%s\") imgConvert from %p",fname,p);
                imgConvert(&p, pf);
//                logDebug("\tto %p",p);
            }
            if(flags&IMG_SHARED)    // поместить рисунок в шару
            {
                p->flags=flags;
//                logDebug("picture::imgLoad insert imap pict %08X, flags=%08X",p,flags);
                imgSys->imap.insert(std::pair<u32_t,tPicture*>(id_crc,p));
            }
        }
        return p;
    }
    catch(std::bad_alloc& ba)
    {
        logError("imgLoad error: %s",ba.what());
    }
    catch(...)
    {
        logError("imgLoad error: ...");
    }
    imgFree(p);
    return NULL;
}

tPicture* imgCreate(u32_t id, int flags, int pf, tImage* image, int palsize, RGBQUAD* palette)
{
    tPicture* ptr=(tPicture*)memAlloc(sizeof(tPicture));
    if(ptr)
    {
        ptr->node.init(ptr);
        ptr->flags=flags;
        ptr->id=id;
        ptr->count=1;
        ptr->bpp=pf;                // Формат пикселя
        
        ptr->image=image;

        ptr->palSize=palsize;
        ptr->_palette=palette;
        ptr->palBegin=0;   // Размер и начало палитры
        imgSys->Register(ptr);
    }
    return ptr;
}

IMAGE imgCreate(int w,int h,int pf,int palsize) // Создание рисунка. palsize используется, если pf=BPP_8bit, 
{
    if(!imgInit())
        return NULL;
//  debug("picture::imgCreate(w="<<w<<",h="<<h<<",pf="<<pf<<",palsize="<<palsize<<") begin\n");
	if(w<=0 || h<=0)
		return NULL;

    size_t lsize=tImage::line_size(w,pf);
    size_t size=tImage::size(w,h,pf);
    switch(pf)          // Проверка формата пикселя и определение размера имиджа
    {
        case BPP_8bit:
            size+=palsize*sizeof(RGBQUAD);
            break;
        case BPP_15bit:
        case BPP_16bit:
        case BPP_24bit:
        case BPP_32bit:
            palsize=0;
            break;
        default:
            logError("picture::imgCreate error: bad bpp=%d",pf);
            return 0;
    }   
//  debug("picture::imgCreate size="<<size<<'\n');
	tImage* image=(tImage*)memAlloc(size);
	if(!image)
		return 0;

    image->width=w;
    image->height=h;
    image->line_width=lsize;        
    memset(image->data,0,lsize*h);  // Очистка рисунка

    return imgCreate(0, 0, pf, image, palsize, palsize?(RGBQUAD*)(image->data+lsize*h):NULL);
}

void imgFree(IMAGE p)
{
    if(!p || !imgSys)
        return;
    if(imgSys->IsRegistered(p))
    {        
        if(p->count>1)
        {
            p->count--;
            imgSys->RefCount--;
        }
        else
        {
            // Убираем рисунок из шары, если есть
            if(p->flags&IMG_SHARED)
            {
                for(std::pair<imgSystem::iterator,imgSystem::iterator> pr=imgSys->imap.equal_range(p->id);pr.first!=pr.second;pr.first++)
                {
//                    logMessage("\tNode p=%08X, key=%08X flags=%08X",pr.first->second,pr.first->first,pr.first->second->flags);
                    if(pr.first->second==p) // нашли, убираем
                    {
                        imgSys->imap.erase(pr.first);
//                        logMessage("\tremove from imap");
                        break;
                    }
                }
            }
            p->node.remove();
            if(!(p->flags&IMG_PACK))
            	memFree(p->image);
            imgSys->Count--;
            memFree(p);
        }
    }
    else
        logError("imgFree: bad handler %08X",p);
}

// функции конвертации строки пикселов, выровнено по двойным словам
// count - колво двойных (!) слов строки
/*static void ConvertLine15x16(u32_t* dest,const u32_t* src,int count)
{   // x5r5g5b -> 5r6g5b
    for(;count--;src++)
        *dest++=(*src&0x001F001F)|((*src&0x7FE07FE0)<<1); //    (src[i]&0x1F)|((src[i]&0x7FE0)<<1) должно было быть, но т.к. используются двойные слова
}
*/
IMAGE imgCopy(IMAGE p,int pf,int flags) // Копирование со сменой формата, по умолчанию формат не меняет
{
    if(!p || !imgInit()) 
        return NULL;
    // источник шара, получатель - шара, формат совпадает

    if((flags&IMG_SHARED) && (p->flags&IMG_SHARED))
    {
        if(pf==p->bpp || pf==BPP_DEFAULT)
        {
            p->count++;
            imgSys->RefCount++;
            return p;
        }
        // ищем такой же
        for(std::pair<imgSystem::iterator,imgSystem::iterator> pr=imgSys->imap.equal_range(p->id);pr.first!=pr.second;pr.first++)
            if(pr.first->second->bpp==pf) // нашли, все ок, возвращаем расшаренную копию
            {
 //              debug("picture::imgLoad(fname=\""<<fname<<"("<<(void*)id_crc<<")) shared from "<<p->owner<<"\n");
                pr.first->second->count++;
                imgSys->RefCount++;
                return pr.first->second;
            }
    }   
    // не так - копируем
    if(pf==BPP_DEFAULT)     // Копируем 
        pf=p->bpp;
    // Проверка корректности входных данных
    switch(pf)
    {       
        case BPP_DEFAULT:
            pf=p->bpp;
            break;
        case BPP_8bit:
            if(p->bpp!=BPP_8bit)    // Конвертация в палитровый режим - наф. не нужна
                return NULL;
            break;
        case BPP_15bit:
        case BPP_16bit:
        case BPP_24bit:
        case BPP_32bit:
            break;                                                                                                     
        default:
            return NULL;
    }
    
    int w=imgWidth(p),
    	h=imgHeight(p);
    tPicture* p2=imgCreate(w, h, pf, pf==BPP_8bit?p->palSize:0);
    if(!p2)
        return NULL;
//  p2->UnregisterAllocator();
    if(pf==p->bpp)  // Прямое копирование
    {
        memcpy(p2->image,p->image,p->image->size()); 
	    if(p2->palSize)
	    	memcpy(p2->GetPalette(), p->GetPalette(), p2->palSize*sizeof(RGBQUAD));
    }
    else    
        switch(pf)
        {
            case BPP_16bit:
                {
                    u16_t* p2data=(u16_t*)p2->image->data;
                    switch(p->bpp)
                    {
                        case BPP_8bit:
                            {   
                                const u8_t* data=p->image->data;
                                u16_t pal16[256];
                                if(p->palSize) // Палитра есть?
                                {
                                    const RGBQUAD* palette=p->GetPalette();
                                    for(int i=p->palSize;i--;)
                                        pal16[i+p->palBegin]=(u16_t(palette[i].rgbRed&0xF8)<<8)|(u16_t(palette[i].rgbGreen&0xFC)<<3)|(u16_t(palette[i].rgbBlue)>>3);
                                }
                                else    // оттенки серого
                                    for(int i=0;i<256;i++)
                                        pal16[i]=((i&0xF8)<<8)|((i&0xFC)<<3)|(i>>3);
                                for(;h--;data+=p->image->line_width)
                                {
                                    for(int x=0;x<w;x++)
                                        p2data[x]=pal16[data[x]];                           
                                    p2data=offset_of(p2data,p2->image->line_width);
                                }
                            }
                            break;
                        case BPP_15bit:
                            {   
                                const u16_t* data=(const u16_t*)p->image->data;
                                for(;h--;data=offset_of(data,p->image->line_width))
                                {
                                    for(int x=0;x<w;x++)
                                        p2data[x]=((data[x]&0xFFE0)<<1)|(data[x]&0x1F);
                                    p2data=offset_of(p2data,p2->image->line_width);
                                } 
                            }
                            break;
                        case BPP_24bit:
                            {
                                for(const u8_t* data=p->image->data;h--;data+=p->image->line_width)
                                {
                                    const u8_t* d=data;
                                    for(int x=0;x<w;d+=3)
                                        p2data[x++]=(u16_t(d[0])>>3)|(u16_t(d[1]&0xFC)<<3)|(u16_t(d[2]&0xF8)<<8);
                                    p2data=offset_of(p2data,p2->image->line_width);
                                }
                            }
                            break;
                        case BPP_32bit:
                            for(int y=0;y<h;y++)
                                for(int x=0;x<w;x++)
                                {
                                    COLOR32 c=imgRGB(p,imgGetPixel(p,x,y));
                                    imgPutPixel(p2,x,y,imgRGB(pf,c.rgb.rgbRed,c.rgb.rgbGreen,c.rgb.rgbBlue));
                                }
                            break;
                    }
                }
                break;
            case BPP_32bit: // конвертация в 32bit
                {
                    RGBQUAD* p2data=(RGBQUAD*)p2->image->data;
                    const u8_t* data=p->image->data;
                    switch(p->bpp)
                    {
                        case BPP_8bit:
                            if(p->palSize)  // Палитра есть?
                            {
                                for(const RGBQUAD* palette=p->GetPalette()-p->palBegin;h--;data+=p->image->line_width)
                                    for(int x=0;x<w;x++)
                                        *p2data++=palette[data[x]];
                            }
                            else
                            {
                                for(;h--;data+=p->image->line_width)
                                    for(int x=0;x<w;x++)
                                    {
                                        p2data->rgbBlue=
                                        p2data->rgbGreen=
                                        p2data->rgbRed=data[x]; // Закрашиваем серым
                                        p2data->rgbReserved=255;
                                        p2data++;
                                    }

                            }
                            break;
                        case BPP_15bit:
                            for(;h--;data+=p->image->line_width)
                            {
                                const u16_t* d16=(const u16_t*)data;
                                for(int x=0;x<w;x++)
                                {
                                    p2data->rgbBlue =(d16[x]<<3)&0xF8;
                                    p2data->rgbGreen=(d16[x]>>2)&0xF8;
                                    p2data->rgbRed  =(d16[x]>>7)&0xF8;
                                    p2data->rgbReserved=d16[x]&0x8000?255:0;
                                    p2data++;
                                }
                            }
                            break;
                        case BPP_16bit:
                            for(;h--;data+=p->image->line_width)
                            {
                                const u16_t* d16=(const u16_t*)data;
                                for(int x=0;x<w;x++)
                                {
                                    p2data->rgbBlue =(d16[x]<<3)&0xF8;
                                    p2data->rgbGreen=(d16[x]>>3)&0xFC;
                                    p2data->rgbRed  =(d16[x]>>8)&0xF8;
                                    p2data->rgbReserved=255;
                                    p2data++;
                                }
                            }
                            break;
                        case BPP_24bit:
                            for(;h--;data+=p->image->line_width)
                            {
                                const u8_t* d2=data;
                                for(int x=0;x<w;x++)
                                {
                                    *((u32_t*)p2data)=*((u32_t*)d2) & 0x00FFFFFF;
                                    p2data->rgbReserved=255;
                                    p2data++;
                                    d2+=3;
                                }
                            }
                            break;
                    // case BPP_32bit - pf==p->bpp уже просмотрен
                    }
                }
                break;          
            default:        // Работает крайне медленно, но и х. с ним
                for(int y=0;y<h;y++)
                    for(int x=0;x<w;x++)
                    {
                        COLOR32 c=imgRGB(p,imgGetPixel(p,x,y));
                        imgPutPixel(p2,x,y,imgRGB(pf,c.rgb.rgbRed,c.rgb.rgbGreen,c.rgb.rgbBlue));
                    }
        }
    // Корректировка данных
    p2->id=p->id;
    try
    {
        if(flags&IMG_SHARED)    // поместить рисунок в шару
        {
            p2->flags=flags;
            imgSys->imap.insert(std::pair<u32_t,tPicture*>(p2->id,p2));
        }

        p2->flags=flags;
        return p2;
    }
    catch(std::bad_alloc& ba)
    {
        logError("imgCopy error: %s",ba.what());
    }
    catch(...)
    {
        logError("imgCopy error: ...");
    }
    imgFree(p2);
    return NULL;
}

// информация о рисунке ////////////////////////////////////////////////////////////////////////////////////////////////
// размеры и координаты верхнего левого угла

int imgWidth(IMAGE pict)
{
    return pict->image->width;
}

int imgHeight(IMAGE pict)
{
    return pict->image->height;
}

int imgFormat(IMAGE pict)
{
    return pict?pict->bpp:0;
}

COLOR32 imgRGB(IMAGE p,color_t color)        // Преобразование цвет -> раскладка по RGB
{
    COLOR32 c32;
    c32.all=0;
    if(p)
        switch(imgFormat(p))
        {
            case BPP_8bit:  
                {
                    if(p->palSize)  // Изображение цветное
                    {
                        color-=p->palBegin;
                        if(color<(u32_t)p->palSize)
                            c32.rgb=p->GetPalette()[color];
                    }
                    else
                        c32.rgb.rgbRed=
                        c32.rgb.rgbGreen=
                        c32.rgb.rgbBlue=color;
                }
                break;
            case BPP_15bit:
                c32.rgb.rgbRed  =(color>>7)&0xF8;
                c32.rgb.rgbGreen=(color>>2)&0xF8;
                c32.rgb.rgbBlue =(color<<3)&0xF8;
                break;
            case BPP_16bit:
                c32.rgb.rgbRed  =(color>>8)&0xF8;
                c32.rgb.rgbGreen=(color>>3)&0xFC;
                c32.rgb.rgbBlue =(color<<3)&0xF8;
                break;
            case BPP_24bit:
                c32.all=color;
                break;
            case BPP_32bit:
                c32.all=color;
                break;
    }
    return c32;
}

COLOR32 imgRGB(int format,color_t color)     // То же, что и пред. , но без учета палитры для 8bit режима
{
    COLOR32 c32;
    c32.all=0;
    switch(format)
    {
        case BPP_8bit:  // преобразовывает  компоненты серого
            c32.rgb.rgbRed=
            c32.rgb.rgbGreen=
            c32.rgb.rgbBlue=color;
            break;
        case BPP_15bit:
            c32.rgb.rgbRed  =(color>>7)&0xF8;
            c32.rgb.rgbGreen=(color>>2)&0xF8;
            c32.rgb.rgbBlue =(color<<3)&0xF8;
            break;
        case BPP_16bit:
            c32.rgb.rgbRed  =(color>>8)&0xF8;
            c32.rgb.rgbGreen=(color>>3)&0xFC;
            c32.rgb.rgbBlue =(color<<3)&0xF8;
            break;
        case BPP_24bit:
            c32.all=color;
            break;
        case BPP_32bit:
            c32.all=color;
            break;
    }
    return c32;
}

color_t imgRGB(IMAGE img,int red,int green,int blue)        // Преобразование цвет <- раскладка по RGB
{
    return img?imgRGB(imgFormat(img),red,green,blue):0;
}

color_t imgRGB(int format,int red,int green,int blue)       // То же, что и пред. , но без учета палитры для 8bit режима
{
    switch(format)
    {
        case BPP_8bit:
            break;
        case BPP_15bit:
            return ((red&0xF8)<<7)|((green&0xF8)<<2)|((blue&0xF8)>>3);
        case BPP_16bit:
            return ((red&0xF8)<<8)|((green&0xFC)<<3)|((blue&0xF8)>>3);
        case BPP_24bit:
            return (red<<16)|(green<<8)|blue;
        case BPP_32bit:
            return (red<<16)|(green<<8)|blue;
    }
    return 0;
}

bool imgConvert(IMAGE* img,int pf)
{
    if(!img)
        return false;
    if(imgFormat(*img)==pf)
        return true;
    IMAGE p2=imgCopy(*img,pf,(*img)->flags);
    if(!p2)
        return false;
    imgFree(*img);
    *img=p2;
    return true;
}

// извлечение образа: imgBuffer() - извлечение указателя, imgLine() - ... указателя на строку
void*   imgBuffer(IMAGE pict)
{
    return pict->image->data;
}

void*   imgLine(IMAGE pict,int line)
{
    tImage* img=pict->image;
    return img->data+img->line_width*line;
}

bool imgPutPixel(IMAGE p,int x,int y,color_t color)
{
    if(p && x>=0 && x<imgWidth(p) && y>=0 && y<imgHeight(p))
    {
        void* line=imgLine(p,y);
        switch(p->bpp)
        {
            case BPP_8bit:
                ((u8_t*)line)[x]=color;
                break;
            case BPP_15bit:
            case BPP_16bit:
                ((u16_t*)line)[x]=color;
                break;
            case BPP_24bit:
                {
                    u8_t* lp=(u8_t*)line;
                    lp+=x*3;
                    lp[0]=color;
                    lp[1]=color>>8;
                    lp[2]=color>>16;
                }
                break;
            case BPP_32bit:
                ((u32_t*)line)[x]=color;       
                break;
            default:
                return false;
        }
        return true;
    }
    return false;
}

color_t  imgGetPixel(IMAGE p,int x,int y)
{
    if(p && x>=0 && x<p->image->width && y>=0 && y<p->image->height)
    {
        void* line=imgLine(p,y);
        switch(p->bpp)
        {
            case BPP_8bit:
                return ((u8_t*)line)[x];
            case BPP_15bit:
            case BPP_16bit:
                return ((u16_t*)line)[x];
            case BPP_24bit:
                return *((u32_t*) ( ((u8_t*)line)+x*3 ) ) & 0x00FFFFFF;
            case BPP_32bit:
                return ((u32_t*)line)[x];
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////
// Palette
///////////////////////////////////////////////////////////////////////////

// Работа с палитрой, поддерживаются только 8bit режимы
// Получение палитры

RGBQUAD* imgPalette(IMAGE p)                    // доступ к палитре имиджа (если есть), доступен для модификации.
{
    return p?p->GetPalette():NULL;
}

int imgPaletteBegin(IMAGE p)            // начальная позиция палитры
{
    return p && p->bpp==BPP_8bit?p->palBegin:0;
}

bool imgPaletteBegin(IMAGE p,int pbegin)    // начальная позиция палитры, установка
{
    if(p && p->bpp==BPP_8bit && pbegin>=0)
    {
        if(pbegin+p->palSize<257)
        {
            p->palBegin=pbegin;
            return true;
        }
    }
    return false;
}

int imgPaletteSize(IMAGE p)                 // размер палитры
{
    return p && p->bpp==BPP_8bit?p->palSize:0;
}
