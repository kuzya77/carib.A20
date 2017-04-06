#include <types.h>
#include <mem.h>
#include <log.h>

#include "ipicture.h"
#include "igf_scr.h"
#include "istuff.h"

template class FlatScreen<u32_t>;
template class FlatScreen<u16_t>;
template class FlatScreen<u8_t>;

/////////////////////////////////////////////////////////////////////////////
// TEMPLATE
/////////////////////////////////////////////////////////////////////////////
template <class T>FlatScreen<T>::~FlatScreen()
{
	mData=NULL;
}

template <class T> void FlatScreen<T>::_PutPixel(int x,int y, color_t color)
{
	offset_of(mData,y*line_width)[x]=color;
}

template <class T> color_t FlatScreen<T>::_GetPixel(int x,int y)
{
	return offset_of(mData,y*line_width)[x];
}

template <class T> void FlatScreen<T>::_vline(int x,int y,int h,color_t color)
{
	for(T* d=offset_of(mData,y*line_width)+x;h--;d=offset_of(d,line_width))
		*d=color;
}

template <class T> void FlatScreen<T>::_line(int xs,int ys,int xe,int ye,color_t color)
{
// rem: xs<=xe
   	T* ptr=offset_of(mData,ys*line_width)+xs;
	*ptr=color;
   	int dx=xe-xs;
	int dy=abs(ye-ys);
#define CHECK_AND_(op)			{ 	\
						if(d>0) 	\
						{ 		\
							d+=d2; 	\
							op; 	\
						} 		\
						else 		\
							d+=d1; 	\
					}

	if(dy<=dx)
	{
		int d,d1,d2=(dy-dx)<<1;
		d=(d1=dy<<1)-dx;
		if(ye>ys)
			for(;dx--;*++ptr=color)
				CHECK_AND_(ptr=offset_of(ptr,line_width))
		else
			for(;dx--;*++ptr=color)
				CHECK_AND_(ptr=offset_of(ptr,-line_width));
	}
	else
	{
		int d,d1,d2=(dx-dy)<<1;
		d=(d1=dx<<1)-dy;
		if(ye>ys)
			for(;dy--;*(ptr=offset_of(ptr,line_width))=color)
				CHECK_AND_(ptr++)
		else
			for(;dy--;*(ptr=offset_of(ptr,-line_width))=color)
				CHECK_AND_(ptr++);
	}
#undef CHECK_AND_
}

// Текст
template <class T> void FlatScreen<T>::_outchar(int x,int y, char ch, color_t color,const cRect* rect)
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
		if(dx>w)
			return;
		x=rect->left;
		smask>>=dx&7;
	}
    else
		dx=0;
	T* d=offset_of(mData,y*line_width);
	for(int i=0;i++<h;d=offset_of(d,line_width))
	{
		u8_t mask=smask,data=*ptr++;
		int xl=x;
        for(int j=dx;j++<w && mask;mask>>=1,xl++)
        {
            if(mask&data)
            	d[xl]=color;
    	}
	}
}


/// Графические примитивы ////////////////////////////////
template <class T> void FlatScreen<T>::_PutImage(int x,int y,const tPicture* image,const cRect* rect)
{
	assert(image);
	assert(rect);
	if(mBpp!=image->bpp)
	{
		logWarning("FlatScreen::_PutImage ignore out(different bpp) scr:%d  img:%d",mBpp,image->bpp);
		return;
	}
	const tImage* img=image->image;
	int w=img->width,h=img->height,rw=w;
	int src_lw=img->line_width;
	const T* src=(const T*)img->data;
	// vertical truncate
	if(rect->bottom<y+h)
		if((h=rect->bottom-y+1)<=0)
			return;
	if(rect->top>y)
	{
		int d=rect->top-y;
		if((h-=d)<=0)
			return;
		src=offset_of(src,d*src_lw);
		y=rect->top;
	}
	T* dest=offset_of(mData,y*line_width)+x;
	if(rect->right<x+w)
		if((rw=rect->right-x+1)<=0)
			return;
	if(x<rect->left)
	{
		int d=rect->left-x;
		src+=d;
		dest+=d;
		if((rw-=d)<0)
			return;
	}
	for(rw*=sizeof(T);h--;src=offset_of(src,src_lw))
	{
		memcpy(dest,src,rw);
		dest=offset_of(dest,line_width);
	}
}

