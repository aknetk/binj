#if INTERFACE
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
};
#endif

#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>

#include <Udon/Application.h>
#include <Udon/Logging/Log.h>

struct TextureAtlasItem {
    int cur_shelf; //< Current shelf number in cache
    int cur_slot; //< Current slot on shelf in cache
    SDL_Rect source; //< Source coordinates on cache surface
    SDL_Rect target; //< Target coordinates on output surface
};

struct Shelf {
    uint16_t width;
    uint16_t height;
    uint16_t count;
};

PUBLIC STATIC TextureAtlas*  TextureAtlas::Create() {
    TextureAtlas* atlas = (TextureAtlas*)calloc(1, sizeof(TextureAtlas));
    if (atlas == NULL) {
        goto exit_0;
    }

    atlas->cur_items = 0;
    atlas->max_items = 1024;
    atlas->max_shelves = 256;
    atlas->w = 0;
    atlas->h = 0;

    // Allocate items. These hold the surfaces that should be in atlas
    atlas->items = calloc(atlas->max_items, sizeof(TextureAtlasItem));
    if (atlas->items == NULL) {
        goto exit_1;
    }

    // Allocate shelves. These describe the used space of the atlas
    atlas->shelves = calloc(atlas->max_shelves, sizeof(Shelf));
    if (atlas->shelves == NULL) {
        goto exit_2;
    }

    return atlas;

    exit_2:
        free(atlas->items);
    exit_1:
        free(atlas);
    exit_0:
    return NULL;
}

PUBLIC        void           TextureAtlas::ClearContent() {
    this->cur_items = 0;
    memset(this->items, 0, this->max_items * sizeof(TextureAtlasItem));
    memset(this->shelves, 0, this->max_shelves * sizeof(Shelf));
}
PUBLIC        void           TextureAtlas::Free() {
    free(this->items);
    free(this->shelves);
    free(this);
}

PUBLIC        void           TextureAtlas::SetItemAllocation(void* itemPtr, SDL_Surface* surface, int shelf, int slot, int x, int y) {
    assert(itemPtr != NULL);

    TextureAtlasItem* item = (TextureAtlasItem*)itemPtr;

    item->cur_shelf = shelf;
    item->cur_slot = slot;
    item->source.x = x;
    item->source.y = y;
    item->source.w = surface->w;
    item->source.h = surface->h;
}
PUBLIC        int            TextureAtlas::FindFreeSlot(SDL_Surface* surface, void* itemPtr) {
    assert(itemPtr != NULL);

    TextureAtlasItem* item = (TextureAtlasItem*)itemPtr;

    int shelf_w;
    int shelf_h;
    int total_remaining_h = this->h;
    int total_reserved_h = 0;

    // First, try to look for a good, existing shelf
    int best_shelf_idx = -1;
    int best_shelf_h = this->h;
    int best_shelf_y = 0;

    // Try to find a good shelf to put this item in
    int shelf_idx;
    for (shelf_idx = 0; shelf_idx < this->max_shelves; shelf_idx++) {
        shelf_w = ((Shelf*)this->shelves)[shelf_idx].width;
        shelf_h = ((Shelf*)this->shelves)[shelf_idx].height;
        if (shelf_h == 0) {
            break;
        }
        total_remaining_h -= shelf_h;
        total_reserved_h += shelf_h;

        // If the item fits, check if the space is better than previous one
        if (surface->w <= (this->w - shelf_w) && surface->h <= shelf_h && shelf_h < best_shelf_h) {
            best_shelf_h = shelf_h;
            best_shelf_idx = shelf_idx;
            best_shelf_y = total_reserved_h - shelf_h;
        }
    }

    // If existing shelf found, put the item there. Otherwise create a new shelf.
    if (best_shelf_idx != -1) {
        SetItemAllocation(
            item,
            surface,
            best_shelf_idx,
            ((Shelf*)this->shelves)[best_shelf_idx].count,
            ((Shelf*)this->shelves)[best_shelf_idx].width,
            best_shelf_y);
        ((Shelf*)this->shelves)[best_shelf_idx].width += surface->w;
        ((Shelf*)this->shelves)[best_shelf_idx].count += 1;
        return 0;
    }
    else if (total_remaining_h >= surface->h) {
        ((Shelf*)this->shelves)[shelf_idx].width = surface->w;
        ((Shelf*)this->shelves)[shelf_idx].height = surface->h;
        ((Shelf*)this->shelves)[shelf_idx].count = 1;
        SetItemAllocation(
            item,
            surface,
            shelf_idx,
            0,
            0,
            total_reserved_h);
        return 0;
    }
    return 1; // Can't fit!
}

PUBLIC        void           TextureAtlas::CheckSize(Texture* texture) {
    assert(texture != NULL);

    // Check if texture size has changed, and clear content if it has.
    // int texture_w;
    // int texture_h;
    // if (SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h) == 0) {
    //     this->w = texture_w;
    //     this->h = texture_h;
    // }
    this->w = texture->Width;
    this->h = texture->Height;
}

PUBLIC        int            TextureAtlas::GetItems(SDL_Rect* sources, SDL_Rect* targets, int limit) {
    assert(limit >= 0);

    int max_count = min(this->cur_items, limit);
    for (int i = 0; i < max_count; i++) {
        TextureAtlasItem* item = &(((TextureAtlasItem*)this->items)[i]);
        if (sources != NULL)
            memcpy(&sources[i], &item->source, sizeof(SDL_Rect));
        if (targets != NULL)
            memcpy(&targets[i], &item->target, sizeof(SDL_Rect));
    }
    return max_count;
}
PUBLIC        int            TextureAtlas::AddItem(Texture* texture, SDL_Surface* surface, const SDL_Rect* target) {
    assert(surface != NULL);
    assert(target != NULL);

    // Make sure there is still room
    if (this->cur_items >= this->max_items)
        return -1;

    // Create a new item
    TextureAtlasItem item;
    memset(&item, 0, sizeof(TextureAtlasItem));
    memcpy(&item.target, target, sizeof(SDL_Rect));
    item.cur_shelf = -1;
    item.cur_slot = -1;

    // Allocate space for the new item
    if (FindFreeSlot(surface, &item) != 0) {
        return -1;
    }

    // And update texture with the surface
    texture->RendererPtr->UpdateTexture(texture, &item.source, surface->pixels, surface->pitch);

    // Room found, add item to the atlas
    memcpy(&(((TextureAtlasItem*)this->items)[this->cur_items++]), &item, sizeof(TextureAtlasItem));
    return 0;
}
