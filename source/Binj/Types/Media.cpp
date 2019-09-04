#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Objects/Image.h>
#include <Udon/Rendering/Texture.h>

#include <Binj/Types/Enums.h>
#include <Binj/Types/Structs.h>
#include <Binj/Types/MediaEpisode.h>
#include <Binj/Types/MediaEpisodeGroup.h>

class Media {
public:
    enum {
        LOADSTATUS_STARTED_REQUEST = 1,
        LOADSTATUS_FINISHED_REQUEST = 2,
        LOADSTATUS_REJECTED_REQUEST = 3,
    };

    Uint32                     ID;
    char                       Title[128];
    char                       Description[256];
    char*                      Filename = NULL;
    int                        Year;
    bool                       IsSeries;
    Image*                     Thumbnail = NULL;
    vector<MediaEpisodeGroup*> EpisodeGroups;

    int                        RequestStatus;
    char*                      RequestStatusURL = NULL;
};
#endif

#include <Binj/Types/Media.h>

#include <Udon/Filesystem/Directory.h>
#include <Udon/Filesystem/File.h>
#include <Udon/Logging/Log.h>
#include <Udon/Network/HTTP.h>
extern "C" {
    #include <Udon/Network/cJSON.h>
}

#ifdef WIN32
#define snprintf _snprintf
#endif

void     ReadU8(SDL_RWops* rw, uint8_t* value) { SDL_RWread(rw, value, 1, 1); }
void     ReadU16(SDL_RWops* rw, uint16_t* value) { SDL_RWread(rw, value, 2, 1); }
void     ReadU32(SDL_RWops* rw, uint32_t* value) { SDL_RWread(rw, value, 4, 1); }
void     ReadU64(SDL_RWops* rw, uint64_t* value) { SDL_RWread(rw, value, 8, 1); }
uint32_t ReadMagic(SDL_RWops* rw) { uint32_t value; SDL_RWread(rw, &value, 4, 1); return value; }
char*    ReadString(SDL_RWops* rw, char** value) {
    uint16_t len;
    SDL_RWread(rw, &len, 2, 1);
    *value = (char*)malloc(len + 1);
    if (len != 0)
        SDL_RWread(rw, *value, len, 1);
    (*value)[len] = 0;
    return *value;
}

void     WriteU8(SDL_RWops* rw, uint8_t value) { SDL_RWwrite(rw, &value, 2, 1); }
void     WriteU16(SDL_RWops* rw, uint16_t value) { SDL_RWwrite(rw, &value, 2, 1); }
void     WriteU32(SDL_RWops* rw, uint32_t value) { SDL_RWwrite(rw, &value, 4, 1); }
void     WriteU64(SDL_RWops* rw, uint64_t value) { SDL_RWwrite(rw, &value, 8, 1); }
void     WriteMagic(SDL_RWops* rw, const char* value) { SDL_RWwrite(rw, value, 4, 1); }
void     WriteString(SDL_RWops* rw, char* value) { uint16_t len = (uint16_t)strlen(value); SDL_RWwrite(rw, &len, 2, 1); if (strlen(value) != 0) SDL_RWwrite(rw, value, strlen(value), 1); }


int32_t  JsonInt(const cJSON* node, const char* name) {
    return cJSON_GetObjectItemCaseSensitive(node, name)->valueint;
}
double   JsonDouble(const cJSON* node, const char* name) {
    return cJSON_GetObjectItemCaseSensitive(node, name)->valuedouble;
}
char*    JsonString(const cJSON* node, const char* name) {
    char* d = cJSON_GetObjectItemCaseSensitive(node, name)->valuestring;
    if (!d) return NULL;
    char* str = (char*)malloc(strlen(d) + 1);
    memcpy(str, d, strlen(d) + 1);
    return str;
}
char*    JsonToString(const cJSON* node, const char* name, char* str, size_t len) {
    char* d = cJSON_GetObjectItemCaseSensitive(node, name)->valuestring;
    if (!d) return NULL;
    strncpy(str, d, len);
    return str;
}

