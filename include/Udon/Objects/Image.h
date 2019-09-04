#ifndef IMAGE_H
#define IMAGE_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Texture;

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

    static Node* Create(const char* id);
    static void GetCallbackResolve(void* ptr, char* data, size_t len);
    void Update();
    void Render();
    void Dispose();
};

#endif /* IMAGE_H */
