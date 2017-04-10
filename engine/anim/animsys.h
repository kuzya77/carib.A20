#ifndef __ANIM_SYS_H__
#define __ANIM_SYS_H__

#include <types.h>
#include <mem.h>

#include "animscene.h"

class AnimSys
{
protected:
    typedef std::unordered_map<u32_t,AnimScene*> ASceneArray;
    ASceneArray scenes;

    AnimScene* GetNode(const char* name);
public:
    AnimSys():CurrentScene(NULL) {}

    AnimScene*  CurrentScene;

    bool select(const char* scene);
    void process()                      { if(CurrentScene) CurrentScene->process(); }
    void draw()                         { if(CurrentScene) CurrentScene->draw();    }
    void reset()                        { if(CurrentScene) CurrentScene->reset();   }
    bool exist(const char* name)        { return CurrentScene?CurrentScene->exist(name):false; }
    void hide(const char* name)         { if(CurrentScene) CurrentScene->hide(name); }
    bool show(const char* name, int i)  { return CurrentScene?CurrentScene->show(name, i):false; }
    bool play(const char* name)         { return CurrentScene?CurrentScene->play(name):false; }
    bool merge(const char* name, const char* prev_anim)         
    		{ return CurrentScene && CurrentScene->merge(name, prev_anim); }
    bool bind(AnimLayer* usr)           { return CurrentScene?CurrentScene->bind(usr):false; }
    bool load(const char* file);

    void setValue(const char* name, const char* value)
    									{ if(CurrentScene) CurrentScene->setValue(name, value); }
	const char* getValue(const char* name)
										{ return CurrentScene?CurrentScene->getValue(name):NULL; }
    ~AnimSys();
};

#endif
