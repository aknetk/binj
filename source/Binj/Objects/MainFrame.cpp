#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Objects/Node.h>
#include <Udon/Objects/Text.h>
#include <Udon/Rendering/Texture.h>

class MainFrame : public Node {
public:
    // Raw datas
    Texture* TextureLogo = NULL;
    // Subnodes
    Text*    TextHeaders[5];
    // State variables
    double   ViewY = 0.0f;
    int      CurrentLibrarySelectionIndex = 0;
    int      PreviousLibrarySelectionIndex = 0;
    // Animation timers
    double   LibrarySelectionFadeTimer = 1.0;
};
#endif

#include <Binj/Objects/MainFrame.h>
#include <Binj/Types/MediaLibrary.h>

#include <Udon/Application.h>
#include <Udon/Filesystem/Directory.h>
#include <Udon/Network/HTTP.h>
#include <Udon/Objects/Video.h>

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

PUBLIC STATIC Node* MainFrame::Create(const char* id) {
    MainFrame* node = new MainFrame;
    node->BackgroundColor = Color::FromHSL(270.0, 0.10, 0.15);
    node->ApplyCreate(id);

    node->Display = Display::NONE;
    node->TextureLogo = Texture::FromFilename("res/images/Logo2x.png");

    node->TextHeaders[0] = (Text*)Text::Create(NULL);
    node->TextHeaders[1] = (Text*)Text::Create(NULL);
    node->TextHeaders[2] = (Text*)Text::Create(NULL);
    node->TextHeaders[3] = (Text*)Text::Create(NULL);
    node->TextHeaders[4] = (Text*)Text::Create(NULL);

    node->TextHeaders[0]->Font =
    node->TextHeaders[1]->Font =
    node->TextHeaders[2]->Font =
    node->TextHeaders[3]->Font = "Hind Regular";

    node->TextHeaders[0]->FontSize =
    node->TextHeaders[1]->FontSize =
    node->TextHeaders[2]->FontSize =
    node->TextHeaders[3]->FontSize = 18;

    if (Application::Platform == Platforms::Switch) {
        node->TextHeaders[0]->FontSize =
        node->TextHeaders[1]->FontSize =
        node->TextHeaders[2]->FontSize =
        node->TextHeaders[3]->FontSize = 24;
    }

    node->TextHeaders[0]->SetText("Home");
    node->TextHeaders[1]->SetText("Series");
    node->TextHeaders[2]->SetText("Movies");
    // node->TextHeaders[3]->SetText("Manga");
    node->TextHeaders[3]->SetText("My Queue");

    return node;
}

PUBLIC void MainFrame::OnEvent(SDL_Event e) {
    switch (e.type) {
        case SDL_KEYDOWN:
            // Left
            if (e.key.keysym.sym == SDLK_LEFT) {
                MoveCursor(-1, 0);
            }
            // Right
            else if (e.key.keysym.sym == SDLK_RIGHT) {
                MoveCursor(1, 0);
            }
            // Up
            else if (e.key.keysym.sym == SDLK_UP) {
                MoveCursor(0, -1);
            }
            // Down
            else if (e.key.keysym.sym == SDLK_DOWN) {
                MoveCursor(0, 1);
            }
            else if (e.key.keysym.sym == SDLK_SPACE) {
                StartVideo();
            }
            break;
        case SDL_JOYAXISMOTION:
            // 0: L-Stick X
            // 1: L-Stick Y
            // 2: R-Stick X
            // 3: R-Stick Y
            break;
        case SDL_JOYBUTTONDOWN:
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

            // Left
            if (e.jbutton.button == 12) {
                MoveCursor(-1, 0);
            }
            // Right
            else if (e.jbutton.button == 14) {
                MoveCursor(1, 0);
            }
            // Up
            else if (e.jbutton.button == 13) {
                MoveCursor(0, -1);
            }
            // Down
            else if (e.jbutton.button == 15) {
                MoveCursor(0, 1);
            }
            else if (e.jbutton.button == 1) {
                StartVideo();
            }
            break;
    }
}