template <class T> void FlatScreen<T>::_GetImage(int x,int y,int w,int h,tPicture* image,const cRect* rect)
{
	assert(image);
	assert(rect);
	if(mBpp!=image->bpp)
	{
		logWarning("FlatScreen::_GetImage ignore out(different bpp) scr:%d  img:%d",mBpp,image->bpp);
		return;
	}
	tImage* img=image->image;
	int src_lw=img->line_width;
	int rw=w;
	T* dest=(T*)img->data;
	// vertical truncate
	if(rect->bottom<y+h)
		if((h=rect->bottom-y+1)<=0)
			return;
	if(rect->top>y)
	{
		int d=rect->top-y;
		if((h-=d)<=0)
			return;
		dest=offset_of(dest,d*src_lw);
		y=rect->top;
	}
	const T* src=offset_of(mData,y*line_width)+x;
	if(rect->right<x+w)
		if((rw=rect->right-x+1)<=0)
			return;
	if(x<rect->left)
	{
		int d=rect->left-x;
		src+=d;
		dest+=d;
		if((rw-=d)<0)
			return;
	}
	for(rw*=sizeof(T);h--;src=offset_of(src,line_width))
	{
		memcpy(dest,src,rw);
		dest=offset_of(dest,src_lw);
	}
}

/////////////////////////////////////////////////////////////////////////////
// fScreen8bit

void fScreen8bit::_hline(int x,int y,int w,color_t color)
{
	memset(offset_of(mData,y*line_width)+x,color,w);
}

void fScreen8bit::_bar(int xs,int ys,int xe,int ye,color_t color)
{
	int l;
	u8_t* ptr=offset_of(mData,ys*line_width)+xs;
	if((l=xe-xs+1)==mWidth)
		memset(ptr,color,line_width*(ye-ys+1));
	else	
		for(;ys++<=ye;ptr=offset_of(ptr,line_width))
			memset(ptr,color,l);
}

color_t fScreen8bit::rgb(u8_t red,u8_t green,u8_t blue)
{
	return 0;
}

COLOR32 fScreen8bit::rgb(color_t color)
{
	COLOR32 c;
	c.all=0;
	return c;
}

void fScreen8bit::_PutImageA(int x,int y,const tPicture* image,const cRect* rect)
{
//	assert(image);		
	_PutImageA(x,y,image,((u8_t*)image->image->data)[0],rect);
}

void fScreen8bit::_PutImageA(int x,int y,const tPicture* image, u32_t alpha, const cRect* rect)
{
//	assert(image);
//	assert(rect);
	if(image->bpp!=BPP_8bit)
		return;

	const tImage* img=image->image;
	int w=img->width,h=img->height,rw=w;
	int src_lw=img->line_width;

	// vertical truncate
	if(rect->bottom<y+h)
		if((h=rect->bottom-y+1)<=0)
			return;
	if(rect->right<x+w)
		if((rw=rect->right-x+1)<=0)
			return;
	const u8_t* src=(const u8_t*)img->data;
	if(rect->top>y)									
	{
		int d=rect->top-y;
		if((h-=d)<=0)
			return;
		src+=d*src_lw;
		y=rect->top;								
	}
	u8_t* dest=mData+y*line_width+x;
	if(x<rect->left)
    {
     	int d=rect->left-x;
        src+=d;
        dest+=d;
        if((rw-=d)<0)
			return;
	}
	u8_t zcolor=alpha;
	for(;h--;src+=src_lw)
	{
        for(int i=0;i<rw;i++)
        	if(src[i]!=zcolor)	//bit15==1 - вывод, ==0 - пропуск
          		dest[i]=src[i];
		dest+=line_width;
    }
}

/////////////////////////////////////////////////////////////////////////////
// fScreen15bit(16)bit
#ifndef __NIOS_ALTERA__
void fScreen15bit::_hline(int x,int y,int w,color_t color)
{
	memsetw(offset_of(mData,y*line_width)+x,color,w);
}

void fScreen15bit::_bar(int xs,int ys,int xe,int ye,color_t color)
{
	int l;
	u16_t* ptr=offset_of(mData,ys*line_width)+xs;
	if((l=xe-xs+1)==mWidth)
		memsetw(ptr,color,(line_width*(ye-ys+1)) >> 1);
	else	
		for(;ys++<=ye;ptr=offset_of(ptr,line_width))
			memsetw(ptr,color,l);
}

color_t  fScreen15bit::rgb(u8_t red,u8_t green,u8_t blue)
{
	return (u16_t(red&0xF8)<<7)|(u16_t(green&0xF8)<<2)|(blue>>3);
}

