//
//  RKPulsePair.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/31/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulsePair.h>

int RKPulsePair(RKScratch *space, RKPulse **input, const uint16_t count, const char *name) {
    usleep(20 * 1000);
    
    return 0;

}

int RKPulsePairHop(RKScratch *space, RKPulse **input, const uint16_t count, const char *name) {
//    struct timeval tic, toc;
//    gettimeofday(&tic, NULL);
    
    #if defined(DEBUG_MM)
    RKPulse *S = input[0];
    RKPulse *E = input[count -1];
    // Beamwidths of azimuth & elevation
    float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees, E->header.azimuthDegrees);
    float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);
    
    RKLog("%s   %04u...%04u   E%4.2f-%.2f %% %4.2f   A%6.2f-%6.2f %% %4.2f\n",
          name, S->header.i, E->header.i,
          S->header.elevationDegrees, E->header.elevationDegrees, deltaElevation,
          S->header.azimuthDegrees,   E->header.azimuthDegrees,   deltaAzimuth);
    #endif

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

    int n, k, p;

    // Get the start pulse to know the capacity
    RKPulse *pulse = input[0];
    const uint32_t capacity = pulse->header.capacity;
    const uint32_t gateCount = pulse->header.gateCount;

    //
    //  ACF
    //

    // Go through each polarization
    for (p = 0; p < 2; p++) {
        
        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], capacity);
        for (k = 0; k < RKMaxLag; k++) {
            RKZeroOutIQZ(&space->R[p][k], capacity);
        }

        RKIQZ *R = &space->R[p][0];

        // Go through all pulses
        n = 0;
        do {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(input[n], p);

            RKSIMD_izadd(&Xn, &space->mX[p], pulse->header.gateCount);                   // mX += X
            // Go through each lag
            for (k = 0; k < MIN(count, RKMaxLag); k++) {
                //RKLog(">Lag %d\n", k);
                if (n >= k) {
                    RKIQZ Xk = RKGetSplitComplexDataFromPulse(input[n - k], p);
                    RKSIMD_zcma(&Xn, &Xk, &R[k], pulse->header.gateCount, 1);            // R[k] += X[n] * X[n - k]'
                }
            }
            n++;
        } while (n != count);
        
        // Divide by n for the average
        RKSIMD_izscl(&space->mX[p], 1.0f / (float)n, gateCount);                         // mX /= n
        
        // ACF
        for (k = 0; k < MIN(count, RKMaxLag); k++) {
            RKSIMD_izscl(&R[k], 1.0 / ((float)(n - k)), gateCount);                      // R[k] /= (n - k)
            RKSIMD_zabs(&R[k], space->aR[p][k], gateCount);                              // aR[k] = abs(R[k])
        }
    }
    
    RKSIMD_zmul(&space->mX[0], &space->mX[1], &space->ts, gateCount, 1);                 // E{Xh} * E{Xv}'
    RKSIMD_zsmul(&space->mX[0], &space->vX[0], gateCount, 1);                            // E{Xh} * E{Xh}' --> varh  (not yet)
    RKSIMD_zsmul(&space->mX[1], &space->vX[1], gateCount, 1);                            // E{Xv} * E{Xv}' --> varv  (not yet)
    RKSIMD_izsub(&space->R[0][0], &space->vX[0], gateCount);                             // Rh[0] - varh   --> varh  (varh is now VAR(Xh))
    RKSIMD_izsub(&space->R[1][0], &space->vX[1], gateCount);                             // Rv[0] - varv   --> varv  (varv is now VAR(Xh))

    for (p = 0; p < 2; p++) {
        for (k = 0; k < MIN(count, RKMaxLag); k++) {
            RKZeroOutFloat(&space->aR[p][k][0], gateCount);
        }
        RKZeroTailIQZ(&space->vX[p], capacity, gateCount);
    }
    
    // NOTE: At this point, one can use space->vX[0] & space->vX[1] as signal power for H & V, respectively.
    // However, within the isodop regions, the zero-Doppler power is may have been filtered out by the clutter filter
    // so S & R are biased unless the filter is turned off. It's common problem with weather radars.

    #if defined(DEBUG_PULSE_PAIR)
    if (count < 8) {
        char variable[32];
        for (p = 0; p < 2; p++) {
            printf("\033[4mChannel %d (%s pol):\033[24m\n", p, p == 0 ? "H" : (p == 1 ? "V" : "X"));
            for (n = 0; n < count; n++) {
                RKIQZ Xn = RKGetSplitComplexDataFromPulse(input[n], p);
                sprintf(variable, "  X[%d] = ", n);
                RKShowVecIQZ(variable, &Xn, 8);
            }
            printf(RKEOL);
            for (k = 0; k < MIN(count, RKMaxLag); k++) {
                sprintf(variable, "  R[%d] = ", k);
                RKShowVecIQZ(variable, &space->R[p][k], 8);
            }
            printf(RKEOL);
            for (k = 0; k < MIN(count, RKMaxLag); k++) {
                sprintf(variable, " aR[%d] = ", k);
                RKShowVecFloat(variable, space->aR[p][k], 8);
            }
            printf(RKEOL);
        }
        printf("\033[4mMean and Variance:\033[24m\n");

        RKShowVecIQZ("  mX[0] = ", &space->mX[0], 8);
        RKShowVecIQZ("  vX[0] = ", &space->vX[0], 8);
        RKShowVecIQZ("  mX[1] = ", &space->mX[1], 8);
        RKShowVecIQZ("  vX[1] = ", &space->vX[1], 8);
    } else {
        RKLog("ERROR. Skipped printing a large array.\n");
    }
    #endif
    
    //
    //  CCF
    //

    RKZeroOutFloat(space->gC, capacity);
    
    const RKFloat N = (RKFloat)3; // Number of lag
    RKFloat w = 0.0f;
    
    int nlag = 3;
    RKIQZ Xh, Xv;
    
    for (int ic = 0; ic < 2 * nlag + 1; ic++) {
        k = ic - nlag;
        
        // Numerator
        if (k < 0) {
            Xh = RKGetSplitComplexDataFromPulse(input[0], 0);
            Xv = RKGetSplitComplexDataFromPulse(input[-k], 1);
            RKSIMD_zmul(&Xh, &Xv, &space->C[ic], gateCount, 1);                          // C = Xh * Xv', flag 1 = conjugate
            for (n = -k + 1; n < count; n++) {
                Xh = RKGetSplitComplexDataFromPulse(input[n + k], 0);
                Xv = RKGetSplitComplexDataFromPulse(input[n], 1);
                RKSIMD_zcma(&Xh, &Xv, &space->C[ic], gateCount, 1);                      // C = C + Xh[] * Xv[]'
            }
            RKSIMD_izscl(&space->C[ic], 1.0f / (RKFloat)(count + k), gateCount);         // E{Xh * Xv'}
        } else {
            Xh = RKGetSplitComplexDataFromPulse(input[k], 0);
            Xv = RKGetSplitComplexDataFromPulse(input[0], 1);
            RKSIMD_zmul(&Xh, &Xv, &space->C[ic], gateCount, 1);                          // C = Xh * Xv', flag 1 = conjugate
            for (n = k + 1; n < count; n++) {
                Xh = RKGetSplitComplexDataFromPulse(input[n], 0);
                Xv = RKGetSplitComplexDataFromPulse(input[n - k], 1);
                RKSIMD_zcma(&Xh, &Xv, &space->C[ic], gateCount, 1);                      // C = C + Xh[] * Xv[]'
            }
            RKSIMD_izscl(&space->C[ic], 1.0f / (RKFloat)(count - k), gateCount);         // E{Xh * Xv'}
        }
        // Comment the following line to include the zero-Doppler signal
        RKSIMD_zabs(&space->C[ic], space->aC[ic], gateCount);                            // | E{Xh * Xv'} - E{Xh} * E{Xv}' | --> absC[ic]
        
        w = (3.0f * N * N + 3.0f * N - 1.0f - 5.0f * (RKFloat)(k * k));
        for (n = 0; n < gateCount; n++) {
            space->gC[n] += w * logf(space->aC[ic][n]);
        }
    }
    
    w = 3.0f / ((2.0f * N - 1.0f) * (2.0f * N + 1.0f) * (2.0f * N + 3.0f));
    for (n = 0; n < gateCount; n++) {
        space->gC[n] = expf(w * space->gC[n]);
    }

//    gettimeofday(&toc, NULL);
//    RKLog("Diff time = %.4f ms", 1.0e3 * RKTimevalDiff(toc, tic));

    return count;

}
