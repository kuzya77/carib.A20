#include "carib.h"

IMAGE intro=NULL;

//------------------------------------------------------------------------------
// Упрощенный вывод всякой фигни, используется при загрузке для отслеживания состояния

void print(color_t color,const char* format, ... )
{
    conSetColor(color);

    va_list args;
    va_start(args,format);    
    conVPrintf(format,args);
    va_end (args);

    if(intro)
        gfxPutImage(0,0,intro);
    else
        gfxCls();
    conDraw();

    gfxFlip();
}

const char* fmtval (float x)
{
  static char buf [256];

  if      (x < 1000.0f) sprintf (buf,"%.2f",x);
  else if (x < 1000000.0f) sprintf (buf," %.2fk",x/1000.0f);
  else sprintf (buf,"%.2fM",x/1000000.0f);
  
  return buf;  
}

const char* fmtval (size_t x)
{
  static char buf [256];

  if      (x < 1000) sprintf (buf,"%d",x);
  else if (x < 1000000) sprintf (buf," %dk",x/1000);
  else sprintf (buf,"%dM",x/1000000);
  
  return buf;
}
