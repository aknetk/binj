#if INTERFACE
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
};
#endif

#include <Udon/Objects/Video.h>

#include <Udon/Application.h>
#include <Udon/Easing/Ease.h>
#include <Udon/Media/Decoders/VideoDecoder.h>
#include <Udon/Media/Utils/MediaPlayerState.h>

#define AutoAnim(name) \
    if (Flag_##name) { \
        Anim_##name = min(Anim_##name + 1.0, Application::FPS * AnLen_##name); \
        Value_##name = Ease::InOutQuad(Anim_##name / Application::FPS / AnLen_##name); \
    } \
    else { \
        Anim_##name = max(Anim_##name - 1.0, 0.0); \
        Value_##name = Ease::InOutQuad(Anim_##name / Application::FPS / AnLen_##name); \
    }

#define ATLAS_WIDTH 4096
#define ATLAS_HEIGHT 4096
#define ATLAS_MAX 1024

SDL_Rect sources[ATLAS_MAX];
SDL_Rect targets[ATLAS_MAX];

PUBLIC STATIC Node* Video::Create(const char* id) {
    Video* node = new Video;
    node->ApplyCreate(id);

    node->TextMediaTitle = (Text*)Text::Create(NULL);
    node->TextMediaTitle->Font = "Hind Medium";

    node->TextTimeRemaining = (Text*)Text::Create(NULL);
    node->TextTimeRemaining->Font = "Helvetica";

    node->TextHeaderAudio = (Text*)Text::Create(NULL);
    node->TextHeaderAudio->Font = "Hind Bold";
    node->TextHeaderAudio->SetText("Audio");

    node->TextHeaderSubtitles = (Text*)Text::Create(NULL);
    node->TextHeaderSubtitles->Font = "Hind Bold";
    node->TextHeaderSubtitles->SetText("Subtitles");

    SDL_Surface* temp = IMG_Load("res/images/IconVideoButtons.png");
    node->IconTexture = Application::RendererPtr->CreateTextureFromSurface(temp);
    SDL_FreeSurface(temp);

    int secondsToBuffer = 15;
    MediaPlayerState::ThreadCount = SDL_GetCPUCount() <= 4 ? SDL_GetCPUCount() : 4;
    MediaPlayerState::VideoBufFrames = 25 * secondsToBuffer;
    MediaPlayerState::AudioBufFrames = 192 * secondsToBuffer;
    MediaPlayerState::SubtitleBufFrames = 64;

    node->PaddingTop = 25;
    node->PaddingLeft = 25;
    node->PaddingRight = 25;

    node->LastMovementTick = SDL_GetTicks();

    return node;
}

bool MouseDown = false;
bool Dragging = false;
bool SpaceDown = false;

PUBLIC void Video::OnEvent(SDL_Event e) {
    switch (e.type) {
        case SDL_MOUSEMOTION:
            LastMovementTick = SDL_GetTicks();
            break;
        case SDL_JOYAXISMOTION:
            LastMovementTick = SDL_GetTicks();
            // printf("controller: %d axis:   0x%X value: %d\n", e.jaxis.which, e.jaxis.axis, e.jaxis.value);
            // 0: L-Stick X
            // 1: L-Stick Y
            // 2: R-Stick X
            // 3: R-Stick Y
            break;
        case SDL_JOYBUTTONDOWN:
            // printf("controller: %d button: 0x%X state: 0x%X\n", e.jbutton.which, e.jbutton.button, e.jbutton.state);
            // A: 0
            // B: 1
            // X: 2
            // Y: 3
            // LStick: 4
            // RStick: 5
            // L: 6
            // R: 7
            // ZL: 8
            // ZR: 9
            // +: 10
            // -: 11
            // Left: 12
            // Up: 13
            // Right: 14
            // Down: 15

            // A
            if (e.jbutton.button == 0) {
                TogglePause();
            }
            // L
            else if (e.jbutton.button == 6) {
                DoSeek(Backend->GetPosition() - 10.0);
                LastMovementTick = SDL_GetTicks();
            }
            // R
            else if (e.jbutton.button == 7) {
                DoSeek(Backend->GetPosition() + 10.0);
                LastMovementTick = SDL_GetTicks();
            }
            break;
    }
}
PUBLIC void Video::DoSeek(double pos) {
    Backend->Seek(pos);
}
PUBLIC void Video::TogglePause() {
    if (Backend->GetPlayerState() == MediaPlayer::KIT_PAUSED) {
        Backend->Play();
    }
    else {
        Backend->Pause();
    }
}

