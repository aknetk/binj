#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Media/Decoder.h>
#include <Udon/Rendering/Texture.h>

class VideoDecoder : public Decoder {
public:
    SwsContext* SWS;
    AVFrame* ScratchFrame;

    int Width;
    int Height;
};
#endif

#include <Udon/Media/Decoders/VideoDecoder.h>

#include <Udon/Application.h>
#include <Udon/Logging/Log.h>
#include <Udon/Media/Utils/RingBuffer.h>
#include <Udon/Media/Utils/MediaPlayerState.h>

#define KIT_VIDEO_SYNC_THRESHOLD 0.02

enum AVPixelFormat supported_list[] =  {
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUYV422,
    AV_PIX_FMT_UYVY422,
    AV_PIX_FMT_NV12,
    AV_PIX_FMT_NV21,
    AV_PIX_FMT_RGB24,
    AV_PIX_FMT_BGR24,
    AV_PIX_FMT_RGB555,
    AV_PIX_FMT_BGR555,
    AV_PIX_FMT_RGB565,
    AV_PIX_FMT_BGR565,
    AV_PIX_FMT_BGRA,
    AV_PIX_FMT_RGBA,
    AV_PIX_FMT_NONE
};

struct VideoPacket {
    double pts;
    AVFrame* frame;
};

PUBLIC                       VideoDecoder::VideoDecoder(MediaSource* src, int stream_index) {
    if (stream_index < 0) {
        Log::Print(Log::LOG_ERROR, "stream_index < 0");
        return;
    }

    Decoder::Create(src,
        stream_index,
        MediaPlayerState::VideoBufFrames,
        VideoDecoder::FreeVideoPacket,
        MediaPlayerState::ThreadCount);

    AVPixelFormat output_format;

    ScratchFrame = av_frame_alloc();
    if (ScratchFrame == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to initialize temporary video frame");
        exit(-1);
        goto exit_2;
    }

    // Find best output format for us
    output_format = avcodec_find_best_pix_fmt_of_list(supported_list, this->CodecCtx->pix_fmt, 1, NULL);

    // Set format configs
    Width = this->CodecCtx->width;
    Height = this->CodecCtx->height;
    Format = FindSDLPixelFormat(output_format);

    // Create scaler for handling format changes
    this->SWS = sws_getContext(
        this->CodecCtx->width, // Source w
        this->CodecCtx->height, // Source h
        this->CodecCtx->pix_fmt, // Source fmt
        this->CodecCtx->width, // Target w
        this->CodecCtx->height, // Target h
        FindAVPixelFormat(Format), // Target fmt
        SWS_BILINEAR,
        NULL, NULL, NULL);
    if (this->SWS == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to initialize video converter context");
        exit(-1);
        goto exit_3;
    }

    // Set callbacks and userdata, and we're go
    this->DecodeFunc = VideoDecoder::DecodeFunction;
    this->CloseFunc  = VideoDecoder::CloseFunction;
    return;

    exit_3:
    av_frame_free(&this->ScratchFrame);
    exit_2:
    // free(video_dec);
    // exit_1:
    // Kit_CloseDecoder(dec);
    // exit_0:
    return;
}

PUBLIC        void*          VideoDecoder::CreateVideoPacket(AVFrame* frame, double pts) {
    VideoPacket* packet = (VideoPacket*)calloc(1, sizeof(VideoPacket));
    packet->frame = frame;
    packet->pts = pts;
    return packet;
}
PUBLIC STATIC void           VideoDecoder::FreeVideoPacket(void* p) {
    VideoPacket* packet = (VideoPacket*)p;
    av_freep(&packet->frame->data[0]);
    av_frame_free(&packet->frame);
    free(packet);
}

