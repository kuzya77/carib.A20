#ifndef __BLITZ_INTERNAL_SCREEN_H__
#define __BLITZ_INTERNAL_SCREEN_H__

#include <types.h>
#include <mem.h>

#include <gfx.h>
#include <assert.h>
#include <stdlib.h>

#include "istl.h"

#include <linux/fb.h>

class Screen;
class fScreen32bitFB;

class gfxSystem: public baseSystem<Screen>
{
public:
    Screen*     CurrentScreen;  // Текущий экран вывода
    Screen*     MainBuffer;     // Контекст окна (windows), vesa-интерфейс (DOS)
    fScreen32bitFB* screen2;

    SDL_Surface* sdl_screen;
    gfxSystem():CurrentScreen(NULL),MainBuffer(NULL),sdl_screen(NULL),screen2(NULL) {}
};

extern gfxSystem gfxSys;

/////////////////////////////////////////////////////////////////////////////
// Иерархия работы с экранами
// Примечание 2: 
//     функции с префиксом _ не проверяют коррекность области отсечения
//     и корректность работы вообще
//////// Экран - протокласс 

class Screen
{
public:
    gfxSystem::tListNode node;  // инфа узла файлов
protected:
    static const u8_t default_font8x16[];
    int mWidth,mHeight,mBpp;
    cRect mClipRect;
  
    virtual void CorrectCW();

    Screen(int w,int h,int bpp):node(this),mWidth(w),mHeight(h),mBpp(bpp),mClipRect(0,0,w-1,h-1) { }

    void vline(int x,int y,int h,color_t color,const cRect* rect);
    void hline(int x,int y,int w,color_t color,const cRect* rect);
    void triangle(const tPoint* p,color_t color,const cRect* rect);
public:
    virtual ~Screen();
//// Работа с характеристиками экрана //////////////////////
    inline int   width()         const      { return mWidth;  }                 // gfxWidth()
    inline int   height()        const      { return mHeight; }                 // gfxHeight()
    inline int   bpp()           const      { return mBpp;   }                  // gfxBpp()
    inline const cRect& GetViewPort(void) const     { return mClipRect; }               // gfxViewPort()
    inline void  SetViewPort(const cRect* w)        { mClipRect=*w; CorrectCW(); }
    inline void  SetViewPort(const cRect& w)        { mClipRect=w; CorrectCW(); }       // gfxViewPort(const cRect& w);

/// Графические примитивы /////////////////////////////////
    virtual void _PutPixel(int x,int y,color_t color)=0;
    virtual color_t _GetPixel(int x,int y)       =0;
    virtual void _vline(int x,int y,int h,color_t color);
    virtual void _hline(int x,int y,int w,color_t color);
    virtual void _line(int x0,int y0,int x1,int y1,color_t color);
    virtual void _bar(int left,int top,int right,int bottom,color_t color);

    void PutPixel(int x,int y,color_t color)     { if(mClipRect.in(x,y)) _PutPixel(x,y,color); } // gfxPutPixel(int x,int y,color_t color);
    color_t GetPixel(int x,int y)                { return mClipRect.in(x,y)?_GetPixel(x,y):0; }  // gfxGetPixel(int x,int y);

    void line(int x0,int y0,int x1,int y1,color_t color,const cRect* rect=NULL);                             // gfxLine(int x0,int y0,int x1,int y1,color_t color,const cRect* rect=NULL);
    void bar(int left,int top,int right,int bottom,color_t color,const cRect* rect=NULL);                    // gfxBar(int x0,int y0,int x1,int y1,color_t color,const cRect* rect=NULL);
    void rectangle(int xs,int ys,int xe,int ye,color_t color,const cRect* rect=NULL);                        // gfxRectangle(int x0,int y0,int x1,int y1,color_t color,const cRect* rect=NULL);
/// Работа с текстом /////////////////////////////////////
    virtual void _outchar(int x,int y, char ch, color_t color,const cRect* rect);
    void outtext(int x,int y,const char* s,color_t color,const cRect* rect=NULL);

