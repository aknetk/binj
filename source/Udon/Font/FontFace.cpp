#if INTERFACE
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
};
#endif

#include <Udon/Font/FontFace.h>

#include <Udon/Application.h>
#include <Udon/Logging/Log.h>

PUBLIC STATIC FontFace* FontFace::Open(const char* filename) {
    // TTF_GetFontKerningSizeGlyphs
    FontFace* font = (FontFace*)calloc(1, sizeof(FontFace));
    font->Count = 0;
    font->Capacity = 8;
    font->Fonts = (TTF_Font**)malloc(font->Capacity * sizeof(TTF_Font*));
    font->FontPoints  = (int*)malloc(font->Capacity * sizeof(int));
    font->FontHeights = (int*)malloc(font->Capacity * sizeof(int));
    font->Filename = filename;

    if (!font->Fonts) {
        Log::Print(Log::LOG_ERROR, "OUT_OF_MEMORY: Could not allocate fontface memory for '%s'!", filename);
        exit(1);
    }
    return font;
}

PUBLIC int              FontFace::LoadSize(int pointSize) {
    // Resource* res = Resources::Load(Filename);
    // if (!res) {
    //     Log::Print(Log::LOG_ERROR, "FILE_OPEN_FAILED: For '%s', %s!", Filename, SDL_GetError());
    //     assert(false);
    //     return 0;
    // }

    // TTF_Font* temp = TTF_OpenFontRW(res->RW, 1, pointSize * Application::RendererPtr->RetinaScale);
    TTF_Font* temp = TTF_OpenFont(Filename, pointSize * Application::RendererPtr->RetinaScale);
    if (!temp) {
        Log::Print(Log::LOG_ERROR, "FONT_OPEN_FAILED: For '%s', %s!", Filename, TTF_GetError());
        assert(false);
        return 0;
    }

    Log::Print(Log::LOG_VERBOSE, "Loaded font: %s %s", TTF_FontFaceFamilyName(temp), TTF_FontFaceStyleName(temp));

    int index = Count;
    if (Count == Capacity) {
        Capacity <<= 1;
        Fonts = (TTF_Font**)realloc(Fonts, Capacity * sizeof(TTF_Font*));
        FontPoints  = (int*)realloc(FontPoints, Capacity * sizeof(int));
        FontHeights = (int*)realloc(FontHeights, Capacity * sizeof(int));
    }
    Count++;

    Fonts[index] = temp;
    FontPoints[index] = pointSize;

    int minY, maxY;
    if (!TTF_GlyphMetrics(Fonts[index], 'F', NULL, NULL, &minY, &maxY, NULL)) {
		FontHeights[index] = (maxY - minY) / Application::RendererPtr->RetinaScale;
    }
    return 1;
}

PUBLIC int              FontFace::FindIndex(int pointSize) {
    int index = -1;
    do {
        for (int i = 0; i < Count && index == -1; i++) {
            if (FontPoints[i] == pointSize)
                index = i;
        }
        if (index == -1) {
            if (!FontFace::LoadSize(pointSize))
                return -1;
        }
    }
    while (index < 0);
    return index;
}
PUBLIC Texture*         FontFace::CreateText(const char* text, int pointSize) {
    int index = FindIndex(pointSize);
    if (index < 0)
        return NULL;

    SDL_Surface* temp;
    temp = TTF_RenderUTF8_Blended(Fonts[index], text, SDL_Color { 0xFF, 0xFF, 0xFF });
    if (!temp) {
        Log::Print(Log::LOG_ERROR, "Could not renderUTF8 text '%s'.", text);
        return NULL;
    }

    Texture* texture = NULL;
    if (!(texture = Application::RendererPtr->CreateTextureFromSurface(temp))) {
        Log::Print(Log::LOG_ERROR, "Could not create texture from surface. '%s'.", text);
        SDL_FreeSurface(temp);
        return NULL;
    }

    texture->Width = temp->w / Application::RendererPtr->RetinaScale;
    texture->Height = temp->h / Application::RendererPtr->RetinaScale;
    SDL_FreeSurface(temp);
    return texture;
}
PUBLIC Texture*         FontFace::CreateTextWrapped(const char* text, int pointSize, int maxWidth, int maxLines) {
    int index = FindIndex(pointSize);
    if (index < 0)
        return NULL;

    char   char_copy;
    char*  text_copy;
    size_t text_length = strlen(text);
    int    line_current = 0;
    char*  line_start_point;
    int    w;

    text_copy = (char*)malloc(text_length + 1);
    memcpy(text_copy, text, text_length + 1);

    line_start_point = text_copy;

    for (int i = 1; i < text_length; i++) {
        char_copy = text_copy[i];
        text_copy[i] = 0;

        TTF_SizeText(Fonts[index], line_start_point, &w, NULL);
        // if we're still smaller than maxWidth, we don't need to do anything
        if (w < maxWidth) {

        }
        else if (char_copy == ' ') {
            
        }
    }
    // NOTE: Don't use _Wrapped functions! Measure text ourself, maybe like a vector of surfaces that
    //   are then blitted onto a master surface.
    return NULL;
}
PUBLIC Texture*         FontFace::CreateTextEllipsis(const char* text, int pointSize, int maxWidth, int maxLines) {
    return NULL;
}

PUBLIC int              FontFace::GetHeight(int pointSize) {
    int index = FindIndex(pointSize);
    if (index < 0)
        return -1;

    return FontHeights[index];
}

PUBLIC void             FontFace::Dispose() {
    for (int i = 0; i < Count; i++) {
        if (Fonts[i])
            TTF_CloseFont(Fonts[i]);
    }
    free(Fonts);
    free(FontPoints);
    free(FontHeights);
    free(this);
}
