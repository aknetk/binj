#ifndef DECODER_H
#define DECODER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Buffer;

#include <Udon/Standard.h>
#include <Udon/Media/LibAV_Includes.h>
#include <Udon/Media/Utils/Buffer.h>
#include <Udon/Media/Utils/Codec.h>
#include <Udon/Media/MediaSource.h>

class Decoder {
public:
    int StreamIndex;
    double ClockSync;
    double ClockPos;
    class Buffer* Buffer[2];
    SDL_mutex* OutputLock;
    AVCodecContext* CodecCtx;
    AVFormatContext* FormatCtx;
    Uint32 Format; // SDL_Format
    int    (*DecodeFunc)(void*, AVPacket*);
    void   (*CloseFunc)(void*);

    static void      FreeInVideoPacketFunc(void* packet);
           void      Create(MediaSource* src, int stream_index, int out_b_size, void (*freeOutFunc)(void*), int thread_count);
           void      Close();
                     ~Decoder();
           int       Run();
           int       GetCodecInfo(Codec* codec);
           int       GetOutputFormat(OutputFormat* output);
           int       GetStreamIndex();
           void      SetClockSync(double sync);
           void      ChangeClockSync(double sync);
           int       WriteInput(AVPacket* packet);
           AVPacket* PeekInput();
           AVPacket* ReadInput();
           bool      CanWriteInput();
           void      AdvanceInput();
           void      ClearInput();
           int       WriteOutput(void* packet);
           void*     PeekOutput();
           void*     ReadOutput();
           bool      CanWriteOutput();
           void      AdvanceOutput();
           void      ClearOutput();
           void      ForEachOutput(void (*cb)(void*, void*), void* userdata);
           Uint32    GetInputLength();
           Uint32    GetOutputLength();
           void      ClearBuffers();
           int       LockOutput();
           void      UnlockOutput();
};

#endif /* DECODER_H */
