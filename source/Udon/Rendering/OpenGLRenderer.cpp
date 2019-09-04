#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Math/Matrix4x4.h>
#include <Udon/Rendering/Renderer.h>
#include <Udon/Rendering/OpenGLShader.h>

#include <stack>

class OpenGLRenderer : public Renderer {
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
};
#endif

#include <Udon/Rendering/OpenGLRenderer.h>

#include <Udon/Application.h>
#include <Udon/Logging/Log.h>
#include <Udon/Rendering/Texture.h>

struct OpenGLTextureData {
    GLuint TextureID;
    GLuint TextureU;
    GLuint TextureV;
    bool   YUV;
    bool   Framebuffer;
    GLuint FBO;
    GLenum TextureTarget;
    GLenum TextureStorageFormat;
    GLenum PixelDataFormat;
    GLenum PixelDataType;
};
int    OpenGLMultisampleSampleCount = 8;

#define GL_SUPPORTS_MULTISAMPLING
#define GL_SUPPORTS_SMOOTHING

#if GL_ES_VERSION_2_0 || GL_ES_VERSION_3_0
#define GL_ES
#undef GL_SUPPORTS_MULTISAMPLING
#undef GL_SUPPORTS_SMOOTHING
#endif

PUBLIC          OpenGLRenderer::OpenGLRenderer() {
    Uint32 windowFlags =
        SDL_WINDOW_SHOWN |
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_ALLOW_HIGHDPI |
        SDL_WINDOW_BORDERLESS;

    switch (Application::Platform) {
        case Platforms::Windows:
            Width = 1280;
            Height = 720;
            break;
        case Platforms::MacOSX:
            Width = 960;
            Height = 540;
            // windowFlags |=  SDL_WINDOW_FULLSCREEN_DESKTOP;
            windowFlags &= ~SDL_WINDOW_BORDERLESS;
            break;
        case Platforms::Switch:
            Width = 1280;
            Height = 720;
            windowFlags |=  SDL_WINDOW_FULLSCREEN;
            windowFlags &= ~SDL_WINDOW_RESIZABLE;
            windowFlags &= ~SDL_WINDOW_ALLOW_HIGHDPI;
            break;
        default:
            Width = 1280;
            Height = 720;
            break;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    #ifdef GL_SUPPORTS_MULTISAMPLING
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, OpenGLMultisampleSampleCount);
    #endif

    Application::Window = SDL_CreateWindow(NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        Width, Height, windowFlags);
    if (!Application::Window) {
        Log::Print(Log::LOG_ERROR, "Could not create window: %s", SDL_GetError());
        exit(0);
    }

    Context = SDL_GL_CreateContext(Application::Window);
    if (!Context) {
        Log::Print(Log::LOG_ERROR, "Could not create OpenGL context: %s", SDL_GetError());
        exit(0);
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        Log::Print(Log::LOG_WARN, "Could not enable V-Sync: %s", SDL_GetError());
    }

    int max, w, h, ww, wh;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
    SDL_GL_GetDrawableSize(Application::Window, &w, &h);
    SDL_GetWindowSize(Application::Window, &ww, &wh);

    RetinaScale = 1;
    if (h > wh)
        RetinaScale = 2;
    BlendColors[0] = BlendColors[1] = BlendColors[2] = BlendColors[3] = 1.0f;

    Log::Print(Log::LOG_INFO, "OpenGL Version: %s", glGetString(GL_VERSION));
    Log::Print(Log::LOG_INFO, "GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    Log::Print(Log::LOG_INFO, "CPU Core Count: %d", SDL_GetCPUCount());
    Log::Print(Log::LOG_INFO, "Max Texture Size: %d x %d", max, max);
    Log::Print(Log::LOG_INFO, "Window Size: %d x %d", ww, wh);
    Log::Print(Log::LOG_INFO, "Drawable Size: %d x %d", w, h);

    // Enable/Disable GL features
    glEnable(GL_BLEND); OpenGLShader::CheckGLError(__LINE__);
    glDisable(GL_DEPTH_TEST); OpenGLShader::CheckGLError(__LINE__);
    // glEnable(GL_TEXTURE_2D); OpenGLShader::CheckGLError(__LINE__);

    #ifdef GL_SUPPORTS_MULTISAMPLING
        glEnable(GL_MULTISAMPLE); OpenGLShader::CheckGLError(__LINE__);
    #endif

    // glBlendFunc(GL_ONE, GL_ONE); OpenGLShader::CheckGLError(__LINE__);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); OpenGLShader::CheckGLError(__LINE__);
    glDepthFunc(GL_LEQUAL); OpenGLShader::CheckGLError(__LINE__);

    #ifdef GL_SUPPORTS_SMOOTHING
        glEnable(GL_LINE_SMOOTH); OpenGLShader::CheckGLError(__LINE__);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); OpenGLShader::CheckGLError(__LINE__);

        glEnable(GL_POLYGON_SMOOTH); OpenGLShader::CheckGLError(__LINE__);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST); OpenGLShader::CheckGLError(__LINE__);
    #endif

    OpenGLRenderer::MakeShaders();
    OpenGLRenderer::MakeShapeBuffers();

    ProjectionMatrix = Matrix4x4::Create();
    ModelViewMatrix.push(Matrix4x4::Create());

    UseShader(ShaderShape);

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &DefaultFramebuffer); OpenGLShader::CheckGLError(__LINE__);

    FramebufferTextureA = CreateTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, Width, Height);
    FramebufferTextureB = CreateTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, Width, Height);
}

