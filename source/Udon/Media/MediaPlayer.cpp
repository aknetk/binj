#if INTERFACE
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
        KIT_BUFFERING_PLAYING,
        KIT_BUFFERING_PAUSED,
    };

    Uint32       State;
    Decoder*     Decoders[3];
    SDL_Thread*  DecoderThread;
    SDL_mutex*   DecoderLock;
    MediaSource* Source;
    double       PauseStarted;
    double       PausedPosition;
};
#endif

#include <Udon/Media/MediaPlayer.h>
#include <Udon/Media/Decoders/VideoDecoder.h>
#include <Udon/Media/Decoders/AudioDecoder.h>
#include <Udon/Media/Decoders/SubtitleDecoder.h>
#include <Udon/Media/Utils/MediaPlayerState.h>

#include <Udon/Logging/Clock.h>
#include <Udon/Logging/Log.h>

enum DecoderIndex {
    KIT_VIDEO_DEC = 0,
    KIT_AUDIO_DEC,
    KIT_SUBTITLE_DEC,
    KIT_DEC_COUNT
};

// Return  0 if stream is good but nothing else to do for now.
// Return -1 if there may still work to be done.
// Return  1 if there was an error or stream end.
PUBLIC STATIC int          MediaPlayer::DemuxStream(MediaPlayer* player) {
    assert(player != NULL);
    AVFormatContext* format_ctx = (AVFormatContext*)player->Source->FormatCtx;

    // If any buffer is full, just stop here for now.
    // Since we don't know what kind of data is going to come out of av_read_frame, we really
    // want to make sure we are prepared for everything.
    for (int i = 0; i < KIT_DEC_COUNT; i++) {
        Decoder* dec = (Decoder*)player->Decoders[i];
        if (dec == NULL)
            continue;
        if (!dec->CanWriteInput())
            return 0;
    }

    // Attempt to read frame. Just return here if it fails.
    int ret;
    AVPacket* packet = av_packet_alloc();
    if ((ret = av_read_frame(format_ctx, packet)) < 0) {
        av_packet_free(&packet);
        // char errorstr[256];
        // av_strerror(ret, errorstr, sizeof(errorstr));
        // Log::Print(Log::LOG_ERROR, "ret: (%s)", errorstr);
        // Log::Print(Log::LOG_INFO, "pos: %f / %f", player->GetPosition(), player->GetDuration());
        return 1;
    }

    // Check if this is a packet we need to handle and pass it on
    for (int i = 0; i < KIT_DEC_COUNT; i++) {
        Decoder* dec = (Decoder*)player->Decoders[i];
        if (dec == NULL)
            continue;
        if (dec->StreamIndex == packet->stream_index) {
            dec->WriteInput(packet);
            return -1;
        }
    }

    // We only get here if packet was not written to a decoder. IF that is the case,
    // disregard and free the packet.
    av_packet_free(&packet);
    return -1;
}

PUBLIC STATIC Uint32       MediaPlayer::GetInputLength(MediaPlayer* player, int i) {
    Decoder* dec = (Decoder*)player->Decoders[i];
    if (dec == NULL)
        return 0;
    return dec->GetInputLength();
}
PUBLIC STATIC Uint32       MediaPlayer::GetOutputLength(MediaPlayer* player, int i) {
    Decoder* dec = (Decoder*)player->Decoders[i];
    if (dec == NULL)
        return 0;
    return dec->GetInputLength();
}

