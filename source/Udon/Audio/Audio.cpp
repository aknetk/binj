#if INTERFACE
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
};
#endif

#include <Udon/Audio/Audio.h>

#include <Udon/Application.h>
#include <Udon/Logging/Log.h>

SDL_AudioDeviceID Audio::Device;
SDL_AudioSpec     Audio::DeviceFormat;
vector<Sound*>    Audio::SoundQueue;
void*             Audio::Userdata = NULL;
SDL_AudioCallback Audio::Callback;
bool              Audio::HasCallback = false;

PUBLIC STATIC void Audio::Init() {
    SDL_AudioSpec want;
    memset(&want, 0, sizeof(want));
    want.channels = 2;
    want.freq = 48000;
    want.format = 0x8010;
    // want.samples = 0x400;
    // want.callback = Audio::AudioCallback;
    // want.userdata = NULL;

    // // Open audio device.
    // Device = SDL_OpenAudioDevice(NULL, 0, &want, &Audio::DeviceFormat, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
    // if (!Device) {
    //     Log::Print(Log::LOG_ERROR, "AUDIO_DEVICE_NOINIT: %s", SDL_GetError());
    //     exit(1);
    // }
    //
    // // Enable audio device.
    // SDL_PauseAudioDevice(Device, 0);

    if (SDL_OpenAudio(&want, &Audio::DeviceFormat) >= 0)
        SDL_PauseAudio(0);
    else
        Log::Print(Log::LOG_ERROR, "Could not open audio device!");
}

PUBLIC STATIC void Audio::Play(Sound* sound) {
    SDL_LockAudio();// SDL_LockAudioDevice(Device);
    Audio::SoundQueue.push_back(sound);
    SDL_UnlockAudio(); // SDL_UnlockAudioDevice(Device);
}

PUBLIC STATIC void Audio::AudioCallback(void* data, uint8_t* stream, int len) {
    memset(stream, 0, len);

    if (Audio::HasCallback) {
        Audio::Callback(Audio::Userdata, stream, len);
    }

    for (int i = 0; i < Audio::SoundQueue.size(); i++) {
        if (Audio::SoundQueue[i]->Buffer) {
            if (Audio::SoundQueue[i]->Length > 0) {
                int maxLen = ((uint32_t)len > Audio::SoundQueue[i]->Length) ? Audio::SoundQueue[i]->Length : (uint32_t)len;

                SDL_MixAudioFormat(stream, Audio::SoundQueue[i]->Buffer, DeviceFormat.format, maxLen, 0xFF);

                Audio::SoundQueue[i]->Buffer += maxLen;
                Audio::SoundQueue[i]->Length -= maxLen;
            }

            if (Audio::SoundQueue[i]->Length == 0) {
                // Audio::SoundQueue[i]->Buffer = NULL;
                // if (audio->Loop[i]) {
                    Audio::SoundQueue[i]->Length = Audio::SoundQueue[i]->Buffer - Audio::SoundQueue[i]->BufferStart;
                    Audio::SoundQueue[i]->Buffer = Audio::SoundQueue[i]->BufferStart;
                // }
                Audio::SoundQueue.erase(Audio::SoundQueue.begin() + i);
            }
        }
    }
}

PUBLIC STATIC void Audio::Dispose() {
    SDL_PauseAudio(1);
    SDL_CloseAudio(); // SDL_CloseAudioDevice(Device);
}
