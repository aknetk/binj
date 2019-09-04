#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Media/Decoder.h>
#include <Udon/Rendering/Texture.h>
#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>
// #include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>

class SubtitleDecoder : public Decoder {
public:
    SubtitleRenderer* Renderer;
    AVSubtitle ScratchFrame;
    TextureAtlas* Atlas;
};
#endif

#include <Udon/Media/Decoders/SubtitleDecoder.h>
#include <Udon/Media/Decoders/Subtitle/SubtitlePacket.h>

#include <Udon/Media/Decoders/Subtitle/ImageSubtitleRenderer.h>
#include <Udon/Media/Decoders/Subtitle/ASSSubtitleRenderer.h>

#include <Udon/Application.h>
#include <Udon/Logging/Log.h>
#include <Udon/Media/Utils/RingBuffer.h>
#include <Udon/Media/Utils/MediaPlayerState.h>

PUBLIC                       SubtitleDecoder::SubtitleDecoder(MediaSource* src, int stream_index, int video_w, int video_h, int screen_w, int screen_h) {
    if (stream_index < 0) {
        Log::Print(Log::LOG_ERROR, "stream_index < 0");
        return;
    }

    Decoder::Create(src,
        stream_index,
        MediaPlayerState::SubtitleBufFrames,
        SubtitleDecoder::FreeSubtitlePacket,
        MediaPlayerState::ThreadCount);

    Format = SDL_PIXELFORMAT_RGBA32;

    // For subtitles, we need a renderer for the stream. Pick one based on codec ID.
    switch (CodecCtx->codec_id) {
        case AV_CODEC_ID_TEXT:
        case AV_CODEC_ID_HDMV_TEXT_SUBTITLE:
        case AV_CODEC_ID_SRT:
        case AV_CODEC_ID_SUBRIP:
        case AV_CODEC_ID_SSA:
        case AV_CODEC_ID_ASS:
            this->Renderer = new ASSSubtitleRenderer(this, video_w, video_h, screen_w, screen_h);
            break;
        case AV_CODEC_ID_DVD_SUBTITLE:
        case AV_CODEC_ID_DVB_SUBTITLE:
        case AV_CODEC_ID_HDMV_PGS_SUBTITLE:
        case AV_CODEC_ID_XSUB:
            this->Renderer = new ImageSubtitleRenderer(this, video_w, video_h, screen_w, screen_h);
            break;
        default:
            Log::Print(Log::LOG_ERROR, "Unrecognized subtitle format");
            break;
    }
    if (this->Renderer == NULL) {
        goto exit_2;
    }

    // Allocate texture atlas for subtitle rectangles
    this->Atlas = TextureAtlas::Create();
    if (this->Atlas == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to allocate subtitle texture atlas");
        goto exit_3;
    }

    // Set callbacks and userdata, and we're go
    this->DecodeFunc = SubtitleDecoder::DecodeFunction;
    this->CloseFunc = SubtitleDecoder::CloseFunction;
    return;

    exit_3:
        Renderer->Close();
    exit_2:
        // free(subtitle_dec);
    // exit_1:
        // Kit_CloseDecoder(dec);
    // exit_0:
        return;
}

PUBLIC STATIC void*          SubtitleDecoder::CreateSubtitlePacket(bool clear, double pts_start, double pts_end, int pos_x, int pos_y, SDL_Surface* surface) {
    SubtitlePacket* packet = (SubtitlePacket*)calloc(1, sizeof(SubtitlePacket));
    packet->pts_start = pts_start;
    packet->pts_end = pts_end;
    packet->x = pos_x;
    packet->y = pos_y;
    packet->surface = surface;
    if (packet->surface != NULL) {
        packet->surface->refcount++; // We dont want to needlessly copy; instead increase refcount.
    }
    packet->clear = clear;
    return packet;
}
PUBLIC STATIC void           SubtitleDecoder::FreeSubtitlePacket(void* p) {
    SubtitlePacket* packet = (SubtitlePacket*)p;
    SDL_FreeSurface(packet->surface);
    free(packet);
}

PUBLIC        int            SubtitleDecoder::GetOutputFormat(OutputFormat* output) {
    output->Format = Format;
    return 0;
}

PUBLIC STATIC int            SubtitleDecoder::DecodeFunction(void* ptr, AVPacket* in_packet) {
    if (in_packet == NULL) {
        return 0;
    }

    SubtitleDecoder* self = (SubtitleDecoder*)ptr;
    double pts;
    double start;
    double end;
    int frame_finished;
    int len;

    if (in_packet->size > 0) {
        len = avcodec_decode_subtitle2(self->CodecCtx, &self->ScratchFrame, &frame_finished, in_packet);
        if (len < 0) {
            return 0;
        }

        if (frame_finished) {
            // Start and end presentation timestamps for subtitle frame
            pts = 0;
            if (in_packet->pts != AV_NOPTS_VALUE) {
                pts = in_packet->pts;
                pts *= av_q2d(self->FormatCtx->streams[self->StreamIndex]->time_base);
            }

            // If subtitle has no ending time, we set some safety value.
            if (self->ScratchFrame.end_display_time == UINT_MAX) {
                self->ScratchFrame.end_display_time = 30000;
            }

            start = self->ScratchFrame.start_display_time / 1000.0f;
            end = self->ScratchFrame.end_display_time / 1000.0f;

            // Create a packet. This should be filled by renderer.
            if (self->Renderer)
                self->Renderer->Run(&self->ScratchFrame, pts, start, end);

            // Free subtitle since it has now been handled
            avsubtitle_free(&self->ScratchFrame);
        }
    }

    return 0;
}
PUBLIC STATIC void           SubtitleDecoder::CloseFunction(void* ptr) {
    SubtitleDecoder* self = (SubtitleDecoder*)ptr;

    self->Atlas->Free();
    if (self->Renderer) {
        delete self->Renderer;
    }
}

PUBLIC        void           SubtitleDecoder::SetSize(int w, int h) {
    if (Renderer)
        Renderer->SetSize(w, h);
}
PUBLIC        int            SubtitleDecoder::GetInfo(Texture *texture, SDL_Rect* sources, SDL_Rect* targets, int limit) {
    if (Atlas)
        return Atlas->GetItems(sources, targets, limit);
    return 0;
}

PUBLIC        void           SubtitleDecoder::GetTexture(Texture* texture, double sync_ts) {
    if (Renderer)
        Renderer->GetData(this->Atlas, texture, sync_ts);
}