PUBLIC STATIC bool         MediaPlayer::IsInputEmpty(MediaPlayer* player) {
    for (int i = 0; i < KIT_DEC_COUNT; i++) {
        Decoder* dec = (Decoder*)player->Decoders[i];
        if (dec == NULL)
            continue;
        if (dec->PeekInput())
            return false;
    }
    return true;
}
PUBLIC STATIC bool         MediaPlayer::IsOutputEmpty(MediaPlayer* player) {
    for (int i = 0; i < KIT_DEC_COUNT; i++) {
        Decoder* dec = (Decoder*)player->Decoders[i];
        if (dec == NULL)
            continue;
        if (dec->PeekOutput())
            return false;
    }
    return true;
}
PUBLIC STATIC int          MediaPlayer::RunDecoder(MediaPlayer* player) {
    int got;
    bool has_room = true;

    do {
        while ((got = MediaPlayer::DemuxStream(player)) == -1);

        // BUG: Fixed?
        // if the demuxer cannot read any more packets (got == 1),
        // AND we've run out of our decoded frames, this means we've KIT_STOPPED
        // This is a problem if the actual video hasn't reached the end
        if (got == 1 && MediaPlayer::IsInputEmpty(player) && MediaPlayer::IsOutputEmpty(player)) {
            return 1;
        }

        for (int i = 0; i < KIT_DEC_COUNT; i++) {
            if (player->Decoders[i])
                while (((Decoder*)player->Decoders[i])->Run() == 1);
        }

        // If there is no room in any decoder input, just stop here since it likely means that
        // at least some decoder output is full.
        for (int i = 0; i < KIT_DEC_COUNT; i++) {
            Decoder* dec = (Decoder*)player->Decoders[i];
            if (dec == NULL)
                continue;
            if (!dec->CanWriteInput() || got == 1) {
                has_room = false;
                break;
            }
        }
    }
    while (has_room);

    if (player->State == KIT_BUFFERING_PLAYING) {
        player->State = KIT_PLAYING;
        printf("%s\n", "done buffer_paying");
    }
    if (player->State == KIT_BUFFERING_PAUSED) {
        player->State = KIT_PAUSED;
        printf("%s\n", "done buffer_pause");
    }

    return 0;
}
PUBLIC STATIC int          MediaPlayer::DecoderThreadFunc(void* ptr) {
    MediaPlayer* player = (MediaPlayer*)ptr;
    bool is_running = true;
    bool is_playing = true;

    while (is_running) {
        if (player->State == KIT_CLOSED) {
            is_running = false;
            continue;
        }
        if (player->State == KIT_PLAYING) {
            is_playing = true;
        }
        while (is_running && is_playing) {
            // Grab the decoder lock, and run demuxer & decoders for a bit.
            if (SDL_LockMutex((SDL_mutex*)player->DecoderLock) == 0) {
                if (player->State == KIT_CLOSED) {
                    is_running = false;
                    goto end_block;
                }
                if (player->State == KIT_STOPPED) {
                    is_playing = false;
                    goto end_block;
                }
                if (MediaPlayer::RunDecoder(player) == 1) {
                    player->State = KIT_STOPPED;
                    goto end_block;
                }

                end_block:
                SDL_UnlockMutex((SDL_mutex*)player->DecoderLock);
            }
            // Delay to make sure this thread does not hog all cpu
            SDL_Delay(2);
        }
        // Just idle while waiting for work.
        SDL_Delay(25);
    }
    return 0;
}

