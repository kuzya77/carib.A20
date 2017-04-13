#include <types.h>
#include <mem.h>
#include <log.h>

#include <stdexcept>

#include "sndlist.h"

SoundStorage::SoundStorage(const char* fname):IdentifiSound(0),i_volume(100)
{
    ParseNode* root = parLoadWXF(fname,512,512);
    if(!root)
        throw std::runtime_error("can't load sounds list");

    for(ParseNode* p=root->first("sound");p;p=p->next("sound"))
    {            
        const char* path=attrs(p,"file");
        if(!path)
        {
              parFree(root);
              throw std::runtime_error("empty path");
        }

        SoundRec sound;

        sound.id=intName(p->list().count()?attrs(p->list(),0):path);

        sound.snd=sndLoad(path);

        if(!sound.snd)
        {
              parFree(root);
              logDebug("Bad sound \"%s\"",path);
              throw std::runtime_error("bad file");
        }
        sounds.push_back(sound);
    }
    sort(sounds.begin(),sounds.end());
    parFree(root);
}

SoundStorage::~SoundStorage()
{
    for(size_t i=0; i<sounds.size(); i++)
        sndFree(sounds[i].snd);
//    sounds.clear();
}

void SoundStorage::SetVolume(int vlmsnd)
{
    if(vlmsnd>=0 && vlmsnd<=100)
        i_volume=vlmsnd;
}

void SoundStorage::stop()
{
    sndStop(IdentifiSound);
    IdentifiSound=0;
}

bool SoundStorage::play(const char *name,bool loop)
{
    if(name) 
    {
        SoundRec key;
        key.id=intName(name); 
        auto pr=equal_range(sounds.begin(),sounds.end(),key);                                                              //=sounds.equal_range(id_crc);
        if(pr.first!=pr.second) // ссылка найдена
        {
            stop();
            IdentifiSound=sndPlay(pr.first->snd,i_volume,loop);
            return true;
        }
    }
    return false;
}

//-------------------------------------------------------------------------------
static SoundStorage *cmdSnd=NULL;

bool cmdSoundPlay(const char* snd_name,bool loop)
{
    return cmdSnd?cmdSnd->play(snd_name,loop):false;
}

void cmdSoundStop()
{
    if(cmdSnd)
        cmdSnd->stop();
}

void cmdSoundSetVolume(int vol)
{
    if(cmdSnd)
        cmdSnd->SetVolume(vol);    
}


static void cmdSndPlay(const AttrList& list)
{
    if(!list.count())
    {
        conPrintf("SndPlay: need sound name");
        return;
    }

    const char* name=list[0];
    if(!cmdSoundPlay(name))
    {
        logDebug("cmdSndPlay: can't play \"%s\"",name);
        conPrintf("SndPlay: can't play \"%s\"",name);
    }
}

void cmdSndVol(const AttrList& list)
{
    if(list.count())
        cmdSoundSetVolume(list[0]);
    else
        conPrintf("cmdSndVol error: no params");
}

void cmdSndStop(const AttrList& list)        
{
    cmdSoundStop();
}


bool cmdSoundInit(const char* file_list)
{
    cmdSoundDone();

    try
    {
        cmdSnd=new SoundStorage(file_list);

        cmdAddCmd("SndPlay",cmdSndPlay);
        cmdAddCmd("SndVol",cmdSndVol);
        cmdAddCmd("SndStop",cmdSndStop);

        return true;
    }
    catch(std::bad_alloc& ba)
    {
        logError("cmdSoundInit  error: %s",ba.what());
        conPrintf("cmdSoundInit  error: %s",ba.what());
    }
    catch(std::runtime_error& re)
    {
        logError("cmdSoundInit::load error: \"%s\"",re.what());
        conPrintf("cmdSoundInit::load error: \"%s\"\n",re.what());
    }

    catch(...)
    {
        logError("cmdSoundInit fucking shit ...");
    }

    cmdSoundDone();

    return false;
}

void cmdSoundDone()
{
      delete cmdSnd;
      cmdSnd=NULL;
}
