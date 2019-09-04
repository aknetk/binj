#ifndef VIDEO_H
#define VIDEO_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

class MediaPlayer;
class MediaSource;
class Texture;
class Texture;
class Texture;
class Text;
class Text;
class Text;
class Text;

#include <Udon/Standard.h>
#include <Udon/Objects/Node.h>
#include <Udon/Objects/Text.h>
#include <Udon/Rendering/Texture.h>
#include <Udon/Media/MediaSource.h>
#include <Udon/Media/MediaPlayer.h>

class Video : public Node {
public:
    MediaPlayer*  Backend = NULL;
    MediaSource*  BackendSource = NULL;
    char*         Source = NULL;
    bool          ShowControls = false;
    Texture*      VideoTexture = NULL;
    Texture*      SubtitleTexture = NULL;
    Texture*      IconTexture = NULL;
    Text*         TextMediaTitle = NULL;
    Text*         TextTimeRemaining = NULL;
    Text*         TextHeaderAudio = NULL;
    Text*         TextHeaderSubtitles = NULL;
    int           TextTimeRemainingWidth = 0;
    Uint32 LastMovementTick = 0;
    bool   Flag_ShowControls = false;
    double Anim_ShowControls = 0.0;
    double AnLen_ShowControls = 0.25;
    double Value_ShowControls = 0.0;
    bool   Flag_ShowPauseIndicator = false;
    double Anim_ShowPauseIndicator = 0.0;
    double AnLen_ShowPauseIndicator = 0.75;
    double Value_ShowPauseIndicator = 0.0;

    static Node* Create(const char* id);
    void OnEvent(SDL_Event e);
    void DoSeek(double pos);
    void TogglePause();
    void WaitForSource();
    void Update();
    void Render();
    void Dispose();
};

#endif /* VIDEO_H */