PUBLIC STATIC MediaPlayer* MediaPlayer::Create(MediaSource* src, int video_stream_index, int audio_stream_index, int subtitle_stream_index, int screen_w, int screen_h) {
    assert(src != NULL);
    assert(screen_w >= 0);
    assert(screen_h >= 0);

    MediaPlayer* player;

    if (!MediaPlayerState::libassHandle) {
        #ifdef USE_DYNAMIC_LIBASS
            MediaPlayerState::assSharedObjectHandle = SDL_LoadObject(DYNAMIC_LIBASS_NAME);
            if (MediaPlayerState::assSharedObjectHandle == NULL) {
                Log::Print(Log::LOG_ERROR, "Unable to load ASS library");
                return NULL;
            }
            load_libass(MediaPlayerState::assSharedObjectHandle);
        #endif
        MediaPlayerState::libassHandle = ass_library_init();
    }

    if (video_stream_index < 0 && subtitle_stream_index >= 0) {
        Log::Print(Log::LOG_ERROR, "Subtitle stream selected without video stream");
        goto exit_0;
    }

    player = (MediaPlayer*)calloc(1, sizeof(MediaPlayer));
    if (player == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to allocate player");
        goto exit_0;
    }

    // Initialize video decoder
    if (video_stream_index >= 0) {
        player->Decoders[KIT_VIDEO_DEC] = new VideoDecoder(src, video_stream_index);
        if (player->Decoders[KIT_VIDEO_DEC] == NULL) {
            goto exit_2;
        }
    }

    // Initialize audio decoder
    if (audio_stream_index >= 0) {
        player->Decoders[KIT_AUDIO_DEC] = new AudioDecoder(src, audio_stream_index);
        if (player->Decoders[KIT_AUDIO_DEC] == NULL) {
            goto exit_1;
        }
    }

    // Initialize subtitle decoder.
    if (subtitle_stream_index >= 0) {
        OutputFormat output;
        ((VideoDecoder*)player->Decoders[KIT_VIDEO_DEC])->GetOutputFormat(&output);
        player->Decoders[KIT_SUBTITLE_DEC] = new SubtitleDecoder(src, subtitle_stream_index, output.Width, output.Height, screen_w, screen_h);
        if (player->Decoders[KIT_SUBTITLE_DEC] == NULL) {
            goto exit_2;
        }
    }

    // Decoder thread lock
    player->DecoderLock = SDL_CreateMutex();
    if (player->DecoderLock == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to create a decoder thread lock mutex: %s", SDL_GetError());
        goto exit_2;
    }

    // Decoder thread
    player->DecoderThread = SDL_CreateThread(MediaPlayer::DecoderThreadFunc, "MediaPlayer::DecoderThreadFunc", player);
    if (player->DecoderThread == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to create a decoder thread: %s", SDL_GetError());
        goto exit_3;
    }

    player->Source = src;
    return player;

    exit_3:
        SDL_DestroyMutex((SDL_mutex*)player->DecoderLock);
    exit_2:
        for (int i = 0; i < KIT_DEC_COUNT; i++) {
            if (!player->Decoders[i]) continue;

            switch (i) {
                case KIT_VIDEO_DEC: {
                    VideoDecoder* dec = (VideoDecoder*)player->Decoders[i];
                    delete dec;
                    break;
                }
                case KIT_AUDIO_DEC: {
                    AudioDecoder* dec = (AudioDecoder*)player->Decoders[i];
                    delete dec;
                    break;
                }
                case KIT_SUBTITLE_DEC: {
                    SubtitleDecoder* dec = (SubtitleDecoder*)player->Decoders[i];
                    delete dec;
                    break;
                }
            }
        }
    exit_1:
        free(player);
    exit_0:
    return NULL;
}
PUBLIC        void         MediaPlayer::Close() {
    // Kill the decoder thread and mutex
    if (SDL_LockMutex(this->DecoderLock) == 0) {
        this->State = KIT_CLOSED;
        SDL_UnlockMutex(this->DecoderLock);
    }
    SDL_WaitThread(this->DecoderThread, NULL);
    SDL_DestroyMutex(this->DecoderLock);

    // Shutdown decoders
    for (int i = 0; i < KIT_DEC_COUNT; i++) {
        if (!this->Decoders[i]) continue;

        switch (i) {
            case KIT_VIDEO_DEC: {
                VideoDecoder* dec = (VideoDecoder*)this->Decoders[i];
                delete dec;
                break;
            }
            case KIT_AUDIO_DEC: {
                AudioDecoder* dec = (AudioDecoder*)this->Decoders[i];
                delete dec;
                break;
            }
            case KIT_SUBTITLE_DEC: {
                SubtitleDecoder* dec = (SubtitleDecoder*)this->Decoders[i];
                delete dec;
                break;
            }
        }
    }

    // Free the player structure itself
    free(this);
}

PUBLIC        void         MediaPlayer::SetScreenSize(int w, int h) {
    SubtitleDecoder* dec = (SubtitleDecoder*)Decoders[KIT_SUBTITLE_DEC];
    if (dec == NULL)
        return;
    dec->SetSize(w, h);
}

PUBLIC        int          MediaPlayer::GetVideoStream() {
    if (!Decoders[KIT_VIDEO_DEC]) return -1;
    return ((Decoder*)Decoders[KIT_VIDEO_DEC])->GetStreamIndex();
}
PUBLIC        int          MediaPlayer::GetAudioStream() {
    if (!Decoders[KIT_AUDIO_DEC]) return -1;
    return ((Decoder*)Decoders[KIT_AUDIO_DEC])->GetStreamIndex();
}
PUBLIC        int          MediaPlayer::GetSubtitleStream() {
    if (!Decoders[KIT_SUBTITLE_DEC]) return -1;
    return ((Decoder*)Decoders[KIT_SUBTITLE_DEC])->GetStreamIndex();
}

