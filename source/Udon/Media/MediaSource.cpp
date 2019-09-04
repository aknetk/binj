#if INTERFACE
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
};
#endif

#include <Udon/Media/MediaSource.h>

#include <Udon/Logging/Log.h>

typedef int     (*ReadCallback)(void* userdata, Uint8* buf, int size);
typedef int64_t (*SeekCallback)(void* userdata, int64_t offset, int whence);

#define AVIO_BUF_SIZE 32768

PUBLIC STATIC int          MediaSource::ScanSource(AVFormatContext* format_ctx) {
    av_opt_set_int(format_ctx, "probesize", INT_MAX, 0);
    av_opt_set_int(format_ctx, "analyzeduration", INT_MAX, 0);
    if (avformat_find_stream_info(format_ctx, NULL) < 0) {
        Log::Print(Log::LOG_ERROR, "Unable to fetch source information");
        return 1;
    }
    return 0;
}

PUBLIC STATIC MediaSource* MediaSource::CreateSourceFromUrl(const char* url) {
    assert(url != NULL);

    MediaSource* src = (MediaSource*)calloc(1, sizeof(MediaSource));
    if (src == NULL) {
        Log::Print(Log::LOG_ERROR, "Unable to allocate source");
        return NULL;
    }

    // Attempt to open source
    if (avformat_open_input((AVFormatContext**)&src->FormatCtx, url, NULL, NULL) < 0) {
        Log::Print(Log::LOG_ERROR, "Unable to open source URL");
        goto exit_0;
    }

    // Scan source information (may seek forwards)
    if (ScanSource((AVFormatContext*)src->FormatCtx)) {
        goto exit_1;
    }

    return src;

    exit_1:
    avformat_close_input((AVFormatContext**)&src->FormatCtx);
    exit_0:
    free(src);
    return NULL;
}
PUBLIC STATIC MediaSource* MediaSource::CreateSourceFromCustom(int (*read_cb)(void*, Uint8*, int), int64_t (*seek_cb)(void*, int64_t, int), void* userdata) {
    return NULL;
}
PUBLIC STATIC MediaSource* MediaSource::CreateSourceFromRW(SDL_RWops* rw_ops) {
    return NULL;
}

PUBLIC        int          MediaSource::GetStreamInfo(Uint32* info, int index) {
    assert(info != NULL);

    AVFormatContext* format_ctx = (AVFormatContext*)this->FormatCtx;
    if (index < 0 || index >= format_ctx->nb_streams) {
        Log::Print(Log::LOG_ERROR, "Invalid stream index");
        return 1;
    }

    AVStream *stream = format_ctx->streams[index];
    enum AVMediaType codec_type;
    #if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
        codec_type = stream->codec->codec_type;
    #else
        codec_type = stream->codecpar->codec_type;
    #endif

    switch (codec_type) {
        case AVMEDIA_TYPE_UNKNOWN: *info = STREAMTYPE_UNKNOWN; break;
        case AVMEDIA_TYPE_DATA: *info = STREAMTYPE_DATA; break;
        case AVMEDIA_TYPE_VIDEO: *info = STREAMTYPE_VIDEO; break;
        case AVMEDIA_TYPE_AUDIO: *info = STREAMTYPE_AUDIO; break;
        case AVMEDIA_TYPE_SUBTITLE: *info = STREAMTYPE_SUBTITLE; break;
        case AVMEDIA_TYPE_ATTACHMENT: *info = STREAMTYPE_ATTACHMENT; break;
        default:
            Log::Print(Log::LOG_ERROR, "Unknown native stream type");
            return 1;
    }

    return 0;
}
PUBLIC        int          MediaSource::GetStreamCount() {
    return ((AVFormatContext*)FormatCtx)->nb_streams;
}
PUBLIC        int          MediaSource::GetBestStream(Uint32 type) {
    int avmedia_type = 0;
    switch (type) {
        case STREAMTYPE_VIDEO: avmedia_type = AVMEDIA_TYPE_VIDEO; break;
        case STREAMTYPE_AUDIO: avmedia_type = AVMEDIA_TYPE_AUDIO; break;
        case STREAMTYPE_SUBTITLE: avmedia_type = AVMEDIA_TYPE_SUBTITLE; break;
        default: return -1;
    }
    int ret = av_find_best_stream((AVFormatContext*)this->FormatCtx, (enum AVMediaType)avmedia_type, -1, -1, NULL, 0);
    if (ret == AVERROR_STREAM_NOT_FOUND) {
        return -1;
    }
    if (ret == AVERROR_DECODER_NOT_FOUND) {
        Log::Print(Log::LOG_ERROR, "Unable to find a decoder for the stream");
        return 1;
    }
    return ret;
}

PUBLIC        void         MediaSource::Close() {
    AVFormatContext* format_ctx = (AVFormatContext*)this->FormatCtx;
    AVIOContext* avio_ctx = (AVIOContext*)this->AvioCtx;
    avformat_close_input(&format_ctx);
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }
    free(this);
}
