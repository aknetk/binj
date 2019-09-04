#ifndef AUDIO_H
#define AUDIO_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Audio/Sound.h>

class Audio {
public:
    static SDL_AudioDeviceID Device;
    static SDL_AudioSpec     DeviceFormat;
    static vector<Sound*>    SoundQueue;
    static void*             Userdata;
    static SDL_AudioCallback Callback;
    static bool              HasCallback;

    static void Init();
    static void Play(Sound* sound);
    static void AudioCallback(void* data, uint8_t* stream, int len);
    static void Dispose();
};

#endif /* AUDIO_H */
