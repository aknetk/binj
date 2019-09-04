#ifndef IMAGESUBTITLERENDERER_H
#define IMAGESUBTITLERENDERER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Rendering/Texture.h>
#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>
#include <Udon/Media/Decoders/Subtitle/SubtitleRenderer.h>

class ImageSubtitleRenderer : public SubtitleRenderer {
public:
    int   video_w;
    int   video_h;
    float scale_x;
    float scale_y;

    static void RenderFunc(SubtitleRenderer* ren, void* sub_src, double pts, double start, double end);
    static int  GetDataFunc(SubtitleRenderer* ren, TextureAtlas* atlas, Texture* texture, double current_pts);
    static void SetSizeFunc(SubtitleRenderer* ren, int w, int h);
    static void CloseFunc(SubtitleRenderer *ren);
                ImageSubtitleRenderer(class Decoder* dec, int video_w, int video_h, int screen_w, int screen_h);
};

#endif /* IMAGESUBTITLERENDERER_H */
