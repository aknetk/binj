#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


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

                          AudioDecoder(MediaSource* src, int stream_index);
           void*          CreateAudioPacket(const char* data, size_t len, double pts);
    static void           FreeAudioPacket(void* p);
           AVSampleFormat FindAVSampleFormat(int format);
           Sint64         FindAVChannelLayout(int channels);
           int            FindChannelLayout(uint64_t channel_layout);
           int            FindBytes(AVSampleFormat fmt);
           int            FindSignedness(AVSampleFormat fmt);
           int            FindSDLSampleFormat(AVSampleFormat fmt);
           int            GetOutputFormat(OutputFormat* output);
    static void           ReadAudio(void* ptr);
    static int            DecodeFunction(void* ptr, AVPacket* in_packet);
    static void           CloseFunction(void* ptr);
           double         GetPTS();
           int            GetAudioDecoderData(Uint8* buf, int len);
};

#endif /* AUDIODECODER_H */
