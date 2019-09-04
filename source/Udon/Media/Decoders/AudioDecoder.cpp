#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Media/Decoder.h>

class AudioDecoder : public Decoder {
public:
    SwrContext* SWR;
    AVFrame* ScratchFrame;

    int SampleRate;
    int Channels;
    int Bytes;
    int IsSigned;
};
#endif

#include <Udon/Media/Decoders/AudioDecoder.h>

#include <Udon/Application.h>
#include <Udon/Logging/Log.h>
#include <Udon/Media/Utils/RingBuffer.h>
#include <Udon/Media/Utils/MediaPlayerState.h>

struct AudioPacket {
    double pts;
    size_t original_size;
    RingBuffer* buffer;
};

#define KIT_AUDIO_SYNC_THRESHOLD 0.05

PUBLIC                       AudioDecoder::AudioDecoder(MediaSource* src, int stream_index) {
    if (stream_index < 0) {
        Log::Print(Log::LOG_ERROR, "stream_index < 0");
        return;
    }

    Decoder::Create(src,
        stream_index,
        MediaPlayerState::AudioBufFrames,
        AudioDecoder::FreeAudioPacket,
        MediaPlayerState::ThreadCount);

    ScratchFrame = av_frame_alloc();
    if (ScratchFrame == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to initialize temporary audio frame");
        goto exit_2;
    }

    SampleRate = CodecCtx->sample_rate;
    Channels = FindChannelLayout(CodecCtx->channel_layout);
    Bytes = FindBytes(CodecCtx->sample_fmt);
    IsSigned = FindSignedness(CodecCtx->sample_fmt);
    Format = FindSDLSampleFormat(CodecCtx->sample_fmt);

    SWR = swr_alloc_set_opts(NULL,
        FindAVChannelLayout(Channels), // Target channel layout
        FindAVSampleFormat(Format), // Target fmt
        SampleRate, // Target samplerate
        CodecCtx->channel_layout, // Source channel layout
        CodecCtx->sample_fmt, // Source fmt
        CodecCtx->sample_rate, // Source samplerate
        0, NULL);

    if (swr_init(SWR) != 0) {
        Log::Print(Log::LOG_ERROR, "Unable to initialize audio resampler context");
        goto exit_3;
    }

    this->DecodeFunc = AudioDecoder::DecodeFunction;
    this->CloseFunc  = AudioDecoder::CloseFunction;

    return;

    exit_3:
    av_frame_free(&ScratchFrame);
    exit_2:
    // exit_1:
    // exit_0:
    return;
}

PUBLIC        void*          AudioDecoder::CreateAudioPacket(const char* data, size_t len, double pts) {
    AudioPacket* packet = (AudioPacket*)calloc(1, sizeof(AudioPacket));
    packet->buffer = new RingBuffer(len);
    packet->buffer->Write(data, len);
    packet->pts = pts;
    return packet;
}
PUBLIC STATIC void           AudioDecoder::FreeAudioPacket(void* p) {
    AudioPacket* packet = (AudioPacket*)p;
    delete packet->buffer;
    free(packet);
}

