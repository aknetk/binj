#ifndef HTTP_H
#define HTTP_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <curl/curl.h>

class HTTP {
private:
    static bool Initialized;
    static CURL* Curl;


public:
    static bool  CheckInit();
    static int   GetThreaded(void* opaque);
    static void  GetAsync(const char* url, void(*resolve)(void*, char*, size_t), void(*reject)(void*, char*, size_t), void* opaque);
    static char* Get(const char* url, size_t* len);
};

#endif /* HTTP_H */