PRIVATE void    OpenGLRenderer::MakeShaders() {
    // "    offset[0] = 0.0;\n",
    // "    offset[1] = 1.3846153846;\n",
    // "    offset[2] = 3.2307692308;\n",
    // "    weight[0] = 0.2270270270;\n",
    // "    weight[1] = 0.3162162162;\n",
    // "    weight[2] = 0.0702702703;\n",
    const GLchar* vertexShaderSource[] = {
        // "precision mediump float;\n",

        "attribute vec3    i_position;\n",
        "attribute vec2    i_uv;\n",
        "varying vec2      o_uv;\n",

        "uniform mat4      u_projectionMatrix;\n",
        "uniform mat4      u_modelViewMatrix;\n",

        "void main() {\n",
        "    gl_Position = u_projectionMatrix * u_modelViewMatrix * vec4(i_position, 1.0);\n",

        "    o_uv = i_uv;\n",
        "}",
    };
    const GLchar* fragmentShaderSource_Shape[] = {
        #ifdef GL_ES
        "precision mediump float;\n",
        #endif
        "varying vec2      o_uv;\n",

        "uniform vec4      u_color;\n",

        "void main() {",
        "    gl_FragColor = u_color;",
        "}",
    };
    const GLchar* fragmentShaderSource_TexturedShape[] = {
        #ifdef GL_ES
        "precision mediump float;\n",
        #endif
        "varying vec2      o_uv;\n",

        "uniform vec4      u_color;\n",
        "uniform sampler2D u_texture;\n",

        "void main() {\n",
        "    gl_FragColor = texture2D(u_texture, o_uv) * u_color;\n",
        "}",
    };
    const GLchar* fragmentShaderSource_TexturedShapeYUV[] = {
        #ifdef GL_ES
        "precision mediump float;\n",
        #endif
        "varying vec2      o_uv;\n",

        "uniform vec4      u_color;\n",
        "uniform sampler2D u_texture;\n",
        "uniform sampler2D u_textureU;\n",
        "uniform sampler2D u_textureV;\n",

        "const vec3 offset = vec3(-0.0625, -0.5, -0.5);\n",
        "const vec3 Rcoeff = vec3(1.164,  0.000,  1.596);\n",
        "const vec3 Gcoeff = vec3(1.164, -0.391, -0.813);\n",
        "const vec3 Bcoeff = vec3(1.164,  2.018,  0.000);\n",

        "void main() {\n",
        "    vec3 yuv, rgb;\n",
        "    vec2 uv = o_uv;\n"

        "    yuv.x = texture2D(u_texture,  uv).r;\n",
        "    yuv.y = texture2D(u_textureU, uv).r;\n",
        "    yuv.z = texture2D(u_textureV, uv).r;\n",
        "    yuv += offset;\n",

        "    rgb.r = dot(yuv, Rcoeff);\n",
        "    rgb.g = dot(yuv, Gcoeff);\n",
        "    rgb.b = dot(yuv, Bcoeff);\n",
        "    gl_FragColor = vec4(rgb, 1.0) * u_color;\n",
        "}",
    };
    const GLchar* fragmentShaderSource_TexturedShapeBlur[] = {
        #ifdef GL_ES
        "precision mediump float;\n",
        #endif
        "varying vec2      o_uv;\n",

        "uniform vec4      u_color;\n",
        "uniform sampler2D u_texture;\n",
        "uniform vec2      u_textureSize;\n",
        "uniform vec2      u_directionVector;\n",

        "const float Quality = 8.0;\n",
        "const float Directions = 16.0;\n",
        "const float Tau = 6.28318530718;\n",

        "void main() {\n",
        "    vec2 trig;\n",
        "    float dib = Quality * (Directions - 1.0);\n"
        "    vec4 color = texture2D(u_texture, o_uv);\n",
        "    vec2 radius = u_directionVector.x / u_textureSize;\n",
        "    for (float d = Tau / Directions; d <= Tau; d += Tau / Directions) {\n",
        "        trig = vec2(cos(d), sin(d));\n",
        "        for (float i = 1.0 / Quality; i <= 1.0; i += 1.0 / Quality) {\n",
        "            color += texture2D(u_texture, o_uv + trig * radius * i);\n",
        "        }\n",
        "    }\n",
        "    color /= dib;\n",
        "    gl_FragColor = color * u_color;\n",
        "}",
    };

    ShaderShape             = new OpenGLShader(vertexShaderSource, sizeof(vertexShaderSource), fragmentShaderSource_Shape, sizeof(fragmentShaderSource_Shape));
    ShaderTexturedShape     = new OpenGLShader(vertexShaderSource, sizeof(vertexShaderSource), fragmentShaderSource_TexturedShape, sizeof(fragmentShaderSource_TexturedShape));
    ShaderTexturedShapeYUV  = new OpenGLShader(vertexShaderSource, sizeof(vertexShaderSource), fragmentShaderSource_TexturedShapeYUV, sizeof(fragmentShaderSource_TexturedShapeYUV));
    ShaderTexturedShapeBlur = new OpenGLShader(vertexShaderSource, sizeof(vertexShaderSource), fragmentShaderSource_TexturedShapeBlur, sizeof(fragmentShaderSource_TexturedShapeBlur));
}
PRIVATE void    OpenGLRenderer::MakeShapeBuffers() {
    float vertices[360 * 3 + 6];
    // Filled Circle
    vertices[0] = 0.0f; vertices[1] = 0.0f; vertices[2] = 0.0f;
    for (int i = 0; i < 361; i++) {
        vertices[3 + i * 3] = cos(i * M_PI / 180.0f);
        vertices[4 + i * 3] = sin(i * M_PI / 180.0f);
        vertices[5 + i * 3] = 0.0f;
    }
    glGenBuffers(1, &BufferCircleFill);
    glBindBuffer(GL_ARRAY_BUFFER, BufferCircleFill);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Stroke Circle
    float vertices2[361 * 3];
    for (int i = 0; i < 361; i++) {
        vertices2[0 + i * 3] = cos(i * M_PI / 180.0f);
        vertices2[1 + i * 3] = sin(i * M_PI / 180.0f);
        vertices2[2 + i * 3] = 0.0f;
    }
    glGenBuffers(1, &BufferCircleStroke);
    glBindBuffer(GL_ARRAY_BUFFER, BufferCircleStroke);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    // Fill Square
    float verts3[8];
    verts3[0] = 0.0f; verts3[1] = 0.0f;
    verts3[2] = 1.0f; verts3[3] = 0.0f;
    verts3[4] = 0.0f; verts3[5] = 1.0f;
    verts3[6] = 1.0f; verts3[7] = 1.0f;
    glGenBuffers(1, &BufferSquareFill);
    glBindBuffer(GL_ARRAY_BUFFER, BufferSquareFill);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts3), verts3, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
