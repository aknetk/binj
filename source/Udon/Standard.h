#ifndef STANDARD_H
#define STANDARD_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fstream>
#include <iostream>

#include <unordered_map>
#include <functional>
#include <algorithm>
#include <vector>
#include <deque>

#if   WIN32
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_events.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_mixer.h>
    #include <SDL2/SDL_ttf.h>
    #include <SDL2/SDL_syswm.h>

    #define GL3_PROTOTYPES 1
    #include <GL/glew.h>

    #include <SDL2/SDL_opengl.h>

    #include <windows.h>
#elif MACOSX
    #include <SDL.h>
    #include <SDL_image.h>
    #include <SDL_mixer.h>
    #include <SDL_ttf.h>
    // #include <SDL_syswm.h>

    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
#elif SWITCH
    #include <switch.h>

    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #include <SDL2/SDL_mixer.h>
    #include <SDL2/SDL_ttf.h>
    #include <SDL2/SDL_syswm.h>

    #include <GLES2/gl2.h>
    #include <SDL2/SDL_opengl.h>
    #include <GLES2/gl2ext.h>
#endif

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323
#endif

enum class Platforms {
    Default,
    Windows,
    MacOSX,
    Linux,
    Ubuntu,
    Switch
};

#endif // STANDARD_H
