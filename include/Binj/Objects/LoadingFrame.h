#ifndef LOADINGFRAME_H
#define LOADINGFRAME_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Texture;
class Texture;
class Texture;
class Text;
class Text;

#include <Udon/Standard.h>
#include <Udon/Objects/Node.h>
#include <Udon/Objects/Text.h>
#include <Udon/Rendering/Texture.h>

class LoadingFrame : public Node {
public:
    // Raw datas
    Texture* TextureLogo = NULL;
    Texture* TextureScreen = NULL;
    // Subnodes
    Text*    StatusText = NULL;
    // State variables
    int      InfoFetchState = -1;
    // Animation timers
    double   FadeInTimer = 0.0;
    double   FadeOutTimer = 0.0;
    double   CircleAnimationTimer = 0.0;

    static Node* Create(const char* id);
    static void GetCallbackResolve(void* ptr, char* data, size_t len);
    static void GetCallbackReject(void* ptr, char* data, size_t len);
    void Update();
    void Render();
    void Dispose();
    ~LoadingFrame();
};

#endif /* LOADINGFRAME_H */