PUBLIC void MainFrame::MoveCursor(int x, int y) {
    int columns = 7;
    PreviousLibrarySelectionIndex = CurrentLibrarySelectionIndex;

    if (x < 0) {
        if ((CurrentLibrarySelectionIndex % columns) > 0) {
            CurrentLibrarySelectionIndex -= 1;
        }
    }
    else if (x > 0) {
        if ((CurrentLibrarySelectionIndex % columns) < (columns - 1)) {
            CurrentLibrarySelectionIndex += 1;
        }
    }

    if (y < 0) {
        if ((CurrentLibrarySelectionIndex / columns) >= 0) {
            CurrentLibrarySelectionIndex -= columns;
        }
    }
    else if (y > 0) {
        if ((CurrentLibrarySelectionIndex / columns) < ((MediaLibrary::List.size() + (columns - 1)) / columns)) {
            CurrentLibrarySelectionIndex += columns;
        }
    }
    CheckRange();
}
PUBLIC void MainFrame::CheckRange() {
    if (CurrentLibrarySelectionIndex < 0)
        CurrentLibrarySelectionIndex = 0;
    if (CurrentLibrarySelectionIndex >= MediaLibrary::List.size())
        CurrentLibrarySelectionIndex =  MediaLibrary::List.size() - 1;

    if (PreviousLibrarySelectionIndex == CurrentLibrarySelectionIndex)
        LibrarySelectionFadeTimer = 1.0;
    else
        LibrarySelectionFadeTimer = 0.0;
}
PUBLIC void MainFrame::StartVideo() {
    Video* vid = (Video*)Video::Create("testVid");
    vid->Display = Display::BLOCK;
    vid->DefaultWidth = 16;
    vid->DefaultHeight = 9;
    vid->WidthValue = Value { ValueType::PERCENT, 1.000f };
    vid->HeightValue = Value { ValueType::PERCENT, 1.000f };
    vid->TextMediaTitle->SetText(MediaLibrary::List[CurrentLibrarySelectionIndex]->Title);

    if (MediaLibrary::List[CurrentLibrarySelectionIndex]->IsSeries) {
        vector<char*> files = Directory::GetFiles(MediaLibrary::List[CurrentLibrarySelectionIndex]->Filename, "*.*", false);
        vid->Source = files[0];
        for (int i = 1; i < files.size(); i++) {
            free(files[i]);
        }
    }
    else {
        vid->Source = MediaLibrary::List[CurrentLibrarySelectionIndex]->Filename;
    }

    Log::Print(Log::LOG_INFO, "Loading media from path: %s", vid->Source);
    Application::Document->Add(vid);
}

