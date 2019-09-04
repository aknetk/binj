#if INTERFACE
#include <Udon/Standard.h>
#include <Udon/Font/FontFace.h>
#include <Udon/Objects/Node.h>
#include <Udon/Rendering/Renderer.h>
#include <Udon/Types/HashMap.h>

class Application {
public:
    static SDL_Window*         Window;
    static Renderer*           RendererPtr;
    static bool                Running;
    static int                 FPS;
    static Node*               Document;
    static HashMap<FontFace*>* Fonts;
    static HashMap<Node*>*     NodeIDList;
    static Platforms           Platform;
};
#endif

#include <Udon/Application.h>

#include <Udon/Audio/Audio.h>
#include <Udon/Logging/Clock.h>
#include <Udon/Logging/Log.h>
#include <Udon/Network/HTTP.h>
#include <Udon/Objects/Image.h>
#include <Udon/Objects/Video.h>
#include <Udon/Rendering/OpenGLRenderer.h>
#include <Udon/Rendering/Texture.h>

#include <Binj/Objects/LoadingFrame.h>
#include <Binj/Objects/MainFrame.h>

SDL_Window*         Application::Window = NULL;
Renderer*           Application::RendererPtr = NULL;
bool                Application::Running = false;
int                 Application::FPS = 60;
Node*               Application::Document = NULL;
HashMap<Node*>*     Application::NodeIDList = NULL;
HashMap<FontFace*>* Application::Fonts = NULL;

#if   WIN32
    Platforms Application::Platform = Platforms::Windows;
#elif MACOSX
    Platforms Application::Platform = Platforms::MacOSX;
#elif LINUX
    Platforms Application::Platform = Platforms::Linux;
#elif UBUNTU
    Platforms Application::Platform = Platforms::Ubuntu;
#elif SWITCH
    Platforms Application::Platform = Platforms::Switch;
#else
    Platforms Application::Platform = Platforms::Default;
#endif

PUBLIC STATIC void Application::Run() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
        Log::Print(Log::LOG_ERROR, "SDL_NOINIT: %s", SDL_GetError());

    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) < 0)
        Log::Print(Log::LOG_ERROR, "SDL_IMAGE_NOINIT: %s", IMG_GetError());

    if (TTF_Init() < 0)
        Log::Print(Log::LOG_ERROR, "SDL_TTF_NOINIT: %s", TTF_GetError());

    SDL_SetEventFilter(Application::EventFilter, NULL);

    Audio::Init();
    Clock::Init();
    // HTTP::CheckInit();

    Application::RendererPtr = new OpenGLRenderer();
    Application::Fonts = new HashMap<FontFace*>(NULL, 16);

    /// Load fonts
    Application::Fonts->Put("Hind Light", FontFace::Open("res/fonts/Hind-Light.ttf"));
    Application::Fonts->Put("Hind Medium", FontFace::Open("res/fonts/Hind-Medium.ttf"));
    Application::Fonts->Put("Hind Regular", FontFace::Open("res/fonts/Hind-Regular.ttf"));
    Application::Fonts->Put("Hind Semibold", FontFace::Open("res/fonts/Hind-SemiBold.ttf"));
    Application::Fonts->Put("Hind Bold", FontFace::Open("res/fonts/Hind-Bold.ttf"));
    Application::Fonts->Put("Helvetica", FontFace::Open("res/fonts/Helvetica.ttf"));

    Renderer* G = Application::RendererPtr;
    Application::Document = Node::Create();
    Application::Document->BackgroundColor = Color::FromARGB(0xFF, 0x000000); // Color::FromHSL(270.0, 0.10, 0.15);
    Application::Document->Width = G->Width;
    Application::Document->Height = G->Height;

    Application::NodeIDList = new HashMap<Node*>(NULL, 16);

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        SDL_JoystickOpen(i);
    }

    Application::Document->Add(MainFrame::Create("MainFrame"));
    Application::Document->Add(LoadingFrame::Create(NULL));

    Application::Document->Update();

    // Texture* textureScreen = G->CreateTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, G->Width, G->Height);
    // Texture* textureScreenBlurred = G->CreateTexture(SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, G->Width, G->Height);

    Uint32 benchmarkFrameCount = 0;
    Uint32 beginFrameBatch = SDL_GetTicks();
    double FPS = 60.0;

    Application::Running = true;
    while (Application::Running) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    Quit();
                    break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE)
                        Quit();
                    break;
            }
            Application::Document->OnEvent(e);
        }

        if (benchmarkFrameCount == 0)
            beginFrameBatch = SDL_GetTicks();

        Uint32 frameTimeStart = SDL_GetTicks();

        G->Clear();

        Application::Document->Update();
        Application::Document->Render();

        G->Present();

        frameTimeStart = SDL_GetTicks() - frameTimeStart;
        if (benchmarkFrameCount < 40) {
            if (frameTimeStart < 16) SDL_Delay(16 - frameTimeStart);
        }
        else {
            if (frameTimeStart < 15) SDL_Delay(15 - frameTimeStart);
        }

        benchmarkFrameCount++;
        if (benchmarkFrameCount == 60) {
            benchmarkFrameCount = 0;

            Uint32 now = SDL_GetTicks();
            FPS = 1000.0 / (now - beginFrameBatch) * 60.0;
        }

        char yup[20];
        // sprintf(yup, "Binj");
        sprintf(yup, "FPS: %.2f", FPS);
        SDL_SetWindowTitle(Application::Window, yup);
    }

    Application::RendererPtr->Dispose();

    Application::Fonts->WithAll([](Uint32 hash, FontFace* self) -> void {
        self->Dispose();
    });

    Application::Document->Dispose();

    Audio::Dispose();

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

PRIVATE STATIC int Application::EventFilter(void* data, SDL_Event* event) {
    #ifndef SWITCH
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        int w = event->window.data1;
        int h = event->window.data2;

        Application::Document->Width = w;
        Application::Document->Height = h;
        Application::RendererPtr->Width = w;
        Application::RendererPtr->Height = h;

        Application::RendererPtr->Clear();
        Application::Document->Update();
        Application::Document->Render();
        Application::RendererPtr->Present();
    }
    #endif
    return 1;
}

PUBLIC STATIC void Application::Quit() {
    Application::Running = false;
}
