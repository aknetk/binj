#if INTERFACE
#include <Udon/Standard.h>
class File {
public:
};
#endif

#include <Udon/Filesystem/File.h>

#if WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

PUBLIC STATIC bool File::Exists(const char* path) {
    #if WIN32
        return _access(path, 0) != -1;
    #elif MACOSX || LINUX || SWITCH || IOS || ANDROID
        return access(path, F_OK) != -1;
    #endif
    return false;
}
PUBLIC STATIC bool File::Remove(const char* path) {
    #if WIN32
        return remove(path) == 0;
    #elif MACOSX || LINUX || SWITCH || IOS || ANDROID
        return remove(path) == 0;
    #endif
    return false;
}

PUBLIC STATIC bool File::WriteAllBytes(const char* path, const char* bytes, size_t len) {
    if (!path) return false;
    if (!*path) return false;
    if (!bytes) return false;

    FILE* f = fopen(path, "wb");
    if (!f) return false;

    fwrite(bytes, len, 1, f);
    fclose(f);
    return true;
}
