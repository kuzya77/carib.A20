#include "ipicture.h"
#include "igf_scr.h"
#include "istuff.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

gfxSystem gfxSys;

// Данные текущего видеорежима
int gfxWidth()
{
    return gfxSys.CurrentScreen?gfxSys.CurrentScreen->width():0;
}

int gfxHeight()
{
    return gfxSys.CurrentScreen?gfxSys.CurrentScreen->height():0;
}

int gfxBpp()
{
    return gfxSys.CurrentScreen?gfxSys.CurrentScreen->bpp():0;
}

///////////////////////////////////////////////////////////////////////////////
// отсечение
const cRect& gfxViewPort()
{
    assert(gfxSys.CurrentScreen);
    return gfxSys.CurrentScreen->GetViewPort();
}

void gfxViewPort(const cRect& w)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->SetViewPort(w);
}

fScreen32bitFB::fScreen32bitFB(int fd, int w, int h, u32_t* ptr, struct fb_var_screeninfo& varFB)
	:fScreen32bit(w,h,ptr)
{
	flatBufferFD = fd;
	varFBInfo = varFB;
	scrPtr = ptr;

	varFBInfo.yoffset = h;
	ioctl(flatBufferFD, FBIOPAN_DISPLAY, &varFBInfo);
}

fScreen32bitFB::~fScreen32bitFB()
{
	munmap(scrPtr, mWidth*mHeight*2*4);		// 2 pages 4bytes per pixel
	close(flatBufferFD);	
}

void fScreen32bitFB::flip()
{
	if(mData==scrPtr)	// #0
	{
		varFBInfo.yoffset = 0;
		mData = scrPtr + mWidth*mHeight;
	}
	else
	{
		varFBInfo.yoffset = mHeight;
		mData = scrPtr;
	}
	ioctl(flatBufferFD, FBIOPAN_DISPLAY, &varFBInfo);
}

fScreen32bitFB* fScreen32bitFB::open(const char* deviceName)
{

	struct fb_fix_screeninfo fixFBInfo;
	struct fb_var_screeninfo varFBInfo;

	int fd = ::open(deviceName, O_RDWR);
	if(fd<0)
	{
		logError("fScreen32bitFB::open Can't open file '%s'(%d)", deviceName, errno);
		return NULL;
	}	

	// get constant part
	if(ioctl(fd, FBIOGET_FSCREENINFO, &fixFBInfo)<0)
	{
		logError("fScreen32bitFB::open Can't get fixed screen info for '%s'\nError: %s\n"
			, deviceName, strerror(errno));
		close(fd);
		return NULL;
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, &varFBInfo)<0)
	{
		logError("fScreen32bitFB::open Can't get variable screen info for '%s'\nError: %s\n"
			, deviceName, strerror(errno));
		close(fd);
		return NULL;
	}
	
	int w = varFBInfo.xres_virtual;
	int h = varFBInfo.yres_virtual;

	// memory map of the screen
	int scrSize = w*h*varFBInfo.bits_per_pixel/8;
	unsigned int* scrPtr = reinterpret_cast<unsigned int*>(mmap(0, scrSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
	if((int)scrPtr == -1) 
	{
		logError("fScreen32bitFB::open Error: failed to map framebuffer device to memory");
		close(fd);
		return NULL;
	}

	return new fScreen32bitFB(fd, w, h/2, scrPtr, varFBInfo);
}


///////////////////////////////////////////////////////////////////////////////
// вывод примитивов
color_t gfxGetPixel(int x,int y)
{
    return gfxSys.CurrentScreen?gfxSys.CurrentScreen->GetPixel(x,y):0;
}

void gfxPutPixel(int x,int y,color_t color)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->PutPixel(x,y,color);
}

void gfxLine(int x0,int y0,int x1,int y1,color_t color,const cRect* rect)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->line(x0,y0,x1,y1,color,rect);
}

void gfxBar(int x0,int y0,int x1,int y1,color_t color,const cRect* rect)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->bar(x0,y0,x1,y1,color,rect);  
}

void gfxRectangle(int x0,int y0,int x1,int y1,color_t color,const cRect* rect)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->rectangle(x0,y0,x1,y1,color,rect);    
}

