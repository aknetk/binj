#ifndef MAINFRAME_H
#define MAINFRAME_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Texture;
class Texture;
class Text;
class Text;

#include <Udon/Standard.h>
#include <Udon/Objects/Node.h>
#include <Udon/Objects/Text.h>
#include <Udon/Rendering/Texture.h>

class MainFrame : public Node {
public:
    // Raw datas
    Texture* TextureLogo = NULL;
    // Subnodes
    Text*    TextHeaders[5];
    // State variables
    double   ViewY = 0.0f;
    int      CurrentLibrarySelectionIndex = 0;
    int      PreviousLibrarySelectionIndex = 0;
    // Animation timers
    double   LibrarySelectionFadeTimer = 1.0;

    static Node* Create(const char* id);
    void OnEvent(SDL_Event e);
    void MoveCursor(int x, int y);
    void CheckRange();
    void StartVideo();
    void Update();
    void Render();
    ~MainFrame();
};

#endif /* MAINFRAME_H */