PUBLIC void MainFrame::Update() {
    // Levenshtein Distance Algorithm would be useful for title searching in case of mispellings
    for (int i = 0; i < MediaLibrary::List.size(); i++) {
        if (MediaLibrary::List[i]->Thumbnail)
            MediaLibrary::List[i]->Thumbnail->Update();
    }

    if (LibrarySelectionFadeTimer < 1.0) {
        LibrarySelectionFadeTimer += 1.0 / 60.0 / 0.125;
        if (LibrarySelectionFadeTimer >= 1.0)
            LibrarySelectionFadeTimer = 1.0;
    }
}
PUBLIC void MainFrame::Render() {
    for (int i = 0; i < MediaLibrary::List.size(); i++) {
        if (MediaLibrary::List[i]->Thumbnail)
            MediaLibrary::List[i]->Thumbnail->Render();
    }
    // Below
    G->SetBlendColor(Color::FromHSL(270.0, 0.10, 0.15));
    G->FillRectangle(0, 0, G->Width, G->Height);

    int margin = 10;
    if (Application::Platform == Platforms::Switch)
        margin = 15;
    int marginLeft = margin * 2;
    double headerHeight = G->Height / 6.75;
    double headerHeightPadded = headerHeight - margin * 2;

    // G->SetBlendColor(Color::FromHSL(270.0, 0.10, 0.05));
    // G->FillRectangle(0, 0, G->Width, headerHeight);

    G->SetBlendColor(1.0, 1.0, 1.0, 1.0);
    G->DrawTexture(TextureLogo, marginLeft, margin, TextureLogo->Width * headerHeightPadded / TextureLogo->Height, headerHeightPadded);

    TextHeaders[0]->X = marginLeft + TextureLogo->Width * headerHeight / TextureLogo->Height + 20;
    TextHeaders[0]->Y = (headerHeight - TextHeaders[0]->Height / 2) / 2;
    TextHeaders[0]->Render();

    TextHeaders[1]->X = TextHeaders[0]->X + TextHeaders[0]->Width + 20;
    TextHeaders[1]->Y = (headerHeight - TextHeaders[1]->Height / 2) / 2;
    TextHeaders[1]->Render();

    TextHeaders[2]->X = TextHeaders[1]->X + TextHeaders[1]->Width + 20;
    TextHeaders[2]->Y = (headerHeight - TextHeaders[2]->Height / 2) / 2;
    TextHeaders[2]->Render();

    TextHeaders[3]->X = TextHeaders[2]->X + TextHeaders[2]->Width + 20;
    TextHeaders[3]->Y = (headerHeight - TextHeaders[3]->Height / 2) / 2;
    TextHeaders[3]->Render();

    G->SetBlendColor(1.0, 1.0, 1.0, 1.0);
    headerHeightPadded = headerHeight - margin * 2 * 2;
    G->FillEllipse(G->Width - headerHeightPadded - margin * 2, margin * 2, headerHeightPadded, headerHeightPadded);

    int    columns = 7; // 6 is good for handheld mode, 7 for TV screen and Retina 540p, 8+ for Retina fullscreen
    double thumbWidthSpace = (G->Width - margin * 2.0) / columns;
    double thumbWidth = thumbWidthSpace - margin * 2.0;
    double thumbHeight = thumbWidth / 0.7;
    double thumbHeightSpace = thumbHeight + margin * 2.0;
    for (int i = 0; i < MediaLibrary::List.size(); i++) {
        double x = (i % columns) * thumbWidthSpace + margin + margin;
        double y = (i / columns) * thumbHeightSpace + margin + margin + headerHeight;

        if (i == CurrentLibrarySelectionIndex || i == PreviousLibrarySelectionIndex) {
            if (i == CurrentLibrarySelectionIndex)
                G->SetBlendColor(Color::FromHSLA(270.0, 1.0, 0.5, LibrarySelectionFadeTimer));
            else
                G->SetBlendColor(Color::FromHSLA(270.0, 1.0, 0.5, 1.0 - LibrarySelectionFadeTimer));
            G->FillRectangle(x - margin/2, y - margin/2, thumbWidth + margin, thumbHeight + margin);
        }

        if (MediaLibrary::List[i]->Thumbnail->TexturePtr) {
            G->SetBlendColor(1.0, 1.0, 1.0, 1.0);
            if (i == CurrentLibrarySelectionIndex)
                G->DrawTextureBlurred(MediaLibrary::List[i]->Thumbnail->TexturePtr, x, y, thumbWidth, thumbHeight);
            else
                G->DrawTexture(MediaLibrary::List[i]->Thumbnail->TexturePtr, x, y, thumbWidth, thumbHeight);
        }
        else {
            G->SetBlendColor(0.6, 0.6, 0.6, 1.0);
            G->FillRectangle(x, y, thumbWidth, thumbHeight);
            G->SetBlendColor(Color::FromHSL(270.0, 0.5, 0.45));
            G->FillRectangle(x + thumbWidth / 4, y + (thumbHeight - thumbWidth / 2) / 2, thumbWidth / 2, thumbWidth / 2);
        }
    }
    // Above
}
PUBLIC MainFrame::~MainFrame() {

}
