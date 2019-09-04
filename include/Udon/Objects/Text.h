#ifndef TEXT_H
#define TEXT_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Texture;
class Color;

#include <Udon/Standard.h>
#include <Udon/Objects/Node.h>
#include <Udon/Rendering/Texture.h>

class Text : public Node {
public:
    Texture*    TexturePtr = NULL;
    char*       StringData = NULL;
    int         MaxStringLength = 0;
    Color       TextColor;
    const char* Font = NULL;
    int         FontSize = 16;

    static Node* Create(const char* id);
    void SetText(const char* text);
    void Update();
    void Render();
    void Dispose();
};

#endif /* TEXT_H */
