//
//  RKPulseATSR.c
//  RadarKit
//
//  Created by Min-Duan Tzeng on 9/14/23.
//  Copyright (c) 2023 Min-Duan Tzeng. All rights reserved.
//

#include <RadarKit/RKPulseATSR.h>

int RKPulseATSR(RKMomentScratch *space, RKPulse **pulses, const uint16_t count) {

    //
    // alternative transmit and simultaneous receive processing
    //
    //  h  v  h  v  h  v
    //  0  1  0  1  0  1
    //
    //  o  o  o  o  o  o
    //  |  |  |  |  |  |
    //  +--+--+--+--+--+-
    //
    // Properties:
    //   - ACF
    //   - R[0][lag] hc (h-copolar) from even pulses H channel receive
    //   - R[1][lag] vc (v-copolar) from odd pulses V channel receive
    //   - RX[0][lag] hx (h-cross-polar) from odd pulses H channel receive
    //   - RX[1][lag] vx (v-cross-polar) from even pulses V channel receive
    //
    //   - CCF
    //   - C[lag] hcvc          hc[] * vc[]'
    //   - CX[0][lag] hcvx       hc[] * vx[]'
    //   - CX[1][lag] vchx       vc[] * hx[]'

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

    int n, j, k, p;

    const uint32_t gateCount = space->gateCount;
    const int K = (gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);

    //
    //  ACF
    //

    // Get the start pulse
    RKPulse *pulse = pulses[0];

    // Go through each polarization
    for (p = 0; p < 2; p++) {

        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], gateCount);
        RKZeroOutIQZ(&space->R[p][0], gateCount);
        RKZeroOutIQZ(&space->R[p][1], gateCount);
        RKZeroOutIQZ(&space->RX[p][0], gateCount);
        RKZeroOutIQZ(&space->RX[p][1], gateCount);
    }
    // Go through the even pulses for hc and vx
    n = 0;
    if (pulse->header.i % 2) {
        // Ignore the first pulse if it is an odd-pulse
        n = 1;
    }

    RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);
    RKSIMD_zcma(&Xn, &Xn, &space->R[0][0], gateCount, 1);
    RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], 1);
    RKSIMD_zcma(&Xn, &Xn, &space->RX[1][0], gateCount, 1);

    n += 2;
    j = 1;
    for (; n < count; n += 2) {
        RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        RKIQZ Xk = RKGetSplitComplexDataFromPulse(pulses[n - 2], 0);
        RKSIMD_zcma(&Xn, &Xn, &space->R[0][0], gateCount, 1);                                                // R[0] += X[n] * X[n]'
        RKSIMD_zcma(&Xn, &Xk, &space->R[0][1], gateCount, 1);                                                // R[k] += X[n] * X[n- k]'
        RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], 1);
        RKIQZ Xk = RKGetSplitComplexDataFromPulse(pulses[n - 2], 1);
        RKSIMD_zcma(&Xn, &Xn, &space->RX[1][0], gateCount, 1);
        RKSIMD_zcma(&Xn, &Xk, &space->RX[1][1], gateCount, 1);
        j++;
    }
    RKSIMD_izscl(&space->R[0][0], 1.0f / (float)(j), gateCount);                                             // R[0] /= j   (unbiased)
    RKSIMD_izscl(&space->RX[1][0], 1.0f / (float)(j), gateCount);
    RKSIMD_izscl(&space->R[0][1], 1.0f / (float)(j-1), gateCount);                                           // R[1] /= (j-1)   (unbiased)
    RKSIMD_izscl(&space->RX[1][1], 1.0f / (float)(j-1), gateCount);

    RKSIMD_zabs(&space->R[0][0], space->aR[0][0], gateCount);
    RKSIMD_zabs(&space->RX[1][0], space->aRX[1][0], gateCount);
    RKSIMD_zabs(&space->R[0][1], space->aR[0][1], gateCount);
    RKSIMD_zabs(&space->RX[1][1], space->aRX[1][1], gateCount);

    // Go through the odd pulses for vc and hx
    n = 1;
    if (pulse->header.i % 2) {
        // Take first pulse if it is an odd-pulse
        n = 0;
    }

    RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);
    RKSIMD_zcma(&Xn, &Xn, &space->RX[0][0], gateCount, 1);
    RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], 1);
    RKSIMD_zcma(&Xn, &Xn, &space->R[1][0], gateCount, 1);

    n += 2;
    j = 1;
    for (; n < count; n += 2) {
        RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        RKIQZ Xk = RKGetSplitComplexDataFromPulse(pulses[n - 2], 0);
        RKSIMD_zcma(&Xn, &Xn, &space->RX[0][0], gateCount, 1);                                                // R[0] += X[n] * X[n]'
        RKSIMD_zcma(&Xn, &Xk, &space->RX[0][1], gateCount, 1);                                                // R[k] += X[n] * X[n- k]'
        RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], 1);
        RKIQZ Xk = RKGetSplitComplexDataFromPulse(pulses[n - 2], 1);
        RKSIMD_zcma(&Xn, &Xn, &space->R[1][0], gateCount, 1);
        RKSIMD_zcma(&Xn, &Xk, &space->R[1][1], gateCount, 1);
        j++;
    }
    RKSIMD_izscl(&space->RX[0][0], 1.0f / (float)(j), gateCount);                                             // R[0] /= j   (unbiased)
    RKSIMD_izscl(&space->R[1][0], 1.0f / (float)(j), gateCount);
    RKSIMD_izscl(&space->RX[0][1], 1.0f / (float)(j-1), gateCount);                                           // R[1] /= (j-1)   (unbiased)
    RKSIMD_izscl(&space->R[1][1], 1.0f / (float)(j-1), gateCount);

    RKSIMD_zabs(&space->RX[0][0], space->aRX[0][0], gateCount);
    RKSIMD_zabs(&space->R[1][0], space->aR[1][0], gateCount);
    RKSIMD_zabs(&space->RX[0][1], space->aRX[0][1], gateCount);
    RKSIMD_zabs(&space->R[1][1], space->aR[1][1], gateCount);

