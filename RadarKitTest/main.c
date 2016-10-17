//
//  main.c
//  RadarKitTest
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit.h>
#include <signal.h>

RKRadar *radar;

void *exitAfterAWhile(void *s) {
    sleep(1);
    RKLog("Forced exit.\n");
    exit(EXIT_SUCCESS);
}

static void handleSignals(int signal) {
    RKLog("User interrupt SIG=%d.\n", signal);
    RKStop(radar);
    pthread_t t;
    pthread_create(&t, NULL, exitAfterAWhile, NULL);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    radar = RKInit();
    RKSetProgramName("iRadar");
    rkGlobalParameters.stream = stdout;

    RKLog("Radar state machine occupies %s bytes\n", RKIntegerToCommaStyleString(radar->memoryUsage));

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);
//    signal(SIGKILL, handleSignals);
//    signal(SIGTERM, handleSignals);

    bool testModuloMath = false;
    
    if (testModuloMath) {
        const int N = 4;
        int i = 0;
        
        RKLog("Test with SlotCount = %d\n", RKBuffer0SlotCount);
        i = 0;                      RKLog("i = %3d --> Next N = %3d\n", i, RKNextNModuloS(i, N, RKBuffer0SlotCount));
        i = RKBuffer0SlotCount - 4; RKLog("i = %3d --> Next N = %3d\n", i, RKNextNModuloS(i, N, RKBuffer0SlotCount));
        i = RKBuffer0SlotCount - 3; RKLog("i = %3d --> Next N = %3d\n", i, RKNextNModuloS(i, N, RKBuffer0SlotCount));
        i = RKBuffer0SlotCount - 2; RKLog("i = %3d --> Next N = %3d\n", i, RKNextNModuloS(i, N, RKBuffer0SlotCount));
        i = RKBuffer0SlotCount - 1; RKLog("i = %3d --> Next N = %3d\n", i, RKNextNModuloS(i, N, RKBuffer0SlotCount));
        i = RKBuffer0SlotCount - 1; RKLog("i = %3d --> Prev N = %3d\n", i, RKPreviousNModuloS(i, N, RKBuffer0SlotCount));
        i = 0;                      RKLog("i = %3d --> Prev N = %3d\n", i, RKPreviousNModuloS(i, N, RKBuffer0SlotCount));
        i = 1;                      RKLog("i = %3d --> Prev N = %3d\n", i, RKPreviousNModuloS(i, N, RKBuffer0SlotCount));
        i = 2;                      RKLog("i = %3d --> Prev N = %3d\n", i, RKPreviousNModuloS(i, N, RKBuffer0SlotCount));
        i = 3;                      RKLog("i = %3d --> Prev N = %3d\n", i, RKPreviousNModuloS(i, N, RKBuffer0SlotCount));
        i = 4;                      RKLog("i = %3d --> Prev N = %3d\n", i, RKPreviousNModuloS(i, N, RKBuffer0SlotCount));
    }

    radar->pulseCompressionEngine->coreCount = 5;
    
    RKGoLive(radar);

    float phi = 0.0f;

    for (int i = 0; i < 50 && radar->active; i++) {
        RKInt16Pulse *pulse = RKGetVacantPulse(radar);
        // Fill in the data...
        //
        //
        pulse->header.gateCount = 2000;
        pulse->header.i = i;

        for (int k = 0; k < 100; k++) {
            pulse->X[0][k].i = (int16_t)(32767.0f * cosf(phi * (float)k));
            pulse->X[0][k].q = (int16_t)(32767.0f * sinf(phi * (float)k));
        }

        phi += 0.02f;

        RKSetPulseReady(pulse);
        usleep(50000);
    }

    RKStop(radar);
    
    RKLog("Freeing radar ...\n");
    RKFree(radar);
    
    return 0;
}
