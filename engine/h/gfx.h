#ifndef __BLITZ_GFX_H__
#define __BLITZ_GFX_H__

// Инициализация и работа в графики
#include <types.h>
#include <picture.h>

#include <SDL/SDL.h>

// Инициализация/завершение
bool    gfxInit(int width,int height,int bpp=BPP_32bit,int mode=0); // mode=0 - оконный(windows) по умолчанию,
                                                                    // 1 - полноэкранный
bool	gfxOpen2nd(const char* second_scr_dev_file);
void    gfxDone();

// Данные текущего видеорежима
int gfxWidth();
int     gfxHeight();
int     gfxBpp();

// двойной буффер и направление вывода
void    gfxFlip();
// TODO: 2nd screen?
void	gfxSet2ndScreen(bool mode);

const cRect& gfxViewPort();
void    gfxViewPort(const cRect& w);

// вывод примитивов
color_t gfxGetPixel (int x,int y);
void    gfxPutPixel (int x,int y,color_t color);
void    gfxLine     (int x0,int y0,int x1,int y1,color_t color,const cRect* rect=NULL);
void    gfxBar      (int x0,int y0,int x1,int y1,color_t color,const cRect* rect=NULL);
void    gfxRectangle(int x0,int y0,int x1,int y1,color_t color,const cRect* rect=NULL);
void    gfxCircle   (int xc,int yc,int r,color_t color,const cRect* rect=NULL);
void    gfxEllipse  (int xc,int yc,int a,int b,color_t color,const cRect* rect=NULL);
void    gfxTriangle (int x1,int y1,int x2,int y2,int x3,int y3,color_t color,const cRect* rect=NULL);
void    gfxPolygone (int count,const tPoint* p,color_t color,const cRect* rect=NULL);

// Преобразования и проч.
color_t  gfxRGB(int red,int green,int blue);
COLOR32 gfxRGB(color_t color);


void    gfxCls(color_t fill=0);

/// Работа с текстом /////////////////////////////////////
const int   GFX_FONT_HEIGHT=16,
            GFX_FONT_WIDTH =8;

void    gfxOutText(int x,int y,const char* s,color_t color);

// Работа с изображением
void    gfxPutImage(int x,int y,IMAGE image,const cRect* rect=NULL);            // Вывод без учета alpha
void    gfxPutImageA(int x,int y,IMAGE image,const cRect* rect=NULL);           // Вывод с alpha
void    gfxPutImageA(int x,int y,IMAGE image,u32_t alpha,const cRect* rect=NULL);  // Вывод с alpha+Глобальный альфа-канал

// Работа с палитрой (если таковая есть :)
void gfxPutPalette(int index, int count, const RGBQUAD* palette);
void gfxGetPalette(int index, int count, RGBQUAD* palette);

inline void gfxPutPalette(const RGBQUAD* palette)   { gfxPutPalette(0, 256, palette); }
inline void gfxGetPalette(RGBQUAD* palette)         { gfxGetPalette(0, 256, palette); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Поддержка платформ
/*
// Замена getch(), kbhit()
#ifdef GetKey
    #undef GetKey
    #undef KeyHit
#endif


#define GetKey      WinGetch
#define KeyHit      WinKbhit

int WinGetch();
bool WinKbhit();
*/
#endif