COLOR32 fScreen15bit::rgb(color_t color)
{
	COLOR32 c32;
	c32.rgb.rgbRed  =(color>>7)&0xF8;
	c32.rgb.rgbGreen=(color>>2)&0xF8;
	c32.rgb.rgbBlue =(color<<3)&0xF8;
	return c32;
}

// Вывод с alpha-каналом, работает режим BPP_15bit ( bit15==1 - вывод, ==0 - пропуск)
// 										 BPP_32bit ( alpha <=>4th u8_t)
void fScreen15bit::_PutImageA(int x,int y,const tPicture* image,const cRect* rect)
{
	assert(image);
	assert(rect);

	const tImage* img=image->image;
	int w=img->width,h=img->height,rw=w;
	int src_lw=img->line_width;

	// vertical truncate
	if(rect->bottom<y+h)
		if((h=rect->bottom-y+1)<=0)
			return;
	if(rect->right<x+w)
		if((rw=rect->right-x+1)<=0)
			return;
#define TOP_LEFT_CUT	if(rect->top>y)									\
						{												\
							int d=rect->top-y;							\
							if((h-=d)<=0)								\
							return;										\
							src=offset_of(src,d*src_lw);				\
							y=rect->top;								\
						}												\
						u16_t* dest=offset_of(mData,y*line_width)+x;	\
				        if(x<rect->left)								\
        				{												\
                			int d=rect->left-x;							\
                			src+=d;										\
                			dest+=d;									\
                			if((rw-=d)<0)								\
                        		return;									\
        				}


	switch(image->bpp)
	{
		case BPP_15bit:
			{
				const u16_t* src=(const u16_t*)img->data;
				TOP_LEFT_CUT;
        		for(;h--;src=offset_of(src,src_lw))
        		{
          			for(int i=0;i<rw;i++)
          				if(src[i]&0x8000)	//bit15==1 - вывод, ==0 - пропуск
          					dest[i]=src[i];
					dest=offset_of(dest,line_width);
        		}
			}
			break;
		case BPP_32bit:
			{
				const RGBQUAD* src=(const RGBQUAD*)img->data;
				TOP_LEFT_CUT;
        		for(;h--;src=offset_of(src,src_lw))
        		{
          			for(int i=0;i<rw;i++)
          			{
          				unsigned alpha=src[i].rgbReserved;
          				if(alpha)
          				{
          					u16_t r=src[i].rgbRed,
							   	   g=src[i].rgbGreen,
							   	   b=src[i].rgbBlue;

          					if(alpha==255)
          					{
          						dest[i]=((r&0xF8)<<7)|((g&0xF8)<<2)|(b>>3);
          					}
          					else
          					{
          						u16_t dr=(dest[i]>>7)&0xF8;
          						dr+=(alpha*(r-dr))>>8;
								u16_t dg=(dest[i]>>2)&0xF8;
          						dg+=(alpha*(g-dg))>>8;
								u16_t db=(dest[i]<<3)&0xF8;
          						db+=(alpha*(b-db))>>8;
								dest[i]=((dr&0xF8)<<7)|((dg&0xF8)<<2)|(db>>3);
							}
          				}
					}
					dest=offset_of(dest,line_width);
        		}
			}
			break;
		default:
			logWarning("FlatScreen::_PutImageA ignore out(different bpp) scr:%d  img:%d",mBpp,image->bpp);
	}
#undef TOP_LEFT_CUT
}

void fScreen15bit::_PutImageA(int x,int y,const tPicture* image,u32_t ga,const cRect* rect)
{
}


color_t  fScreen16bit::rgb(u8_t red,u8_t green,u8_t blue)
{
	return (u16_t(red&0xF8)<<8)|(u16_t(green&0xFC)<<3)|(blue>>3);
}

COLOR32 fScreen16bit::rgb(color_t color)
{
	COLOR32 c32;
	c32.rgb.rgbRed  =(color>>8)&0xF8;
	c32.rgb.rgbGreen=(color>>3)&0xFC;
	c32.rgb.rgbBlue =(color<<3)&0xF8;
	return c32;
}

