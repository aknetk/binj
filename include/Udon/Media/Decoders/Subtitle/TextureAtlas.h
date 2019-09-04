#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Rendering/Texture.h>

class TextureAtlas {
public:
    int cur_items;   //< Current items count
    int max_items;   //< Maximum items count
    int max_shelves; //< Maximum shelf count
    int w;           //< Current atlas width
    int h;           //< Current atlas height
    void* items;     //< Cached items // TextureAtlasItem
    void* shelves;   //< Atlas shelves // Shelf

    static TextureAtlas*  Create();
           void           ClearContent();
           void           Free();
           void           SetItemAllocation(void* itemPtr, SDL_Surface* surface, int shelf, int slot, int x, int y);
           int            FindFreeSlot(SDL_Surface* surface, void* itemPtr);
           void           CheckSize(Texture* texture);
           int            GetItems(SDL_Rect* sources, SDL_Rect* targets, int limit);
           int            AddItem(Texture* texture, SDL_Surface* surface, const SDL_Rect* target);
};

#endif /* TEXTUREATLAS_H */
