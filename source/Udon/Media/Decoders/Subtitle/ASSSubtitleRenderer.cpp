#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Rendering/Texture.h>
#include <Udon/Media/Decoders/Subtitle/Libass.h>
#include <Udon/Media/Decoders/Subtitle/TextureAtlas.h>
#include <Udon/Media/Decoders/Subtitle/SubtitleRenderer.h>

class ASSSubtitleRenderer : public SubtitleRenderer {
public:
    ASS_Renderer* renderer;
    ASS_Track* track;
};
#endif

#include <Udon/Media/Decoders/Subtitle/ASSSubtitleRenderer.h>
#include <Udon/Media/Decoders/Subtitle/SubtitlePacket.h>

#include <Udon/Media/Decoder.h>
#include <Udon/Media/Decoders/SubtitleDecoder.h>
#include <Udon/Media/Utils/MediaPlayerState.h>

#include <Udon/Logging/Log.h>

PUBLIC STATIC void ASSSubtitleRenderer::ProcessAssImage(SDL_Surface* surface, const ASS_Image* img) {
    unsigned char  r = ((img->color) >> 24) & 0xFF;
    unsigned char  g = ((img->color) >> 16) & 0xFF;
    unsigned char  b = ((img->color) >>  8) & 0xFF;
    unsigned char  a = 0xFF - ((img->color) & 0xFF);
    unsigned char* src = (unsigned char*)img->bitmap;
    unsigned char* dst = (unsigned char*)surface->pixels;
    unsigned int   x;
    unsigned int   y;
    unsigned int   rx;

    for (y = 0; y < img->h; y++) {
        for (x = 0; x < img->w; x++) {
            rx = x * 4;
            dst[rx + 0] = r;
            dst[rx + 1] = g;
            dst[rx + 2] = b;
            dst[rx + 3] = (a * src[x]) >> 8;
        }
        src += img->stride;
        dst += surface->pitch;
    }
}

PUBLIC STATIC void ASSSubtitleRenderer::RenderFunc(SubtitleRenderer* ren, void* sub_src, double pts, double start, double end) {
    assert(ren != NULL);
    assert(sub_src != NULL);

    ASSSubtitleRenderer* self = (ASSSubtitleRenderer*)ren;
    AVSubtitle* sub = (AVSubtitle*)sub_src;

    class Decoder* dec = (class Decoder*)self->Decoder;

    // Read incoming subtitle packets to libASS
    long long start_ms = (start + pts) * 1000;
    long long end_ms = end * 1000;
    if (dec->LockOutput() == 0) {
        for (int r = 0; r < sub->num_rects; r++) {
            if (sub->rects[r]->ass == NULL)
                continue;
        #if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57,25,100)
            ass_process_data(
                self->track,
                sub->rects[r]->ass,
                strlen(sub->rects[r]->ass));
        #else
            // This requires the sub_text_format codec_opt set for ffmpeg
            ass_process_chunk(
                self->track,
                sub->rects[r]->ass,
                strlen(sub->rects[r]->ass),
                start_ms,
                end_ms);
        #endif
        }
        dec->UnlockOutput();
    }
}
PUBLIC STATIC int  ASSSubtitleRenderer::GetDataFunc(SubtitleRenderer* ren, TextureAtlas* atlas, Texture* texture, double current_pts) {
    ASSSubtitleRenderer* self = (ASSSubtitleRenderer*)ren;
    SDL_Surface* dst = NULL;
    ASS_Image* src = NULL;
    int change = 0;
    long long now = current_pts * 1000;

    class Decoder* dec = (class Decoder*)self->Decoder;

    if (dec->LockOutput() == 0) {
        // Tell ASS to render some images
        src = ass_render_frame(self->renderer, self->track, now, &change);

        // If there was no change, stop here
        if (change == 0) {
            dec->UnlockOutput();
            return 0;
        }

        // There was some change, process images and add them to atlas
        atlas->ClearContent();
        atlas->CheckSize(texture);
        for (; src; src = src->next) {
            if (src->w == 0 || src->h == 0)
                continue;
            dst = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, SDL_PIXELFORMAT_RGBA32);
            ASSSubtitleRenderer::ProcessAssImage(dst, src);
            SDL_Rect target;
            target.x = src->dst_x;
            target.y = src->dst_y;
            target.w = dst->w;
            target.h = dst->h;
            atlas->AddItem(texture, dst, &target);
            SDL_FreeSurface(dst);
        }

        dec->UnlockOutput();
    }

    dec->ClockPos = current_pts;
    return 0;
}
PUBLIC STATIC void ASSSubtitleRenderer::SetSizeFunc(SubtitleRenderer* ren, int w, int h) {
    ASSSubtitleRenderer* self = (ASSSubtitleRenderer*)ren;
    ass_set_frame_size(self->renderer, w, h);
}
PUBLIC STATIC void ASSSubtitleRenderer::CloseFunc(SubtitleRenderer* ren) {
    if (ren == NULL) return;

    ASSSubtitleRenderer* self = (ASSSubtitleRenderer*)ren;
    ass_free_track(self->track);
    ass_renderer_done(self->renderer);
}

