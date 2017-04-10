#ifndef __ANIM_H__
#define __ANIM_H__

#include <types.h>
#include <mem.h>
#include <gfx.h>
#include <stuff.h>
#include <stuff/parser.h>

#include <string>

/*
cmd support:
  AnimSelectScene <scene>
    AnimHide <animation>
    AnimShow <animation> [#frame]
    AnimPlay <animation>
    AnimMove <layer> <id> <x> <y> <speed>
    AnimMerge <animation> ?
    AnimResetScene ?
    AnimLoad <file>
    AnimSetIndicator <name> <value>
    AnimSetText <layer> <id> <x> <y> <font> <text>
    AnimSetPicture <layer> <id> <x> <y> <picture> [#frame]
    AnimRemove <layer> <id>
    AnimClear <layer>
*/

class NamedObj
{
protected:
    std::string i_name;
public:
    NamedObj() {}
    NamedObj(ParseNode* node);
    NamedObj(const char* s)     { if(s) i_name=s; }
    const char* name()  const   { return i_name.c_str(); }
};

class AnimLayer:public NamedObj
{
protected:
    u32_t   startTime;
    int 	i_layer;

public:
    tPoint  pos;

    virtual int   getCFrame() const;  // get current frame.
    virtual void  setCFrame(int frame);

    virtual void  draw()=0;
    virtual bool  process(u32_t t);
    virtual const char* getRcName();

    virtual void  reset();
    void      	start(u32_t t)  { reset(); startTime=t; } 
    u32_t      	getStart() const { return startTime; }

    int       layer() const    { return i_layer; }

    AnimLayer(int _layer,const char* _name="");
  	AnimLayer(ParseNode* node);
    virtual ~AnimLayer();
};

bool animInit();
void animDone();

bool animLoad(const char* file_name);

bool animSelectScene(const char* name);

void animProcess();
void animDraw();

void animReset();

bool animExist(const char* name);

void animHide(const char* name);
bool animShow(const char* name, int idx=0);     // показ определенной картинки. -1 - начиная от конца
bool animPlay(const char* name);

// Действует как animPlay, но есть различие:
// 1. Подставляет время и тек. кадр старой анимации.
// 2. Запускает новую.
bool animMerge(const char* name, const char* prev_anim);

bool animBind(AnimLayer* usr);

void animSetIndicator(const char* name, const char* value);
const char* animGetIndicator(const char* name);

// static layer commands
bool animSetText(const char* layer, int id, int x, int y, const char* font, const char* text);
bool animSetPicture(const char* layer, int id, int x, int y, const char* anim_name, int idx=0);
void animRemove(const char* layer, int id);
void animClear(const char* layer);
void animMove(const char* layer, int id, int x, int y, float speed);

//-----------------------------------------------------------------------------------------
// Sound support

bool cmdSoundInit(const char* file_list);//command interpreter sound support
void cmdSoundDone();
bool cmdSoundPlay(const char* snd_name,bool loop=false);
void cmdSoundStop();
void cmdSoundSetVolume(int vol);

#endif