// Вывод с alpha-каналом, работает режим BPP_15bit ( bit15==1 - вывод, ==0 - пропуск)
// 										 BPP_32bit ( alpha <=>4th u8_t)
void fScreen16bit::_PutImageA(int x,int y,const tPicture* image,const cRect* rect)
{
	assert(image);
	assert(rect);

	const tImage* img=image->image;
	int w=img->width,h=img->height,rw=w;
	int src_lw=img->line_width;

	// vertical truncate
	if(rect->bottom<y+h)
		if((h=rect->bottom-y+1)<=0)
			return;
	if(rect->right<x+w)
		if((rw=rect->right-x+1)<=0)
			return;
#define TOP_LEFT_CUT	if(rect->top>y)									\
						{												\
							int d=rect->top-y;							\
							if((h-=d)<=0)								\
							return;										\
							src=offset_of(src,d*src_lw);				\
							y=rect->top;								\
						}												\
						u16_t* dest=offset_of(mData,y*line_width)+x;	\
				        if(x<rect->left)								\
        				{												\
                			int d=rect->left-x;							\
                			src+=d;										\
                			dest+=d;									\
                			if((rw-=d)<0)								\
                        		return;									\
        				}


	switch(image->bpp)
	{
		case BPP_15bit:
			{
				const u16_t* src=(const u16_t*)img->data;
				TOP_LEFT_CUT;
        		for(;h--;src=offset_of(src,src_lw))
        		{
          			for(int i=0;i<rw;i++)
          				if(src[i]&0x8000)	//bit15==1 - вывод, ==0 - пропуск
          					dest[i]=((src[i]<<1)&0xFFC0)|(src[i]&0x1F);
					dest=offset_of(dest,line_width);
        		}
			}
			break;
		case BPP_32bit:
			{
				const RGBQUAD* src=(const RGBQUAD*)img->data;
				TOP_LEFT_CUT;
        		for(;h--;src=offset_of(src,src_lw))
        		{
          			for(int i=0;i<rw;i++)
          			{
          				unsigned alpha=src[i].rgbReserved;
          				if(alpha)
          				{
          					u16_t r=src[i].rgbRed,
							   	   g=src[i].rgbGreen,
							   	   b=src[i].rgbBlue;

          					if(alpha==255)
          					{
          						dest[i]=((r&0xF8)<<8)|((g&0xFC)<<3)|(b>>3);
          					}
          					else
          					{
          						u16_t dr=(dest[i]>>8)&0xF8;
          						dr+=(alpha*(r-dr))>>8;
								u16_t dg=(dest[i]>>3)&0xFC;
          						dg+=(alpha*(g-dg))>>8;
								u16_t db=(dest[i]<<3)&0xF8;
          						db+=(alpha*(b-db))>>8;
								dest[i]=((dr&0xF8)<<8)|((dg&0xFC)<<3)|(db>>3);
							}
          				}
					}
					dest=offset_of(dest,line_width);
        		}
			}
			break;
		default:
			logWarning("FlatScreen::_PutImageA ignore out(different bpp) scr:%d  img:%d",mBpp,image->bpp);
	}
#undef TOP_LEFT_CUT
}

void fScreen16bit::_PutImageA(int x,int y,const tPicture* image,u32_t ga,const cRect* rect)
{
}

/////////////////////////////////////////////////////////////////////////////
// fScreen32bit

void fScreen32bit::_hline(int x,int y,int w,color_t color)
{
	memsetd(offset_of(mData,y*line_width)+x,color,w);
}

void fScreen32bit::_bar(int xs,int ys,int xe,int ye,color_t color)
{
	int l;
	u32_t* ptr=offset_of(mData,ys*line_width)+xs;
	if((l=xe-xs+1)==mWidth)
		memsetd(ptr,color,mWidth*(ye-ys+1));		// Высегда выровнен, нет необходимости вводить дополнительное выравнивание
	else	
		for(;ys++<=ye;ptr=offset_of(ptr,line_width))
			memsetd(ptr,color,l);
}

color_t fScreen32bit::rgb(u8_t red,u8_t green,u8_t blue)
{    	  
	return (u32_t(red)<<16)|(u32_t(green)<<8)|blue;
}

COLOR32 fScreen32bit::rgb(color_t color)
{
	COLOR32 c;
	c.all=color;
	return c;
}

