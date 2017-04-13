/*
	Entry point of Caribean poker A20 app
*/

#include "carib.h"
#include "terminal.h"
#include "head.h"

HeadStateWord hsw;

bool Init()
{
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, 50);
	bfAddPath("rc");
	intro = imgLoad("intro.jpg", gfxBpp(), IMG_SHARED);
	conInit(gfxWidth()/GFX_FONT_WIDTH,gfxHeight()/GFX_FONT_HEIGHT);
	if(intro)
	{
		gfxPutImage(0, 0, intro);
		gfxFlip();
		gfxSet2ndScreen(true);
		gfxPutImage(0, 0, intro);
		gfxFlip();
		gfxSet2ndScreen(false);
	}

   	print(WHITE, "AnimScene load...");
    if(animLoad("anim.wxf"))
    	print(GREEN, "Ok\n");
    else
    {
    	print(RED, ". Failed\n");
    	return false;
    }

    print(WHITE, "cmdSound...");
    if(cmdSoundInit("sound.wxf"))
    	print(GREEN, "Ok\n");
    else
    {
    	print(RED, ". Failed\n");
    	return false;
    }

	return Terminal::Init() && Head::Init();
}

void Done()
{
    Head::BlackBoxWriteState();
	NULL_DESTROY(imgFree,intro);
	cmdSoundDone();
}

int main(int argc, char* argv[])
{
	logInit();	
	logTitle("Carib has started");
	bfInit();

    if(!gfxInit(800,600))
	{
		SDL_Quit();
		logDone();
		return 1;
	}

	sndInit();

	// init 2nd screen (or head)
	gfxOpen2nd("/dev/fb1");
	animInit();

	Init();

	// Start terminal part
    animSelectScene("main");
    animPlay("bkground");
    animShow("credit");
    animShow("win");
    animShow("lastwin");
	animShow("paid");
	animPlay("ante10");

	for(bool done=false;!done;)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type & SDL_KEYUP)
			{
				if(event.key.keysym.sym==SDLK_ESCAPE)
					done = true;
				Terminal::keyPress(event.key.keysym.sym);
			}
		}

		// Terminal process:
		//gfxSet2ndScreen(false);
		Terminal::process();
		Head::process();
		animProcess();
        animDraw();
        gfxFlip();
        gfxSet2ndScreen(true);
        gfxFlip();
        gfxSet2ndScreen(false);
//        SDL_Delay(10);
	}

	Done();
	animDone();
	sndDone();
	gfxDone();
	bfDone();
	logTitle("end");
	logDone();
	return 0;
}
