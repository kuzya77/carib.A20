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
		printf("Can't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	printf("Hello world!\n");

	logTitle("end");
	return 0;
}
