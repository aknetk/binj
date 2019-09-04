#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Rendering/Texture.h>
#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>
#include <Udon/Media/Decoders/Subtitle/SubtitleRenderer.h>

class ImageSubtitleRenderer : public SubtitleRenderer {
public:
    int   video_w;
    int   video_h;
    float scale_x;
    float scale_y;
};
#endif

#include <Udon/Media/Decoders/Subtitle/ImageSubtitleRenderer.h>
#include <Udon/Media/Decoders/Subtitle/SubtitlePacket.h>

#include <Udon/Media/Decoder.h>
#include <Udon/Media/Decoders/SubtitleDecoder.h>

#include <Udon/Logging/Log.h>

PUBLIC STATIC void ImageSubtitleRenderer::RenderFunc(SubtitleRenderer* ren, void* sub_src, double pts, double start, double end) {
    assert(sub_src != NULL);

    ImageSubtitleRenderer* self = (ImageSubtitleRenderer*)ren;
    AVSubtitle*  sub = (AVSubtitle*)sub_src;
    SDL_Surface* dst = NULL;
    SDL_Surface* src = NULL;
    double start_pts = pts + start;
    double end_pts = pts + end;

    // If this subtitle has no rects, we still need to clear screen from old subs
    if (sub->num_rects == 0) {
        ((class Decoder*)self->Decoder)->WriteOutput(SubtitleDecoder::CreateSubtitlePacket(true, start_pts, end_pts, 0, 0, NULL));
        return;
    }

    // Convert subtitle images from paletted to RGBA8888
    for (int n = 0; n < sub->num_rects; n++) {
        AVSubtitleRect* r = sub->rects[n];
        if (r->type != SUBTITLE_BITMAP)
            continue;

        src = SDL_CreateRGBSurfaceWithFormatFrom(r->data[0], r->w, r->h, 8, r->linesize[0], SDL_PIXELFORMAT_INDEX8);
        SDL_SetPaletteColors(src->format->palette, (SDL_Color*)r->data[1], 0, 256);
        dst = SDL_CreateRGBSurfaceWithFormat(0, r->w, r->h, 32, SDL_PIXELFORMAT_RGBA32);

        // Blit source to target and free source surface.
        SDL_BlitSurface(src, NULL, dst, NULL);

        // Create a new packet and write it to output buffer
        ((class Decoder*)self->Decoder)->WriteOutput(SubtitleDecoder::CreateSubtitlePacket(false, start_pts, end_pts, r->x, r->y, dst));

        // Free surfaces
        SDL_FreeSurface(src);
        SDL_FreeSurface(dst);
    }
}
PUBLIC STATIC int  ImageSubtitleRenderer::GetDataFunc(SubtitleRenderer* ren, TextureAtlas* atlas, Texture* texture, double current_pts) {
    ImageSubtitleRenderer* self = (ImageSubtitleRenderer*)ren;
    SubtitlePacket* packet = NULL;

    class Decoder* dec = (class Decoder*)self->Decoder;

    atlas->CheckSize(texture);
    while ((packet = (SubtitlePacket*)dec->PeekOutput()) != NULL) {
        // Clear dead packets
        if (packet->pts_end < current_pts) {
            dec->AdvanceOutput();
            SubtitleDecoder::FreeSubtitlePacket(packet);
            continue;
        }

        // Show visible ones
        if (packet->pts_start < current_pts) {
            if (packet->clear) {
                atlas->ClearContent();
            }
            if (packet->surface != NULL) {
                SDL_Rect target;
                target.x = packet->x * self->scale_x;
                target.y = packet->y * self->scale_y;
                target.w = packet->surface->w * self->scale_x;
                target.h = packet->surface->h * self->scale_y;
                atlas->AddItem(texture, packet->surface, &target);
            }
            dec->AdvanceOutput();
            SubtitleDecoder::FreeSubtitlePacket(packet);
            dec->ClockPos = current_pts;
            continue;
        }
        break;
    }

    return 0;
}
PUBLIC STATIC void ImageSubtitleRenderer::SetSizeFunc(SubtitleRenderer* ren, int w, int h) {
    ImageSubtitleRenderer* self = (ImageSubtitleRenderer*)ren;
    self->scale_x = (float)w / (float)self->video_w;
    self->scale_y = (float)h / (float)self->video_h;
}
PUBLIC STATIC void ImageSubtitleRenderer::CloseFunc(SubtitleRenderer *ren) {

}

PUBLIC             ImageSubtitleRenderer::ImageSubtitleRenderer(class Decoder* dec, int video_w, int video_h, int screen_w, int screen_h) {
    assert(dec != NULL);
    assert(video_w >= 0);
    assert(video_h >= 0);
    assert(screen_w >= 0);
    assert(screen_h >= 0);

    SubtitleRenderer::Create(dec);

    // Only renderer required, no other data.
    this->video_w = video_w;
    this->video_h = video_h;
    this->scale_x = (float)screen_w / (float)video_w;
    this->scale_y = (float)screen_h / (float)video_h;
    this->RenderFuncPtr  = ImageSubtitleRenderer::RenderFunc;
    this->GetDataFuncPtr = ImageSubtitleRenderer::GetDataFunc;
    this->SetSizeFuncPtr = ImageSubtitleRenderer::SetSizeFunc;
    this->CloseFuncPtr   = ImageSubtitleRenderer::CloseFunc;
    return;
}
