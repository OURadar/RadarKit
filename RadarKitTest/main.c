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

    bool testModuloMath = false;
    bool testSIMD = true;

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

    if (testSIMD) {
        RKSIMDDemo(0);
    }
    

    RKSetProgramName("iRadar");
    rkGlobalParameters.stream = stdout;

    RKLog("Initializing ...\n");

    radar = RKInit();

    RKLog("Radar state machine occupies %s bytes\n", RKIntegerToCommaStyleString(radar->memoryUsage));

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);
//    signal(SIGKILL, handleSignals);
//    signal(SIGTERM, handleSignals);

    RKGoLive(radar);

    float phi = 0.0f;

    for (int i = 0; i < 40000 && radar->active; i++) {
        RKInt16Pulse *pulse = RKGetVacantPulse(radar);
        // Fill in the data...
        //
        //
        pulse->header.gateCount = 10000;
        pulse->header.i = i;

        for (int k = 0; k < 100; k++) {
            pulse->X[0][k].i = (int16_t)(32767.0f * cosf(phi * (float)k));
            pulse->X[0][k].q = (int16_t)(32767.0f * sinf(phi * (float)k));
        }

        phi += 0.02f;

        RKSetPulseReady(pulse);

        if (i % 1000 == 0) {
            printf("dat @ %5u  eng @ %5d : %5u  %5u  %5u  %5u  %5u  %5u  %5u  %5u  |  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f\n",
                   i % RKBuffer0SlotCount,
                   *radar->pulseCompressionEngine->index,
                   radar->pulseCompressionEngine->pid[0],
                   radar->pulseCompressionEngine->pid[1],
                   radar->pulseCompressionEngine->pid[2],
                   radar->pulseCompressionEngine->pid[3],
                   radar->pulseCompressionEngine->pid[4],
                   radar->pulseCompressionEngine->pid[5],
                   radar->pulseCompressionEngine->pid[6],
                   radar->pulseCompressionEngine->pid[7],
                   radar->pulseCompressionEngine->dutyCycle[0],
                   radar->pulseCompressionEngine->dutyCycle[1],
                   radar->pulseCompressionEngine->dutyCycle[2],
                   radar->pulseCompressionEngine->dutyCycle[3],
                   radar->pulseCompressionEngine->dutyCycle[4],
                   radar->pulseCompressionEngine->dutyCycle[5],
                   radar->pulseCompressionEngine->dutyCycle[6],
                   radar->pulseCompressionEngine->dutyCycle[7]
                   );
        }

        usleep(200);
    }

    sleep(2);
    
    RKStop(radar);
    
    RKLog("Freeing radar ...\n");
    RKFree(radar);
    
    return 0;
}
