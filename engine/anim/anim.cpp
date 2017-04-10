#include <types.h>
#include <mem.h>
#include <log.h>

#include "i_anim.h"
#include <stuff/cmd.h>

AnimLayer::AnimLayer(ParseNode* node):NamedObj(node)
{
	i_layer=attri(node,"layer",0);
	pos.x=pos.y=0;
}

AnimLayer::AnimLayer(int _layer, const char* _name):NamedObj(_name),i_layer(_layer)
{
    pos.x=pos.y=0;
}

AnimLayer::~AnimLayer() {}

bool AnimLayer::process(u32_t) 
{
    return false;
}

void 		AnimLayer::reset() {}
int 		AnimLayer::getCFrame() const { return 0; }
void 		AnimLayer::setCFrame(int frame) {}
const char* AnimLayer::getRcName()	{ return ""; }
//-------------------------------------------------------------------------------

int  Animation::getCFrame() const
{
	return currentFrame;
}

void Animation::setCFrame(int frame)
{
	if(frame<0)
		frame = ilist->count()-frame;
	if(frame<0 || frame>=ilist->count())
		frame = 0;
	currentFrame=currentLongFrame=frame;
}

const char* Animation::getRcName()
{
	return ilist->name();
}

void Animation::EventProcess(int maxFrame)
{
    const ImageList::eventsArray& ev=ilist->events;
    // Находим границы выполнения
    int i=currentEvent;
    for(;currentEvent<(int)ev.size() && ev[currentEvent].frame<=maxFrame;currentEvent++);

//    logDebug("Animation(\"%s\") events[%d,%d], Frame=%d",name(),i,currentEvent-1,maxFrame);

    for(int i_end=currentEvent;i<i_end;i++) // выполняем все до текущего кадра включительно
        ev[i].exec();    
}

void Animation::draw()
{
    ilist->draw(pos.x,pos.y,currentFrame);
}

bool Animation::process(u32_t t) // пересчет анимации, параметр - текущее время
{
    const int fps=ilist->fps();
    int newFrame=fps?(t-startTime)*u32_t(fps)/1000:currentFrame+1; // фича для кариба
    if(newFrame==currentLongFrame)  // ничего не делаем
        return true;    
//    logDebug("Animation(\"%s\") frame %d",name(),newFrame);
    currentLongFrame=newFrame;//!!
    
    if(newFrame>=ilist->count())
    {
        if(IsLoop)
        {
            newFrame=newFrame%ilist->count();                            //!!  
            if(currentFrame>newFrame)                                    //!!  
            {                                                            //!!  
                EventProcess(ilist->count());
                currentEvent=0;                                         //!!
            }                                                            //!!
            currentFrame=newFrame;                                       //!!
        }
        else
        {
            currentFrame=ilist->count()-1;
//            logDebug("Animation(\"%s\") cutted-end to frame %d",name(),currentFrame);
            EventProcess(currentFrame);
            return !currentLongFrame;
        }
    }
    else
        currentFrame=newFrame;
    // Перебираем события
    EventProcess(currentFrame);
    return true;
}

void Animation::reset()
{
    currentEvent=0;
    currentLongFrame=currentFrame=0;
}

Animation::Animation(ParseNode* node):AnimLayer(node),ilist(NULL)
{
    IsLoop=attri(node,"loop",0);

    const AttrList& p=attrlist(node,"pos");

    if(p.count()>1)
    {
        pos.x=p[0];
        pos.y=p[1];
    }

    const char* s=attrs(node,"imagelist");
    if(s)
        ilist=LoadImageList(s);
    else
    {
        ParseNode* ni=node->first("imagelist");
        if(ni)
            ilist=LoadImageList(ni);
    }
    if(!ilist)
        throw std::runtime_error("Animation::Animation error: bad imagelist");

    reset();  
}

Animation::~Animation()
{
    FreeImageList(ilist);
}

Animation* Animation::load(ParseNode* node)
{
    Animation* anim=NULL;
    try
    {   
        anim=new Animation(node);
        return anim;
    }
    catch(std::runtime_error& re)
    {
        logError("Animation::load error: %s", re.what());
    }
    catch(std::bad_alloc& ba)
    {
        logError("Animation::load error: %s",ba.what());
    }
    catch(...)
    {
        logError("Animation::load shit happens(...)");
    }
    delete anim;
    return NULL;
}