    virtual void _PutImage(int x,int y,const tPicture* image,const cRect* rect);

    virtual void _GetImage(int x,int y,int w,int h,tPicture* image,const cRect* rect);
    virtual void _PutImageA(int x,int y,const tPicture* image,const cRect* rect);                   // Вывод с Alpha 8:24
    virtual void _PutImageA(int x,int y,const tPicture* image,u32_t alpha,const cRect* rect);      // Вывод с Alpha* 8:24

    inline void PutImage(int x,int y,const tPicture* image,const cRect* rect=NULL);
    inline void GetImage(int x,int y,tPicture* image);
    inline void PutImageA(int x,int y,const tPicture* image,const cRect* rect=NULL);
    inline void PutImageA(int x,int y,const tPicture* image,u32_t alpha,const cRect* rect=NULL);
// other
    void cls(color_t fill=0)                                 { _bar(mClipRect.left,mClipRect.top,mClipRect.right,mClipRect.bottom,fill); }

    virtual color_t  rgb(u8_t red,u8_t green,u8_t blue)  =0;
    virtual COLOR32 rgb(color_t color)                   =0;

    void polyline(int count,const tPoint* p,color_t color,const cRect* rect=NULL);

    void circle(int xc,int yc,int r,color_t color,const cRect* rect=NULL);
    void ellipse(int xc,int yc,int a,int b,color_t color,const cRect* rect=NULL);
    void polygone(int count,const tPoint* p,color_t color,const cRect* rect=NULL);
    inline void triangle(int x1,int y1,int x2,int y2,int x3,int y3,color_t color,const cRect* rect=NULL);