PUBLIC        int          MediaPlayer::GetVideoData(Texture* texture) {
    Decoder* dec = (Decoder*)Decoders[KIT_VIDEO_DEC];
    if (dec == NULL) {
        return 0;
    }

    // If paused or stopped, do nothing
    if (this->State == KIT_PAUSED) {
        return 0;
    }
    if (this->State == KIT_STOPPED) {
        return 0;
    }

    return ((VideoDecoder*)dec)->GetVideoDecoderData(texture);
}
PUBLIC        int          MediaPlayer::GetVideoDataForPaused(Texture* texture) {
    Decoder* dec = (Decoder*)Decoders[KIT_VIDEO_DEC];
    if (dec == NULL) {
        return 0;
    }

    return ((VideoDecoder*)dec)->GetVideoDecoderData(texture);
}
PUBLIC        int          MediaPlayer::GetAudioData(unsigned char* buffer, int length) {
    assert(buffer != NULL);

    Decoder* dec = (Decoder*)Decoders[KIT_AUDIO_DEC];
    if (dec == NULL) {
        return 0;
    }

    // If asked for nothing, don't return anything either :P
    if (length == 0) {
        return 0;
    }

    // If paused or stopped, do nothing
    if (this->State == KIT_PAUSED) {
        return 0;
    }
    if (this->State == KIT_STOPPED) {
        return 0;
    }

    return ((AudioDecoder*)dec)->GetAudioDecoderData(buffer, length);
}
PUBLIC        int          MediaPlayer::GetSubtitleData(Texture* texture, SDL_Rect* sources, SDL_Rect* targets, int limit) {
    assert(texture != NULL);
    assert(sources != NULL);
    assert(targets != NULL);
    assert(limit >= 0);

    SubtitleDecoder* sub_dec = (SubtitleDecoder*)Decoders[KIT_SUBTITLE_DEC];
    VideoDecoder* video_dec  = (VideoDecoder*)Decoders[KIT_VIDEO_DEC];
    if (sub_dec == NULL || video_dec == NULL) {
        return 0;
    }

    // If paused, just return the current items
    if (this->State == KIT_PAUSED) {
        return sub_dec->GetInfo(texture, sources, targets, limit);
    }

    // If stopped, do nothing.
    if (this->State == KIT_STOPPED) {
        return 0;
    }

    // Refresh texture, then refresh rects and return number of items in the texture.
    sub_dec->GetTexture(texture, video_dec->ClockPos);
    return sub_dec->GetInfo(texture, sources, targets, limit);
}

PUBLIC        void         MediaPlayer::GetInfo(PlayerInfo* info) {
    assert(info != NULL);

    PlayerStreamInfo* streams[3] = { &info->Video, &info->Audio, &info->Subtitle };
    for (int i = 0; i < KIT_DEC_COUNT; i++) {
        Decoder* dec = this->Decoders[i];
        if (!dec) continue;
        PlayerStreamInfo* stream = streams[i];
        dec->GetCodecInfo(&stream->Codec);
        switch (i) {
            case KIT_VIDEO_DEC: {
                VideoDecoder* dec = (VideoDecoder*)this->Decoders[i];
                dec->GetOutputFormat(&stream->Output);
                break;
            }
            case KIT_AUDIO_DEC: {
                AudioDecoder* dec = (AudioDecoder*)this->Decoders[i];
                dec->GetOutputFormat(&stream->Output);
                break;
            }
            case KIT_SUBTITLE_DEC: {
                SubtitleDecoder* dec = (SubtitleDecoder*)this->Decoders[i];
                dec->GetOutputFormat(&stream->Output);
                break;
            }
        }
    }
}

PUBLIC        void         MediaPlayer::SetClockSync() {
    double sync = MediaPlayerState::GetSystemTime();
    for (int i = 0; i < KIT_DEC_COUNT; i++) {
        if (Decoders[i])
            ((Decoder*)Decoders[i])->SetClockSync(sync);
    }
}
PUBLIC        void         MediaPlayer::ChangeClockSync(double delta) {
    for (int i = 0; i < KIT_DEC_COUNT; i++) {
        if (Decoders[i])
            ((Decoder*)Decoders[i])->ChangeClockSync(delta);
    }
}

PUBLIC        Uint32       MediaPlayer::GetPlayerState() {
    return this->State;
}

