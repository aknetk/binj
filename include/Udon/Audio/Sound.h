#ifndef SOUND_H
#define SOUND_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>

class Sound {
public:
    Uint8*      Buffer;
    Uint8*      BufferStart;
    Uint32      Length;
    const char* Filename = NULL;

    Sound(const char* filename);
    void Dispose();
};

#endif /* SOUND_H */
