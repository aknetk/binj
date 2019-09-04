#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Rendering/Texture.h>

class MediaEpisode {
public:
    char*    Title;
    char*    Description;
    double   Length;
    Texture* Thumbnail;

    bool     HasIntro;
    double   IntroStart;
    double   IntroLength;

    bool     HasCredits;
    double   CreditsStart;
    double   CreditsLength;
    
    bool     HasPostCredits;
    bool     HasPreview;
};
#endif

#include <Binj/Types/MediaEpisode.h>
