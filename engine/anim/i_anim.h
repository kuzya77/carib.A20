#ifndef __I_ANIM_H__
#define __I_ANIM_H__

#include <types.h>
#include <mem.h>


#include "imagelist.h"

class Animation:public AnimLayer
{
protected:
    ImageList* ilist;

    bool IsLoop;
    bool secondScreen;	// uses second screen

    int currentFrame,
        currentLongFrame;   // номер текущего кадра
    int currentEvent;   // номер текущего события

    void EventProcess(int maxFrame);    // Обработка до указанного кадра

public:
    int  	getCFrame() const;	// get current frame.
    void 	setCFrame(int frame);
    const char* getRcName();

    void 	draw();
    bool 	process(u32_t t); // пересчет анимации, параметр - текущее время,
                            // return true, если анимация не завершена
    void 	reset();           // Сброс анимации в нулевое состояние

    Animation(ParseNode*);
    virtual ~Animation();

    static Animation* load(ParseNode* node);
};

#endif