void gfxCircle(int xc,int yc,int r,color_t color,const cRect* rect)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->circle(xc,yc,r,color,rect);
}

void gfxEllipse(int xc,int yc,int a,int b,color_t color,const cRect* rect)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->ellipse(xc,yc,a,b,color,rect);
}

void gfxTriangle(int x1,int y1,int x2,int y2,int x3,int y3,color_t color,const cRect* rect)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->triangle(x1,y1,x2,y2,x3,y3,color,rect);
}

void gfxPolygone(int count,const tPoint* p,color_t color,const cRect* rect)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->polygone(count,p,color,rect);
}

///////////////////////////////////////////////////////////////////////////////
// Преобразования и проч.
color_t  gfxRGB(int red,int green,int blue)
{
    return gfxSys.CurrentScreen?gfxSys.CurrentScreen->rgb(red,green,blue):0;
}

COLOR32 gfxRGB(color_t color)
{
    if(!gfxSys.CurrentScreen)
    {
        COLOR32 c32;
        c32.all=0;
        return c32;
    }
    return gfxSys.CurrentScreen->rgb(color);
}


void gfxCls(color_t fill)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->cls(fill);
}

// Работа с изображением
void gfxPutImage(int x, int y, IMAGE image, const cRect* rect) // Вывод без учета alpha
{
    if(gfxSys.CurrentScreen && image)
        gfxSys.CurrentScreen->PutImage(x, y, image, rect);
}

void gfxGetImage(int x,int y,IMAGE image)
{
    if(gfxSys.CurrentScreen && image)
        gfxSys.CurrentScreen->GetImage(x,y,image);
}

bool gfxInit(int width, int height, int bpp, int)
{
	if(gfxSys.sdl_screen)	// already done
	{
		logMessage("gfxInit already passed");
		return false;
	}

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0)
	{
		logError("Can't initialize SDL: %s\n", SDL_GetError());
		return false;
	}
	
	gfxSys.sdl_screen = SDL_SetVideoMode(width, height, bpp, SDL_DOUBLEBUF|(bpp==BPP_8bit?SDL_HWPALETTE:0));
	if(!gfxSys.sdl_screen)
	{
		logError("gfxInit: couldn't set mode %dx%d %d bit", width, height, bpp);
		return false;
	}
	// create and point the screen:
	switch(bpp)
    {
    	case BPP_8bit:  // палитровый режим...
        	gfxSys.MainBuffer=new fScreen8bit(width, height, reinterpret_cast<u8_t*>(gfxSys.sdl_screen->pixels), gfxSys.sdl_screen->w);
            break;
        case BPP_16bit: // фича...                              
        case BPP_15bit:
        	gfxSys.MainBuffer=new fScreen15bit(width, height, reinterpret_cast<u16_t*>(gfxSys.sdl_screen->pixels), gfxSys.sdl_screen->w*2);
        case BPP_32bit:
        	gfxSys.MainBuffer=new fScreen32bit(width, height, reinterpret_cast<u32_t*>(gfxSys.sdl_screen->pixels));
            break;
    }
	gfxSys.CurrentScreen = gfxSys.MainBuffer;
	return true;
}

/// Работа с текстом /////////////////////////////////////
void gfxOutText(int x,int y,const char* s,color_t color)
{
    if(gfxSys.CurrentScreen)
        gfxSys.CurrentScreen->outtext(x,y,s,color);
}

void gfxSet2ndScreen(bool mode)
{
	gfxSys.CurrentScreen = mode?gfxSys.screen2:gfxSys.MainBuffer;
}


void gfxDone()
{
	if(gfxSys.CurrentScreen)
	{	
		gfxSet2ndScreen(false);

		if(gfxSys.screen2)
		{
			delete gfxSys.screen2;
			gfxSys.screen2 = NULL;
		}

		if(gfxSys.CurrentScreen!=gfxSys.MainBuffer)
			delete gfxSys.MainBuffer;
		delete gfxSys.CurrentScreen;
		gfxSys.CurrentScreen = gfxSys.MainBuffer = NULL;
		gfxSys.sdl_screen = NULL;
		SDL_Quit();
		logMessage("gfxDone()");
	}
}

