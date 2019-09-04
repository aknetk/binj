#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Media/Decoder.h>
#include <Udon/Rendering/Texture.h>

class VideoDecoder : public Decoder {
public:
    SwsContext* SWS;
    AVFrame* ScratchFrame;
    int Width;
    int Height;

                          VideoDecoder(MediaSource* src, int stream_index);
           void*          CreateVideoPacket(AVFrame* frame, double pts);
    static void           FreeVideoPacket(void* p);
           AVPixelFormat  FindAVPixelFormat(Uint32 format);
           int            FindSDLPixelFormat(AVPixelFormat fmt);
           int            GetOutputFormat(OutputFormat* output);
    static void           ReadVideo(void* ptr);
    static int            DecodeFunction(void* ptr, AVPacket* in_packet);
    static void           CloseFunction(void* ptr);
           double         GetPTS();
           double         GetLargestOutputPTS();
           int            GetVideoDecoderData(Texture* texture);
};

#endif /* VIDEODECODER_H */
