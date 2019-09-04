#ifndef RENDERER_H
#define RENDERER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Texture;

#include <Udon/Standard.h>
#include <Udon/Graphics/Color.h>

class Renderer {
public:
    SDL_Renderer* InternalRenderer;
    int           Width;
    int           Height;
    int           RetinaScale;

                     Renderer();
    virtual Texture* CreateTexture(Uint32 format, Uint32 access, Uint32 width, Uint32 height);
    virtual Texture* CreateTextureFromSurface(SDL_Surface* surface);
    virtual int      LockTexture(Texture* texture, void** pixels, int* pitch);
    virtual int      UpdateTexture(Texture* texture, void* pixels, int pitch);
    virtual int      UpdateTexture(Texture* texture, SDL_Rect* src, void* pixels, int pitch);
    virtual int      UpdateYUVTexture(Texture* texture, Uint8* pixelsY, int pitchY, Uint8* pixelsU, int pitchU, Uint8* pixelsV, int pitchV);
    virtual void     UnlockTexture(Texture* texture);
    virtual void     DisposeTexture(Texture* texture);
    virtual void     Clear();
    virtual void     Present();
    virtual void     SetRenderTarget(Texture* texture);
    virtual void     UpdateViewport();
    virtual void     SetClip(int x, int y, int width, int height);
    virtual void     ClearClip();
    virtual void     Save();
    virtual void     Translate(float x, float y, float z);
    virtual void     Rotate(float x, float y, float z);
    virtual void     Scale(float x, float y, float z);
    virtual void     Restore();
    virtual void     SetBlendColor(float r, float g, float b, float a);
    virtual void     SetBlendColor(Color color);
    virtual void     SetLineWidth(float n);
    virtual void     StrokeLine(float x1, float y1, float x2, float y2);
    virtual void     StrokeCircle(float x, float y, float rad);
    virtual void     StrokeEllipse(float x, float y, float w, float h);
    virtual void     StrokeRectangle(float x, float y, float w, float h);
    virtual void     FillCircle(float x, float y, float rad);
    virtual void     FillEllipse(float x, float y, float w, float h);
    virtual void     FillRectangle(float x, float y, float w, float h);
    virtual void     DrawTexture(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h);
    virtual void     DrawTexture(Texture* texture, float x, float y, float w, float h);
    virtual void     DrawTexture(Texture* texture, float x, float y);
    virtual void     DrawTextureBlurred(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h);
    virtual void     DrawTextureBlurred(Texture* texture, float x, float y, float w, float h);
    virtual void     DrawTextureBlurred(Texture* texture, float x, float y);
    virtual void     Dispose();
};

#endif /* RENDERER_H */
