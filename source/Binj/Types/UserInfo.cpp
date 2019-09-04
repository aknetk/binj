#if INTERFACE
#include <Udon/Standard.h>
#include <Binj/Types/Enums.h>
#include <Binj/Types/Structs.h>

class UserInfo {
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
};
#endif

#include <Binj/Types/UserInfo.h>

PUBLIC int UserInfo::SetEmailAddress(int number) {
    return 0;
}
