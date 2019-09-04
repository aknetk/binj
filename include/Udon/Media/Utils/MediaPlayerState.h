#ifndef MEDIAPLAYERSTATE_H
#define MEDIAPLAYERSTATE_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Media/Decoders/Subtitle/Libass.h>

class MediaPlayerState {
public:
    static Uint32       InitFlags;
    static Uint32       ThreadCount;
    static Uint32       FontHinting;
    static Uint32       VideoBufFrames;
    static Uint32       AudioBufFrames;
    static Uint32       SubtitleBufFrames;
    static ASS_Library* libassHandle;
    static void*        assSharedObjectHandle;

    static double GetSystemTime();
    static bool   AttachmentIsFont(void* p);
};

#endif /* MEDIAPLAYERSTATE_H */
