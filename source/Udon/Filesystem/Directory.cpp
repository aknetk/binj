#if INTERFACE
#include <Udon/Standard.h>

class Directory {
private:
};
#endif

#include <Udon/Filesystem/Directory.h>

#if WIN32

#elif MACOSX || LINUX || SWITCH || IOS || ANDROID
    #include <dirent.h>
    #include <sys/stat.h>
#endif

bool WildcardMatch(const char* first, const char* second) {
    if (*first == 0 && *second == 0)
        return true;
    if (*first == 0 && *second == '*' && *(second + 1) != 0)
        return false;
    if (*first == *second || *second == '?')
        return WildcardMatch(first + 1, second + 1);
    if (*second == '*')
        return WildcardMatch(first, second + 1) || WildcardMatch(first + 1, second);
    return false;
}
bool SortFunction(char* i, char* j) {
    return strcmp(i, j) <= 0;
}

PUBLIC STATIC bool          Directory::Exists(const char* path) {
    #if WIN32

    #elif MACOSX || LINUX || SWITCH || IOS || ANDROID
        DIR* dir = opendir(path);
        if (dir) {
            closedir(dir);
            return true;
        }
    #endif
    return false;
}
PUBLIC STATIC bool          Directory::Create(const char* path) {
    #if WIN32

    #elif MACOSX || LINUX || SWITCH || IOS || ANDROID
        return mkdir(path, 0777) == 0;
    #endif
    return false;
}

PUBLIC STATIC void          Directory::GetFiles(vector<char*>* files, const char* path, const char* searchPattern, bool allDirs) {
    #if WIN32

    #elif MACOSX || LINUX || SWITCH || IOS || ANDROID || NX
        DIR* dir = opendir(path);
        if (dir) {
            int i;
            struct dirent* d;

            int indexBegin = files->size();

            while ((d = readdir(dir)) != NULL) {
                if (d->d_name[0] == '.' && !d->d_name[1]) continue;
                if (d->d_name[0] == '.' && d->d_name[1] == '.' && !d->d_name[2]) continue;

                if (d->d_type == DT_DIR) {
                    if (allDirs) {
                        char fullpath[128];
                        sprintf(fullpath, "%s/%s", path, d->d_name);
                        Directory::GetFiles(files, fullpath, searchPattern, true);
                    }
                }
                else if (WildcardMatch(d->d_name, searchPattern)) {
                    i = strlen(d->d_name) + strlen(path) + 1;
                    char* str = (char*)calloc(1, i + 1);
                    sprintf(str, "%s/%s", path, d->d_name);
                    str[i] = 0;
                    files->push_back(str);
                }
            }
            closedir(dir);

            int indexEnd = files->size();

            std::sort(files->begin() + indexBegin, files->begin() + indexEnd, SortFunction);
        }
    #endif
}
PUBLIC STATIC vector<char*> Directory::GetFiles(const char* path, const char* searchPattern, bool allDirs) {
    vector<char*> files;
    Directory::GetFiles(&files, path, searchPattern, allDirs);
    return files;
}

PUBLIC STATIC void          Directory::GetDirectories(vector<char*>* files, const char* path, const char* searchPattern, bool allDirs) {
    #if WIN32

    #elif MACOSX || LINUX || SWITCH || IOS || ANDROID || NX
        DIR* dir = opendir(path);
        if (dir) {
            int i;
            struct dirent* d;

            int indexBegin = files->size();

            while ((d = readdir(dir)) != NULL) {
                if (d->d_name[0] == '.' && !d->d_name[1]) continue;
                if (d->d_name[0] == '.' && d->d_name[1] == '.' && !d->d_name[2]) continue;

                if (d->d_type == DT_DIR) {
                    if (WildcardMatch(d->d_name, searchPattern)) {
                        i = strlen(d->d_name) + strlen(path) + 1;
                        char* str = (char*)calloc(1, i + 1);
                        sprintf(str, "%s/%s", path, d->d_name);
                        str[i] = 0;
                        files->push_back(str);
                    }
                    if (allDirs) {
                        char fullpath[128];
                        sprintf(fullpath, "%s/%s", path, d->d_name);
                        Directory::GetDirectories(files, fullpath, searchPattern, true);
                    }
                }
            }
            closedir(dir);

            int indexEnd = files->size();

            std::sort(files->begin() + indexBegin, files->begin() + indexEnd, SortFunction);
        }
    #endif
}
PUBLIC STATIC vector<char*> Directory::GetDirectories(const char* path, const char* searchPattern, bool allDirs) {
    vector<char*> files;
    Directory::GetDirectories(&files, path, searchPattern, allDirs);
    return files;
}
