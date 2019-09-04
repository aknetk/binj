#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Objects/Node.h>
#include <Udon/Rendering/Texture.h>

class Image : public Node {
public:
    Texture* TexturePtr = NULL;
    SDL_Surface* SurfacePtr = NULL;
    const char* SourceFile = NULL;
    const char* SourceURL = NULL;
    bool Busy = false;
};
#endif

#include <Udon/Objects/Image.h>

#include <Udon/Application.h>
#include <Udon/Network/HTTP.h>

PUBLIC STATIC Node* Image::Create(const char* id) {
    Image* node = new Image;
    node->ApplyCreate(id);
    return node;
}

PUBLIC STATIC void Image::GetCallbackResolve(void* ptr, char* data, size_t len) {
    Image* self = (Image*)ptr;
    SDL_RWops* rw = SDL_RWFromMem(data, len);
    self->SurfacePtr = IMG_Load_RW(rw, false);
    self->DefaultWidth = self->SurfacePtr->w;
    self->DefaultHeight = self->SurfacePtr->h;
    SDL_RWclose(rw);

    // FILE* f = fopen(cacheFilename, "wb");
    // if (f) {
    //     fwrite(data, len, 1, f);
    //     fclose(f);
    // }
}

PUBLIC void Image::Update() {
    if (!TexturePtr) {
        if (!SurfacePtr) {
            if (SourceFile) {
                SurfacePtr = IMG_Load(SourceFile);
                if (SurfacePtr) {
                    DefaultWidth = SurfacePtr->w;
                    DefaultHeight = SurfacePtr->h;
                }
                else {
                    Log::Print(Log::LOG_ERROR, "Could not load file from \"%s\".", SourceFile);
                }
            }
            else if (SourceURL && !Busy) {
                // Uint32 hash = CRC32::EncryptString(SourceURL);
                // char cacheFilename[64];
                // sprintf(cacheFilename, "res/cache/%X", hash);
                // if (File::Exists(cacheFilename)) {
                //     SurfacePtr = IMG_Load(SourceFile);
                //     DefaultWidth = SurfacePtr->w;
                //     DefaultHeight = SurfacePtr->h;
                // }
                // else {
                    HTTP::GetAsync(SourceURL, Image::GetCallbackResolve, NULL, this);
                    Busy = true;
                // }
            }
        }
    }
}
PUBLIC void Image::Render() {
    if (!TexturePtr && SurfacePtr) {
        TexturePtr = G->CreateTextureFromSurface(SurfacePtr);
        SDL_FreeSurface(SurfacePtr);
        SurfacePtr = NULL;
    }
    if (TexturePtr) {
        G->SetBlendColor(Color::FromARGB(0xFF, 0xFFFFFF));
        G->DrawTexture(TexturePtr, X, Y, Width, Height);
    }
    else {
        G->SetBlendColor(Color::FromHSL(270.0, 1.0, 1.0));
        G->FillRectangle(X, Y, Width, Height);
    }
}
PUBLIC void Image::Dispose() {
    if (TexturePtr) {
        G->DisposeTexture(TexturePtr);
        TexturePtr = NULL;
    }
    if (SurfacePtr) {
        SDL_FreeSurface(SurfacePtr);
        SurfacePtr = NULL;
    }
}
