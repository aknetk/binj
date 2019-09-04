#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Graphics/Color.h>

need_t Texture;

class Renderer {
public:
    SDL_Renderer* InternalRenderer;
    int           Width;
    int           Height;
    int           RetinaScale;
};
#endif

#include <Udon/Rendering/Renderer.h>

#include <Udon/Rendering/Texture.h>

PUBLIC                  Renderer::Renderer() {

}

PUBLIC VIRTUAL Texture* Renderer::CreateTexture(Uint32 format, Uint32 access, Uint32 width, Uint32 height) {
    Texture* texture = (Texture*)malloc(sizeof(Texture));
    texture->RendererPtr = this;
    texture->Format = format;
    texture->Access = access;
    texture->Width = width;
    texture->Height = height;
    return texture;
}
PUBLIC VIRTUAL Texture* Renderer::CreateTextureFromSurface(SDL_Surface* surface) {
    Texture* texture = new Texture;
    return texture;
}
PUBLIC VIRTUAL int      Renderer::LockTexture(Texture* texture, void** pixels, int* pitch) {
    return 0;
}
PUBLIC VIRTUAL int      Renderer::UpdateTexture(Texture* texture, void* pixels, int pitch) {
    return 0;
}
PUBLIC VIRTUAL int      Renderer::UpdateTexture(Texture* texture, SDL_Rect* src, void* pixels, int pitch) {
    return 0;
}
PUBLIC VIRTUAL int      Renderer::UpdateYUVTexture(Texture* texture, Uint8* pixelsY, int pitchY, Uint8* pixelsU, int pitchU, Uint8* pixelsV, int pitchV) {
    return 0;
}
PUBLIC VIRTUAL void     Renderer::UnlockTexture(Texture* texture) {

}
PUBLIC VIRTUAL void     Renderer::DisposeTexture(Texture* texture) {

}

PUBLIC VIRTUAL void     Renderer::Clear() {

}
PUBLIC VIRTUAL void     Renderer::Present() {

}

PUBLIC VIRTUAL void     Renderer::SetRenderTarget(Texture* texture) {

}
PUBLIC VIRTUAL void     Renderer::UpdateViewport() {

}

PUBLIC VIRTUAL void     Renderer::SetClip(int x, int y, int width, int height) {

}
PUBLIC VIRTUAL void     Renderer::ClearClip() {

}

PUBLIC VIRTUAL void     Renderer::Save() {

}
PUBLIC VIRTUAL void     Renderer::Translate(float x, float y, float z) {

}
PUBLIC VIRTUAL void     Renderer::Rotate(float x, float y, float z) {

}
PUBLIC VIRTUAL void     Renderer::Scale(float x, float y, float z) {

}
PUBLIC VIRTUAL void     Renderer::Restore() {

}

PUBLIC VIRTUAL void     Renderer::SetBlendColor(float r, float g, float b, float a) {

}
PUBLIC VIRTUAL void     Renderer::SetBlendColor(Color color) {

}

PUBLIC VIRTUAL void     Renderer::SetLineWidth(float n) {

}
PUBLIC VIRTUAL void     Renderer::StrokeLine(float x1, float y1, float x2, float y2) {

}
PUBLIC VIRTUAL void     Renderer::StrokeCircle(float x, float y, float rad) {

}
PUBLIC VIRTUAL void     Renderer::StrokeEllipse(float x, float y, float w, float h) {

}
PUBLIC VIRTUAL void     Renderer::StrokeRectangle(float x, float y, float w, float h) {

}

PUBLIC VIRTUAL void     Renderer::FillCircle(float x, float y, float rad) {

}
PUBLIC VIRTUAL void     Renderer::FillEllipse(float x, float y, float w, float h) {

}
PUBLIC VIRTUAL void     Renderer::FillRectangle(float x, float y, float w, float h) {

}

PUBLIC VIRTUAL void     Renderer::DrawTexture(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h) {

}
PUBLIC VIRTUAL void     Renderer::DrawTexture(Texture* texture, float x, float y, float w, float h) {

}
PUBLIC VIRTUAL void     Renderer::DrawTexture(Texture* texture, float x, float y) {

}

PUBLIC VIRTUAL void     Renderer::DrawTextureBlurred(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h) {

}
PUBLIC VIRTUAL void     Renderer::DrawTextureBlurred(Texture* texture, float x, float y, float w, float h) {

}
PUBLIC VIRTUAL void     Renderer::DrawTextureBlurred(Texture* texture, float x, float y) {

}

PUBLIC VIRTUAL void     Renderer::Dispose() {

}
