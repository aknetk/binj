#ifndef ACCOUNTINFO_H
#define ACCOUNTINFO_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <Binj/Types/Enums.h>
#include <Binj/Types/Structs.h>
#include <Binj/Types/UserInfo.h>

class AccountInfo {
private:
    // private:
    //     char                      EmailAddress[128];


public:
    static Uint32            CurrentUser;
    static vector<UserInfo*> Users;

    static UserInfo* User();
    static bool LoadFromFile(const char* filename);
    static bool LoadFromRW(SDL_RWops* rw);
    static bool LoadFromBytes(Uint8* data, size_t len);
};

#endif /* ACCOUNTINFO_H */
