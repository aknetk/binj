#if INTERFACE
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
};
#endif

#include <Udon/Media/Decoders/Subtitle/SubtitleRenderer.h>

#include <Udon/Media/Decoder.h>

#include <Udon/Logging/Log.h>

PUBLIC void SubtitleRenderer::Create(void* dec) {
    // SubtitleRenderer* ren = (SubtitleRenderer*)calloc(1, sizeof(SubtitleRenderer));
    // if (ren == NULL) {
    //     Log::Print(Log::LOG_ERROR, "Unable to allocate kit subtitle renderer");
    //     return;
    // }
    this->Decoder = dec;
}
PUBLIC void SubtitleRenderer::Run(void* src, double pts, double start, double end) {
    if (this->RenderFuncPtr)
        this->RenderFuncPtr(this, src, pts, start, end);
}
PUBLIC int  SubtitleRenderer::GetData(TextureAtlas* atlas, Texture* texture, double current_pts) {
    if (this->GetDataFuncPtr)
        return this->GetDataFuncPtr(this, atlas, texture, current_pts);
    return 0;
}
PUBLIC void SubtitleRenderer::SetSize(int w, int h) {
    if (this->SetSizeFuncPtr)
        this->SetSizeFuncPtr(this, w, h);
}
PUBLIC void SubtitleRenderer::Close() {
    if (this->CloseFuncPtr)
        this->CloseFuncPtr(this);
}
PUBLIC      SubtitleRenderer::~SubtitleRenderer() {
    this->Close();
    // free(this);
}
