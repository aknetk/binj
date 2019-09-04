#include <Udon/Application.h>

int main(int argc, char** argv) {
    #ifdef SWITCH
        socketInitializeDefault();
        nxlinkStdio();

        // normal: 1020000000
        // overclock: 1581000000
        // strong overclock: 1785000000

        pcvInitialize();
        // uint32_t clock_rate;
        // pcvGetClockRate(PcvModule module, &clock_rate);
        pcvSetClockRate(PcvModule_CpuBus, 1785000000);
    #endif

    Application::Run();

    #ifdef SWITCH
        pcvSetClockRate(PcvModule_CpuBus, 1020000000);
        // pcvSetClockRate(PcvModule_CpuBus, clock_rate);
        pcvExit();
        socketExit();
        exit(1);
    #endif
    return 0;
}