PUBLIC        void         MediaPlayer::Play() {
    MediaPlayer* player = this;

    double tmp;
    if (SDL_LockMutex((SDL_mutex*)player->DecoderLock) == 0) {
        switch (player->State) {
            case KIT_PLAYING:
            case KIT_CLOSED:
                break;
            case KIT_PAUSED:
                tmp = MediaPlayerState::GetSystemTime() - player->PauseStarted;
                player->ChangeClockSync(tmp);
                player->State = KIT_PLAYING;
                break;
            case KIT_STOPPED:
                for (int i = 0; i < KIT_DEC_COUNT; i++) {
                    Decoder* dec = (Decoder*)player->Decoders[i];
                    if (dec == NULL)
                        continue;
                    dec->ClearBuffers();
                }
                // MediaPlayer::RunDecoder(player); // Fill some buffers before starting playback
                player->SetClockSync();
                player->State = KIT_PLAYING;
                break;
        }
        SDL_UnlockMutex((SDL_mutex*)player->DecoderLock);
    }
}
PUBLIC        void         MediaPlayer::Stop() {
    MediaPlayer* player = this;
    if (SDL_LockMutex((SDL_mutex*)player->DecoderLock) == 0) {
        switch (player->State) {
            case KIT_STOPPED:
            case KIT_CLOSED:
                break;
            case KIT_PLAYING:
            case KIT_PAUSED:
                player->State = KIT_STOPPED;
                for (int i = 0; i < KIT_DEC_COUNT; i++) {
                    if (player->Decoders[i])
                        ((Decoder*)player->Decoders[i])->ClearBuffers();
                }
                break;
        }
        SDL_UnlockMutex((SDL_mutex*)player->DecoderLock);
    }
}
PUBLIC        void         MediaPlayer::Pause() {
    MediaPlayer* player = this;
    player->State = KIT_PAUSED;
    player->PauseStarted = MediaPlayerState::GetSystemTime();
}
PUBLIC        int          MediaPlayer::Seek(double seek_set) {
    MediaPlayer* player = this;
    double position;
    double duration = 1.0;
    int64_t seek_target;
    int flags = AVSEEK_FLAG_ANY;

    if (SDL_LockMutex((SDL_mutex*)player->DecoderLock) == 0) {
        position = player->GetPosition();
        duration = player->GetDuration();
        if (seek_set <= 0) {
            seek_set = 0;
        }
        if (seek_set >= duration) {
            seek_set = duration;
            // Just do nothing if trying to skip to the end
            SDL_UnlockMutex((SDL_mutex*)player->DecoderLock);
            return 0;
        }

        // Log::Print(Log::LOG_INFO, "trying to seek to: %f / %f", seek_set, duration);

        // Set source to timestamp
        AVFormatContext* format_ctx = (AVFormatContext*)player->Source->FormatCtx;
        seek_target = seek_set * AV_TIME_BASE;
        if (seek_set < position) {
            flags |= AVSEEK_FLAG_BACKWARD;
        }

        // Log::Print(Log::LOG_VERBOSE, "Starting seek to: %f / %f", seek_set, duration);

        double clok = Clock::GetTicks();

        // First, tell ffmpeg to seek stream. If not capable, stop here.
        // Failure here probably means that stream is unseekable someway, eg. streamed media
        // NOTE: this sets the read position of the stream: if we don't need it, we should skip this
        //    if the desired position is already loaded.
        if (avformat_seek_file(format_ctx, -1, seek_target, seek_target, INT64_MAX, flags) < 0) {
            Log::Print(Log::LOG_ERROR, "Unable to seek source");
            SDL_UnlockMutex((SDL_mutex*)player->DecoderLock);
            return 1;
        }

        // Log::Print(Log::LOG_VERBOSE, "We have seeked, time to adjust/clear buffers. (%.3f ms)", Clock::GetTicks() - clok);

        /*
        //// CHECK TO SEE IF POINT WE SEEKED TO IS IN THE BUFFERS SO WE DONT HAVE TO RELOAD THE WHOLE THING
        enum {
            BUFFER_IN,
            BUFFER_OUT,
        };
        double startRange, endRange;
        double videoInPtsRange[4];
        videoInPtsRange[2] = 999999999999.0;
        videoInPtsRange[3] = 0.0;
        double audioInPtsRange[4];
        audioInPtsRange[2] = 999999999999.0;
        audioInPtsRange[3] = 0.0;
        for (int i = 0; i < KIT_DEC_COUNT; i++) {
            Decoder* dec = player->Decoders[i];
            if (dec) {
                if (i == KIT_VIDEO_DEC) {
                    dec->Buffer[BUFFER_OUT]->WithEachItemInBuffer([](void* data, void* userdata) -> void {
                        double* ranges = (double*)userdata;
                        double  packet = *(double*)data;
                        if (ranges[0] > packet)
                            ranges[0] = packet;
                        if (ranges[1] < packet)
                            ranges[1] = packet;
                    }, &videoInPtsRange[2]);
                }
                else if (i == KIT_AUDIO_DEC) {
                    dec->Buffer[BUFFER_OUT]->WithEachItemInBuffer([](void* data, void* userdata) -> void {
                        double* ranges = (double*)userdata;
                        double  packet = *(double*)data;
                        if (ranges[0] > packet)
                            ranges[0] = packet;
                        if (ranges[1] < packet)
                            ranges[1] = packet;
                    }, &audioInPtsRange[2]);
                }
            }
        }
        startRange = videoInPtsRange[2] > audioInPtsRange[2] ? videoInPtsRange[2] : audioInPtsRange[2];
        endRange = videoInPtsRange[3] < audioInPtsRange[3] ? videoInPtsRange[3] : audioInPtsRange[3];
        printf("%.2f -> %.2f\n", startRange, endRange);
        //*/

        clok = Clock::GetTicks();
        // Clean old buffers and try to fill them with new data
        for (int i = 0; i < KIT_DEC_COUNT; i++) {
            if (player->Decoders[i])
                ((Decoder*)player->Decoders[i])->ClearBuffers();
        }

        double clear = Clock::GetTicks() - clok;

        // if (player->State == KIT_PLAYING)
        //     player->State = KIT_BUFFERING_PLAYING;
        // else if (player->State == KIT_PAUSED)
        //     player->State = KIT_BUFFERING_PAUSED;

        clok = Clock::GetTicks();
        // This is necessary to fix a visual bug, but we want this gone
        MediaPlayer::RunDecoder(player);
        // int littlebit = 24;
        // while (MediaPlayer::DemuxStream(player) == -1 && --littlebit);

        // Log::Print(Log::LOG_VERBOSE, "Done seek! (clear: %.3f ms, run decoder: %.3f ms)", clear, Clock::GetTicks() - clok);

        // Try to get a precise seek position from the next audio/video frame
        // (depending on which one is used to sync)
        double precise_pts = -1.0;
        if (player->Decoders[KIT_VIDEO_DEC] != NULL) {
            precise_pts = ((VideoDecoder*)player->Decoders[KIT_VIDEO_DEC])->GetPTS();
        }
        else if (player->Decoders[KIT_AUDIO_DEC] != NULL) {
            precise_pts = ((AudioDecoder*)player->Decoders[KIT_AUDIO_DEC])->GetPTS();
        }

        // If we got a legit looking value, set it as seek value. Otherwise use
        // the seek value we requested.
        if (precise_pts >= 0) {
            player->ChangeClockSync(position - precise_pts);
        }
        else {
            player->ChangeClockSync(position - seek_set);
        }

        PausedPosition = seek_set;

        // That's it. Unlock and continue.
        SDL_UnlockMutex((SDL_mutex*)player->DecoderLock);
    }

    return 0;
}

PUBLIC        double       MediaPlayer::GetDuration() {
    AVFormatContext* fmt_ctx = (AVFormatContext*)this->Source->FormatCtx;
    return (fmt_ctx->duration / AV_TIME_BASE);
}
PUBLIC        double       MediaPlayer::GetPosition() {
    if (State != KIT_PLAYING)
        return PausedPosition;

    if (this->Decoders[KIT_VIDEO_DEC]) {
        PausedPosition = ((Decoder*)this->Decoders[KIT_VIDEO_DEC])->ClockPos;
        return PausedPosition;
    }
    if (this->Decoders[KIT_AUDIO_DEC]) {
        PausedPosition = ((Decoder*)this->Decoders[KIT_AUDIO_DEC])->ClockPos;
        return PausedPosition;
    }
    return 0;
}
PUBLIC        double       MediaPlayer::GetBufferPosition() {
    if (this->Decoders[KIT_VIDEO_DEC]) {
        return ((VideoDecoder*)this->Decoders[KIT_VIDEO_DEC])->GetLargestOutputPTS();
    }
    return 0;
}
