#ifndef __BLITZ_PICTURE_H__
#define __BLITZ_PICTURE_H__

#include <binfile.h>

#include <FreeImage.h>

class tPicture;

typedef tPicture* 	IMAGE;

/* Внутреннее представление цвета. используемого в системе */
typedef u32_t 		color_t;


enum
{ 
    BPP_DEFAULT=0,
    BPP_8bit=8,
    BPP_15bit=15,
    BPP_16bit=16,
    BPP_24bit=24,
    BPP_32bit=32
};

enum
{ 
    NONE=0,
    BMP=0x100,BMP8,BMP15,BMP24,BMP32,
    PCX=0x200,PCX8,PCX24,PCX32,
    JPG=0x300
};

// images mode
enum
{
    IMG_SHARED=1,
    IMG_NOPALETTE=2         // Отключить загрузку палитры
};

#pragma pack(1)

typedef union _COLOR32
{
    color_t all;
    RGBQUAD rgb;
    u8_t    r[4];
} COLOR32;

#pragma pack()

// Загрузка/создание/удаление/копирование

bool imgInit();
void imgDone();

bool imgLoadPack(const char* name);
void imgReleasePack(const char* name);

IMAGE   imgLoad(const char* fname,int pf=BPP_DEFAULT,int flags=0);  // при использовании IMG_SHARED копии рисунка 

IMAGE   imgCreate(int w,int h,int pf=BPP_32bit, int palsize=256); // Создание рисунка. palsize используется, если pf=BPP_8bit, 
                                                                 // смещение палитры при создании ==0, смещение модифицируемо (см. работу с палитрой)
void    imgFree(IMAGE pict);

IMAGE   imgCopy(IMAGE pict,int pf=BPP_DEFAULT,int flags=0);      // Копирование со сменой формата, по умолчанию формат не меняет

// информация о рисунке ////////////////////////////////////////////////////////////////////////////////////////////////
// размеры и координаты верхнего левого угла

int     imgWidth(IMAGE pict);
int     imgHeight(IMAGE pict);

int     imgFormat(IMAGE pict);
COLOR32 imgRGB(IMAGE pict,color_t color);        // Преобразование цвет -> раскладка по RGB
COLOR32 imgRGB(int format,color_t color);        // То же, что и пред. , но без учета палитры для 8bit режима

color_t imgRGB(IMAGE pict,int red,int green,int blue);      // Преобразование цвет <- раскладка по RGB
color_t imgRGB(int format,int red,int green,int blue);      // То же, что и пред. , но без учета палитры для 8bit режима

bool    imgPutPixel(IMAGE pict,int x,int y, color_t color);
color_t imgGetPixel(IMAGE pict,int x,int y);

bool    imgConvert(IMAGE* pict,int pf);

// извлечение образа: imgBuffer() - извлечение указателя, imgLine() - ... указателя на строку
void*   imgBuffer(IMAGE pict);
void*   imgLine(IMAGE pict, int line);


///////////////////////////////////////////////////////////////////////////
// Palette
///////////////////////////////////////////////////////////////////////////

// Работа с палитрой, поддерживаются только 8bit режимы
// Получение палитры

RGBQUAD* imgPalette(IMAGE pict);                    // доступ к палитре имиджа (если есть), доступен для модификации.
int      imgPaletteBegin(IMAGE pict);               // начальная позиция палитры
bool     imgPaletteBegin(IMAGE pict,int pbegin);    // начальная позиция палитры, установка
int      imgPaletteSize(IMAGE pict);                // размер палитры

class cRect
{
public:
    int left,top,right,bottom;
    cRect() {}
    cRect(int l,int t,int r,int b)      { setup(l,t,r,b); }
    bool inX(int x) const           { return x>=left && x<=right; }  
    bool inY(int y)const            { return y>=top && y<=bottom; }
    bool in(int x,int y) const      { return inX(x) && inY(y);    }
    bool empty() const          { return left>right || top>bottom; }
    cRect& operator=(const cRect& r)    { left=r.left; right=r.right; top=r.top; bottom=r.bottom; return *this; }
    cRect& operator&=(const cRect& r)
                        { 
                            if(left<r.left)     
                                left=r.left;
                            if(right>r.right)  
                                right=r.right; 
                            if(top<r.top)      
                                top=r.top;
                            if(bottom>r.bottom)
                                bottom=r.bottom;
                            return *this;
                        }                               
    cRect& operator|=(const cRect& r)
                        { 
                            if(left>r.left)     
                                left=r.left;
                            if(right<r.right)  
                                right=r.right; 
                            if(top>r.top)      
                                top=r.top;
                            if(bottom<r.bottom)
                                bottom=r.bottom;
                            return *this;
                        }                               

    void setup(int l,int t,int r,int b)     { left=l; right=r; top=t; bottom=b; }
    cRect& add(int x,int y)             { left+=x; right+=x; top+=y; bottom+=y; return *this; }
    int width()  const          { return right-left+1; }
    int height() const          { return bottom-top+1; }
//  void dump(ostream& os) const { os<<'('<<left<<','<<top<<")-("<<right<<','<<bottom<<")\n"; }
//  inline friend ostream& operator<<(ostream& os,const cRect& c)   { c.dump(os); return os; }
};

typedef struct
{
    int x,y;
} tPoint;

#endif
