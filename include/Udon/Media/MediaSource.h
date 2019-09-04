#ifndef MEDIASOURCE_H
#define MEDIASOURCE_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Udon/Media/LibAV_Includes.h>

class MediaSource {
public:
    enum {
    STREAMTYPE_UNKNOWN,   ///< Unknown stream type
    STREAMTYPE_VIDEO,     ///< Video stream
    STREAMTYPE_AUDIO,     ///< Audio stream
    STREAMTYPE_DATA,      ///< Data stream
    STREAMTYPE_SUBTITLE,  ///< Subtitle streawm
    STREAMTYPE_ATTACHMENT ///< Attachment stream (images, etc)
    }; 
    void* FormatCtx;          ///< FFmpeg: Videostream format context
    void* AvioCtx;            ///< FFmpeg: AVIO context

    static int          ScanSource(AVFormatContext* format_ctx);
    static MediaSource* CreateSourceFromUrl(const char* url);
    static MediaSource* CreateSourceFromCustom(int (*read_cb)(void*, Uint8*, int), int64_t (*seek_cb)(void*, int64_t, int), void* userdata);
    static MediaSource* CreateSourceFromRW(SDL_RWops* rw_ops);
           int          GetStreamInfo(Uint32* info, int index);
           int          GetStreamCount();
           int          GetBestStream(Uint32 type);
           void         Close();
};

#endif /* MEDIASOURCE_H */
