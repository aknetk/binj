#ifndef TEXTURE_H
#define TEXTURE_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Renderer;

#include <Udon/Standard.h>
#include <Udon/Rendering/Renderer.h>

class Texture {
public:
    Uint32 Format;
    Uint32 Access;
    Uint32 Width;
    Uint32 Height;
    Renderer* RendererPtr;
    void* Pixels;
    int   Pitch;
    void* DriverData;

    static Texture* FromResource(const char* filename);
    static Texture* FromFilename(const char* filename);
};

#endif /* TEXTURE_H */
