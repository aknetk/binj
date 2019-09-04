#if INTERFACE
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
    class Node* Parent = NULL;
    vector<class Node*> Children;
    vector<class Node*> ChildrenBuffer;
};
#endif

#include <Udon/Objects/Node.h>

#include <Udon/Application.h>

PUBLIC STATIC Node* Node::Create() {
    Node* node = new Node;
    node->WidthValue = Value { ValueType::AUTO, 0.0f };
    node->HeightValue = Value { ValueType::AUTO, 0.0f };
    node->ApplyCreate(NULL);
    return node;
}
PUBLIC void Node::ApplyCreate(const char* id) {
    this->G = Application::RendererPtr;
    if (id) {
        Application::NodeIDList->Put(id, this);
        this->IDHash = Application::NodeIDList->HashFunction(id);
    }
}

PUBLIC VIRTUAL void Node::Add(Node* child) {
    child->Parent = this;
    ChildrenBuffer.push_back(child);
}

PUBLIC VIRTUAL void Node::CalcSize() {
    int ParentWidth = G->Width;
    int ParentHeight = G->Height;
    if (Parent) {
        ParentWidth = Parent->Width;
        ParentHeight = Parent->Height;
    }

    switch (WidthValue.Type) {
        case ValueType::PERCENT:
            Width = ParentWidth * WidthValue.Value - MarginLeft - MarginRight;
            break;
        case ValueType::PIXELS:
            Width = WidthValue.Value;
            break;
    }
    switch (HeightValue.Type) {
        case ValueType::PERCENT:
            Height = ParentHeight * HeightValue.Value - MarginTop - MarginBottom;
            break;
        case ValueType::PIXELS:
            Height = HeightValue.Value;
            break;
    }
    // Autos
    switch (WidthValue.Type) {
        case ValueType::AUTO:
            Width = Height * DefaultWidth / DefaultHeight;
            break;
    }
    switch (HeightValue.Type) {
        case ValueType::AUTO:
            Height = Width * DefaultHeight / DefaultWidth;
            break;
    }
}

PUBLIC VIRTUAL void Node::Update() {
    for (Uint32 i = 0, iSz = ChildrenBuffer.size(); i < iSz; i++) {
        Children.push_back(ChildrenBuffer[i]);
    }
    ChildrenBuffer.clear();

    int x = X;
    int y = Y;
    float maxLineHeight = 0.0;
    for (Uint32 i = 0, iSz = Children.size(); i < iSz; i++) {
        switch (Children[i]->Display) {
            case Display::NONE:
                break;
            case Display::BLOCK:
                Children[i]->CalcSize();

                x = X;
                y += maxLineHeight;
                maxLineHeight = 0;
                maxLineHeight = max(Children[i]->Height + Children[i]->MarginTop + Children[i]->MarginBottom, maxLineHeight);
                Children[i]->X = x + Children[i]->MarginLeft;
                Children[i]->Y = y + Children[i]->MarginTop;
                x += Children[i]->Width + Children[i]->MarginLeft + Children[i]->MarginRight;
                break;
            case Display::INLINE_BLOCK:
                Children[i]->CalcSize();

                if (x + Children[i]->Width > Width - X) {
                    x = X;
                    y += maxLineHeight;
                    maxLineHeight = 0;
                }
                maxLineHeight = max(Children[i]->Height + Children[i]->MarginTop + Children[i]->MarginBottom, maxLineHeight);
                Children[i]->X = x + Children[i]->MarginLeft;
                Children[i]->Y = y + Children[i]->MarginTop;
                x += Children[i]->Width + Children[i]->MarginRight;
                break;
            default:
                break;
        }

        Children[i]->Update();
    }
}
PUBLIC VIRTUAL void Node::OnEvent(SDL_Event e) {
    for (Uint32 i = 0, iSz = Children.size(); i < iSz; i++) {
        Children[i]->OnEvent(e);
    }
}
PUBLIC VIRTUAL void Node::OnWindowResize(int width, int height) {
    for (Uint32 i = 0, iSz = Children.size(); i < iSz; i++) {
        Children[i]->OnWindowResize(width, height);
    }
}
PUBLIC VIRTUAL void Node::Render() {
    G->SetBlendColor(BackgroundColor);
    G->FillRectangle(X, Y, Width, Height);
    for (Uint32 i = 0, iSz = Children.size(); i < iSz; i++) {
        if (Children[i]->Display != Display::NONE)
            Children[i]->Render();
    }
}
PUBLIC VIRTUAL void Node::Dispose() {
    for (Uint32 i = 0, iSz = Children.size(); i < iSz; i++) {
        Children[i]->Dispose();
        delete Children[i];
    }
    Children.clear();
}
PUBLIC VIRTUAL Node::~Node() {

}