    static Screen* create(IMAGE);   // Создает flatScreen по рисунку

// Hack for refresh pages
    virtual void SetPtr(void* data) {};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// inline
void Screen::PutImage(int x,int y,const tPicture* image,const cRect* rect)
{       
    if(rect)
    {
        cRect r2(*rect);
        r2&=mClipRect;
        _PutImage(x,y,image,&r2); 
    }
    else
        _PutImage(x,y,image,&mClipRect); 
}

void Screen::GetImage(int x,int y,tPicture* image)
{
    _GetImage(x,y,image->image->width,image->image->height,image,&mClipRect); 
}

void Screen::PutImageA(int x,int y,const tPicture* image,const cRect* rect)
{
    if(rect)
    {
        cRect r2(*rect);
        r2&=mClipRect;
        _PutImageA(x,y,image,&r2); 
    }
    else
        _PutImageA(x,y,image,&mClipRect); 
}

void Screen::PutImageA(int x,int y,const tPicture* image,u32_t alpha,const cRect* rect)
{
    if(rect)
    {
        cRect r2(*rect);
        r2&=mClipRect;
        _PutImageA(x,y,image,alpha,&r2); 
    }
    else
        _PutImageA(x,y,image,alpha,&mClipRect); 
}

void Screen::triangle(int x1,int y1,int x2,int y2,int x3,int y3,color_t color,const cRect* rect)
{
    tPoint p[3]={{x1,y1},{x2,y2},{x3,y3}};
    triangle(p,color,rect);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////// Экран с линейным доступом

template <class T> class FlatScreen:public Screen
{
protected:
    int line_width;
    T* mData;

    FlatScreen(int w,int h,int bpp,size_t lw,T* ptr)
            :Screen(w,h,bpp),line_width(lw?lw:w*sizeof(T)),mData(ptr) { assert(mData); }
public:
    void* GetPtr()                 { return mData; }
    virtual void SetPtr(void* data) { mData = reinterpret_cast<T*>(data); }

    virtual ~FlatScreen();
/// Графические примитивы /////////////////////////////////
    virtual void _PutPixel(int x,int y,color_t color);
    virtual color_t _GetPixel(int x,int y);

    virtual void _vline(int x,int y,int h,color_t color);
    virtual void _line(int x0,int y0,int x1,int y1,color_t color);

/// Работа с текстом /////////////////////////////////////
    virtual void _outchar(int x,int y, char ch, color_t color,const cRect* rect);
/// Графические примитивы ////////////////////////////////
    virtual void _PutImage(int x,int y,const tPicture* image,const cRect* rect);
    virtual void _GetImage(int x,int y,int w,int h,tPicture* image,const cRect* rect);
};

class fScreen8bit:public FlatScreen<u8_t>
{
public:
    fScreen8bit(int w,int h,u8_t* ptr,size_t lw=0):FlatScreen<u8_t>(w,h,BPP_8bit,lw,ptr) { }

    virtual void _hline(int x,int y,int w,color_t color);
    virtual void _bar(int left,int top,int right,int bottom,color_t color);

    virtual color_t  rgb(u8_t red,u8_t green,u8_t blue);
    virtual COLOR32 rgb(color_t color);

    virtual void _PutImageA(int x,int y,const tPicture* image,const cRect* rect);
    virtual void _PutImageA(int x,int y,const tPicture* image,u32_t alpha,const cRect* rect);
};

class fScreen15bit:public FlatScreen<u16_t>
{
public:
    fScreen15bit(int w,int h,u16_t* ptr,size_t lw=0):FlatScreen<u16_t>(w,h,BPP_15bit,lw,ptr)  {}
/// Графические примитивы /////////////////////////////////
    virtual void _hline(int x,int y,int w,color_t color);
    virtual void _bar(int left,int top,int right,int bottom,color_t color);

    virtual color_t  rgb(u8_t red,u8_t green,u8_t blue);
    virtual COLOR32 rgb(color_t color);

    virtual void _PutImageA(int x,int y,const tPicture* image,const cRect* rect);
    virtual void _PutImageA(int x,int y,const tPicture* image,u32_t alpha,const cRect* rect);
};

class fScreen16bit:public fScreen15bit
{
public:
    fScreen16bit(int w,int h,u16_t* ptr,size_t lw=0):fScreen15bit(w,h,ptr,lw)  { mBpp=BPP_16bit; }
/// Графические примитивы /////////////////////////////////
    virtual color_t  rgb(u8_t red,u8_t green,u8_t blue);
    virtual COLOR32 rgb(color_t color);

    virtual void _PutImageA(int x,int y,const tPicture* image,const cRect* rect);
    virtual void _PutImageA(int x,int y,const tPicture* image,u32_t alpha,const cRect* rect);
};

class fScreen32bit:public FlatScreen<u32_t>
{
public:
    fScreen32bit(int w,int h,u32_t* ptr):FlatScreen<u32_t>(w,h,BPP_32bit,0,ptr)   {}
/// Графические примитивы /////////////////////////////////
    virtual void _hline(int x,int y,int w,color_t color);
    virtual void _bar(int left,int top,int right,int bottom,color_t color);

    virtual color_t  rgb(u8_t red,u8_t green,u8_t blue);
    virtual COLOR32 rgb(color_t color);

    virtual void _PutImageA(int x,int y,const tPicture* image,const cRect* rect);
    virtual void _PutImageA(int x,int y,const tPicture* image,u32_t alpha,const cRect* rect);
};

/* Supposed a lot of things:
	the screen is 32bit;
	there is a shadow page
*/
class fScreen32bitFB: public fScreen32bit
{
	int flatBufferFD;
	struct fb_var_screeninfo varFBInfo;
	u32_t* scrPtr;
	fScreen32bitFB(int fd, int w, int h, u32_t* ptr, struct fb_var_screeninfo& varFB);
public:
	~fScreen32bitFB();

	void flip();

	static fScreen32bitFB* open(const char* fb_device);
};

#endif
