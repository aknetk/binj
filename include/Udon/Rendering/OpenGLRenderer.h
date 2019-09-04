#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class OpenGLShader;
class OpenGLShader;
class OpenGLShader;
class OpenGLShader;
class OpenGLShader;
class Matrix4x4;
class Texture;
class Texture;
class Texture;

#include <Udon/Standard.h>
#include <Udon/Math/Matrix4x4.h>
#include <Udon/Rendering/Renderer.h>
#include <Udon/Rendering/OpenGLShader.h>
#include <stack>

class OpenGLRenderer : public Renderer {
private:
    void    MakeShaders();
    void    MakeShapeBuffers();
    void    UseShader(OpenGLShader* shader);
    void    DrawTextureRaw(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h);

public:
    SDL_GLContext     Context;
    OpenGLShader*     CurrentShader = NULL;
    OpenGLShader*     ShaderShape = NULL;
    OpenGLShader*     ShaderTexturedShape = NULL;
    OpenGLShader*     ShaderTexturedShapeYUV = NULL;
    OpenGLShader*     ShaderTexturedShapeBlur = NULL;
    Matrix4x4*        ProjectionMatrix;
    stack<Matrix4x4*> ModelViewMatrix;
    GLint             DefaultFramebuffer;
    GLuint            BufferCircleFill;
    GLuint            BufferCircleStroke;
    GLuint            BufferSquareFill;
    float             BlendColors[4];
    Texture*          FramebufferTextureA = NULL;
    Texture*          FramebufferTextureB = NULL;
    Texture*          CurrentRenderTarget = NULL;

             OpenGLRenderer();
    Texture* CreateTexture(Uint32 format, Uint32 access, Uint32 width, Uint32 height);
    Texture* CreateTextureFromSurface(SDL_Surface* surface);
    int      LockTexture(Texture* texture, void** pixels, int* pitch);
    int      UpdateTexture(Texture* texture, void* pixels, int pitch);
    int      UpdateTexture(Texture* texture, SDL_Rect* src, void* pixels, int pitch);
    int      UpdateYUVTexture(Texture* texture, Uint8* pixelsY, int pitchY, Uint8* pixelsU, int pitchU, Uint8* pixelsV, int pitchV);
    void     UnlockTexture(Texture* texture);
    void     DisposeTexture(Texture* texture);
    void     Clear();
    void     Present();
    void     SetRenderTarget(Texture* texture);
    void     UpdateViewport();
    void     SetClip(int x, int y, int width, int height);
    void     ClearClip();
    void     Save();
    void     Translate(float x, float y, float z);
    void     Rotate(float x, float y, float z);
    void     Scale(float x, float y, float z);
    void     Restore();
    void     SetBlendColor(float r, float g, float b, float a);
    void     SetBlendColor(Color color);
    void     SetLineWidth(float n);
    void     StrokeLine(float x1, float y1, float x2, float y2);
    void     StrokeCircle(float x, float y, float rad);
    void     StrokeEllipse(float x, float y, float w, float h);
    void     StrokeRectangle(float x, float y, float w, float h);
    void     FillCircle(float x, float y, float rad);
    void     FillEllipse(float x, float y, float w, float h);
    void     FillRectangle(float x, float y, float w, float h);
    void     DrawTexture(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h);
    void     DrawTexture(Texture* texture, float x, float y, float w, float h);
    void     DrawTexture(Texture* texture, float x, float y);
    void     DrawTextureBlurred(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h);
    void     DrawTextureBlurred(Texture* texture, float x, float y, float w, float h);
    void     DrawTextureBlurred(Texture* texture, float x, float y);
    void     Dispose();
};

#endif /* OPENGLRENDERER_H */
