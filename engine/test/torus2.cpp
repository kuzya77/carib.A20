#include <gfx.h>
#include <stuff.h>

#include <vector>
#include <algorithm>

#include "vector3d.h"

int halfWidth,halfHeight;

int N1=20;
int N2=20;

typedef float 		    dType;
typedef Vector3D<dType> dVector3D;
typedef Matrix<dType>   dMatrix;


const dType FL_PI=3.1415926;

const dType scrScale=64.0;

const dType R1=3;         //radius low
const dType R2=1;         //radius high

const int PeriodT=200;

struct vCashe
{
    tPoint p;
    int    frame;
};

struct Facet
{
    int       index[3];       //graph of vertex
    dVector3D n;
    dType     depth;
}; 

std::vector<vCashe> vcashe;
std::vector<Facet*> tmp;
std::vector<dVector3D> vertex0,vertex;

dMatrix     trans=dMatrix::rotateX(FL_PI);
dMatrix     transStep=dMatrix::rotateY(FL_PI/PeriodT)*dMatrix::rotateX(FL_PI/PeriodT);

std::vector<Facet> torus;


void   DrawTorus(int frame);
void   UpdateTorus();

void    FastCls()
{
    static IMAGE img=NULL;
    if(!img)
    {
        img=imgCreate(gfxWidth(), gfxHeight(), BPP_32bit);
        logMessage("FastCls: img=%p",img);
        if(!img)
        {
            logError("Error: can't create image");
            return;
        }
        RGBQUAD* p=(RGBQUAD*)imgLine(img,0);
        for(int i=0;i<imgHeight(img);i++)
            for(int j=0;j<imgWidth(img);j++)
                (p++)->rgbGreen = (i*j)&0x3F;

    }
    gfxPutImage(0, 0, img);
}


void InitTorus()
{
    logDebug("Start calc precashe vertexes");

    vcashe.resize(N1*N2);
    tmp.resize(N1*N2*2);
    vertex0.resize(N1*N2);
    vertex.resize(N1*N2);
    torus.resize(N1*N2*2);

    int   k = 0;
    for(int i=0;i<N1;i++)
    {
        dType phi = FL_PI*i*2/N1;
        dType cphi=cos(phi),sphi=sin(phi);

        for(int j=0;j<N2;j++,k++)
        {
            dType psi = FL_PI*j*2/N2;
        
            vertex0[k].x = (R1+R2*cos(psi)) * cphi;
            vertex0[k].y = (R1+R2*cos(psi)) * sphi;
            vertex0[k].z = R2 * sin(psi);
//            logMessage("vertex[%d]={%f,%f,%f}",k,vertex0[k].x,vertex0[k].y,vertex0[k].z);
        }
    }

    logDebug("End calc precashe vertexes");

    for(int i=0;i<N1*N2;i++)
        vcashe[i].frame=-1;

    auto tk=torus.begin();

    for(int i=0;i<N1;i++)
    {
        int j,iN2=i*N2,i1N2=((i+1)%N1)*N2;

        for(j=0;j<N2-1;j++)
        {
            tk->index[0]=iN2+j;
            tk->index[1]=i1N2+j;
            tk->index[2]=i1N2+j+1;
            tk++;

            tk->index [0] = iN2 + j;
            tk->index [1] = i1N2+j+1;
            tk->index [2] = iN2 + j+1;
            tk++;
        }
        
        tk->index [0] = iN2 + j;
        tk->index [1] = i1N2+j;
        tk->index [2] = i1N2;
        tk++;

        tk->index [0] = iN2 + j;
        tk->index [1] = i1N2;
        tk->index [2] = iN2;

        tk++;
    }
}

int main(int argc,char* argv[])
{
    logInit();
    logTitle("Lets torus demo begin!");
    try
    {
         if(!gfxInit(800, 600))
	{
             return -1;
	}
         logMessage("Size= %dx%d",gfxWidth(),gfxHeight());

         halfWidth=gfxWidth()/2;
         halfHeight=gfxHeight()/2;

         logMessage("After palette");
         FastCls();
         logMessage("Calc torus params");
         InitTorus();

         uint32 t0=MilliSecs();

         gfxOpen2nd("/dev/fb1");			

	 bool done = false;
         for(int i=0;!done;i++)
         {
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type & SDL_KEYDOWN)
			{
				printf("key down[0x%x](%x): '%c'\n", event.key.keysym.scancode, event.key.keysym.mod, event.key.keysym.unicode);
			}
			if(event.type & SDL_KEYUP)
			{
				printf("key up[0x%x](%x): '%c'\n", event.key.keysym.scancode, event.key.keysym.mod, event.key.keysym.unicode);
				if(event.key.keysym.sym==SDLK_ESCAPE)
					done = true;
			}
		}

			gfxSet2ndScreen(false);
             FastCls();
             UpdateTorus();
             DrawTorus(i);
             gfxFlip();

             gfxSet2ndScreen(true);
             gfxCls();
             UpdateTorus();
             DrawTorus(i);

             trans=dMatrix::rotateX(FL_PI)*dMatrix::rotateY(FL_PI*i/PeriodT)*dMatrix::rotateX(FL_PI*i/PeriodT);
                 
             char s[100];
             sprintf(s,"N1=%d,N2=%d FPS=%0.3f",N1,N2,float(i)*1e3/float(MilliSecs()-t0));
             gfxOutText(0,0,s,0xFFFFFF);
             gfxFlip();
             SDL_Delay(10);
         }
    }
    catch(std::bad_alloc&)
    {
        logError("Error: std::bad_alloc");
    }
    catch(...)
    {
        logError("Хуйня какая-то(...)");
    }

    gfxDone();
    return 0;
}

inline bool FacetLess(Facet* f1,Facet* f2)
{
    return f1->depth<f2->depth;
}

void UpdateTorus()
{
    for(int k=0;k<N1*N2;k++)
        vertex[k]=trans*vertex0[k];        //rotate torus
}

void DrawTorus(int frame)
{
    int count=0;

    for(auto f=torus.begin(); f!=torus.end(); f++)
    {
        f->n=(vertex[f->index[1]]-vertex[f->index[0]])^(vertex[f->index[2]]-vertex[f->index[1]]);

        if(f->n.z<=0)    //отсечение не лицевых граней
            continue;
        f->depth=vertex[f->index[0]].z;

        for(int j=1;j<3;j++)       //vertex loop
        {
            dType d=vertex[f->index[j]].z;
            if(d<f->depth)
                f->depth=d;
        }
        tmp[count++]=&*f;
    }

    sort(tmp.begin(),tmp.begin()+count,FacetLess);

    tPoint edges[3];

    for(int i=0;i<count;i++)
    {
            for (int k=0;k<3;k++)       //vertex loop
            {
                int pindex=tmp[i]->index[k];
                if(vcashe[pindex].frame!=frame) // update vertex projection data
                {       //parallel projection
                    const dVector3D& v=vertex[pindex];    //rotate torus
                    vcashe[pindex].p.x=int(v.x*scrScale)+halfWidth;
                    vcashe[pindex].p.y=int(v.y*scrScale)+halfHeight;
                    vcashe[pindex].frame=frame;
                }
                edges[k]=vcashe[pindex].p;
            }
            int color=80+int(tmp[i]->n.z*43);
            if(color>255)
                color=255;
            gfxPolygone(3,edges,color*0x10101);
    }
}