PUBLIC        AVPixelFormat  VideoDecoder::FindAVPixelFormat(Uint32 format) {
    switch (format) {
        case SDL_PIXELFORMAT_YV12:   return AV_PIX_FMT_YUV420P;
        case SDL_PIXELFORMAT_YUY2:   return AV_PIX_FMT_YUYV422;
        case SDL_PIXELFORMAT_UYVY:   return AV_PIX_FMT_UYVY422;
        case SDL_PIXELFORMAT_NV12:   return AV_PIX_FMT_NV12;
        case SDL_PIXELFORMAT_NV21:   return AV_PIX_FMT_NV21;
        case SDL_PIXELFORMAT_ARGB32: return AV_PIX_FMT_ARGB;
        case SDL_PIXELFORMAT_RGBA32: return AV_PIX_FMT_RGBA;
        case SDL_PIXELFORMAT_BGR24:  return AV_PIX_FMT_BGR24;
        case SDL_PIXELFORMAT_RGB24:  return AV_PIX_FMT_RGB24;
        case SDL_PIXELFORMAT_RGB555: return AV_PIX_FMT_RGB555;
        case SDL_PIXELFORMAT_BGR555: return AV_PIX_FMT_BGR555;
        case SDL_PIXELFORMAT_RGB565: return AV_PIX_FMT_RGB565;
        case SDL_PIXELFORMAT_BGR565: return AV_PIX_FMT_BGR565;
        default:                     return AV_PIX_FMT_NONE;
    }
}
PUBLIC        int            VideoDecoder::FindSDLPixelFormat(AVPixelFormat fmt) {
    switch (fmt) {
        case AV_PIX_FMT_YUV420P:
            return SDL_PIXELFORMAT_YV12;
        case AV_PIX_FMT_YUYV422:
            return SDL_PIXELFORMAT_YUY2;
        case AV_PIX_FMT_UYVY422:
            return SDL_PIXELFORMAT_UYVY;
        case AV_PIX_FMT_NV12:
            return SDL_PIXELFORMAT_NV12;
        case AV_PIX_FMT_NV21:
            return SDL_PIXELFORMAT_NV21;
        default:
            return SDL_PIXELFORMAT_RGBA32;
    }
}

PUBLIC        int            VideoDecoder::GetOutputFormat(OutputFormat* output) {
    output->Format = Format;
    output->Width = Width;
    output->Height = Height;
    return 0;
}

PUBLIC STATIC void           VideoDecoder::ReadVideo(void* ptr) {
    #if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
    VideoDecoder* self = (VideoDecoder*)ptr;
    AVFrame* out_frame = NULL;
    VideoPacket* out_packet = NULL;
    double pts;
    int ret = 0;

    while (!ret && self->CanWriteOutput()) {
        ret = avcodec_receive_frame(self->CodecCtx, self->ScratchFrame);
        if (!ret) {
            out_frame = av_frame_alloc();
            av_image_alloc(
                    out_frame->data,
                    out_frame->linesize,
                    self->CodecCtx->width,
                    self->CodecCtx->height,
                    self->FindAVPixelFormat(self->Format),
                    1);

            // Scale from source format to target format, don't touch the size
            sws_scale(
                self->SWS,
                (const unsigned char * const *)self->ScratchFrame->data,
                self->ScratchFrame->linesize,
                0,
                self->CodecCtx->height,
                out_frame->data,
                out_frame->linesize);

            // Get presentation timestamp
            pts = self->ScratchFrame->best_effort_timestamp;
            pts *= av_q2d(self->FormatCtx->streams[self->StreamIndex]->time_base);

            // Lock, write to audio buffer, unlock
            out_packet = (VideoPacket*)self->CreateVideoPacket(out_frame, pts);
            self->WriteOutput(out_packet);
        }
    }
    #endif
}
PUBLIC STATIC int            VideoDecoder::DecodeFunction(void* ptr, AVPacket* in_packet) {
    if (in_packet == NULL) {
        return 0;
    }
    VideoDecoder* self = (VideoDecoder*)ptr;

    #if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
        // Try to clear the buffer first. We might have too much content in the ffmpeg buffer,
        // so we want to clear it of outgoing data if we can.
        VideoDecoder::ReadVideo(self);

        // Write packet to the decoder for handling.
        if (avcodec_send_packet(self->CodecCtx, in_packet) < 0) {
            return 1;
        }

        // Some input data was put in successfully, so try again to get frames.
        VideoDecoder::ReadVideo(self);
    #else
        AVFrame* out_frame = NULL;
        VideoPacket* out_packet = NULL;
        int frame_finished;
        int len;
        double pts;

        while (in_packet->size > 0) {
            len = avcodec_decode_video2(self->CodecCtx, self->ScratchFrame, &frame_finished, in_packet);
            if (len < 0) {
                return 0;
            }

            if (frame_finished) {
                // Target frame
                out_frame = av_frame_alloc();
                av_image_alloc(
                        out_frame->data,
                        out_frame->linesize,
                        self->CodecCtx->width,
                        self->CodecCtx->height,
                        self->FindAVPixelFormat(self->Format),
                        1);

                // Scale from source format to target format, don't touch the size
                sws_scale(
                    this->SWS,
                    (const unsigned char * const *)this->ScratchFrame->data,
                    this->ScratchFrame->linesize,
                    0,
                    this->CodecCtx->height,
                    out_frame->data,
                    out_frame->linesize);

                // Get presentation timestamp
                #ifndef FF_API_FRAME_GET_SET
                    pts = av_frame_get_best_effort_timestamp(this->ScratchFrame);
                #else
                    pts = this->ScratchFrame->best_effort_timestamp;
                #endif
                    pts *= av_q2d(this->FormatCtx->streams[this->StreamIndex]->time_base);

                // Lock, write to audio buffer, unlock
                out_packet = (VideoPacket*)self->CreateVideoPacket(out_frame, pts);
                self->WriteOutput(out_packet);
            }
            in_packet->size -= len;
            in_packet->data += len;
        }
    #endif
    return 0;
}
PUBLIC STATIC void           VideoDecoder::CloseFunction(void* ptr) {
    VideoDecoder* self = (VideoDecoder*)ptr;
    if (self->ScratchFrame != NULL) {
        av_frame_free(&self->ScratchFrame);
    }
    if (self->SWS != NULL) {
        sws_freeContext(self->SWS);
    }
}

