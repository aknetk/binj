#if INTERFACE
#include <Udon/Standard.h>

class Sound {
public:
    Uint8*      Buffer;
    Uint8*      BufferStart;
    Uint32      Length;
    const char* Filename = NULL;
};
#endif

#include <Udon/Audio/Sound.h>

#include <Udon/Application.h>
#include <Udon/Audio/Audio.h>

PUBLIC Sound::Sound(const char* filename) {
    // Resource* res = Resources::Load(filename);
    // if (!res) {
    //     Log::Print(Log::LOG_ERROR, "SOUND_FILE_MISSING: \"%s\"!", filename);
    //     exit(1);
    // }

    Filename = filename;
    SDL_AudioSpec format;
    // if (!SDL_LoadWAV_RW(res->RW, false, &format, &Buffer, &Length)) {
    //     Log::Print(Log::LOG_ERROR, "SOUND_OPEN_FAILED: %s", SDL_GetError());
    //     exit(1);
    // }

    SDL_AudioCVT convert;
    if (SDL_BuildAudioCVT(&convert,
        format.format, format.channels, format.freq,
        Audio::DeviceFormat.format, Audio::DeviceFormat.channels, Audio::DeviceFormat.freq) > 0) {
        convert.buf = (uint8_t*)malloc(Length * convert.len_mult);
        convert.len = Length;
        memcpy(convert.buf, Buffer, Length);
        SDL_free(Buffer);
        SDL_ConvertAudio(&convert);

        Buffer = convert.buf;
        Length = convert.len_cvt;
    }

    BufferStart = Buffer;

    // Resources::Close(res);
}

PUBLIC void Sound::Dispose() {
	if (BufferStart)
		SDL_free(BufferStart);
}
