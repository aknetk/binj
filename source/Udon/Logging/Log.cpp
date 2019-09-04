#if INTERFACE
class Log {
public:
    enum LogLevels {
        LOG_VERBOSE = -1,
        LOG_INFO = 0,
        LOG_WARN = 1,
        LOG_ERROR = 2,
        LOG_IMPORTANT = 3,
    };

    static int LogLevel;
};
#endif

#include <Udon/Standard.h>
#include <Udon/Logging/Log.h>

#ifdef ANDROID
    #include <android/log.h>
#endif

#include <stdarg.h>

int Log::LogLevel = -1;

PUBLIC STATIC void Log::SetLogLevel(int sev) {
    Log::LogLevel = sev;
}
PUBLIC STATIC void Log::Print(int sev, const char* format, ...) {
    if (sev < Log::LogLevel) return;

    int ColorCode = 0;
    char string[1000];
    bool WriteToFile = false;
    const char* severityText = NULL;

    va_list args;
    va_start(args, format);
    vsprintf(string, format, args);

    #if ANDROID
    switch (sev) {
        case -1: __android_log_print(ANDROID_LOG_VERBOSE, "IE3", "%s", string); return;
        case  0: __android_log_print(ANDROID_LOG_INFO,    "IE3", "%s", string); return;
        case  1: __android_log_print(ANDROID_LOG_WARN,    "IE3", "%s", string); return;
        case  2: __android_log_print(ANDROID_LOG_ERROR,   "IE3", "%s", string); return;
        case  3: __android_log_print(ANDROID_LOG_FATAL,   "IE3", "%s", string); return;
    }
    #endif

    #if WIN32 || SWITCH
    WriteToFile = false;
    #endif

    FILE* f = NULL;
    if (WriteToFile)
        f = fopen("Binj.log", "a");

    #if WIN32
    switch (sev) {
        case -1: ColorCode = 0xD; break;
        case  0: ColorCode = 0x8; break;
        case  1: ColorCode = 0xE; break;
        case  2: ColorCode = 0xC; break;
        case  3: ColorCode = 0xB; break;
    }
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
        WORD wColor = (csbi.wAttributes & 0xF0) + ColorCode;
        SetConsoleTextAttribute(hStdOut, wColor);
    }
    #elif MACOSX || LINUX
    switch (sev) {
        case -1: ColorCode = 95; break;
        case  0: ColorCode = 00; break;
        case  1: ColorCode = 93; break;
        case  2: ColorCode = 91; break;
        case  3: ColorCode = 96; break;
    }
    if (!WriteToFile)
        printf("\x1b[%d;1m", ColorCode);
    #endif

    switch (sev) {
        case -1: severityText = "  VERBOSE: "; break;
        case  0: severityText = "     INFO: "; break;
        case  1: severityText = "  WARNING: "; break;
        case  2: severityText = "    ERROR: "; break;
        case  3: severityText = "IMPORTANT: "; break;
    }

    printf("%s", severityText);
    if (WriteToFile && f)
        fprintf(f, "%s", severityText);

    #if WIN32
		WORD wColor = (csbi.wAttributes & 0xF0) | 0x07;
        SetConsoleTextAttribute(hStdOut, wColor);
    #elif MACOSX || LINUX
        if (!WriteToFile)
            printf("%s", "\x1b[0m");
    #endif

    printf("%s\n", string);
    fflush(stdout);

    if (WriteToFile && f) {
        fprintf(f, "%s\r\n", string);
        fclose(f);
    }
}
