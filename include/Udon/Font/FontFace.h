#ifndef FONTFACE_H
#define FONTFACE_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Rendering/Renderer.h>
#include <Udon/Rendering/Texture.h>

class FontFace {
public:
    TTF_Font** Fonts = NULL;
    int*       FontPoints = NULL;
    int*       FontHeights = NULL;
    int        Capacity = 0;
    int        Count = 0;
    const char* Filename = NULL;

    static FontFace* Open(const char* filename);
    int              LoadSize(int pointSize);
    int              FindIndex(int pointSize);
    Texture*         CreateText(const char* text, int pointSize);
    Texture*         CreateTextWrapped(const char* text, int pointSize, int maxWidth, int maxLines);
    Texture*         CreateTextEllipsis(const char* text, int pointSize, int maxWidth, int maxLines);
    int              GetHeight(int pointSize);
    void             Dispose();
};

#endif /* FONTFACE_H */
