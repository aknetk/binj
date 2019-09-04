#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Decoder;
class MediaSource;

#include <Udon/Standard.h>
#include <Udon/Media/Decoder.h>
#include <Udon/Media/MediaSource.h>
#include <Udon/Rendering/Texture.h>

class MediaPlayer {
public:
    enum {
    KIT_STOPPED = 0, ///< Playback stopped or has not started yet.
    KIT_PLAYING,     ///< Playback started & player is actively decoding.
    KIT_PAUSED,      ///< Playback paused; player is actively decoding but no new data is given out.
    KIT_CLOSED,      ///< Playback is stopped and player is closing.
    KIT_BUFFERING_PLAYING,         KIT_BUFFERING_PAUSED,
    }; 
    Uint32       State;
    Decoder*     Decoders[3];
    SDL_Thread*  DecoderThread;
    SDL_mutex*   DecoderLock;
    MediaSource* Source;
    double       PauseStarted;
    double       PausedPosition;

    static int          DemuxStream(MediaPlayer* player);
    static Uint32       GetInputLength(MediaPlayer* player, int i);
    static Uint32       GetOutputLength(MediaPlayer* player, int i);
    static bool         IsInputEmpty(MediaPlayer* player);
    static bool         IsOutputEmpty(MediaPlayer* player);
    static int          RunDecoder(MediaPlayer* player);
    static int          DecoderThreadFunc(void* ptr);
    static MediaPlayer* Create(MediaSource* src, int video_stream_index, int audio_stream_index, int subtitle_stream_index, int screen_w, int screen_h);
           void         Close();
           void         SetScreenSize(int w, int h);
           int          GetVideoStream();
           int          GetAudioStream();
           int          GetSubtitleStream();
           int          GetVideoData(Texture* texture);
           int          GetVideoDataForPaused(Texture* texture);
           int          GetAudioData(unsigned char* buffer, int length);
           int          GetSubtitleData(Texture* texture, SDL_Rect* sources, SDL_Rect* targets, int limit);
           void         GetInfo(PlayerInfo* info);
           void         SetClockSync();
           void         ChangeClockSync(double delta);
           Uint32       GetPlayerState();
           void         Play();
           void         Stop();
           void         Pause();
           int          Seek(double seek_set);
           double       GetDuration();
           double       GetPosition();
           double       GetBufferPosition();
};

#endif /* MEDIAPLAYER_H */