PUBLIC             ASSSubtitleRenderer::ASSSubtitleRenderer(class Decoder* dec, int video_w, int video_h, int screen_w, int screen_h) {
    assert(dec != NULL);
    assert(video_w >= 0);
    assert(video_h >= 0);
    assert(screen_w >= 0);
    assert(screen_h >= 0);

    ASS_Track* ass_track;

    // Make sure that libass library has been initialized + get handle
    if (MediaPlayerState::libassHandle == NULL) {
        Log::Print(Log::LOG_ERROR, "Libass library has not been initialized");
        return;
    }

    SubtitleRenderer::Create(dec);

    // Initialize libass renderer
    ASS_Renderer* ass_renderer = ass_renderer_init(MediaPlayerState::libassHandle);
    if (ass_renderer == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to initialize libass renderer");
        goto exit_2;
    }

    // Read fonts from attachment streams and give them to libass
    for (int j = 0; j < dec->FormatCtx->nb_streams; j++) {
        AVStream* st = dec->FormatCtx->streams[j];
        #if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
            AVCodecContext* codec = st->codec;
        #else
            AVCodecParameters* codec = st->codecpar;
        #endif
        if (codec->codec_type == AVMEDIA_TYPE_ATTACHMENT && MediaPlayerState::AttachmentIsFont(st)) {
            const AVDictionaryEntry* tag = av_dict_get(
                st->metadata,
                "filename",
                NULL,
                AV_DICT_MATCH_CASE);
            if (tag) {
                ass_add_font(
                    MediaPlayerState::libassHandle,
                    tag->value,                         // filename of the font
                    (char*)codec->extradata,
                    codec->extradata_size);
            }
        }
    }

    // Init libass fonts and window frame size
    ass_set_fonts(
        ass_renderer,
        NULL, "sans-serif",
        ASS_FONTPROVIDER_AUTODETECT,
        NULL, 1);
    ass_set_storage_size(ass_renderer, video_w, video_h);
    ass_set_frame_size(ass_renderer, screen_w, screen_h);
    ass_set_hinting(ass_renderer, (ASS_Hinting)MediaPlayerState::FontHinting);

    // Initialize libass track
    ass_track = ass_new_track(MediaPlayerState::libassHandle);
    if (ass_track == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to initialize libass track");
        goto exit_3;
    }

    // Set up libass track headers (ffmpeg provides these)
    if (dec->CodecCtx->subtitle_header) {
        ass_process_codec_private(
            ass_track,
            (char*)dec->CodecCtx->subtitle_header,
            dec->CodecCtx->subtitle_header_size);
    }

    // Set callbacks and userdata, and we're go
    this->renderer = ass_renderer;
    this->track = ass_track;
    this->RenderFuncPtr  = ASSSubtitleRenderer::RenderFunc;
    this->GetDataFuncPtr = ASSSubtitleRenderer::GetDataFunc;
    this->SetSizeFuncPtr = ASSSubtitleRenderer::SetSizeFunc;
    this->CloseFuncPtr   = ASSSubtitleRenderer::CloseFunc;
    return;

    exit_3:
    ass_renderer_done(ass_renderer);
    exit_2:
    // exit_1:
    // Kit_CloseSubtitleRenderer(ren);
    // exit_0:
    return;
}
