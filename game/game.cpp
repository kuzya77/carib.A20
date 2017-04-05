/*
	Entry point of Caribean poker A20 app
*/

#include <SDL/SDL.h>

#include "picture.h"

int main(int argc, char* argv[])
{
	logInit();	
	logTitle("Carib has started");
	bfInit();

	IMAGE bg = imgLoad("mortal_kombat_001.jpg", BPP_32bit, IMG_SHARED);
	if(bg==NULL)
	{
		logError("Can't load mortal_kombat_001.jpg");
	}

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)<0)
	{
		logError("Can't initialize SDL: %s\n", SDL_GetError());
		logDone();
		exit(1);
	}
	printf("Hello world!\n");

	SDL_Surface* screen = SDL_SetVideoMode(1024, 768, 32, SDL_SWSURFACE);
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
			if((i&32) && bg)
				memcpy(buffer, imgBuffer(bg), screen->w*screen->h*4);
			else
				for(int y=0; y<screen->h; y++)
				{
					for(int x=0; x<screen->w; x++)
						*buffer++ = x*(y+i);
				}
			SDL_UnlockSurface(screen);
			SDL_Flip(screen);
		}
	}

	SDL_Quit();
	bfDone();
	logTitle("end");
	logDone();
	return 0;
}
