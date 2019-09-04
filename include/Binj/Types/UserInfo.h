#ifndef USERINFO_H
#define USERINFO_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Texture;

#include <Udon/Standard.h>
#include <Binj/Types/Enums.h>
#include <Binj/Types/Structs.h>

class UserInfo {
private:
    // private:
    //     Uint32                    Age;
    //     Uint32                    Gender;
    //     char                      LocationCountry[32];
    //     char                      LocationRegion[32];


public:
    char                      Name[32];
    Uint32                    AudioLanguage;
    Uint32                    ApplicationLanguage;
    Texture*                  ProfileImage;
    vector<QueueItem*>        Queue;
    vector<WatchHistoryItem*> WatchHistory;
    vector<char*>             SearchHistory;

    int SetEmailAddress(int number);
};

#endif /* USERINFO_H */
