#include <types.h>
#include <mem.h>
#include <log.h>
#include <sound.h>
#include <binfile.h>

#include "istuff.h"
#include "istl.h"

#include <SDL/SDL_mixer.h>

class sndSystem:public baseSystem<sndItem>
{
public:
    sndSystem() {}
};

class sndItem
{
public:
	Mix_Chunk* hsound;

    sndSystem::tListNode node;
    void Free()
        {
            NULL_DESTROY(Mix_FreeChunk,hsound);	// Note: It's a bad idea to free a chunk that is still being played...
        }
    void Stop()
        {
// TODO: implement it
//            if(hsound) BASS_SampleStop(hsound);
        }
    sndItem(Mix_Chunk* h):node(this),hsound(h) {}
    ~sndItem() { Stop(); Free(); node.remove(); }
};

sndSystem sndSys;

bool sndInit()
{
    if(sndSys.IsInit)
        return true;

	int flags=MIX_INIT_OGG|MIX_INIT_MOD|MIX_INIT_MP3;
	int initted=Mix_Init(flags);
	if(initted&flags != flags) 
	{
		logWarning("sndInit(): Mix_Init flags are different: %x vs %x", flags, initted);
    	logWarning("\terror desc: %s", Mix_GetError());
	}

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024)==-1) 
	{
    	logError("sndInit(): Mix_OpenAudio: %s\n", Mix_GetError());
    	Mix_Quit();
    	return false;
	}

	sndSys.IsInit=true;
	return true;
}

	
void sndDone()
{
    if(sndSys.IsInit)
    {
        logMessage("sndSys::sndDone(Count=%d)",sndSys.Count);
        Mix_HaltChannel(-1);
        while(sndSys.first())
            sndFree((SOUND)sndSys.first()->owner);
        Mix_CloseAudio();
		// force a quit
		while(Mix_Init(0))
		    Mix_Quit();
        sndSys.IsInit=false;
    }
}

SOUND sndLoad(const char* fname)
{   
	if(!sndInit())
    	return NULL;
    BINFILE f=bfOpen(fname,OPEN_RO);
    if(!f)
        return NULL;
    size_t size=bfSize(f);
    void* buff=sndSys.alloc(size);
    if(buff)
        bfRead(f, buff, size);
    bfClose(f);
    if(!buff)
    {
    	logError("sndLoad(\"%s\") not enough memory:%d", fname, size);
        return NULL;
    }

	SDL_RWops* dataPtr = SDL_RWFromConstMem(buff, size);
	if(!dataPtr)
	{
		logError("sndLoad(\"%s\") couldnt create SDL_RWops", fname);
		sndSys.free(buff);
		return NULL;
	}

	Mix_Chunk *hsound = Mix_LoadWAV_RW(dataPtr, true);
	if(!hsound)
	{
		logError("sndLoad(\"%s\"): Mix_LoadWAV_RW: %s\n", fname, Mix_GetError());
	}

	sndSys.free(buff);

    sndItem* snd=NULL;
    if(hsound)
    {
        snd=new sndItem(hsound);
        sndSys.Register(snd);
    }
    return snd;
}

void sndFree(SOUND snd)
{
    if(!(snd && sndSys.IsInit))
        return;
    // Проверка на наличие в цепочке
    if(sndSys.IsRegistered(snd))
    {
        sndSys.Count--;
        delete snd;
    }
    else
        logWarning("sndFree: warning: incorrect sound handler %08X", snd);
}

u32_t sndPlay(SOUND snd, int vol, bool loop)
{
    u32_t ch=0;
    if(sndSys.IsInit && snd)
    {
        if(snd->hsound) // Sound sample
        {
        	ch = Mix_PlayChannel(-1, snd->hsound, loop?-1:0)+1;
        	if(!ch)
        	{
				logError("sndPlay error: Mix_PlayChannel: %s\n",Mix_GetError());
				return 0;
			}
        }
    }
    return ch;
}

bool sndPlaying(u32_t handle)
{
    return handle>0 && Mix_Playing(handle-1)>0;
}

bool sndStop(u32_t handle)             // =NULL - stop all
{
	Mix_HaltChannel(handle-1);
	return true;
}

bool sndSetVolume(u32_t handle, int vol)    // vol=[0..100]
{
//    return BASS_ChannelSetAttributes(handle,-1,vol,-101);
	return false;
}

int sndGetVolume(u32_t handle)
{
//    DWORD volume;
//    return BASS_ChannelGetAttributes(handle,NULL,&volume,NULL)?volume:-1;
	return 0;
}

bool sndSetVolume(int vol)
{   
//    return BASS_SetVolume(vol);
	return false;
}

int sndGetVolume()
{
//    return BASS_GetVolume();
	return 0;
}