void fScreen32bit::_PutImageA(int x,int y,const tPicture* image,const cRect* rect)
{
	assert(image);
	assert(rect);

	const tImage* img=image->image;
	int w=img->width,h=img->height,rw=w;
	int src_lw=img->line_width;

	// vertical truncate
	if(rect->bottom<y+h)
		if((h=rect->bottom-y+1)<=0)
			return;
	if(rect->right<x+w)
		if((rw=rect->right-x+1)<=0)
			return;
#define TOP_LEFT_CUT	if(rect->top>y)									\
						{												\
							int d=rect->top-y;							\
							if((h-=d)<=0)								\
							return;										\
							src=offset_of(src,d*src_lw);				\
							y=rect->top;								\
						}												\
						RGBQUAD* dest=offset_of((RGBQUAD*)mData,y*line_width)+x;	\
				        if(x<rect->left)								\
        				{												\
                			int d=rect->left-x;							\
                			src+=d;										\
                			dest+=d;									\
                			if((rw-=d)<0)								\
                        		return;									\
        				}

	switch(image->bpp)
	{
		case BPP_15bit:
			{
				const u16_t* src=(const u16_t*)img->data;
				TOP_LEFT_CUT;
        		for(;h--;src=offset_of(src,src_lw))
        		{
          			for(int i=0;i<rw;i++)
          				if(src[i]&0x8000)	//bit15==1 - вывод, ==0 - пропуск
						{
          					dest[i].rgbRed  =(src[i]>>7)&0xF8;
          					dest[i].rgbGreen=(src[i]>>2)&0xF8;
          					dest[i].rgbBlue =(src[i]<<3)&0xF8;
						}
					dest+=mWidth;
        		}
			}
			break;
		case BPP_32bit:
			{
				const RGBQUAD* src=(const RGBQUAD*)img->data;
				TOP_LEFT_CUT;
        		for(;h--;src+=w)
        		{
          			for(int i=0;i<rw;i++)
          			{
          				unsigned alpha=src[i].rgbReserved;
            			if(alpha)
            			{
              				if(alpha==255)
                				dest[i]=src[i];
              				else
              				{
                				#define ACONV(d,s,alp) d+=(alp*(s-d))>>8;		
                				#define ACONV3(d,s,alp) ACONV(d.rgbRed,s.rgbRed,alp);ACONV(d.rgbGreen,s.rgbGreen,alp);ACONV(d.rgbBlue,s.rgbBlue,alp);
                				ACONV3(dest[i],src[i],alpha);
              				}
            			}
					}
					dest+=mWidth;
        		}
			}
			break;
		default:
			logWarning("FlatScreen::_PutImageA ignore out(different bpp) scr:%d  img:%d",mBpp,image->bpp);
	}
}


void fScreen32bit::_PutImageA(int x,int y,const tPicture* image,u32_t ga, const cRect* rect)
{
	assert(image);
	assert(rect);

	const tImage* img=image->image;
	int w=img->width,h=img->height,rw=w;
	int src_lw=img->line_width;

	// vertical truncate
	if(rect->bottom<y+h)
		if((h=rect->bottom-y+1)<=0)
			return;
	if(rect->right<x+w)
		if((rw=rect->right-x+1)<=0)
			return;

	switch(image->bpp)
	{
		case BPP_15bit:
			{
				const u16_t* src=(const u16_t*)img->data;
				TOP_LEFT_CUT;
        		for(;h--;src=offset_of(src,src_lw))
        		{
          			for(int i=0;i<rw;i++)
          				if(src[i]&0x8000)	//bit15==1 - вывод, ==0 - пропуск
						{
          					dest[i].rgbRed  =(src[i]>>7)&0xF8;
          					dest[i].rgbGreen=(src[i]>>2)&0xF8;
          					dest[i].rgbBlue =(src[i]<<3)&0xF8;
						}
					dest+=mWidth;
        		}
			}
			break;
		case BPP_32bit:
			{
				const RGBQUAD* src=(const RGBQUAD*)img->data;
				TOP_LEFT_CUT;
        		for(;h--;src+=w)
        		{
          			for(int i=0;i<rw;i++)
          			{
          				unsigned alpha=(ga*u32_t(src[i].rgbReserved))>>8;
            			if(alpha)
            			{
              				if(alpha==255)
                				dest[i]=src[i];
              				else
              				{
                				#define ACONV(d,s,alp) d+=(alp*(s-d))>>8;		
                				#define ACONV3(d,s,alp) ACONV(d.rgbRed,s.rgbRed,alp);ACONV(d.rgbGreen,s.rgbGreen,alp);ACONV(d.rgbBlue,s.rgbBlue,alp);
                				ACONV3(dest[i],src[i],alpha);
              				}
            			}
					}
					dest+=mWidth;
        		}
			}
			break;
		default:
			logWarning("FlatScreen::_PutImageA ignore out(different bpp) scr:%d  img:%d",mBpp,image->bpp);
	}
}

#undef TOP_LEFT_CUT
#endif
