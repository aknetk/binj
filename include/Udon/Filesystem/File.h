#ifndef FILE_H
#define FILE_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>

class File {
public:
    static bool Exists(const char* path);
    static bool Remove(const char* path);
    static bool WriteAllBytes(const char* path, const char* bytes, size_t len);
};

#endif /* FILE_H */
