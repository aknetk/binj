#ifndef MEDIAEPISODEGROUP_H
#define MEDIAEPISODEGROUP_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Binj/Types/MediaEpisode.h>

class MediaEpisodeGroup {
public:
    char                  Title[32];
    vector<MediaEpisode*> Episodes;

};

#endif /* MEDIAEPISODEGROUP_H */
