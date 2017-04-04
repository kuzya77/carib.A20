/*
	Entry point of Caribean poker A20 app
*/

#include <SDL/SDL.h>

#include "binfile.h"

int main(int argc, char* argv[])
{
	logInit();	
	logTitle("Carib has started");
	bfInit();

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0)
	{
		logError("Can't initialize SDL: %s\n", SDL_GetError());
		logDone();
		exit(1);
	}
	printf("Hello world!\n");

	SDL_Surface* screen = SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE);
	if(screen==NULL)
	{
		logError("Couldn't set display mode: %s", SDL_GetError());
		SDL_Quit();
		logDone();
		exit(1);
	}

	logMessage("Screen mode is %d x %d", screen->w, screen->h);

	int i = 0;
	for(bool done=false;!done; i++)
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

		if(SDL_LockSurface(screen)>=0)
		{
			uint32* buffer = (uint32*)screen->pixels;
			SDL_Rect rect = {400, 300, 200, 150};
			for(int y=0; y<screen->h; y++)
				for(int x=0; x<screen->w; x++)
					*buffer++ = x*(y+i);
			SDL_UnlockSurface(screen);
			SDL_FillRect(screen, &rect, 0x00FF0000);
			SDL_Flip(screen);
		}
	}

	SDL_Quit();
	bfDone();
	logTitle("end");
	logDone();
	return 0;
}
