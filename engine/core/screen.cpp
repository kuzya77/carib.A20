#include <types.h>
#include <mem.h>
#include <log.h>

#include "ipicture.h"
#include "igf_scr.h"
#include "istuff.h"

Screen::~Screen()
{
    node.remove();  // remove link from ScrList's
}

void Screen::CorrectCW()
{
    if(mClipRect.left<0)
        mClipRect.left=0;
    if(mClipRect.right>=mWidth)
        mClipRect.right=mWidth-1;
    if(mClipRect.top<0)
        mClipRect.top=0;
    if(mClipRect.bottom>=mHeight)
        mClipRect.bottom=mHeight-1;
}

void Screen::_vline(int x,int y,int h,color_t color)
{
    for(int ye=y+h;y<ye;_PutPixel(x,y++,color));
}

void Screen::_hline(int x,int y,int w,color_t color)
{
    for(int xe=x+w;x<xe;_PutPixel(x++,y,color));
}

// Вывод линии, алг. брезенхама
// 1е: линия уже отсечена
// 2е: xs<=xe
void Screen::_line(int xs,int ys,int xe,int ye,color_t color)
{
// rem: xs<=xe
    _PutPixel(xs,ys,color);
    int dx=xe-xs;
    int dy=abs(ye-ys);

#define CHECK_AND_(op)          {                   \
                                        if(d>0)     \
                                        {           \
                                            d+=d2;  \
                                            op;     \
                                        }           \
                                        else        \
                                            d+=d1;  \
                                }
#define FOR_PUTPIXEL(var_for,px,py,check_var) { for(;var_for--;_PutPixel(px,py,color)) CHECK_AND_(check_var); }

    if(dy<=dx)
    {
        int d,d1,d2=(dy-dx)<<1;
        d=(d1=dy<<1)-dx;
        if(ye>ys)
            FOR_PUTPIXEL(dx,++xs,ys,ys++)
        else
            FOR_PUTPIXEL(dx,++xs,ys,ys--);
    }
    else
    {
        int d,d1,d2=(dx-dy)<<1;
        d=(d1=dx<<1)-dy;
        if(ye>ys)
            FOR_PUTPIXEL(dy,xs,++ys,xs++)
        else
            FOR_PUTPIXEL(dy,xs,--ys,xs++);
    }

#undef FOR_PUTPIXEL
#undef CHECK_AND_
}

void Screen::_bar(int left,int top,int right,int bottom,color_t color)
{
    for(int w=right-left+1;top<=bottom;_hline(left,top++,w,color));
}

void Screen::vline(int x,int y,int h,color_t color,const cRect* rect)
{
    if(!rect->inX(x))
        return;
    if(h<0)
        y-=h=-h;
    LowTo(h,rect->bottom+1-y);
    if(y<rect->top)
    {
        h+=y-rect->top;
        y=rect->top;
    }
    if(h>0)
        _vline(x,y,h,color);
}

void Screen::hline(int x,int y,int w,color_t color,const cRect* rect)
{
    if(!rect->inY(y))
        return;
    if(w<0)
        x-=w=-w;
    LowTo(w,rect->right+1-x);
    if(x<rect->left)
    {
        w+=x-rect->left;
        x=rect->left;
    }
    if(w>0)
        _hline(x,y,w,color);
}

void Screen::line(int xs,int ys,int xe,int ye,color_t color,const cRect* rect)
{
    if(!rect)
        rect=&mClipRect;
    if(xs>xe)
    {
        std::swap(xs,xe);
        std::swap(ys,ye);
    }
    if(xs>rect->right || xe<rect->left)
        return;
    int dx=xe-xs;
    int dy=ye-ys;
#define CHECK_VLINE if(!dx) { vline(xs,ys,dy+1,color,rect); return; }
    CHECK_VLINE;
    if(xs<rect->left)
    {
        ys+=dy*(rect->left-xs)/dx;
        dy=ye-ys;
        dx=xe-(xs=rect->left);
        CHECK_VLINE;
    }
    if(xe>rect->right)
    {
        
        ye+=dy*(rect->right-xe)/dx;
        dy=ye-ys;
        dx=(xe=rect->right)-xs;
        CHECK_VLINE;
    }
#undef CHECK_VLINE
    if(!dy)
    {
        hline(xs,ys,dx+1,color,rect);
        return;
    }
#define CHECK_HLINE if(!dy) { _hline(xs,ys,dx+1,color); return; }
    if(ys<rect->top)
    {
        if(ye<rect->top)
            return;
        xs+=dx*(rect->top-ys)/dy;
        dx=xe-xs;
        if(dx<0)
            return;
        dy=ye-(ys=rect->top);
        CHECK_HLINE;
    }
    else
        if(ys>rect->bottom)
        {
            if(ye>rect->bottom)
                return;
            xs+=dx*(rect->bottom-ys)/dy;
            dx=xe-xs;
            if(dx<0)
                return;
            dy=ye-(ys=rect->bottom);
            CHECK_HLINE;
        }
    if(ye<rect->top)
    {
        xe+=dx*(rect->top-ye)/dy;
        dx=xe-xs;
        dy=(ye=rect->top)-ys;
        CHECK_HLINE;
    }
    else
        if(ye>rect->bottom)
        {
            xe+=dx*(rect->bottom-ye)/dy;
            dx=xe-xs;
            dy=(ye=rect->bottom)-ys;
            CHECK_HLINE;
        }
#undef CHECK_HLINE
    _line(xs,ys,xe,ye,color);
}

