#ifndef __ANIM_MOVE_CONTROLLER__
#define __ANIM_MOVE_CONTROLLER__

#include <types.h>
#include <mem.h>

class StaticLayer;

class MoveController
{
	StaticLayer* _layer;
	int _id;	
	float curx, cury;
	u32_t t0;
public:
	float speed;
	float destx, desty;

	MoveController(StaticLayer*, int id, int x, int y, float spd);
	bool process(u32_t t);
};

#endif
