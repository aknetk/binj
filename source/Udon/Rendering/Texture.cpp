#if INTERFACE
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
};
#endif

#include <Udon/Application.h>

PUBLIC STATIC Texture* Texture::FromResource(const char* filename) {
    return NULL;
}
PUBLIC STATIC Texture* Texture::FromFilename(const char* filename) {
    Texture* tex;
    SDL_Surface* temp = IMG_Load(filename);
    tex = Application::RendererPtr->CreateTextureFromSurface(temp);
    SDL_FreeSurface(temp);
    return tex;
}