void Screen::bar(int xs,int ys,int xe,int ye,color_t color,const cRect* rect)
{
    if(xs>xe)
        std::swap(xs,xe);
    if(ys>ye)
        std::swap(ys,ye);
    cRect r2(xs,ys,xe,ye);
    r2&=rect?*rect:mClipRect;
    if(!r2.empty())
        _bar(r2.left,r2.top,r2.right,r2.bottom,color);
}

void Screen::rectangle(int xs,int ys,int xe,int ye,color_t color,const cRect* rect)
{
    if(!rect)
        rect=&mClipRect;
    if(xs>xe)
        std::swap(xs,xe);
    if(ys>ye)
        std::swap(ys,ye);
    hline(xs,ys,xe-xs+1,color,rect);
    hline(xs,ye,xe-xs+1,color,rect);
    vline(xs,ys+1,ye-ys-1,color,rect);
    vline(xe,ys+1,ye-ys-1,color,rect);
}

void Screen::_PutImage(int x,int y,const tPicture* image,const cRect* rect)
{
    int bpp;
    switch(image->bpp)
    {
        case BPP_8bit:
            bpp=1;
            break;
        case BPP_15bit:
        case BPP_16bit:
            bpp=2;
            break;
        case BPP_24bit:
            bpp=3;
            break;
        case BPP_32bit:
            bpp=4;
            break;
        default:
            return;
    }
    int wb=image->image->line_width;
    const u8_t* data=image->image->data;
    int h=image->image->height,
    	w=image->image->width;
    if(y<rect->top)
        {
            if((h-=rect->top-y)<=0)
                return;
            data+=wb*(rect->top-y);
            y=rect->top;
        }
    if(y+h>rect->bottom+1)
        h=rect->bottom-y+1;
    if(x+w>rect->right+1)
        if((w=rect->right-x+1)<=0)
            return;
    int i,j,stx;
    if(x<rect->left)
        data+=(stx=rect->left-x)*bpp;
    else
        stx=0;
    for(i=0;i++<h;y++,data+=wb)
    {
        const u8_t* ldata=data;
        for(int xl=x+(j=stx);j<w;j++,ldata+=bpp)
            _PutPixel(xl++,y,*((long*)ldata));
    }
}

void Screen::_GetImage(int x,int y,int w,int h,tPicture* image,const cRect* rect)
{
    if(x>rect->right || y>rect->bottom)
            return;
    if(w>image->image->width)
        w=image->image->width;
    if(h>image->image->height)
        h=image->image->height;
    int LineW=image->image->line_width;
    u8_t* data=image->image->data;
    if(y<rect->top)
    {
        if((h-=rect->top-y)<=0)
            return;
        data+=(rect->top-y)*LineW;
        y=rect->top;
    }
    LowTo(h,rect->bottom-y+1);
    LowTo(w,rect->right-x+1);
    int stx=x<rect->left?rect->left-x:0;
    int i;
    switch(image->bpp)
        {
        case BPP_8bit:
            for(i=0;i<h;i++,y++,data+=LineW)
                for(int j=stx;j<w;j++)
                    data[j]=_GetPixel(x+j,y);
            break;
        case BPP_15bit:
        case BPP_16bit:
            for(i=0;i<h;i++,y++,data+=LineW)
            {
                u16_t* d=(u16_t*)data;
                for(int j=stx;j<w;j++)
                    d[j]=_GetPixel(x+j,y);
            }
            break;
        case BPP_24bit:
            for(i=0;i<h;i++,y++,data+=LineW)
            {
                RGBStruct* d=(RGBStruct*)data;
                for(int j=stx;j<w;j++)
                {
                    u32_t l=_GetPixel(x+j,y);
                    d[j].red=HIWORD(l);
                    d[j].green=HIBYTE(l);
                    d[j].blue=LOBYTE(l);
                }
            }
            break;
        case BPP_32bit:
            for(i=0;i<h;i++,y++,data+=LineW)
            {
                u32_t* d=(u32_t*)data;
                for(int j=stx;j<w;j++)
                    d[j]=_GetPixel(x+j,y);
            }
            break;
        }
}

