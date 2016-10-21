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
        printf("X =                    F =                           Y =\n");
        for (int k = 0; k < 8; k++) {
            printf("    %6d %s %6di         %9.2f %s %9.2fi         %9.2f %s %9.2fi\n",
                   X[k].i, X[k].q < 0 ? "-" : "+", abs(X[k].q),
                   F[k].i, F[k].q < 0.0f ? "-" : "+", fabs(F[k].q),
                   Y[k].i, Y[k].q < 0.0f ? "-" : "+", fabs(Y[k].q));
        }
    }
}

void simulateDataStream(void) {
    int k;
    float phi = 0.0f;
    struct timeval t0, t1;
    double dt = 0.0;
    const int chunkSize = 500;

    int g = 0;

    gettimeofday(&t0, NULL);

    RKSetLogfile(NULL);

    //const int gateCount = (int)(75.0e3f / 3.0f);

    float bw = 50.0e6;
    const int gateCount = (int)(75.0e3 / 3.0e8 * bw * 2.0);

    RKLog("Using gate count %s\n", RKIntegerToCommaStyleString(gateCount));

    for (int i = 0; i < 10000000 && radar->active; i += chunkSize) {

        for (int j = 0; j < chunkSize; j++) {
            RKPulse *pulse = RKGetVacantPulse(radar);
            // Fill in the data...
            pulse->header.gateCount = gateCount;
            for (k = 0; k < 1000; k++) {
                pulse->X[0][k].i = (int16_t)(32767.0f * cosf(phi * (float)k));
                pulse->X[0][k].q = (int16_t)(32767.0f * sinf(phi * (float)k));
            }
            phi += 0.02f;
            RKSetPulseReady(pulse);
        }

        if (i % (2 * chunkSize) == 0) {
            RKPulseCompressionEngineLogStatus(radar->pulseCompressionEngine);
        }

        // Wait to simulate 5000-Hz PRF
        g = 0;
        do {
            gettimeofday(&t1, NULL);
            dt = RKTimevalDiff(t1, t0);
            usleep(1000);
            g++;
        } while (dt < 0.0002 * chunkSize);
        t0 = t1;
    }
}


void moduloMathTest(void) {
    int k;
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

int main(int argc, const char * argv[]) {

    bool testModuloMath = false;
    bool testSIMD = false;

    if (testModuloMath) {
        moduloMathTest();
    }

    if (testSIMD) {
        RKSIMDDemo(RKSIMDDemoFlagPerformanceTest);
    }

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);

    RKSetProgramName("iRadar");
    RKSetWantScreenOutput(true);

    RKLog("Initializing ...\n");

    radar = RKInit();

    RKLog("Radar state machine occupies %s B (%s GiB)\n", RKIntegerToCommaStyleString(radar->memoryUsage), RKFloatToCommaStyleString(1.0e-9f * radar->memoryUsage));

    RKGoLive(radar);

    //pulseCompressionTest(FlagShowResults);
    //simulateDataStream();

    sleep(30);

    RKStop(radar);
    
    RKLog("Freeing radar ...\n");
    RKFree(radar);

    return 0;
}
