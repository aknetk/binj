#ifndef LOG_H
#define LOG_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL



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

    static void SetLogLevel(int sev);
    static void Print(int sev, const char* format, ...);
};

#endif /* LOG_H */