void Screen::_PutImageA(int x,int y,const tPicture* image,const cRect* rect)                    // Вывод с Alpha 8:24
{
}

void Screen::_PutImageA(int x,int y,const tPicture* image,u32_t alpha,const cRect* rect)                   // Вывод с Alpha 8:24
{
}

// Графические примитивы ////////////////////////////////
#define CIRCLE_POINT(x,y)   if(rect->in(x,y)) _PutPixel(x,y,color);

void Screen::circle(int xc,int yc,int r,color_t color,const cRect* rect)
{
    if(!rect)
        rect=&mClipRect;
    int y=r,
        d=1-r,
        delta1=3,
        delta2=-2*r+5;
#define CIRCLE_POINT2(x,y)  CIRCLE_POINT(x+xc,y+yc); CIRCLE_POINT(y+xc,x+yc);
    for(int x=0;x<=y;x++)
    {
        CIRCLE_POINT2(x,y);
        CIRCLE_POINT2(-x,y);
        CIRCLE_POINT2(x,-y);
        CIRCLE_POINT2(-x,-y);
        if(d<0)
        {
            d+=delta1;
            delta2+=2;
        }
        else
        {
            d+=delta2;
            delta2+=4;
            y--;
        }
        delta1+=2;
    }
#undef CIRCLE_POINT2
}

void Screen::ellipse(int x,int y,int a,int b,color_t color,const cRect* rect)
{
    if(!rect)
        rect=&mClipRect;
    int lx=0,ly=b;
    long aa=a*a, aa2=2*aa, bb=long(b)*long(b) ,bb2=bb*2;
    long d=bb-aa*long(b)+aa/4, dx=0, dy=aa2*long(b);

    CIRCLE_POINT(x,y-ly);
    CIRCLE_POINT(x,y+ly);
    CIRCLE_POINT(x-a,y);
    CIRCLE_POINT(x+a,y);

#define CIRCLE_POINT2(_x,_y)    CIRCLE_POINT(x+_x,y+_y); CIRCLE_POINT(x-_x,y+_y); CIRCLE_POINT(x+_x,y-_y); CIRCLE_POINT(x-_x,y-_y);

    for(;dx<dy;d+=bb+dx)
    {
        if(d>0)
        {
            --ly;
            dy-=aa2;
            d-=dy;
        }
        ++lx;
        dx+=bb2;
        CIRCLE_POINT2(lx,ly);
    }
    for(d+=((3*(aa-bb)/2-(dx+dy))/2);ly>0;d+=aa-dy)
    {
        if(d<0)
        {
            ++lx;
            dx+=bb2;
            d+=bb+dx;
        }
        --ly;
        dy-=aa2;
        CIRCLE_POINT2(lx,ly);
    }
#undef CIRCLE_POINT2
}

#undef CIRCLE_POINT

inline long fraction(int a,int b) { return (a<<16)/b; }
inline long frac0(int a,int b) { return b?fraction(a,b):0; }

void Screen::triangle(const tPoint* p,color_t color,const cRect* rect)
{
    if(!rect)
        rect=&mClipRect;
    int iMax=0,iMin=0,iMid=0,i;
    if(p[1].y<p[iMin].y)
        iMin=1;
    else
        if(p[1].y>p[iMax].y)
            iMax=1;
        if(p[2].y<p[iMin].y)
            iMin=2;
        else
        if(p[2].y>p[iMax].y)
            iMax=2;
    iMid=3-iMin-iMax;
    long dx1,dx2,x1,x2;
    x1=x2=p[iMin].x<<16;
    int ym=p[iMid].y;

    dx1=frac0(p[iMax].x-p[iMin].x,p[iMax].y-p[iMin].y);
    dx2=frac0(p[iMin].x-p[iMid].x,p[iMin].y-p[iMid].y);
    for(i=p[iMin].y;i<=ym;i++)
    {
        hline(x1>>16,i,((x2-x1)>>16)+1,color,rect);
        x1+=dx1;
        x2+=dx2;
    }
    int dyxn;
    if((dyxn=p[iMax].y-p[iMid].y)!=0)
    {
        dx2=fraction(p[iMax].x-p[iMid].x,dyxn);
        for(ym=p[iMax].y;i<=ym;i++)
        {
            x1+=dx1;
            x2+=dx2;
            hline(x1>>16,i,((x2-x1)>>16)+1,color,rect);
        }
    }
}

void Screen::polyline(int count,const tPoint* p,color_t color,const cRect* rect)
{
    for(int i=1;i<count;i++)
        line(p[i-1].x,p[i-1].y,p[i].x,p[i].y,color,rect);
}