void LoadFromKitsuResolve(void* opaque, char* source_text, size_t len) {
    Media* media = (Media*)opaque;
    media->RequestStatus = Media::LOADSTATUS_FINISHED_REQUEST;

    const cJSON* data = NULL;
    const cJSON* attributes = NULL;
    const cJSON* titles = NULL;
    const cJSON* relationships = NULL;
    const cJSON* episodes = NULL;
    const cJSON* links = NULL;
    const cJSON* self = NULL;
    const cJSON* posterImage = NULL;

    int status = 0;
    cJSON* json = cJSON_Parse(source_text);
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        status = 1;
        cJSON_Delete(json);
        return;
    }

    data = cJSON_GetObjectItemCaseSensitive(json, "data");
    attributes = cJSON_GetObjectItemCaseSensitive(data, "attributes");
    titles = cJSON_GetObjectItemCaseSensitive(attributes, "titles");
    relationships = cJSON_GetObjectItemCaseSensitive(data, "relationships");
    episodes = cJSON_GetObjectItemCaseSensitive(relationships, "episodes");
    links = cJSON_GetObjectItemCaseSensitive(episodes, "links");
    self = cJSON_GetObjectItemCaseSensitive(links, "self");
    posterImage = cJSON_GetObjectItemCaseSensitive(attributes, "posterImage");

    char* mediaType = JsonString(data, "type");
    char* mediaSubType = JsonString(attributes, "subtype");
    char* year = JsonString(attributes, "startDate");
    char* idString = JsonString(data, "id");

    media->ID = atoi(idString);

    if (cJSON_GetObjectItemCaseSensitive(titles, "en_us"))
        JsonToString(titles, "en_us", media->Title, sizeof(media->Title));
    else if (cJSON_GetObjectItemCaseSensitive(titles, "en"))
        JsonToString(titles, "en", media->Title, sizeof(media->Title));
    else if (cJSON_GetObjectItemCaseSensitive(titles, "en_jp"))
        JsonToString(titles, "en_jp", media->Title, sizeof(media->Title));
    else
        JsonToString(attributes, "canonicalTitle", media->Title, sizeof(media->Title));
    JsonToString(attributes, "synopsis", media->Description, sizeof(media->Description));
    media->Year = atoi(year);
    // media->EpisodeCount = JsonInt(attributes, "episodeCount");
    // int EpisodeLength = JsonInt(attributes, "episodeLength");
    if (!strcmp(mediaSubType, "movie"))
        media->IsSeries = false;
    else
        media->IsSeries = true;
    // media->SeasonCount = 1;

    /*

    char* episodes_text = HTTP::Get(JsonString(links, "self"), NULL, NULL);
    cJSON_Delete(json);
    json = cJSON_Parse(episodes_text);
    data = cJSON_GetObjectItemCaseSensitive(json, "data");

    media->Episodes = (BinjEpisode*)calloc(media->EpisodeCount, sizeof(BinjEpisode));

    const cJSON *epdata = NULL;
    cJSON_ArrayForEach(epdata, data) {
        char heeeep[64];
        sprintf(heeeep, "https://kitsu.io/api/edge/episodes/%s", JsonString(epdata, "id"));

        char* re = NULL;
        if (http)
            re = http->Get(heeeep);

        cJSON *json2 = cJSON_Parse(re);
        const cJSON *episode_data = cJSON_GetObjectItemCaseSensitive(json2, "data");
        episode_data = cJSON_GetObjectItemCaseSensitive(episode_data, "attributes");

        int index = JsonInt(episode_data, "number") - 1;
        if (index < 0)
            index = 0;

        media->Episodes[index].Title = NULL;
        media->Episodes[index].Description = NULL;

        if (media->MediaType != 2)
            media->Episodes[index].Title = JsonString(episode_data, "canonicalTitle");

        if (media->MediaType != 2)
            media->Episodes[index].Description = JsonString(episode_data, "synopsis");

        if (re)
            free(re);

        cJSON_Delete(json2);
    }

    for (int i = 0; i < media->EpisodeCount; i++) {
        media->Episodes[i].Length = EpisodeLength;
        media->Episodes[i].IntroStart = 0;
        media->Episodes[i].IntroLength = 90000;
        media->Episodes[i].CreditsStart = EpisodeLength * 60 * 1000 - 90000;
    }

    //*/

    // char toots[50];
    // sprintf(toots, FILENAME("cache/%d.bjmf"), media->ID);
    // SaveMediaInfo(media, toots);

    // Download textures
    char filesave[50], posterImageUrl[128];
    sprintf(filesave, "res/cache/%d.jpg", media->ID);
    media->Thumbnail = (Image*)Image::Create(NULL);
    if (!File::Exists(filesave)) {
        if (!Directory::Exists("res/cache"))
            Directory::Create("res/cache");

        JsonToString(posterImage, "medium", posterImageUrl, sizeof(posterImageUrl));
        size_t imgLen;
        char*  imgData = HTTP::Get(posterImageUrl, &imgLen);
        File::WriteAllBytes(filesave, imgData, imgLen);

        SDL_RWops* rw = SDL_RWFromMem(imgData, imgLen);
        media->Thumbnail->SurfacePtr = IMG_Load_RW(rw, false);
        SDL_RWclose(rw);
        free(imgData);
    }
    else {
        media->Thumbnail->SurfacePtr = IMG_Load(filesave);
        // Basically, if corrupted download
        if (media->Thumbnail->SurfacePtr == NULL) {
            if (!File::Remove(filesave)) {
                Log::Print(Log::LOG_ERROR, "Could not delete corrupted file at \"%s\".", filesave);
            }
        }
    }

    cJSON_Delete(json);
    free(mediaType);
    free(mediaSubType);
    free(year);
    free(idString);
}
void LoadFromKitsuReject(void* opaque, char* data, size_t len) {
    Media* self = (Media*)opaque;
    self->RequestStatus = Media::LOADSTATUS_REJECTED_REQUEST;

    printf("%*d: %s\n", 5, self->ID, "REJECTED");
}

