#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>

class RingBuffer {
public:
    int   Size;
    int   Length;
    int   WritePos;
    int   ReadPos;
    char* Data;

           RingBuffer(Uint32 size);
           ~RingBuffer();
    int    Write(const char* data, int len);
    void   ReadData(char* data, const int len);
    int    Read(char* data, int len);
    int    Peek(char *data, int len);
    int    Advance(int len);
    int    GetLength();
    int    GetSize();
    int    GetFree();
};

#endif /* RINGBUFFER_H */
