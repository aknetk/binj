#if INTERFACE
#include <Udon/Standard.h>
#include <curl/curl.h>

class HTTP {
    static bool Initialized;
    static CURL* Curl;
};
#endif

#include <Udon/Network/HTTP.h>

#include <Udon/Logging/Log.h>

#if WIN32
    // #include <io.h>
#else
    #include <sys/select.h>
#endif

bool  HTTP::Initialized = false;
CURL* HTTP::Curl = NULL;

struct HTTPData {
    void*  Data;
    size_t Length;
};
struct HTTPFunction {
    const char* url;
    void(*resolve)(void*, char*, size_t);
    void(*reject)(void*, char*, size_t);
    void* opaque;
    bool free;
    char* data;
    size_t len;
};

size_t HTTP_WriteCallback(char* src, size_t count, size_t size, HTTPData* dest) {
    size_t new_len = dest->Length + size;
    dest->Data = realloc(dest->Data, new_len + 1);

    if (dest->Data == NULL) {
        Log::Print(Log::LOG_ERROR, "Could not reallocate memory!");
        exit(EXIT_FAILURE);
    }

    memcpy((char*)dest->Data + dest->Length, src, size);
    *((char*)dest->Data + new_len) = 0;
    dest->Length = new_len;
    return size;
}

PUBLIC STATIC bool  HTTP::CheckInit() {
    if (HTTP::Initialized) return false;

    if (Curl)
        return false;

    Curl = curl_easy_init();
    if (!Curl)
        return true;

    HTTP::Initialized = true;
    return false;
}

PUBLIC STATIC int   HTTP::GetThreaded(void* opaque) {
    HTTPFunction* functions = (HTTPFunction*)opaque;

    // Log::Print(Log::LOG_INFO, "Starting GET request to '%s'.", functions->url);
    HTTPData data { malloc(1), 0 };
    curl_easy_reset(HTTP::Curl);
    curl_easy_setopt(HTTP::Curl, CURLOPT_URL, functions->url);
    curl_easy_setopt(HTTP::Curl, CURLOPT_VERBOSE, 0);
    // curl_easy_setopt(HTTP::Curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(HTTP::Curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(HTTP::Curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(HTTP::Curl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(HTTP::Curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(HTTP::Curl, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(HTTP::Curl, CURLOPT_WRITEFUNCTION, HTTP_WriteCallback);
	CURLcode res = curl_easy_perform(HTTP::Curl);
	if (res != 0) {
		Log::Print(Log::LOG_ERROR, "HTTP::Get Error (%d) '%s' for URL: %s", res, curl_easy_strerror(res), functions->url);

        if (functions->reject)
            functions->reject(functions->opaque, (char*)data.Data, data.Length);

        functions->data = (char*)data.Data;
        functions->len = data.Length;

        if (functions->free)
            free(data.Data);
        free(opaque);
        return 1;
	}
    // Log::Print(Log::LOG_INFO, "Finished request to '%s'.", functions->url);

    if (functions->resolve)
        functions->resolve(functions->opaque, (char*)data.Data, data.Length);

    // Log::Print(Log::LOG_INFO, "Obtained %zu bytes of data.", data.Length);

    functions->data = (char*)data.Data;
    functions->len = data.Length;

    if (functions->free)
        free(data.Data);
    free(opaque);
    return 0;
}
PUBLIC STATIC void  HTTP::GetAsync(const char* url, void(*resolve)(void*, char*, size_t), void(*reject)(void*, char*, size_t), void* opaque) {
    if (HTTP::CheckInit()) return;

    HTTPFunction* functions = (HTTPFunction*)calloc(1, sizeof(HTTPFunction));
    functions->url = url;
    functions->resolve = resolve;
    functions->reject = reject;
    functions->opaque = opaque;
    functions->free = true;
    functions->data = NULL;

    SDL_DetachThread(SDL_CreateThread(HTTP::GetThreaded, "HTTP::GetAsync", functions));
}
PUBLIC STATIC char* HTTP::Get(const char* url, size_t* len) {
    if (HTTP::CheckInit()) return NULL;

    HTTPFunction* functions = (HTTPFunction*)calloc(1, sizeof(HTTPFunction));
    functions->url = url;
    // functions->resolve = resolve;
    // functions->reject = reject;
    // functions->opaque = opaque;
    functions->free = false;
    functions->data = NULL;

    HTTP::GetThreaded(functions);

    *len = functions->len;
    return functions->data;
}