PUBLIC bool Media::LoadFromKitsu() {
    if (!ID) return false;

    if (RequestStatusURL == NULL) {
        RequestStatusURL = (char*)malloc(51);
        if (RequestStatusURL == NULL) {
            return false;
        }
    }
    snprintf(RequestStatusURL, 50, "https://kitsu.io/api/edge/anime/%d", ID);

    RequestStatus = LOADSTATUS_STARTED_REQUEST;

    size_t len;
    char*  data = HTTP::Get(RequestStatusURL, &len);
    if (data == NULL) {
        free(RequestStatusURL);
        RequestStatusURL = NULL;
        return false;
    }
    LoadFromKitsuResolve(this, data, len);
    return true;
}
PUBLIC bool Media::LoadFromCache() {
    if (!ID) return false;

    char resPath[50];
    sprintf(resPath, "res/cache/%d.bmcache", ID);
    if (!File::Exists(resPath))
        return false;

    char cachedImage[50];
    sprintf(cachedImage, "res/cache/%d.jpg", ID);
    Thumbnail = (Image*)Image::Create(NULL);
    if (File::Exists(cachedImage)) {
        Thumbnail->SurfacePtr = IMG_Load(cachedImage);
        if (Thumbnail->SurfacePtr == NULL) {
            if (!File::Remove(cachedImage)) {
                Log::Print(Log::LOG_ERROR, "Could not delete corrupted file at \"%s\".", cachedImage);
            }
        }

        RequestStatus = Media::LOADSTATUS_FINISHED_REQUEST;
        return true;
    }

    return false;
}

PUBLIC void Media::LoadInfo() {
    if (Media::LoadFromCache())
        return;
    if (Media::LoadFromKitsu())
        return;

    printf("%s\n", "Couldn't find info anywhere chief!");
}

PUBLIC void Media::SetTitle(const char* title) {
    strncpy(Title, title, 32);
}
PUBLIC void Media::SetDescription(const char* description) {
    strncpy(Description, description, 256);
}
PUBLIC void Media::SetFilename(const char* filename) {
    if (!filename) return;

    size_t len = strlen(filename);
    if (!Filename) {
        Filename = (char*)malloc(len + 1);
        memcpy(Filename, filename, len);
        Filename[len] = 0;
    }
    else {
        size_t ourlen = strlen(Filename);
        if (len > ourlen) {
            Filename = (char*)realloc(Filename, len + 1);
        }
        strcpy(Filename, filename);
    }
}

PUBLIC void Media::Dispose() {
    if (RequestStatusURL)
        free(RequestStatusURL);
}
