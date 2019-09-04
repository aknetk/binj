#if INTERFACE
#include <Udon/Standard.h>

#include <ratio>
#include <chrono>

class Clock {
private:
    static std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
    static std::chrono::time_point<std::chrono::high_resolution_clock> GameStartTime;
};
#endif

#include <Udon/Logging/Log.h>
#include <Udon/Logging/Clock.h>

std::chrono::time_point<std::chrono::high_resolution_clock> Clock::StartTime;
std::chrono::time_point<std::chrono::high_resolution_clock> Clock::GameStartTime;

PUBLIC STATIC void   Clock::Init() {
    Clock::GameStartTime = std::chrono::high_resolution_clock::now();
}
PUBLIC STATIC void   Clock::Start() {
    Clock::StartTime = std::chrono::high_resolution_clock::now();
}
PUBLIC STATIC double Clock::GetTicks() {
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - Clock::GameStartTime);
    return time_span.count() * 1000.0;
}
PUBLIC STATIC double Clock::End() {
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - Clock::StartTime);
    return time_span.count() * 1000.0;
}
