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

typedef int Flag;
enum Flag {
    FlagNone        = 0,
    FlagShowResults = 1
};

void pulseCompressionTest(Flag flag) {
    RKPulse *pulse = RKGetVacantPulse(radar);
    pulse->header.gateCount = 6;

    RKInt16 *X = radar->pulses[0].X[0];
    memset(X, 0, pulse->header.gateCount * sizeof(RKComplex));
    pulse->X[0][0].i = 1;
    pulse->X[0][0].q = 0;
    pulse->X[0][1].i = 2;
    pulse->X[0][1].q = 0;
    RKSetPulseReady(pulse);

    while ((pulse->header.s & RKPulseStatusCompressed) == 0) {
        usleep(1000);
    }

    RKComplex *F = &radar->pulseCompressionEngine->filters[0][0][0];
    RKComplex *Y = radar->pulses[0].Y[0];

    if (flag & FlagShowResults) {
        printf("X =                    F =                    Y =\n");
        for (int k = 0; k < 8; k++) {
            printf("  %5d%+5di            %9.2f%+9.2fi            %9.2f%+9.2fi\n", X[k].i, X[k].q, F[k].i, F[k].q, Y[k].i, Y[k].q);
        }
    }
}

int main(int argc, const char * argv[]) {

    int k = 0;
    bool testModuloMath = false;
    bool testSIMD = false;

    if (testModuloMath) {
        const int N = 4;

        RKLog("Test with SlotCount = %d\n", RKBuffer0SlotCount);
        k = 0;                      RKLog("k = %3d --> Next N = %3d\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
        k = RKBuffer0SlotCount - 4; RKLog("k = %3d --> Next N = %3d\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
        k = RKBuffer0SlotCount - 3; RKLog("k = %3d --> Next N = %3d\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
        k = RKBuffer0SlotCount - 2; RKLog("k = %3d --> Next N = %3d\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
        k = RKBuffer0SlotCount - 1; RKLog("k = %3d --> Next N = %3d\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
        k = RKBuffer0SlotCount - 1; RKLog("k = %3d --> Prev N = %3d\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
        k = 0;                      RKLog("k = %3d --> Prev N = %3d\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
        k = 1;                      RKLog("k = %3d --> Prev N = %3d\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
        k = 2;                      RKLog("k = %3d --> Prev N = %3d\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
        k = 3;                      RKLog("k = %3d --> Prev N = %3d\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
        k = 4;                      RKLog("k = %3d --> Prev N = %3d\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    }

    if (testSIMD) {
        RKSIMDDemo(RKSIMDDemoFlagPerformanceTest);
    }

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);
    //    signal(SIGKILL, handleSignals);
    //    signal(SIGTERM, handleSignals);


    RKSetProgramName("iRadar");
    rkGlobalParameters.stream = stdout;

    RKLog("Initializing ...\n");

    radar = RKInit();

    RKLog("Radar state machine occupies %s B (%s GiB)\n", RKIntegerToCommaStyleString(radar->memoryUsage), RKFloatToCommaStyleString(1.0e-9f * radar->memoryUsage));

    RKGoLive(radar);

//    float phi = 0.0f;
//
//    for (int i = 0; i < 40000 && radar->active; i++) {
//        RKPulse *pulse = RKGetVacantPulse(radar);
//        // Fill in the data...
//        //
//        //
//        pulse->header.gateCount = 10000;
//
//        for (k = 0; k < 100; k++) {
//            pulse->X[0][k].i = (int16_t)(32767.0f * cosf(phi * (float)k));
//            pulse->X[0][k].q = (int16_t)(32767.0f * sinf(phi * (float)k));
//        }
//
//        phi += 0.02f;
//
//        RKSetPulseReady(pulse);
//
//        if (i % 1000 == 0) {
//            printf("dat @ %5u  eng @ %5d : %5u  %5u  %5u  %5u  %5u  %5u  %5u  %5u  |  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f  %.2f\n",
//                   i % RKBuffer0SlotCount,
//                   *radar->pulseCompressionEngine->index,
//                   radar->pulseCompressionEngine->pid[0],
//                   radar->pulseCompressionEngine->pid[1],
//                   radar->pulseCompressionEngine->pid[2],
//                   radar->pulseCompressionEngine->pid[3],
//                   radar->pulseCompressionEngine->pid[4],
//                   radar->pulseCompressionEngine->pid[5],
//                   radar->pulseCompressionEngine->pid[6],
//                   radar->pulseCompressionEngine->pid[7],
//                   radar->pulseCompressionEngine->dutyCycle[0],
//                   radar->pulseCompressionEngine->dutyCycle[1],
//                   radar->pulseCompressionEngine->dutyCycle[2],
//                   radar->pulseCompressionEngine->dutyCycle[3],
//                   radar->pulseCompressionEngine->dutyCycle[4],
//                   radar->pulseCompressionEngine->dutyCycle[5],
//                   radar->pulseCompressionEngine->dutyCycle[6],
//                   radar->pulseCompressionEngine->dutyCycle[7]
//                   );
//        }
//
//        usleep(200);
//    }

    pulseCompressionTest(FlagShowResults);

    RKStop(radar);
    
    RKLog("Freeing radar ...\n");
    RKFree(radar);

    return 0;
}
