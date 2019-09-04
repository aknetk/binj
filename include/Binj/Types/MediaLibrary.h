#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Binj/Types/Media.h>

class MediaLibrary {
public:
    static vector<Media*> List;

};

#endif /* MEDIALIBRARY_H */
