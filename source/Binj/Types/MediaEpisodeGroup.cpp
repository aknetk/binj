#if INTERFACE
#include <Udon/Standard.h>
#include <Binj/Types/MediaEpisode.h>

class MediaEpisodeGroup {
public:
    char                  Title[32];
    vector<MediaEpisode*> Episodes;
};
#endif

#include <Binj/Types/MediaEpisodeGroup.h>
