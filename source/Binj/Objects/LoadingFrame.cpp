#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Objects/Node.h>
#include <Udon/Objects/Text.h>
#include <Udon/Rendering/Texture.h>

class LoadingFrame : public Node {
public:
    // Raw datas
    Texture* TextureLogo = NULL;
    Texture* TextureScreen = NULL;
    // Subnodes
    Text*    StatusText = NULL;
    // State variables
    int      InfoFetchState = -1;
    // Animation timers
    double   FadeInTimer = 0.0;
    double   FadeOutTimer = 0.0;
    double   CircleAnimationTimer = 0.0;
};
#endif

#include <Binj/Objects/LoadingFrame.h>
#include <Binj/Types/Media.h>
#include <Binj/Types/MediaLibrary.h>

#include <Udon/Application.h>
#include <Udon/Easing/Ease.h>
#include <Udon/Filesystem/Directory.h>
#include <Udon/Network/HTTP.h>

namespace InfoFetchStates { enum {
    WAIT_FOR_FADE_IN = -1,
    START_CONNECT,
    WAIT_FOR_CONNECT,
    WAIT_FOR_MEDIA_SCAN,
    START_MEDIA_INFO_FIND,
    WAIT_FOR_MEDIA_INFO_FIND,
    WRAP_UP,
    DONE,
};};

#ifndef WIN32
#define max std::max
#endif

bool CheckForID(char* str, int* id) {
    *id = 0;
    bool numeric = true;
    char* check = str;
    while (*check) {
        if (*check == '[') {
            check++;
            while (*check) {
                if (*check == ']') break;
                numeric &= (*check >= '0' && *check <= '9');
                if (numeric) *id = *id * 10 + (*check - '0');
                check++;
            }
            if (*check == 0) break;
        }
        check++;
    }
    return numeric;
}
bool MediaSortByID(Media* a, Media* b) {
    return a->ID < b->ID;
}
int  MediaInfoFind(void* op) {
    for (int i = 0; i < MediaLibrary::List.size(); i++) {
        MediaLibrary::List[i]->LoadInfo();
    }
    return 0;
}

PUBLIC STATIC Node* LoadingFrame::Create(const char* id) {
    LoadingFrame* node = new LoadingFrame;
    node->BackgroundColor = Color::FromHSL(270.0, 0.10, 0.15);
    node->ApplyCreate(id);

    node->TextureLogo = Texture::FromFilename("res/images/Logo.png");
    node->TextureScreen = Application::RendererPtr->CreateTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
        Application::RendererPtr->Width, Application::RendererPtr->Height);

    node->StatusText = (Text*)Text::Create(NULL);
    node->StatusText->Font = "Hind Regular";

    node->InfoFetchState = InfoFetchStates::WAIT_FOR_FADE_IN;

    node->FadeInTimer = 2.0;
    node->FadeOutTimer = 0.0;
    node->CircleAnimationTimer = 0.0;

    return node;
}

bool BusyBody = false;

PUBLIC STATIC void LoadingFrame::GetCallbackResolve(void* ptr, char* data, size_t len) {
    LoadingFrame* self = (LoadingFrame*)ptr;
    Log::Print(Log::LOG_WARN, "You are kinda weird chief...%zu bytes.", len);
    self->InfoFetchState = InfoFetchStates::WAIT_FOR_MEDIA_SCAN;
}
PUBLIC STATIC void LoadingFrame::GetCallbackReject(void* ptr, char* data, size_t len) {
    LoadingFrame* self = (LoadingFrame*)ptr;
    Log::Print(Log::LOG_ERROR, "Could not obtain GET request successfully...%zu bytes.", len);
    self->InfoFetchState = InfoFetchStates::WAIT_FOR_MEDIA_SCAN;
}

