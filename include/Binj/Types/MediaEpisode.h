#ifndef MEDIAEPISODE_H
#define MEDIAEPISODE_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class Texture;

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

#endif /* MEDIAEPISODE_H */
