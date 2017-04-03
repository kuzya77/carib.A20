/*
	Entry point of Caribean poker A20 app
*/

#include <SDL/SDL.h>

#include "log.h"

int main(int argc, char* argv[])
{
	logInit();
	logTitle("Carib has started");

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0)
	{
		logError("Can't initialize SDL: %s\n", SDL_GetError());
		logDone();
		exit(1);
	}
	printf("Hello world!\n");

	logTitle("end");
	logDone();
	return 0;
}