PUBLIC void Video::WaitForSource() {
    if (!Source) return;
    if (BackendSource) return;
    // #ifdef SWITCH
    // node->BackendSource = MediaSource::CreateSourceFromUrl("full.mkv");
    // node->TextMediaTitle->SetText("My Hero Academia");
    // // node->BackendSource = MediaSource::CreateSourceFromUrl("../media/Your Name [11614].mkv");
    // // node->BackendSource = MediaSource::CreateSourceFromUrl("../media/Spider-Man Into The Spiderverse [0000].mp4");
    // // node->TextMediaTitle->SetText("Spider-Man: Into The Spiderverse");
    // #else
    // // node->BackendSource = MediaSource::CreateSourceFromUrl("/Volumes/Liquid/Movies & Media/Spider-Man Into The Spiderverse [0000].mp4");
    // node->BackendSource = MediaSource::CreateSourceFromUrl("/Volumes/Liquid/Movies & Media/Your Name [11614].mkv");
    // node->TextMediaTitle->SetText("Your Name");
    // #endif
    // node->BackendSource = MediaSource::CreateSourceFromUrl("rtsp://192.168.1.6/3471_0000.mkv");

    BackendSource = MediaSource::CreateSourceFromUrl(Source);

    if (!BackendSource) {
        Log::Print(Log::LOG_ERROR, "Unable to load video source!");
        return;
    }

    Backend = MediaPlayer::Create(
        BackendSource,
        BackendSource->GetBestStream(MediaSource::STREAMTYPE_VIDEO),
        BackendSource->GetBestStream(MediaSource::STREAMTYPE_AUDIO),
        #ifdef SWITCH
        -1,
        #else
        -1,
        // BackendSource->GetBestStream(MediaSource::STREAMTYPE_SUBTITLE),
        #endif
        1280, 720);
    if (!Backend) {
        Log::Print(Log::LOG_ERROR, "Unable to create player!");
        return;
    }

    PlayerInfo playerInfo;
    Backend->GetInfo(&playerInfo);

    Log::Print(Log::LOG_WARN, "VIDEO STREAM:");
    Log::Print(Log::LOG_INFO,    "    Resolution:  %d x %d", playerInfo.Video.Output.Width, playerInfo.Video.Output.Height);
    Log::Print(Log::LOG_WARN, "AUDIO STREAM:");
    Log::Print(Log::LOG_INFO,    "    Sample Rate: %d", playerInfo.Audio.Output.SampleRate);
    Log::Print(Log::LOG_INFO,    "    Bit Depth:   %d-bit", playerInfo.Audio.Output.Format & 0xFF);
    Log::Print(Log::LOG_INFO,    "    Channels:    %d", playerInfo.Audio.Output.Channels);

    VideoTexture = G->CreateTexture(
        playerInfo.Video.Output.Format,
        SDL_TEXTUREACCESS_STATIC,
        playerInfo.Video.Output.Width,
        playerInfo.Video.Output.Height);
    if (!VideoTexture) {
        Log::Print(Log::LOG_ERROR, "Unable to create video texture!");
        return;
    }

    if (Backend->GetSubtitleStream() >= 0) {
        SubtitleTexture = G->CreateTexture(
            playerInfo.Subtitle.Output.Format,
            SDL_TEXTUREACCESS_STATIC,
            ATLAS_WIDTH, ATLAS_HEIGHT);

        if (!SubtitleTexture) {
            Log::Print(Log::LOG_ERROR, "Unable to create subtitle texture!");
            return;
        }
    }

    Backend->Play();
}

