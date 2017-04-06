/*
	Entry point of Caribean poker A20 app
*/

#include <gfx.h>

int main(int argc, char* argv[])
{
	logInit();	
	logTitle("Carib has started");
	bfInit();

	if(!gfxInit(1024, 768, 32))
	{
		SDL_Quit();
		logDone();
		return 1;
	}


	IMAGE bg = imgLoad("mortal_kombat_001.jpg", BPP_32bit, IMG_SHARED);
	if(bg==NULL)
	{
		logError("Can't load mortal_kombat_001.jpg");
	}

	SDL_Surface* screen = SDL_GetVideoSurface();

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
				gfxPutImage(0,0, bg);
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
