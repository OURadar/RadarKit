//
//  RKTest.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/25/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKTest.h>
#include <getopt.h>

#define RKFMT  "%4d"

void RKTestModuloMath(void) {
    int k;
    const int N = 4;

    RKLog("Test with SlotCount = %d, N = %d\n", RKBuffer0SlotCount, N);
    k = 0;                      RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 4; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 3; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 2; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 1; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 1; RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 0;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 1;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 2;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 3;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 4;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    
    k = 4899;                   RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, 100 - 1, RKBuffer0SlotCount));
}

RKTransceiver RKTestSimulateDataStream(RKRadar *radar, void *input) {
    int k;
    float phi = 0.0f;
    float tau = 0.0f;
    float azimuth = 0.0f;
    struct timeval t0, t1;
    double dt = 0.0;
    double prt = 0.0002;
    float fs = 50.0e6;
    int g = 0;

    gettimeofday(&t0, NULL);

    RKSetLogfile(NULL);

    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("RKTestSimulateDataStream%s", (char *)input);
    }
    
    // Parse out input parameters
    if (input) {
        char *sb = (char *)input, *se = NULL, *sv = NULL;
        while (*sb == ' ') {
            sb++;
        }
        while ((se = strchr(sb, ' ')) != NULL) {
            sv = se + 1;
            switch (*sb) {
                case 'f':
                    prt = 1.0 / (double)atof(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">prf = %s Hz", RKIntegerToCommaStyleString((long)(1.0f / prt)));
                    }
                    break;
                case 'F':
                    fs = atof(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">fs = %s Hz", RKIntegerToCommaStyleString((long)fs));
                    }
                    break;
            }
            sb = strchr(sv, ' ');
            if (sb == NULL) {
                break;
            } else {
                while (*sb == ' ') {
                    sb++;
                }
            }
        }
    }

    const int gateCount = MIN(radar->pulses[0].header.capacity, (int)(60.0e3 / 3.0e8 * fs * 2.0));
    const int chunkSize = (int)floor(0.1f / prt);

    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Using fs = %s MHz   PRF = %s Hz   gate count = %s (%.1f km)   chunk %d\n",
              RKFloatToCommaStyleString(1.0e-6 * fs),
              RKIntegerToCommaStyleString((int)(1.0f / prt)),
              RKIntegerToCommaStyleString(gateCount),
              gateCount * 1.5e5 / fs,
              chunkSize);
    }

    while (radar->active) {

        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("%s %s",
                  RKPulseCompressionEngineStatusString(radar->pulseCompressionEngine),
                  RKMomentEngineStatusString(radar->momentEngine));
        }

        for (int j = 0; radar->active && j < chunkSize; j++) {
            RKPulse *pulse = RKGetVacantPulse(radar);
            // Fill in the data...
            pulse->header.gateCount = gateCount;
            pulse->header.azimuthDegrees = azimuth;
            pulse->header.elevationDegrees = 2.41f;
            RKInt16C *X = RKGetInt16DataFromPulse(pulse, 0);
            for (k = 0; k < gateCount; k++) {
                X->i = (int16_t)(32767.0f * cosf(phi * (float)k));
                X->q = (int16_t)(32767.0f * sinf(phi * (float)k));
                X++;
            }
            phi += 0.02f;
            azimuth = fmodf(50.0f * tau, 360.0f);

            RKSetPulseReady(pulse);

            tau += prt;
        }

        // Wait to simulate the PRF
        g = 0;
        do {
            gettimeofday(&t1, NULL);
            dt = RKTimevalDiff(t1, t0);
            usleep(1000);
            g++;
        } while (radar->active && dt < prt * chunkSize);
        t0 = t1;
    }
    return NULL;
}

void RKTestPulseCompression(RKRadar *radar, RKTestFlag flag) {
    RKPulse *pulse = RKGetVacantPulse(radar);
    pulse->header.gateCount = 6;

    RKInt16C *X = RKGetInt16DataFromPulse(pulse, 0);
    memset(X, 0, pulse->header.gateCount * sizeof(RKInt16C));
    X[0].i = 1;
    X[0].q = 0;
    X[1].i = 2;
    X[1].q = 0;
    X[2].i = 4;
    X[2].q = 0;
    X[3].i = 2;
    X[3].q = 0;
    X[4].i = 1;
    X[4].q = 0;
    RKSetPulseReady(pulse);

    while ((pulse->header.s & RKPulseStatusCompressed) == 0) {
        usleep(1000);
    }

    RKComplex *F = &radar->pulseCompressionEngine->filters[0][0][0];
    RKComplex *Y = RKGetComplexDataFromPulse(pulse, 0);
    RKIQZ Z = RKGetSplitComplexDataFromPulse(pulse, 0);

    if (flag & RKTestFlagShowResults) {
        printf("X =                       F =                     Y =                             Z =\n");
        for (int k = 0; k < 8; k++) {
            printf("    [ %6d %s %6di ]      [ %5.2f %s %5.2fi ]      [ %9.2f %s %9.2fi ]      [ %9.2f %s %9.2fi ]\n",
                   X[k].i, X[k].q < 0 ? "-" : "+", abs(X[k].q),
                   F[k].i, F[k].q < 0.0f ? "-" : "+", fabs(F[k].q),
                   Y[k].i, Y[k].q < 0.0f ? "-" : "+", fabs(Y[k].q),
                   Z.i[k], Z.q[k] < 0.0f ? "-" : "+", fabs(Z.q[k]));
        }
    }
}
