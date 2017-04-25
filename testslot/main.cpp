/*
 * main.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: serg
 */

#include <gfx.h>

#include <cstdlib>
#include <ctime>

IMAGE Background = NULL;
IMAGE SpriteImage[10] = {};

const int MaxSpriteSymb = 10;
int aReelSymb[6][5];

const int Xbase = 51;
const int Ybase = 90 - 130;
const int YendReel = 480;
const int YbegReel = 90;

bool ProgramIsRunning(); //event process
bool LoadFiles();
void FreeFiles();

int main(int argc, char* args[])
{
	int X=0, Y=0;

	logInit();
	logTitle("Moving Image has started");
	bfInit();
	bfAddPath("res");
	// bfAddPath("/home/serg/test_sdl/test.slot/res");	// if prefer use global path

    if(!gfxInit(800,600))
	{
		SDL_Quit();
		logDone();
		return 1;
	}

	srand(time(NULL));

	if(!LoadFiles())
	{
		logMessage("Failed to load files!");
		FreeFiles();

		gfxDone();
		bfDone();
		logTitle("end");
		logDone();
		return 0;
	}

	for(int i=0; i < 6; i++)
		for(int j=0; j < 5; j++)
			aReelSymb[i][j] = rand()%10;

	X = Xbase ; Y = 0;

	while(ProgramIsRunning())
	{
		if(Y >= 130){
			Y = 0;
			for(int i=5; i>0; i--)
				for(int j=0; j<5; j++)
					aReelSymb[i][j] = aReelSymb[i-1][j];
			for(int i=0; i<5; i++)
				aReelSymb[0][i]  = rand()%10;
		}

		gfxPutImage(0, 0, Background);

		for(int row=0; row < 6; row++)
			for(int colmn=0; colmn < 5; colmn++)
			{				
				cRect rect(X+142*colmn, YbegReel, X+142*(colmn+1), YendReel);
				gfxPutImageA(X+142*colmn, row*130+YbegReel+Y-130, SpriteImage[aReelSymb[row][colmn]], &rect);
			}

		Y += 5;

//		SDL_Delay(5);
		gfxFlip();

	}
	FreeFiles();
	gfxDone();
	bfDone();
	logTitle("end");
	logDone();
	return 0;
}

bool LoadFiles()
{
	Background = imgLoad("backfon.png", gfxBpp(), IMG_SHARED);

	if(Background == NULL)
		return false;

	const char* picts[] = { "ace.png", "jack.png", "king.png", "pic1.png", "pic2.png", "pic3.png", "queen.png", "scatr.png", "ten.png", "top.png" }; 

	for(int i=0; i<10; i++)
	{
    	SpriteImage[i] = imgLoad(picts[i], gfxBpp(), IMG_SHARED);

		if(!SpriteImage[i])
			return false;
	}
	return true;
}

void FreeFiles()
{
	if(Background != NULL)
	{
		imgFree(Background);
		Background = NULL;
	}

	for(int i=0; i<10; i++)
		{
			imgFree(SpriteImage[i]);
			SpriteImage[i] = NULL;
		}
}

bool ProgramIsRunning()
{
	SDL_Event event;
	bool running = true;

	while(SDL_PollEvent(&event))
	{
		switch(event.type){
			case SDL_QUIT:	 {
				running = false;
				break;
			}
			case SDL_KEYDOWN: {
				running = false;
				break;
			}
		}
//		if(event.type == SDL_QUIT)
//			running = false;
	}
	return running;
}



