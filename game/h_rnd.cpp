#include "head.h"

// Генератор случайных чисел для кариба

//-------------variables for random procedure-----------------------------------
const uint32 mult1 = 0xBB40E64D,
         mult2 = 0xA205B065;

#define x hsw.math.lastrand

static uint32 randnumb[64];
static uint32 y=0x226930B1;

void randomize()
{
        for (int i = 0; i < 64; i++)
        {
                randnumb[i] = x;
                x = x * mult1 + mult2;
        }
}

uint32 rand32()
{
        y = y * mult2 + mult1;
        uint32 z = x * mult1 + mult2;
        x = randnumb [y>>26];
        randnumb [y>>26] = z;
        return x;
}

uint32 rand32 (uint32 max)
{
        y = y * mult2 + mult1;
        uint32 z = x * mult1 + mult2;
        x = randnumb [y>>26];
        randnumb [y>>26] = z;
        return x % max;
}

// Поиск k - го по величине элемента в массиве p[size]
float midsearch_f (float* p, int size, uint32 k) 
{
        int l=0;
        int r=size-1;
        while (l<r)
        {
                float mid = p[k];
                int i=l;
                int j=r;
                do
                {
                        while (p[i]<mid) 
                            i++;
                        while (p[j]>mid) 
                            j--;
                        if (i<=j)
                        {
                                float y=p[i];
                                p[i]=p[j];
                                p[j]=y;
                                i++;
                                j--;
                        };
                }
                while (i<=j);
                if (j<k) l=i;
                if (k<i) r=j;
        }
        return p[k];
}