void Screen::polygone(int n,const tPoint* p,color_t color,const cRect* rect)
{
    if(!rect)
        rect=&mClipRect;
    int yMin,yMax,topPointIndex,i=1,pTop;
    for(pTop=yMin=yMax=p[topPointIndex=0].y;i<n;i++)
    {
        int piy=p[i].y;
        if(piy<pTop)
            pTop=p[topPointIndex=i].y;
        else
            if(piy>yMax)
                yMax=piy;
    }
    if((yMin=pTop)==yMax)
    {
        int xMin,xMax;
        xMin=xMax=p[0].x;
        for(i=1;i<n;i++)
        {
            int pix=p[i].x;
            if(pix<xMin)
                xMin=pix;
            else
                if(pix>xMax)
                    xMax=pix;
        }
        hline(xMin,yMin,xMax-xMin+1,color,rect);
        return;
    }
    int i1Next,i2Next,i1,i2;
    for(i1=topPointIndex;;)
    {
        if(p[i1Next=i1<1?n-1:i1-1].y<p[i1].y)
            return;
        else
            if(p[i1Next].y==p[i1].y)
                i1=i1Next;
            else
                break;
    }
    for(i2=topPointIndex;;)
    {
        if(p[i2Next=(i2+1)%n].y<p[i2].y)
            return;
        else
            if(p[i2Next].y==p[i2].y)
                i2=i2Next;
            else
                break;
    }
    int y1=p[i1Next].y,y2=p[i2Next].y;
    long x1=p[i1].x<<16,
        x2=p[i2].x<<16,
        dx1=fraction(p[i1Next].x-p[i1].x,y1-p[i1].y),
        dx2=fraction(p[i2Next].x-p[i2].x,y2-p[i2].y);
    for(int y=yMin;y<=yMax;)
    {
        hline(x1>>16,y,((x2-x1)>>16)+1,color,rect);
        x1+=dx1;
        x2+=dx2;
        y++;
        if(y==y1)
        {
            i1=i1Next;
            if(--i1Next<0)
                i1Next=n-1;
            if(p[i1].y==(y1=p[i1Next].y))
                break;
            dx1=fraction(p[i1Next].x-p[i1].x,y1-p[i1].y);
        }
        if(y==y2)
        {
            i2=i2Next;
            if(p[i2].y==(y2=p[i2Next=(i2Next+1)%n].y))
                break;
            dx2=fraction(p[i2Next].x-p[i2].x,y2-p[i2].y);
        }
    }
}

#include "sysfont.h"

void Screen::_outchar(int x,int y, char ch, color_t color,const cRect* rect)
{
    int h=GFX_FONT_HEIGHT,w=GFX_FONT_WIDTH;
    const u8_t* ptr=default_font8x16+u8_t(ch)*h;
    if(y<rect->top)
    {
        if((h-=rect->top-y)<=0)
            return;
        ptr+=rect->top-y;
        y=rect->top;
    }
    LowTo(h,rect->bottom+1-y);
    LowTo(w,rect->right+1-x);
    int dx;
    u8_t smask=128;
    if(x<rect->left)
    {
        dx=rect->left-x;
        x=rect->left;
        ptr+=dx/8;
        smask>>=dx&7;
    }
    else
        dx=0;
    for(int i=0;i++<h;y++)
    {
        u8_t mask=smask,data=*ptr++;
        int xl=x;
        for(int j=dx;j++<w && mask;mask>>=1,xl++)
        {
            if(mask&data)
                _PutPixel(xl,y,color);
        }
    }
}

void Screen::outtext(int x,int y,const char* s,color_t color,const cRect* rect)
{
    if(!rect)
        rect=&mClipRect;
    if(y>rect->bottom)
        return;
    for(;*s && x<=rect->right;s++)
    {
        _outchar(x, y, *s, color,rect);
        x+=GFX_FONT_WIDTH;
    }   
}

Screen* Screen::create(IMAGE p)
{
    if(!p)
        return NULL;
    Screen* scr=NULL;
    try
    {
        const int w=imgWidth(p),h=imgHeight(p);
        switch(imgFormat(p))
        {
            case BPP_8bit:
                scr=new fScreen8bit(w,h,p->image->data,p->image->line_width);
                    break;
            case BPP_15bit:
                scr=new fScreen15bit(w,h,(u16_t*)p->image->data,p->image->line_width);
                break;
            case BPP_16bit:
                scr=new fScreen16bit(w,h,(u16_t*)p->image->data,p->image->line_width);
                break;
            case BPP_32bit:
                scr=new fScreen32bit(w,h,(u32_t*)p->image->data);
                break;
        }
    }
    catch(std::bad_alloc& ba)
    {
        logError("Screen::create: error: %s",ba.what());
        delete scr;
    }    
    return scr;
}
