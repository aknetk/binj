#if INTERFACE
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
};
#endif

#include <Udon/Objects/Text.h>

#include <Udon/Application.h>
#include <Udon/Network/HTTP.h>

PUBLIC STATIC Node* Text::Create(const char* id) {
    Text* node = new Text;
    node->StringData = (char*)malloc(256);
    node->StringData[0] = '\0';
    node->MaxStringLength = 256;
    node->TextColor = Color::FromARGB(0xFF, 0xFFFFFF);
    node->FontSize = 18;
    node->ApplyCreate(id);
    return node;
}

PUBLIC void Text::SetText(const char* text) {
    if (!Font) return;
    if (strcmp(StringData, text) == 0) return;

    strncpy(StringData, text, MaxStringLength);

    if (TexturePtr) {
        G->DisposeTexture(TexturePtr);
    }

    TexturePtr = Application::Fonts->Get(Font)->CreateText(StringData, FontSize);
    Width = TexturePtr->Width;
    Height = TexturePtr->Height;
}

PUBLIC void Text::Update() {

}
PUBLIC void Text::Render() {
    if (TexturePtr) {
        Color textColor = TextColor;
        textColor.A = int(textColor.A * Opacity);

        G->SetBlendColor(textColor);

        G->DrawTexture(TexturePtr, X, Y, Width, Height);
    }
}
PUBLIC void Text::Dispose() {
    if (TexturePtr) {
        G->DisposeTexture(TexturePtr);
        TexturePtr = NULL;
    }
}
