#ifndef SUBTITLEDECODER_H
#define SUBTITLEDECODER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class SubtitleRenderer;
class TextureAtlas;

#include <Udon/Standard.h>
#include <Udon/Media/Decoder.h>
#include <Udon/Rendering/Texture.h>
#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>
#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>

class SubtitleDecoder : public Decoder {
public:
    SubtitleRenderer* Renderer;
    AVSubtitle ScratchFrame;
    TextureAtlas* Atlas;

                          SubtitleDecoder(MediaSource* src, int stream_index, int video_w, int video_h, int screen_w, int screen_h);
    static void*          CreateSubtitlePacket(bool clear, double pts_start, double pts_end, int pos_x, int pos_y, SDL_Surface* surface);
    static void           FreeSubtitlePacket(void* p);
           int            GetOutputFormat(OutputFormat* output);
    static int            DecodeFunction(void* ptr, AVPacket* in_packet);
    static void           CloseFunction(void* ptr);
           void           SetSize(int w, int h);
           int            GetInfo(Texture *texture, SDL_Rect* sources, SDL_Rect* targets, int limit);
           void           GetTexture(Texture* texture, double sync_ts);
};

#endif /* SUBTITLEDECODER_H */
