/*
	Entry point of Caribean poker A20 app
*/

#include "carib.h"
#include "terminal.h"

bool Init()
{
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

	return Terminal::Init();
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
	
	// init 2nd screen (or head)
	gfxOpen2nd("/dev/fb1");
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
				printf("key up[0x%x](%x): '%c'\n", event.key.keysym.scancode, event.key.keysym.mod, event.key.keysym.unicode);
				if(event.key.keysym.sym==SDLK_ESCAPE)
					done = true;
				switch(event.key.keysym.sym)
				{
					case SDLK_1:
					{	// probe
						NetworkMsg msg;
						msg.state.from = 0;
						msg.state.to = 2;
						msg.state.cmd = NCMD_T_UPDATE_STATE;
						msg.state.tstate.game = 1;           // 1 - 1я раздача, 2- 2я, и т.д. 0- начало игры или вне игры (paid==0)
						msg.state.tstate.credit = 15;
						msg.state.tstate.win = 17;
    					msg.state.tstate.LastWin = 42;
   						msg.state.tstate.paid = 24;
    					msg.state.tstate.ante = 50;           // текущее значение ставки
   						msg.state.tstate.cards[0] = 1;       // 0 - карты нет, 1-52 карты, 53 - джокер, 54 - рубашка, +100 - hold
   						msg.state.tstate.cards[1] = 101;
   						msg.state.tstate.cards[2] = 53;
   						msg.state.tstate.cards[3] = 54;
   						msg.state.tstate.cards[4] = 153;
   						Terminal::send(msg);
						break;
					}
					case SDLK_2:
					{	// probe
						NetworkMsg msg;
						msg.state.from = 0;
						msg.state.to = 2;
						msg.state.cmd = NCMD_T_UPDATE_STATE;
						msg.state.tstate.game = 2;           // 1 - 1я раздача, 2- 2я, и т.д. 0- начало игры или вне игры (paid==0)
						msg.state.tstate.credit = 15;
						msg.state.tstate.win = 17;
    					msg.state.tstate.LastWin = 42;
   						msg.state.tstate.paid = 24;
    					msg.state.tstate.ante = 100;           // текущее значение ставки
   						msg.state.tstate.cards[0] = 1;       // 0 - карты нет, 1-52 карты, 53 - джокер, 54 - рубашка, +100 - hold
   						msg.state.tstate.cards[1] = 101;
   						msg.state.tstate.cards[2] = 54;
   						msg.state.tstate.cards[3] = 53;
   						msg.state.tstate.cards[4] = 153;
   						Terminal::send(msg);
						break;
					}
				}
				Terminal::keyPress(event.key.keysym.sym);
			}
		}

		// Terminal process:
		//gfxSet2ndScreen(false);
		Terminal::process();
		animProcess();
        animDraw();
        gfxFlip();
        SDL_Delay(10);
	}

	gfxDone();
	bfDone();
	logTitle("end");
	logDone();
	return 0;
}