PUBLIC void Video::Update() {
    WaitForSource();
    if (!Source) return;

    int mx, my;
    if ((SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        float barx = X + PaddingLeft;
        float bary = Y + Height * 6 / 7;
        float barwidth = Width - PaddingLeft - PaddingRight - TextTimeRemainingWidth - PaddingRight / 2;
        float barpos = barx + barwidth * Backend->GetPosition() / Backend->GetDuration();

        if (!MouseDown && my < bary - 15) {
            TogglePause();
        }

        if (std::abs(my - bary) < 15 || Dragging) {
            if (!MouseDown && std::abs(mx - barpos) < 15) {
                Dragging = true;
            }
            if (!MouseDown || Dragging) {
                double pos = (double)(mx - barx) / barwidth * Backend->GetDuration();
                if (Backend->GetPlayerState() == MediaPlayer::KIT_STOPPED)
                    Backend->Play();

                DoSeek(pos);
                if (!Dragging)
                    Backend->GetVideoDataForPaused(VideoTexture);
            }
        }
    }
    MouseDown = (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT));
    if (!MouseDown) {
        Dragging = false;
    }

    char time_remaining_text[16];
    int  time_remaining = (int)(Backend->GetDuration() - Backend->GetPosition());
    // In seconds (ads)
    if (Backend->GetDuration() < 60.0) {
        sprintf(time_remaining_text, "%ds", time_remaining);
        if (TextTimeRemainingWidth == 0) {
            TextTimeRemaining->SetText("XXs");
            TextTimeRemainingWidth = TextTimeRemaining->Width;
        }
    }
    // In minutes and seconds (short-form, episodic media  <1 hour)
    else if (Backend->GetDuration() < 60.0 * 60.0) {
        sprintf(time_remaining_text, "%d:%02d", time_remaining / 60, time_remaining % 60);
        if (TextTimeRemainingWidth == 0) {
            if (Backend->GetDuration() < 60.0 * 10.0)
                TextTimeRemaining->SetText("X:XX");
            else
                TextTimeRemaining->SetText("XX:XX");
            TextTimeRemainingWidth = TextTimeRemaining->Width;
        }
    }
    // In hours, minutes and seconds (long-form, movie media  >1 hour)
    else {
        sprintf(time_remaining_text, "%d:%02d:%02d", time_remaining / 3600, (time_remaining / 60) % 60, time_remaining % 60);
        if (TextTimeRemainingWidth == 0) {
            TextTimeRemaining->SetText("X:XX:XX");
            TextTimeRemainingWidth = TextTimeRemaining->Width;
        }
    }
    TextTimeRemaining->SetText(time_remaining_text);

    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_SPACE] && !SpaceDown) {
        // TogglePause();
    }
    SpaceDown = state[SDL_SCANCODE_SPACE];

    Flag_ShowPauseIndicator = Backend->GetPlayerState() == MediaPlayer::KIT_PAUSED;
    AutoAnim(ShowPauseIndicator);

    if (SDL_GetTicks() - LastMovementTick > 5000) {
        Flag_ShowControls = false;
    }
    else {
        Flag_ShowControls = true;
    }
    AutoAnim(ShowControls);
}
PUBLIC void Video::Render() {
    if (!Source) return;
    G->SetBlendColor(Color::FromARGB(0xFF, 0x000000));
    G->FillRectangle(X, Y, Width, Height);

    if (VideoTexture) {
        #define AUDIOBUFFER_SIZE 0x8000
        Uint8 audioBuf[AUDIOBUFFER_SIZE];
        Backend->GetVideoData(VideoTexture);
        // Backend->GetAudioData(audioBuf, AUDIOBUFFER_SIZE);

        int queued = SDL_GetQueuedAudioSize(1);
        if (queued < AUDIOBUFFER_SIZE) {
            int ret = Backend->GetAudioData(audioBuf, AUDIOBUFFER_SIZE - queued);
            if (ret > 0) {
                SDL_QueueAudio(1, audioBuf, ret);
            }
        }
    }

    float rect[4];
    switch (0) {
        // Contain
        case 0:
            if (VideoTexture->Width / Width < VideoTexture->Height / Height)
                rect[2] = VideoTexture->Width * Height / VideoTexture->Height, rect[3] = Height;
            else
                rect[2] = Width, rect[3] = VideoTexture->Height * Width / VideoTexture->Width;
            break;
        // Cover
        case 1:
            if (VideoTexture->Width / Width > VideoTexture->Height / Height)
                rect[2] = VideoTexture->Width * Height / VideoTexture->Height, rect[3] = Height;
            else
                rect[2] = Width, rect[3] = VideoTexture->Height * Width / VideoTexture->Width;
            break;
        // Stretch
        case 2:
            rect[2] = Width, rect[3] = Height;
            break;
    }
    rect[0] = (Width - rect[2]) / 2;
    rect[1] = (Height - rect[3]) / 2;

    G->SetClip(X, Y, Width, Height);
    G->SetBlendColor(Color::FromARGB(0xFF, 0xFFFFFF));
    G->DrawTexture(VideoTexture, X + rect[0], Y + rect[1], rect[2], rect[3]);
    G->ClearClip();

    if (SubtitleTexture) {
        // int got = Backend->GetSubtitleData(SubtitleTexture, sources, targets, ATLAS_MAX);
        // for (int i = 0; i < got; i++) {
        //     SDL_Rect src = sources[i];
        //     SDL_Rect dst = targets[i];
        //     G->DrawTexture(SubtitleTexture, src.x, src.y, src.w, src.h, dst.x, dst.y, dst.w, dst.h);
        // }
    }

    // Media Player Overlays
    // TextMediaTitle->SetText("My Hero Academia");
    TextMediaTitle->Opacity = Value_ShowControls;
    TextMediaTitle->X = X + PaddingLeft;
    TextMediaTitle->Y = Y + PaddingTop - TextMediaTitle->Height / 4;
    TextMediaTitle->Render();

    // Pause, Play, Jump Back 10, Jump Forward 10
    // Options, HD, SD, Volume
    // Subtitle, Close, Back

    // /*

    double icon_size    = Height * 1 / 7 - 30;
    double icon_space   = Height * 1 / 7;
    double icon_ceny    = Height * 13 / 14;
    double icon_middlex = Width / 2 - icon_size / 2;
    // #ifdef SWITCH
    icon_size *= 0.75;
    icon_space *= 0.75;
    // #endif
    // Pause/Play
    G->SetBlendColor(Color::FromARGB(int(0xC0 * Value_ShowControls), 0xFFFFFF));
    G->DrawTexture(IconTexture, (Backend->GetPlayerState() == MediaPlayer::KIT_PAUSED) * 160, 0, 160, 160, icon_middlex, icon_ceny - icon_size / 2, icon_size, icon_size);

    // Jump Back
    G->SetBlendColor(Color::FromARGB(int(0xC0 * Value_ShowControls), 0xFFFFFF));
    G->DrawTexture(IconTexture, 320, 0, 160, 160, icon_middlex - icon_space, icon_ceny - icon_size / 2, icon_size, icon_size);
    // Jump Forward
    G->SetBlendColor(Color::FromARGB(int(0xC0 * Value_ShowControls), 0xFFFFFF));
    G->DrawTexture(IconTexture, 480, 0, 160, 160, icon_middlex + icon_space, icon_ceny - icon_size / 2, icon_size, icon_size);

    // Volume
    G->SetBlendColor(Color::FromARGB(int(0xC0 * Value_ShowControls), 0xFFFFFF));
    G->DrawTexture(IconTexture, 480, 160, 160, 160, X + PaddingLeft, icon_ceny - icon_size / 2, icon_size, icon_size);
    // Language
    G->SetBlendColor(Color::FromARGB(int(0xC0 * Value_ShowControls), 0xFFFFFF));
    G->DrawTexture(IconTexture, 0, 320, 160, 160, X + Width - PaddingLeft - icon_size, icon_ceny - icon_size / 2, icon_size, icon_size);

    //*/

    /*
    // A good media file needs:
    -  Both streams (video & audio) need to end at the exact same time.
    -  Japanese Audio at least, Funi/other English Audio is bonus
    -  Full subtitles are always best (avoid only "Signs Only", is nice to have though)
    -  h.264 Video (720p)
    -  AAC/mp4a Audio (48k sample rate)
    //*/

    // Media Player Controls
    float barx = X + PaddingLeft;
    float bary = Y + Height * 6 / 7;
    float barwidth = Width - PaddingLeft - PaddingRight - TextTimeRemainingWidth - PaddingRight / 2;
    G->SetBlendColor(Color::FromARGB(int(0x40 * Value_ShowControls), 0xFFFFFF));
    G->FillRectangle(barx, bary - 2.5, barwidth, 5.0);
    G->SetBlendColor(Color::FromHSLA(270.0, 0.00, 0.75, Value_ShowControls * 0.5));
    G->FillRectangle(barx, bary - 2.5, barwidth * Backend->GetBufferPosition() / Backend->GetDuration(), 5.0);
    G->SetBlendColor(Color::FromHSLA(270.0, 1.00, 0.50, Value_ShowControls));
    G->FillRectangle(barx, bary - 2.5, barwidth * Backend->GetPosition() / Backend->GetDuration(), 5.0);
    // G->SetBlendColor(Color::FromARGB(int(0xFF * Value_ShowControls), 0xFFFFFF));
    // G->FillCircle(barx + barwidth * Backend->GetPosition() / Backend->GetDuration(), bary, 7.5);

    TextTimeRemaining->Opacity = Value_ShowControls;
    TextTimeRemaining->X = X + Width - PaddingRight - TextTimeRemaining->Width;
    TextTimeRemaining->Y = bary - TextTimeRemaining->Height * 0.375;
    TextTimeRemaining->Render();

    double yesyesyes = std::sin(Value_ShowPauseIndicator * M_PI);
    G->SetBlendColor(Color::FromARGB(int(0x80 * yesyesyes), 0x000000));
    G->FillCircle(Width / 2, Height / 2, 32 + Value_ShowPauseIndicator * 32);
    G->SetBlendColor(Color::FromARGB(int(0x80 * Value_ShowPauseIndicator * yesyesyes), 0xFFFFFF));
    G->FillRectangle(Width / 2 - 24, Height / 2 - 32, 16, 64);
    G->FillRectangle(Width / 2 + 8, Height / 2 - 32, 16, 64);


    switch (Backend->GetPlayerState()) {
        case MediaPlayer::KIT_STOPPED:
            G->SetBlendColor(Color::FromARGB(0xFF, 0xFF0000));
            break;
        case MediaPlayer::KIT_PLAYING:
            G->SetBlendColor(Color::FromHSL(270.0, 1.00, 0.50));
            break;
        case MediaPlayer::KIT_PAUSED:
            G->SetBlendColor(Color::FromARGB(0xFF, 0xFFFFFF));
            break;
        case MediaPlayer::KIT_CLOSED:
            G->SetBlendColor(Color::FromARGB(0xFF, 0x000000));
            break;
    }
    // G->FillRectangle(X + Width - 25, Y, 25, 25);
}
PUBLIC void Video::Dispose() {
    BackendSource->Close();
    Backend->Close();

    TextMediaTitle->Dispose(); delete TextMediaTitle;
    TextTimeRemaining->Dispose(); delete TextTimeRemaining;
    TextHeaderAudio->Dispose(); delete TextHeaderAudio;
    TextHeaderSubtitles->Dispose(); delete TextHeaderSubtitles;
}
