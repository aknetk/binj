#ifndef ASSSUBTITLERENDERER_H
#define ASSSUBTITLERENDERER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Rendering/Texture.h>
#include <Udon/Media/Decoders/Subtitle/Libass.h>
#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>
#include <Udon/Media/Decoders/Subtitle/SubtitleRenderer.h>

class ASSSubtitleRenderer : public SubtitleRenderer {
public:
    ASS_Renderer* renderer;
    ASS_Track* track;

    static void ProcessAssImage(SDL_Surface* surface, const ASS_Image* img);
    static void RenderFunc(SubtitleRenderer* ren, void* sub_src, double pts, double start, double end);
    static int  GetDataFunc(SubtitleRenderer* ren, TextureAtlas* atlas, Texture* texture, double current_pts);
    static void SetSizeFunc(SubtitleRenderer* ren, int w, int h);
    static void CloseFunc(SubtitleRenderer* ren);
                ASSSubtitleRenderer(class Decoder* dec, int video_w, int video_h, int screen_w, int screen_h);
};

#endif /* ASSSUBTITLERENDERER_H */
