#ifndef SUBTITLERENDERER_H
#define SUBTITLERENDERER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Rendering/Texture.h>
#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>

class SubtitleRenderer {
public:
    void*  Decoder;
    void (*RenderFuncPtr)(SubtitleRenderer*, void*, double, double, double);
    int  (*GetDataFuncPtr)(SubtitleRenderer*, TextureAtlas*, Texture*, double);
    void (*SetSizeFuncPtr)(SubtitleRenderer*, int, int);
    void (*CloseFuncPtr)(SubtitleRenderer*);

    void Create(void* dec);
    void Run(void* src, double pts, double start, double end);
    int  GetData(TextureAtlas* atlas, Texture* texture, double current_pts);
    void SetSize(int w, int h);
    void Close();
         ~SubtitleRenderer();
};

#endif /* SUBTITLERENDERER_H */
