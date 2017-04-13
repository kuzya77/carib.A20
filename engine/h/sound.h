#ifndef __BLITZ_SOUND_H__
#define __BLITZ_SOUND_H__

#include <types.h>

class sndItem;

typedef sndItem* SOUND;

bool sndInit();
void sndDone();

SOUND 	sndLoad(const char* fname);
void 	sndFree(SOUND snd);

u32_t	sndPlay(SOUND snd,int vol=-1,bool loop=false);	// vol_def (no change) if -1
bool	sndPlaying(u32_t ch);

bool	sndStop(u32_t ch);

bool	sndSetVolume(u32_t handle,int vol);	// vol=[0..100]
int	sndGetVolume(u32_t handle);

bool	sndSetVolume(int vol);
int	sndGetVolume();

#endif
