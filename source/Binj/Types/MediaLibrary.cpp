#if INTERFACE
#include <Udon/Standard.h>
#include <Binj/Types/Media.h>

class MediaLibrary {
public:
    static vector<Media*> List;
};
#endif

#include <Binj/Types/MediaLibrary.h>

vector<Media*> MediaLibrary::List;
