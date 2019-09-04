#ifndef NODE_H
#define NODE_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Color;
class Renderer;
class Node;

#include <Udon/Standard.h>
#include <Udon/Graphics/Color.h>
#include <Udon/Objects/Enums.h>

class Node {
public:
    Uint32    IDHash = 0x0U;
    int       Display = Display::BLOCK;
    int       X = 0;
    int       Y = 0;
    float     Width = 1.0f;
    float     Height = 1.0f;
    float     DefaultWidth = 1.0f;
    float     DefaultHeight = 1.0f;
    Value     WidthValue;
    Value     HeightValue;
    Color     BackgroundColor;
    int       MarginTop = 0;
    int       MarginLeft = 0;
    int       MarginRight = 0;
    int       MarginBottom = 0;
    int       PaddingTop = 0;
    int       PaddingLeft = 0;
    int       PaddingRight = 0;
    int       PaddingBottom = 0;
    float     Opacity = 1.0;
    Renderer* G = NULL;
    Node* Parent = NULL;
    vector<class Node*> Children;
    vector<class Node*> ChildrenBuffer;

    static Node* Create();
    void ApplyCreate(const char* id);
    virtual void Add(Node* child);
    virtual void CalcSize();
    virtual void Update();
    virtual void OnEvent(SDL_Event e);
    virtual void OnWindowResize(int width, int height);
    virtual void Render();
    virtual void Dispose();
    virtual ~Node();
};

#endif /* NODE_H */
