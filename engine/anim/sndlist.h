#ifndef ___SNDLIST_H__
#define ___SNDLIST_H__

#include <types.h>
#include <mem.h>

#include <sound.h>

#include <stuff.h>
#include <stuff/parser.h>
#include <stuff/menu.h>
#include <stuff/cmd.h>

#include <vector>

inline u32_t intName(const char *name) { return crc32(name,strlen(name)); }

class SoundRec
{
public:
    u32_t id;
    SOUND snd;
    SoundRec():id(0),snd(NULL) {}
    bool operator < (const SoundRec& sr) const { return id<sr.id; }
};

class SoundStorage
{
protected:
      u32_t   IdentifiSound;
      int     i_volume;

      std::vector<SoundRec>  sounds;
public:
      SoundStorage(const char* fname);

      bool play(const char *name,bool loop);
      void SetVolume(int vlmsnd);
      void stop();
      ~SoundStorage();
};

bool cmdSoundInit(const char* file_list);//Поддержка проигрывания звука из commandа
void cmdSoundDone();// освобождение ресурса
bool cmdSoundPlay(const char* snd_name,bool loop=false);
void cmdSoundStop();
void cmdSoundSetVolume(int vol);

#endif
