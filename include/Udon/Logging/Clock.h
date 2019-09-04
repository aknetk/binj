#ifndef CLOCK_H
#define CLOCK_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>
#include <ratio>
#include <chrono>

class Clock {
private:
    static std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
    static std::chrono::time_point<std::chrono::high_resolution_clock> GameStartTime;


public:
    static void   Init();
    static void   Start();
    static double GetTicks();
    static double End();
};

#endif /* CLOCK_H */