PUBLIC        double         VideoDecoder::GetPTS() {
    VideoPacket* packet = (VideoPacket*)PeekOutput();
    if (packet == NULL) {
        return -1.0;
    }
    return packet->pts;
}
PUBLIC        double         VideoDecoder::GetLargestOutputPTS() {
    double maxPTS = 0.0;
    if (SDL_LockMutex(OutputLock) == 0) {
        int KIT_DEC_BUF_OUT = 1;
        Buffer[KIT_DEC_BUF_OUT]->WithEachItemInBuffer([](void* data, void* ptsPTR) -> void {
            double* maPTS = (double*)ptsPTR;
            VideoPacket* packet = (VideoPacket*)data;
            if (*maPTS < packet->pts)
                *maPTS = packet->pts;
        }, &maxPTS);
        SDL_UnlockMutex(OutputLock);
    }
    return maxPTS;
}
PUBLIC        int            VideoDecoder::GetVideoDecoderData(Texture* texture) {
    VideoPacket* packet = NULL;
    double sync_ts = 0.0;
    Uint32 limit_rounds = 0;

    // First, peek the next packet. Make sure we have something to read.
    packet = (VideoPacket*)PeekOutput();
    if (packet == NULL) {
        return 0;
    }

    // If packet should not yet be played, stop here and wait.
    // If packet should have already been played, skip it and try to find a better packet.
    // For video, we *try* to return a frame, even if we are out of sync. It is better than
    // not showing anything.
    sync_ts = MediaPlayerState::GetSystemTime() - ClockSync;
    if (packet->pts > sync_ts + KIT_VIDEO_SYNC_THRESHOLD) {
        return 0;
    }
    limit_rounds = GetOutputLength();
    while (packet != NULL && packet->pts < sync_ts - KIT_VIDEO_SYNC_THRESHOLD && --limit_rounds) {
        AdvanceOutput();
        FreeVideoPacket(packet);
        packet = (VideoPacket*)PeekOutput();
    }
    if (packet == NULL) {
        return 0;
    }

    // BUG:
    // We want to separate the getting of packets into a function that will be called Application::FPS
    // times per second, and the below just gets whatever the last packet is and converted it in the
    // main thread (Textures can only be handled in the main thread).
    // This will potentially fix the bug where the video freezes when the Window is dragging or resizing.

    // Update output texture with current video data.
    // Take formats into account.
    switch (Format) {
        case SDL_PIXELFORMAT_YV12:
        case SDL_PIXELFORMAT_IYUV:
            texture->RendererPtr->UpdateYUVTexture(
                texture,
                packet->frame->data[0], packet->frame->linesize[0],
                packet->frame->data[1], packet->frame->linesize[1],
                packet->frame->data[2], packet->frame->linesize[2]);
            break;
        default:
            texture->RendererPtr->UpdateTexture(
                texture,
                packet->frame->data[0],
                packet->frame->linesize[0]);
            break;
    }

    // Advance buffer, and free the decoded frame.
    AdvanceOutput();
    ClockPos = packet->pts;
    FreeVideoPacket(packet);
    return 0;
}