PUBLIC        AVSampleFormat AudioDecoder::FindAVSampleFormat(int format) {
    switch(format) {
        case AUDIO_U8: return AV_SAMPLE_FMT_U8;
        case AUDIO_S16SYS: return AV_SAMPLE_FMT_S16;
        case AUDIO_S32SYS: return AV_SAMPLE_FMT_S32;
        default: return AV_SAMPLE_FMT_NONE;
    }
}
PUBLIC        Sint64         AudioDecoder::FindAVChannelLayout(int channels) {
    switch(channels) {
        case 1:  return AV_CH_LAYOUT_MONO;
        case 2:  return AV_CH_LAYOUT_STEREO;
        default: return AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
}
PUBLIC        int            AudioDecoder::FindChannelLayout(uint64_t channel_layout) {
    switch (channel_layout) {
        case AV_CH_LAYOUT_MONO: return 1;
        case AV_CH_LAYOUT_STEREO: return 2;
        default: return 2;
    }
    return 2;
}
PUBLIC        int            AudioDecoder::FindBytes(AVSampleFormat fmt) {
    switch(fmt) {
        case AV_SAMPLE_FMT_U8P:
        case AV_SAMPLE_FMT_U8:
            return 1;
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_S32:
            return 4;
        default:
            return 2;
    }
}
PUBLIC        int            AudioDecoder::FindSignedness(AVSampleFormat fmt) {
    switch(fmt) {
        case AV_SAMPLE_FMT_U8P:
        case AV_SAMPLE_FMT_U8:
            return 0;
        default:
            return 1;
    }
}
PUBLIC        int            AudioDecoder::FindSDLSampleFormat(AVSampleFormat fmt) {
    switch(fmt) {
        case AV_SAMPLE_FMT_U8P:
        case AV_SAMPLE_FMT_U8:
            return AUDIO_U8;
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_S32:
            return AUDIO_S32SYS;
        default:
            return AUDIO_S16SYS;
    }
}

PUBLIC        int            AudioDecoder::GetOutputFormat(OutputFormat* output) {
    output->Format = Format;
    output->SampleRate = SampleRate;
    output->Channels = Channels;
    output->Bytes = Bytes;
    output->IsSigned = IsSigned;
    return 0;
}

PUBLIC STATIC void           AudioDecoder::ReadAudio(void* ptr) {
    #if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
    AudioDecoder* self = (AudioDecoder*)ptr;
    int          len, dst_linesize, dst_nb_samples, dst_bufsize;
    double       pts;
    Uint8**      dst_data;
    AudioPacket* out_packet = NULL;
    int          ret = 0;

    while (!ret && self->CanWriteOutput()) {
        ret = avcodec_receive_frame(self->CodecCtx, self->ScratchFrame);
        if (!ret) {
            dst_nb_samples = av_rescale_rnd(
                self->ScratchFrame->nb_samples,
                self->SampleRate,               // Target samplerate
                self->CodecCtx->sample_rate,    // Source samplerate
                AV_ROUND_UP);

            av_samples_alloc_array_and_samples(
                &dst_data,
                &dst_linesize,
                self->Channels,
                dst_nb_samples,
                self->FindAVSampleFormat(self->Format),
                0);

            len = swr_convert(
                self->SWR,
                dst_data,
                dst_nb_samples,
                (const Uint8**)self->ScratchFrame->extended_data,
                self->ScratchFrame->nb_samples);

            dst_bufsize = av_samples_get_buffer_size(
                &dst_linesize,
                self->Channels,
                len,
                self->FindAVSampleFormat(self->Format), 1);

            // Get presentation timestamp
            pts = self->ScratchFrame->best_effort_timestamp;
            pts *= av_q2d(self->FormatCtx->streams[self->StreamIndex]->time_base);

            // Lock, write to audio buffer, unlock
            out_packet = (AudioPacket*)self->CreateAudioPacket((char*)dst_data[0], (size_t)dst_bufsize, pts);
            self->WriteOutput(out_packet);

            // Free temps
            av_freep(&dst_data[0]);
            av_freep(&dst_data);
        }
    }
    #endif
}
PUBLIC STATIC int            AudioDecoder::DecodeFunction(void* ptr, AVPacket* in_packet) {
    if (in_packet == NULL) {
        return 0;
    }
    AudioDecoder* self = (AudioDecoder*)ptr;

    #if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
        // Try to clear the buffer first. We might have too much content in the ffmpeg buffer,
        // so we want to clear it of outgoing data if we can.
        AudioDecoder::ReadAudio(self);

        // Write packet to the decoder for handling.
        if (avcodec_send_packet(self->CodecCtx, in_packet) < 0) {
            return 1;
        }

        // Some input data was put in successfully, so try again to get frames.
        AudioDecoder::ReadAudio(self);
    #else
        int frame_finished;
        int len;
        int len2;
        int dst_linesize;
        int dst_nb_samples;
        int dst_bufsize;
        double pts;
        Uint8** dst_data;
        AudioPacket* out_packet = NULL;

        // Decode as long as there is data
        while (in_packet->size > 0) {
            len = avcodec_decode_audio4(self->CodecCtx, self->ScratchFrame, &frame_finished, in_packet);
            if (len < 0) {
                return 0;
            }

            if (frame_finished) {
                dst_nb_samples = av_rescale_rnd(
                    self->ScratchFrame->nb_samples,
                    self->SampleRate,  // Target samplerate
                    self->CodecCtx->sample_rate,  // Source samplerate
                    AV_ROUND_UP);

                av_samples_alloc_array_and_samples(
                    &dst_data,
                    &dst_linesize,
                    self->Channels,
                    dst_nb_samples,
                    self->FindAVSampleFormat(self->Format),
                    0);

                len2 = swr_convert(
                    self->SWR,
                    dst_data,
                    self->ScratchFrame->nb_samples,
                    (const Uint8**)self->ScratchFrame->extended_data,
                    self->ScratchFrame->nb_samples);

                dst_bufsize = av_samples_get_buffer_size(
                    &dst_linesize,
                    self->Channels,
                    len2,
                    self->FindAVSampleFormat(self->Format), 1);

                // Get presentation timestamp
            #ifndef FF_API_FRAME_GET_SET
                pts = av_frame_get_best_effort_timestamp(self->ScratchFrame);
            #else
                pts = self->ScratchFrame->best_effort_timestamp;
            #endif
                pts *= av_q2d(self->FormatCtx->streams[self->StreamIndex]->time_base);

                // Lock, write to audio buffer, unlock
                out_packet = (AudioPacket*)self->CreateAudioPacket((char*)dst_data[0], (size_t)dst_bufsize, pts);
                self->WriteOutput(out_packet);

                // Free temps
                av_freep(&dst_data[0]);
                av_freep(&dst_data);
            }

            in_packet->size -= len;
            in_packet->data += len;
        }
    #endif
    return 0;
}
PUBLIC STATIC void           AudioDecoder::CloseFunction(void* ptr) {
    AudioDecoder* self = (AudioDecoder*)ptr;

    if (self->ScratchFrame != NULL) {
        av_frame_free(&self->ScratchFrame);
    }
    if (self->SWR != NULL) {
        swr_free(&self->SWR);
    }
}

PUBLIC        double         AudioDecoder::GetPTS() {
    AudioPacket* packet = (AudioPacket*)PeekOutput();
    if (packet == NULL) {
        return -1.0;
    }
    return packet->pts;
}
PUBLIC        int            AudioDecoder::GetAudioDecoderData(Uint8* buf, int len) {
    AudioPacket* packet = NULL;
    int ret = 0;
    int bytes_per_sample = 0;
    double bytes_per_second = 0.0;
    double sync_ts = 0.0;

    // First, peek the next packet. Make sure we have something to read.
    packet = (AudioPacket*)PeekOutput();
    if (packet == NULL) {
        return 0;
    }

    // If packet should not yet be played, stop here and wait.
    // If packet should have already been played, skip it and try to find a better packet.
    // For audio, it is possible that we cannot find good packet. Then just don't read anything.
    sync_ts = MediaPlayerState::GetSystemTime() - ClockSync;
    if (packet->pts > sync_ts + KIT_AUDIO_SYNC_THRESHOLD) {
        return 0;
    }
    while (packet != NULL && packet->pts < sync_ts - KIT_AUDIO_SYNC_THRESHOLD) {
        AdvanceOutput();
        FreeAudioPacket(packet);
        packet = (AudioPacket*)PeekOutput();
    }
    if (packet == NULL) {
        return 0;
    }

    // Read data from packet ringbuffer
    if (len > 0) {
        ret = packet->buffer->Read((char*)buf, len);
        if (ret) {
            bytes_per_sample = Bytes * Channels;
            bytes_per_second = bytes_per_sample * SampleRate;
            packet->pts += ((double)ret) / bytes_per_second;
        }
    }
    ClockPos = packet->pts;

    // If ringbuffer is cleared, kill packet and advance buffer.
    // Otherwise forward the pts value for the current packet.
    if (packet->buffer->GetLength() == 0) {
        AdvanceOutput();
        FreeAudioPacket(packet);
    }
    return ret;
}