PUBLIC void LoadingFrame::Update() {
    Node::Update();

    /*
    // Order of loading things:
    1. Load userinfo from user database.
       IF program fails to get info
          USE local copy of userinfo (if none make one)
       ELSEIF userinfo Checksum is not equal to local copy
          UPLOAD local copy, and then USE it
       ELSE use downloaded info and copy it to local copy, and USE it
          (during regular program use, whenever userinfo changes, update to local, and UPLOAD local)
    2. Scan media directory for Media IDs
    3. Try to download the Media Info from Kitsu.io, and cache it
    //*/

    switch (InfoFetchState) {
        // Connecting to server...
        case InfoFetchStates::START_CONNECT: {
            StatusText->SetText("Connecting to server...");
            HTTP::GetAsync("https://aurum.vg", LoadingFrame::GetCallbackResolve, LoadingFrame::GetCallbackReject, this);
            InfoFetchState = InfoFetchStates::WAIT_FOR_CONNECT;
            break;
        }
        // Scanning media directory...
        case InfoFetchStates::WAIT_FOR_MEDIA_SCAN: {
            StatusText->SetText("Scanning media directory...");
            Media*        media;
            vector<char*> fileList;
            int           ide = 0, start = 0;
            const char*   mediaFolder = "/Users/justin/Dropbox/STREAM/res/media";
            if (Application::Platform == Platforms::Switch)
                mediaFolder = "/media";

            // Find movies
            Directory::GetFiles(&fileList, mediaFolder, "*[*].mkv", false);
            Directory::GetFiles(&fileList, mediaFolder, "*[*].mp4", false);
            for (int i = start; i < fileList.size(); i++) {
                if (CheckForID(fileList[i] + strlen(mediaFolder) + 1, &ide)) {
                    media = new Media();
                    media->SetFilename(fileList[i]);
                    media->IsSeries = false;
                    media->ID = ide;
                    MediaLibrary::List.push_back(media);
                }
            }

            // Find series
            start = fileList.size();
            Directory::GetDirectories(&fileList, mediaFolder, "*[*]", false);
            for (int i = start; i < fileList.size(); i++) {
                if (CheckForID(fileList[i] + strlen(mediaFolder) + 1, &ide)) {
                    media = new Media();
                    media->SetFilename(fileList[i]);
                    media->IsSeries = true;
                    media->ID = ide;
                    MediaLibrary::List.push_back(media);
                }
            }

            std::sort(MediaLibrary::List.begin(), MediaLibrary::List.end(), MediaSortByID);
            for (int i = 0; i < MediaLibrary::List.size(); i++) {
                // Log::Print(Log::LOG_IMPORTANT, "%*d: %s", 5, MediaLibrary::List[i]->ID, MediaLibrary::List[i]->Filename);
            }

            InfoFetchState = InfoFetchStates::START_MEDIA_INFO_FIND;
            break;
        }
        // Finding media info...
        case InfoFetchStates::START_MEDIA_INFO_FIND: {
            StatusText->SetText("Finding media info...");
            SDL_DetachThread(SDL_CreateThread(MediaInfoFind, "MediaInfoFind", this));
            InfoFetchState = InfoFetchStates::WAIT_FOR_MEDIA_INFO_FIND;
            break;
        }
        case InfoFetchStates::WAIT_FOR_MEDIA_INFO_FIND: {
            bool checker = true;
            for (int i = 0; i < MediaLibrary::List.size(); i++) {
                checker &= MediaLibrary::List[i]->RequestStatus > Media::LOADSTATUS_STARTED_REQUEST;
                if (!checker) break;
            }
            if (checker)
                InfoFetchState = InfoFetchStates::WRAP_UP;
            break;
        }
        case InfoFetchStates::WRAP_UP: {
            if (FadeInTimer > 0.0) break;

            Application::NodeIDList->Get("MainFrame")->Display = Display::BLOCK;

            for (int i = 0; i < MediaLibrary::List.size(); i++) {
                Log::Print(Log::LOG_IMPORTANT, "%*d: %s", 5, MediaLibrary::List[i]->ID, MediaLibrary::List[i]->Title);
            }

            StatusText->SetText(" ");
            InfoFetchState = InfoFetchStates::DONE;
            FadeOutTimer = 0.75;
            break;
        }
    }

    CircleAnimationTimer -= 1.5 / 60.0;

    if (InfoFetchState == InfoFetchStates::WAIT_FOR_FADE_IN)
        InfoFetchState = InfoFetchStates::START_CONNECT;

    if (FadeInTimer > 0.0) {
        FadeInTimer -= 1.0 / 60.0;
        if (FadeInTimer <= 0.0) {
            FadeInTimer = 0.0;
        }
        Opacity = Ease::InOutQuad(1.0 - FadeInTimer * 1.0 / 2.0);
    }
    if (FadeOutTimer > 0.0) {
        FadeOutTimer -= 1.0 / 60.0;
        if (FadeOutTimer <= 0.0) {
            FadeOutTimer = 0.0;
        }
        Opacity = Ease::InOutQuad(FadeOutTimer * 1.0 / 0.75);
    }
}
PUBLIC void LoadingFrame::Render() {
    // Below
    if (Opacity == 1.0)
        G->SetRenderTarget(NULL);
    else
        G->SetRenderTarget(TextureScreen);

    G->SetBlendColor(Color::FromHSL(270.0, 0.10, 0.15));
    G->FillRectangle(0, 0, G->Width, G->Height);

    float textureLogoWidth = TextureLogo->Width * 0.5;
    float textureLogoHeight = TextureLogo->Height * 0.5;
    float circleRadius = 9.0;
    float centerHeight = textureLogoHeight + 20.0 + circleRadius * 2.0;

    G->SetBlendColor(1.0, 1.0, 1.0, 1.0);

    int parts = 8;
    double cosV, sinV, PI_PART = 2 * M_PI / parts;
    for (int i = 0; i < parts; i++) {
        cosV = std::cos(CircleAnimationTimer + PI_PART * i);
        sinV = std::sin(CircleAnimationTimer + PI_PART * i);
        G->FillCircle(
            G->Width / 2.0 + (cosV - sinV) * circleRadius * 3.0,
            G->Height / 2.0 - centerHeight / 2.0 + textureLogoHeight + 20.0 + circleRadius,
            max((sinV + cosV) * 0.707 * circleRadius, 0.0));
    }
    G->DrawTexture(TextureLogo, G->Width / 2.0 - textureLogoWidth / 2.0, G->Height / 2.0 - centerHeight / 2.0, textureLogoWidth, textureLogoHeight);

    // StatusText->X = G->Width / 2.0 - StatusText->Width / 2.0;
    // StatusText->Y = G->Height * 0.875 - StatusText->Height * 0.375;
    // StatusText->Render();

    Node::Render();
    // Above

    if (Opacity < 1.0) {
        G->SetRenderTarget(NULL);
        G->SetBlendColor(1.0, 1.0, 1.0, Opacity);
        G->DrawTexture(TextureScreen, 0, 0);
    }
}
PUBLIC void LoadingFrame::Dispose() {
    G->DisposeTexture(TextureLogo);
    G->DisposeTexture(TextureScreen);

    StatusText->Dispose();
    delete StatusText;

    for (Uint32 i = 0, iSz = Children.size(); i < iSz; i++) {
        Children[i]->Dispose();
        delete Children[i];
    }
    Children.clear();
}
PUBLIC LoadingFrame::~LoadingFrame() {

}
