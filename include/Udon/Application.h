#ifndef APPLICATION_H
#define APPLICATION_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Font/FontFace.h>
#include <Udon/Objects/Node.h>
#include <Udon/Rendering/Renderer.h>
#include <Udon/Types/HashMap.h>

class Application {
private:
    static int EventFilter(void* data, SDL_Event* event);

public:
    static SDL_Window*         Window;
    static Renderer*           RendererPtr;
    static bool                Running;
    static int                 FPS;
    static Node*               Document;
    static HashMap<FontFace*>* Fonts;
    static HashMap<Node*>*     NodeIDList;
    static Platforms           Platform;

    static void Run();
    static void Quit();
};

#endif /* APPLICATION_H */
