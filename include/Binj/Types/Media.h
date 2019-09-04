#ifndef MEDIA_H
#define MEDIA_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Image;

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

    bool LoadFromKitsu();
    bool LoadFromCache();
    void LoadInfo();
    void SetTitle(const char* title);
    void SetDescription(const char* description);
    void SetFilename(const char* filename);
    void Dispose();
};

#endif /* MEDIA_H */
