#if INTERFACE
#include <Udon/Standard.h>
#include <Binj/Types/Enums.h>
#include <Binj/Types/Structs.h>
#include <Binj/Types/UserInfo.h>

class AccountInfo {
// private:
//     char                      EmailAddress[128];
public:
    static Uint32            CurrentUser;
    static vector<UserInfo*> Users;
};
#endif

#include <Binj/Types/AccountInfo.h>

Uint32            AccountInfo::CurrentUser = 0;
vector<UserInfo*> AccountInfo::Users;

PUBLIC STATIC UserInfo* AccountInfo::User() {
    return AccountInfo::Users[AccountInfo::CurrentUser];
}

PUBLIC STATIC bool AccountInfo::LoadFromFile(const char* filename) {
    bool res;
    SDL_RWops* rw = SDL_RWFromFile(filename, "rb");
    if (!rw) {
        return false;
    }
    res = AccountInfo::LoadFromRW(rw);
    SDL_RWclose(rw);
    return res;
}
PUBLIC STATIC bool AccountInfo::LoadFromRW(SDL_RWops* rw) {
    bool res;
    size_t len = SDL_RWsize(rw);
    Uint8* data = (Uint8*)malloc(len);
    if (!data) {
        return false;
    }
    SDL_RWread(rw, data, len, 1);
    res = AccountInfo::LoadFromBytes(data, len);
    free(data);
    return res;
}
PUBLIC STATIC bool AccountInfo::LoadFromBytes(Uint8* data, size_t len) {
    // Uint8* p = data;

    return true;
}