PRIVATE void    OpenGLRenderer::UseShader(OpenGLShader* shader) {
    if (CurrentShader != shader) {
        CurrentShader = shader;
        CurrentShader->Use();

        glEnableVertexAttribArray(CurrentShader->LocPosition);
        // glEnableVertexAttribArray(CurrentShader->LocTexCoord);
    }

    Matrix4x4::Ortho(ProjectionMatrix, 0.0f, Width, Height, 0.0f, -10.0f, 10.0f);
    glUniformMatrix4fv(CurrentShader->LocProjectionMatrix, 1, false, ProjectionMatrix->Values);
    glUniform4f(CurrentShader->LocColor, BlendColors[0], BlendColors[1], BlendColors[2], BlendColors[3]);
}

PUBLIC Texture* OpenGLRenderer::CreateTexture(Uint32 format, Uint32 access, Uint32 width, Uint32 height) {
    Texture* texture = Renderer::CreateTexture(format, access, width, height);
    texture->DriverData = calloc(1, sizeof(OpenGLTextureData));

    OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;

    textureData->TextureTarget = GL_TEXTURE_2D;

    textureData->TextureStorageFormat = GL_RGBA;
    textureData->PixelDataFormat = GL_RGBA;
    textureData->PixelDataType = GL_UNSIGNED_BYTE;
    switch (texture->Format) {
        case SDL_PIXELFORMAT_ARGB8888:
            textureData->TextureStorageFormat = GL_RGBA;
            textureData->PixelDataFormat = GL_BGRA;
            textureData->PixelDataType = GL_UNSIGNED_BYTE;
            break;
        case SDL_PIXELFORMAT_YV12:
        case SDL_PIXELFORMAT_IYUV:
        case SDL_PIXELFORMAT_NV12:
        case SDL_PIXELFORMAT_NV21:
            textureData->TextureStorageFormat = GL_RED;
            textureData->PixelDataFormat = GL_RED;
            textureData->PixelDataType = GL_UNSIGNED_BYTE;
            break;
        default:
            break;
    }

    switch (access) {
        case SDL_TEXTUREACCESS_TARGET:
            textureData->Framebuffer = true;
            glGenFramebuffers(1, &textureData->FBO); OpenGLShader::CheckGLError(__LINE__);

            width *= RetinaScale;
            height *= RetinaScale;
            if (false) { // If Multisampling enabled
                textureData->TextureTarget = GL_TEXTURE_2D_MULTISAMPLE;
            }
            break;
        case SDL_TEXTUREACCESS_STREAMING:
            texture->Pitch = texture->Width * SDL_BYTESPERPIXEL(texture->Format);

            size_t size = texture->Pitch * texture->Height;
            if (texture->Format == SDL_PIXELFORMAT_YV12 ||
                texture->Format == SDL_PIXELFORMAT_IYUV) {
                // Need to add size for the U and V planes.
                size += 2 * ((texture->Height + 1) / 2) * ((texture->Pitch + 1) / 2);
            }
            if (texture->Format == SDL_PIXELFORMAT_NV12 ||
                texture->Format == SDL_PIXELFORMAT_NV21) {
                // Need to add size for the U/V plane.
                size += 2 * ((texture->Height + 1) / 2) * ((texture->Pitch + 1) / 2);
            }
            texture->Pixels = calloc(1, size);
            break;
    }

    glGenTextures(1, &textureData->TextureID);
    glBindTexture(textureData->TextureTarget, textureData->TextureID);
    switch (textureData->TextureTarget) {
        // case GL_TEXTURE_2D_MULTISAMPLE:
        //     glTexImage2DMultisample(textureData->TextureTarget, OpenGLMultisampleSampleCount, textureData->PixelDataFormat, width, height, false);
        //     break;
        default:
            glTexImage2D(textureData->TextureTarget, 0, textureData->TextureStorageFormat, width, height, 0, textureData->PixelDataFormat, textureData->PixelDataType, NULL);
            break;
    }
    // glTexParameteri(textureData->TextureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(textureData->TextureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(textureData->TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(textureData->TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(textureData->TextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(textureData->TextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (texture->Format == SDL_PIXELFORMAT_YV12 ||
        texture->Format == SDL_PIXELFORMAT_IYUV) {
        textureData->YUV = true;

        glGenTextures(1, &textureData->TextureU);
        glGenTextures(1, &textureData->TextureV);

        glBindTexture(textureData->TextureTarget, textureData->TextureU);
        glTexParameteri(textureData->TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(textureData->TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(textureData->TextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(textureData->TextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(textureData->TextureTarget, 0, textureData->TextureStorageFormat, (width + 1) / 2, (height + 1) / 2, 0, textureData->PixelDataFormat, textureData->PixelDataType, NULL);

        glBindTexture(textureData->TextureTarget, textureData->TextureV);
        glTexParameteri(textureData->TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(textureData->TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(textureData->TextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(textureData->TextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(textureData->TextureTarget, 0, textureData->TextureStorageFormat, (width + 1) / 2, (height + 1) / 2, 0, textureData->PixelDataFormat, textureData->PixelDataType, NULL);
    }

    glBindTexture(textureData->TextureTarget, 0);

    return texture;
}
PUBLIC Texture* OpenGLRenderer::CreateTextureFromSurface(SDL_Surface* surface) {
    surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);

    Texture* texture = OpenGLRenderer::CreateTexture(surface->format->format, SDL_TEXTUREACCESS_STATIC, surface->w, surface->h);
    OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;

    switch (surface->format->format) {
        case SDL_PIXELFORMAT_ARGB8888:
            textureData->TextureStorageFormat = GL_RGBA;
            textureData->PixelDataFormat = GL_BGRA;
            textureData->PixelDataType = GL_UNSIGNED_BYTE;
            break;
        case SDL_PIXELFORMAT_YV12:
        case SDL_PIXELFORMAT_IYUV:
        case SDL_PIXELFORMAT_NV12:
        case SDL_PIXELFORMAT_NV21:
            textureData->TextureStorageFormat = GL_RED;
            textureData->PixelDataFormat = GL_RED;
            textureData->PixelDataType = GL_UNSIGNED_BYTE;
            break;
        default:
            textureData->TextureStorageFormat = GL_RGBA;
            textureData->PixelDataFormat = GL_BGRA;
            textureData->PixelDataType = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->w);
    glBindTexture(textureData->TextureTarget, textureData->TextureID);
    glTexImage2D(textureData->TextureTarget, 0, textureData->TextureStorageFormat,
        texture->Width, texture->Height, 0,
        textureData->PixelDataFormat, textureData->PixelDataType, surface->pixels);
    glBindTexture(textureData->TextureTarget, 0);

    SDL_FreeSurface(surface);
    return texture;
}
PUBLIC int      OpenGLRenderer::LockTexture(Texture* texture, void** pixels, int* pitch) {
    return 0;
}
PUBLIC int      OpenGLRenderer::UpdateTexture(Texture* texture, void* pixels, int pitch) {
    return OpenGLRenderer::UpdateTexture(texture, NULL, pixels, pitch);
}
PUBLIC int      OpenGLRenderer::UpdateTexture(Texture* texture, SDL_Rect* src, void* pixels, int pitch) {
    int inputPixelsX = 0;
    int inputPixelsY = 0;
    int inputPixelsW = texture->Width;
    int inputPixelsH = texture->Height;
    if (src) {
        // inputPixelsX = src->x;
        // inputPixelsY = src->y;
        // inputPixelsW = src->w;
        // inputPixelsH = src->h;
    }

    OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;
    glBindTexture(textureData->TextureTarget, textureData->TextureID);
    glTexSubImage2D(textureData->TextureTarget, 0,
        inputPixelsX, inputPixelsY, inputPixelsW, inputPixelsH,
        textureData->PixelDataFormat, textureData->PixelDataType, pixels);

    if (textureData->YUV) {
        pixels = (void*)((Uint8*)pixels + inputPixelsH * pitch);
        glBindTexture(textureData->TextureTarget, texture->Format == SDL_PIXELFORMAT_YV12 ? textureData->TextureV : textureData->TextureU);

        inputPixelsX = inputPixelsX / 2;
        inputPixelsY = inputPixelsY / 2;
        inputPixelsW = (inputPixelsW + 1) / 2;
        inputPixelsH = (inputPixelsH + 1) / 2;

        glTexSubImage2D(textureData->TextureTarget, 0,
            inputPixelsX, inputPixelsY, inputPixelsW, inputPixelsH,
            textureData->PixelDataFormat, textureData->PixelDataType, pixels);

        pixels = (void*)((Uint8*)pixels + inputPixelsH * ((pitch + 1) / 2));
        glBindTexture(textureData->TextureTarget, texture->Format == SDL_PIXELFORMAT_YV12 ? textureData->TextureU : textureData->TextureV);

        glTexSubImage2D(textureData->TextureTarget, 0,
            inputPixelsX, inputPixelsY, inputPixelsW, inputPixelsH,
            textureData->PixelDataFormat, textureData->PixelDataType, pixels);
    }
    return 0;
}
PUBLIC int      OpenGLRenderer::UpdateYUVTexture(Texture* texture, Uint8* pixelsY, int pitchY, Uint8* pixelsU, int pitchU, Uint8* pixelsV, int pitchV) {
    int inputPixelsX = 0;
    int inputPixelsY = 0;
    int inputPixelsW = texture->Width;
    int inputPixelsH = texture->Height;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;

    glPixelStorei(GL_UNPACK_ROW_LENGTH, pitchY);
    glBindTexture(textureData->TextureTarget, textureData->TextureID);
    glTexSubImage2D(textureData->TextureTarget, 0,
        inputPixelsX, inputPixelsY, inputPixelsW, inputPixelsH,
        textureData->PixelDataFormat, textureData->PixelDataType, pixelsY);

    inputPixelsX = inputPixelsX / 2;
    inputPixelsY = inputPixelsY / 2;
    inputPixelsW = (inputPixelsW + 1) / 2;
    inputPixelsH = (inputPixelsH + 1) / 2;

    glPixelStorei(GL_UNPACK_ROW_LENGTH, pitchU);
    glBindTexture(textureData->TextureTarget, textureData->TextureU);
    glTexSubImage2D(textureData->TextureTarget, 0,
        inputPixelsX, inputPixelsY, inputPixelsW, inputPixelsH,
        textureData->PixelDataFormat, textureData->PixelDataType, pixelsU);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, pitchV);
    glBindTexture(textureData->TextureTarget, textureData->TextureV);
    glTexSubImage2D(textureData->TextureTarget, 0,
        inputPixelsX, inputPixelsY, inputPixelsW, inputPixelsH,
        textureData->PixelDataFormat, textureData->PixelDataType, pixelsV);
    return 0;
}
PUBLIC void     OpenGLRenderer::UnlockTexture(Texture* texture) {

}
PUBLIC void     OpenGLRenderer::DisposeTexture(Texture* texture) {
    OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;
    if (texture->Access == SDL_TEXTUREACCESS_TARGET) {
        glDeleteFramebuffers(1, &textureData->FBO);
    }
    else if (texture->Access == SDL_TEXTUREACCESS_STREAMING) {
        free(texture->Pixels);
    }
    if (textureData->YUV) {
        glDeleteTextures(1, &textureData->TextureU);
        glDeleteTextures(1, &textureData->TextureV);
    }
    glDeleteTextures(1, &textureData->TextureID);
    free(texture->DriverData);
    free(texture);
}

PUBLIC void     OpenGLRenderer::Clear() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0.0, 0.0, Width * RetinaScale, Height * RetinaScale);
}
PUBLIC void     OpenGLRenderer::Present() {
    SDL_GL_SwapWindow(Application::Window);
}

PUBLIC void     OpenGLRenderer::SetRenderTarget(Texture* texture) {
    CurrentRenderTarget = texture;
    if (texture == NULL) {
        glBindFramebuffer(GL_FRAMEBUFFER, DefaultFramebuffer); OpenGLShader::CheckGLError(__LINE__);
    }
    else {
        OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;

        glBindFramebuffer(GL_FRAMEBUFFER, textureData->FBO); OpenGLShader::CheckGLError(__LINE__);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureData->TextureTarget, textureData->TextureID, 0); OpenGLShader::CheckGLError(__LINE__);

        // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            // return SDL_SetError("glFramebufferTexture2D() failed");
        // }
    }
}
PUBLIC void     OpenGLRenderer::UpdateViewport() {

}

PUBLIC void     OpenGLRenderer::SetClip(int x, int y, int width, int height) {
    glEnable(GL_SCISSOR_TEST);
    // glScissor(x, Height - y, width, height);
    glScissor(
        x * RetinaScale,
        (Height - y - height) * RetinaScale,
        width * RetinaScale,
        height * RetinaScale);
}
PUBLIC void     OpenGLRenderer::ClearClip() {
    glDisable(GL_SCISSOR_TEST);
}

PUBLIC void     OpenGLRenderer::Save() {
    Matrix4x4* top = ModelViewMatrix.top();
    Matrix4x4* push = Matrix4x4::Create();
    Matrix4x4::Copy(push, top);

    if (ModelViewMatrix.size() == 256) { Log::Print(Log::LOG_ERROR, "Stack too big."); exit(0); }

    ModelViewMatrix.push(push);
}
PUBLIC void     OpenGLRenderer::Translate(float x, float y, float z) {
    if (ModelViewMatrix.size() == 1) return;

    Matrix4x4::Translate(ModelViewMatrix.top(), ModelViewMatrix.top(), x, y, z);
}
PUBLIC void     OpenGLRenderer::Rotate(float x, float y, float z) {

}
PUBLIC void     OpenGLRenderer::Scale(float x, float y, float z) {
    if (ModelViewMatrix.size() == 1) return;

    Matrix4x4::Scale(ModelViewMatrix.top(), ModelViewMatrix.top(), x, y, z);
}
PUBLIC void     OpenGLRenderer::Restore() {
    if (ModelViewMatrix.size() == 1) return;

    ModelViewMatrix.pop();
}

PUBLIC void     OpenGLRenderer::SetBlendColor(float r, float g, float b, float a) {
    BlendColors[0] = r;
    BlendColors[1] = g;
    BlendColors[2] = b;
    BlendColors[3] = a;
    glUniform4f(CurrentShader->LocColor, BlendColors[0], BlendColors[1], BlendColors[2], BlendColors[3]);
}
PUBLIC void     OpenGLRenderer::SetBlendColor(Color color) {
    SetBlendColor(color.R / 255.0, color.G / 255.0, color.B / 255.0, color.A / 255.0);
}

PUBLIC void     OpenGLRenderer::SetLineWidth(float n) {
    glLineWidth(n);
}
PUBLIC void     OpenGLRenderer::StrokeLine(float x1, float y1, float x2, float y2) {
    UseShader(ShaderShape);

    Save();
        glUniformMatrix4fv(CurrentShader->LocModelViewMatrix, 1, false, ModelViewMatrix.top()->Values);

        float v[6];
        v[0] = x1; v[1] = y1; v[2] = 0.0f;
        v[3] = x2; v[4] = y2; v[5] = 0.0f;

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(CurrentShader->LocPosition, 3, GL_FLOAT, GL_FALSE, 0, v);
        glDrawArrays(GL_LINES, 0, 2);
    Restore();
}
PUBLIC void     OpenGLRenderer::StrokeCircle(float x, float y, float rad) {
    UseShader(ShaderShape);

    Save();
    Translate(x, y, 0.0f);
    Scale(rad, rad, 1.0f);
        glUniformMatrix4fv(CurrentShader->LocModelViewMatrix, 1, false, ModelViewMatrix.top()->Values);

        glBindBuffer(GL_ARRAY_BUFFER, BufferCircleStroke);
        glVertexAttribPointer(CurrentShader->LocPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_LINE_STRIP, 0, 361);
    Restore();
}
PUBLIC void     OpenGLRenderer::StrokeEllipse(float x, float y, float w, float h) {
    UseShader(ShaderShape);

    Save();
    Translate(x + w / 2, y + h / 2, 0.0f);
    Scale(w / 2, h / 2, 1.0f);
        glUniformMatrix4fv(CurrentShader->LocModelViewMatrix, 1, false, ModelViewMatrix.top()->Values);

        glBindBuffer(GL_ARRAY_BUFFER, BufferCircleStroke);
        glVertexAttribPointer(CurrentShader->LocPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_LINE_STRIP, 0, 361);
    Restore();
}
PUBLIC void     OpenGLRenderer::StrokeRectangle(float x, float y, float w, float h) {
    StrokeLine(x, y, x + w, y);
    StrokeLine(x, y + h, x + w, y + h);

    StrokeLine(x, y, x, y + h);
    StrokeLine(x + w, y, x + w, y + h);
}

PUBLIC void     OpenGLRenderer::FillCircle(float x, float y, float rad) {
    UseShader(ShaderShape);

    Save();
    Translate(x, y, 0.0f);
    Scale(rad, rad, 1.0f);
        glUniformMatrix4fv(CurrentShader->LocModelViewMatrix, 1, false, ModelViewMatrix.top()->Values);

        glBindBuffer(GL_ARRAY_BUFFER, BufferCircleFill);
        glVertexAttribPointer(CurrentShader->LocPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 362);
    Restore();
}
PUBLIC void     OpenGLRenderer::FillEllipse(float x, float y, float w, float h) {
    UseShader(ShaderShape);

    Save();
    Translate(x + w / 2, y + h / 2, 0.0f);
    Scale(w / 2, h / 2, 1.0f);
        glUniformMatrix4fv(CurrentShader->LocModelViewMatrix, 1, false, ModelViewMatrix.top()->Values);

        glBindBuffer(GL_ARRAY_BUFFER, BufferCircleFill);
        glVertexAttribPointer(CurrentShader->LocPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 362);
    Restore();
}
PUBLIC void     OpenGLRenderer::FillRectangle(float x, float y, float w, float h) {
    UseShader(ShaderShape);

    Save();
    Translate(x, y, 0.0f);
    Scale(w, h, 1.0f);
        glUniformMatrix4fv(CurrentShader->LocModelViewMatrix, 1, false, ModelViewMatrix.top()->Values);

        glBindBuffer(GL_ARRAY_BUFFER, BufferSquareFill);
        glVertexAttribPointer(CurrentShader->LocPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    Restore();
}

PRIVATE void    OpenGLRenderer::DrawTextureRaw(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h) {
    OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;
    Save();
        if (textureData->FBO) {
            Translate(x, y + h, 0.0f);
            Scale(w, -h, 1.0f);
        }
        else {
            Translate(x, y, 0.0f);
            Scale(w, h, 1.0f);
        }
        glUniformMatrix4fv(CurrentShader->LocModelViewMatrix, 1, false, ModelViewMatrix.top()->Values);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(CurrentShader->LocTexture, 0);
        glBindTexture(GL_TEXTURE_2D, textureData->TextureID);

        if (textureData->YUV) {
            glActiveTexture(GL_TEXTURE0 + 1);
            glUniform1i(CurrentShader->LocTextureU, 1);
            glBindTexture(GL_TEXTURE_2D, textureData->TextureU);

            glActiveTexture(GL_TEXTURE0 + 2);
            glUniform1i(CurrentShader->LocTextureV, 2);
            glBindTexture(GL_TEXTURE_2D, textureData->TextureV);
        }

        glEnableVertexAttribArray(CurrentShader->LocTexCoord);

        glBindBuffer(GL_ARRAY_BUFFER, BufferSquareFill);
        glVertexAttribPointer(CurrentShader->LocPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);
        if (sx < 0) {
            glVertexAttribPointer(CurrentShader->LocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        else {
            float v[8];
            v[0] = (sx) / texture->Width;      v[1] = (sy) / texture->Height;
            v[2] = (sx + sw) / texture->Width; v[3] = (sy) / texture->Height;
            v[4] = (sx) / texture->Width;      v[5] = (sy + sh) / texture->Height;
            v[6] = (sx + sw) / texture->Width; v[7] = (sy + sh) / texture->Height;
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribPointer(CurrentShader->LocTexCoord, 2, GL_FLOAT, GL_FALSE, 0, v);
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(CurrentShader->LocTexCoord);
    Restore();
}

PUBLIC void     OpenGLRenderer::DrawTexture(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h) {
    OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;
    if (textureData->YUV)
        UseShader(ShaderTexturedShapeYUV);
    else
        UseShader(ShaderTexturedShape);
    DrawTextureRaw(texture, sx, sy, sw, sh, x, y, w, h);
}
PUBLIC void     OpenGLRenderer::DrawTexture(Texture* texture, float x, float y, float w, float h) {
    DrawTexture(texture, -1, -1, -1, -1, x, y, w, h);
}
PUBLIC void     OpenGLRenderer::DrawTexture(Texture* texture, float x, float y) {
    DrawTexture(texture, x, y, texture->Width, texture->Height);
}

PUBLIC void     OpenGLRenderer::DrawTextureBlurred(Texture* texture, float sx, float sy, float sw, float sh, float x, float y, float w, float h) {
    UseShader(ShaderTexturedShapeBlur);
    OpenGLTextureData* textureData = (OpenGLTextureData*)texture->DriverData;
    if (textureData->FBO)
        glUniform2f(CurrentShader->LocTextureSize, w * RetinaScale, h * RetinaScale);
    else
        glUniform2f(CurrentShader->LocTextureSize, w, h);
    glUniform2f(CurrentShader->LocDirectionVector, 8.0f, 0.0f);

    DrawTextureRaw(texture, sx, sy, sw, sh, x, y, w, h);
}
PUBLIC void     OpenGLRenderer::DrawTextureBlurred(Texture* texture, float x, float y, float w, float h) {
    DrawTextureBlurred(texture, -1, -1, -1, -1, x, y, w, h);
}
PUBLIC void     OpenGLRenderer::DrawTextureBlurred(Texture* texture, float x, float y) {
    DrawTextureBlurred(texture, x, y, texture->Width, texture->Height);
}

PUBLIC void     OpenGLRenderer::Dispose() {
    delete ProjectionMatrix;
    while (ModelViewMatrix.size()) {
        delete ModelViewMatrix.top();
        ModelViewMatrix.pop();
    }

    DisposeTexture(FramebufferTextureA);
    DisposeTexture(FramebufferTextureB);

    glDeleteBuffers(1, &BufferCircleFill);
    glDeleteBuffers(1, &BufferCircleStroke);
    glDeleteBuffers(1, &BufferSquareFill);

    ShaderShape->Dispose(); delete ShaderShape;
    ShaderTexturedShape->Dispose(); delete ShaderTexturedShape;
    ShaderTexturedShapeYUV->Dispose(); delete ShaderTexturedShapeYUV;
    ShaderTexturedShapeBlur->Dispose(); delete ShaderTexturedShapeBlur;

    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Application::Window);
}

/*

*/
