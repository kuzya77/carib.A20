#include <types.h>
#include <mem.h>

#include "move_controller.h"
#include "static_layer.h"
#include <math.h>

inline float distance(float x, float y)
{
	return sqrt(x*x+y*y);
}

MoveController::MoveController(StaticLayer* layer, int id, int x, int y, float sp)
{
	_layer = layer;
	slElement* el=layer->get(id);
	t0 = 0;
	_id=id;
	if(el)
	{
		tPoint p = el->getpos();
		curx = p.x;
		cury = p.y;
	}
	else	
		curx=cury=0;

	destx = x;
	desty = y;
	speed = sp;
}

bool MoveController::process(u32_t t)
{
	slElement* el=_layer->get(_id);
	if(!el)
		return false;
	if(!t0)
	{
		t0 = t;
		return true;
	}
	float dist = distance(destx-curx, desty-cury);
	int dt = t-t0;
	t0 = t;

	float cur_speed=speed*dt;

	if(cur_speed>=dist)
	{
		el->setpos(destx, desty);
		return false;
	}
	float k = cur_speed/dist;
	curx += (destx-curx)*k;
	cury += (desty-cury)*k;
	el->setpos(curx, cury);
	return true;
}
