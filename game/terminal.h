#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "carib.h"

namespace Terminal
{
	bool Init();
	void Done();

	void send(const NetworkMsg& msg);	// msg: * -> terminal
	void process();

	void keyPress(SDLKey key);
}

#endif