bool gfxOpen2nd(const char* second_scr_dev_file)
{
	if(gfxSys.screen2==NULL)
		gfxSys.screen2 = fScreen32bitFB::open(second_scr_dev_file);
	return gfxSys.screen2!=NULL;
}

void gfxFlip()
{
    if(gfxSys.CurrentScreen==gfxSys.MainBuffer && gfxSys.CurrentScreen)
    {
		SDL_Flip(gfxSys.sdl_screen);
		gfxSys.MainBuffer->SetPtr(gfxSys.sdl_screen->pixels);
    }
    else
    	if(gfxSys.CurrentScreen==gfxSys.screen2 && gfxSys.screen2)
    		gfxSys.screen2->flip();		
}

    
#if 0

#include <types.h>
#include <mem.h>


#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
// двойной буффер и направление вывода

#ifndef __NIOS_ALTERA__

bool gfxBackBuffer(bool use)
{
    if(!gfxSys.MainBuffer)  // к моменту вызова система графики уже инициализирована
        return false;
    if(use) // Создание и направление буфера
    {
        if(!gfxSys.BackBuffer)  // Экрана нет?
        {                       // создаем
            int w=gfxSys.MainBuffer->width(),
                h=gfxSys.MainBuffer->height();
            gfxSys.BackImage=imgCreate(w,h,gfxSys.MainBuffer->bpp(),0);
            gfxSys.BackBuffer=Screen::create(gfxSys.BackImage);
            if(!gfxSys.BackBuffer)  // создать не удалось
            {
                imgFree(gfxSys.BackImage);
                gfxSys.BackImage=NULL;
                return false;
            }
        }
        // Перенаправление текущего контекста вывода
        if(gfxSys.CurrentScreen!=gfxSys.BackBuffer)
        {
            if(gfxSys.CurrentScreen!=gfxSys.MainBuffer && gfxSys.CurrentScreen)
                delete gfxSys.CurrentScreen;    // контекст на текстуру, не нужен, удаляем
            gfxSys.CurrentScreen=gfxSys.BackBuffer;
        }
    }   
    else    // удаление бэкбуфера
        if(gfxSys.BackBuffer)
        {
            if(gfxSys.BackBuffer==gfxSys.CurrentScreen) // Возвращаем на место главный экран
                gfxSys.CurrentScreen=gfxSys.MainBuffer;
            delete gfxSys.BackBuffer;
            gfxSys.BackBuffer=NULL;
            imgFree(gfxSys.BackImage);
            gfxSys.BackImage=0;
        }
    return true;
}

#endif

bool gfxSetBuffer(IMAGE image)      // NULL - вывод на экран
{
    // удаление старого экрана
    if(gfxSys.CurrentScreen!=gfxSys.MainBuffer && gfxSys.CurrentScreen!=gfxSys.BackBuffer && gfxSys.CurrentScreen)
        delete gfxSys.CurrentScreen;
    if(image)   // перенаправление потока
    {      
        gfxSys.CurrentScreen=Screen::create(image);
        if(!gfxSys.CurrentScreen)
        {
            gfxSetBuffer(0);
            return false;
        }
    }
    else
        gfxSys.CurrentScreen=gfxBackBuffer()?gfxSys.BackBuffer:gfxSys.MainBuffer;
    return true;
}

#ifndef __NIOS_ALTERA__

void gfxPutImageA(int x,int y,IMAGE image,const cRect* rect)    // Вывод с alpha
{
    if(gfxSys.CurrentScreen && image)
        gfxSys.CurrentScreen->PutImageA(x, y, image, rect);
}

void gfxPutImageA(int x,int y,IMAGE image,u32_t alpha,const cRect* rect)   // Вывод с alpha
{
    if(gfxSys.CurrentScreen && image && alpha)
    {
        if(alpha==256)
            gfxSys.CurrentScreen->PutImageA(x, y, image, rect);
        else
            gfxSys.CurrentScreen->PutImageA(x, y, image, alpha, rect);
    }
}

#endif

#endif