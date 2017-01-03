//
//  RKMultiLag.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/2/17.
//  Copyright (c) 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKMultiLag.h>

int RKMultiLag(RKScratch *space, RKPulse **input, const uint16_t count, const char *name) {
    //    struct timeval tic, toc;
    //    gettimeofday(&tic, NULL);
    
//#if defined(DEBUG_MM)
    RKPulse *S = input[0];
    RKPulse *E = input[count -1];
    // Beamwidths of azimuth & elevation
    float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees, E->header.azimuthDegrees);
    float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);
    
    RKLog("%s   %04u...%04u   E%4.2f-%.2f %% %4.2f   A%6.2f-%6.2f %% %4.2f\n",
          name, S->header.i, E->header.i,
          S->header.elevationDegrees, E->header.elevationDegrees, deltaElevation,
          S->header.azimuthDegrees,   E->header.azimuthDegrees,   deltaAzimuth);
//#endif
    
    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W
    
    int n, j, k, p;
    
    // Get the start pulse to know the capacity
    RKPulse *pulse = input[0];
    const uint32_t capacity = pulse->header.capacity;
    const uint32_t gateCount = pulse->header.gateCount;
    const int lagCount = MIN(count, space->lagCount);
    //
    //  ACF
    //
    
    // Go through each polarization
    for (p = 0; p < 2; p++) {
        
        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], capacity);
        for (k = 0; k < lagCount; k++) {
            RKZeroOutIQZ(&space->R[p][k], capacity);
        }
        
        RKIQZ *R = &space->R[p][0];
        
        // Go through all pulses
        n = 0;
        do {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(input[n], p);
            
            RKSIMD_izadd(&Xn, &space->mX[p], pulse->header.gateCount);                   // mX += X
            // Go through each lag
            for (k = 0; k < lagCount; k++) {
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
        for (k = 0; k < lagCount; k++) {
            RKSIMD_izscl(&R[k], 1.0 / ((float)(n - k)), gateCount);                      // R[k] /= (n - k)   (unbiased)
            RKSIMD_zabs(&R[k], space->aR[p][k], gateCount);                              // aR[k] = abs(R[k])
        }
    }
    
    RKSIMD_zmul(&space->mX[0], &space->mX[1], &space->ts, gateCount, 1);                 // E{Xh} * E{Xv}'
    RKSIMD_zsmul(&space->mX[0], &space->vX[0], gateCount, 1);                            // E{Xh} * E{Xh}' --> varh  (not yet)
    RKSIMD_zsmul(&space->mX[1], &space->vX[1], gateCount, 1);                            // E{Xv} * E{Xv}' --> varv  (not yet)
    RKSIMD_izsub(&space->R[0][0], &space->vX[0], gateCount);                             // Rh[0] - varh   --> varh  (varh is now var(Xh, 1))
    RKSIMD_izsub(&space->R[1][0], &space->vX[1], gateCount);                             // Rv[0] - varv   --> varv  (varv is now var(Xv, 1))
    
    // NOTE: At this point, one can use space->vX[0] & space->vX[1] as signal power for H & V, respectively.
    // However, within the isodop regions, the zero-Doppler power is may have been filtered out by the clutter filter
    // so S & R are biased unless the filter is turned off. It's common problem with weather radars.
    
    //
    //  CCF
    //
    
    RKZeroOutFloat(space->gC, capacity);
    
    const RKFloat N = (RKFloat)lagCount; // Number of lags
    RKFloat w = 0.0f;
    
    RKIQZ Xh, Xv;
    
    for (j = 0; j < 2 * lagCount - 1; j++) {
        k = j - lagCount + 1;
        
        // Numerator
        if (k < 0) {
            Xh = RKGetSplitComplexDataFromPulse(input[ 0], 0);
            Xv = RKGetSplitComplexDataFromPulse(input[-k], 1);
            RKSIMD_zmul(&Xh, &Xv, &space->C[j], gateCount, 1);                          // C = Xh * Xv', flag 1 = conjugate
            for (n = -k + 1; n < count; n++) {
                Xh = RKGetSplitComplexDataFromPulse(input[n + k], 0);
                Xv = RKGetSplitComplexDataFromPulse(input[n    ], 1);
                RKSIMD_zcma(&Xh, &Xv, &space->C[j], gateCount, 1);                      // C = C + Xh[] * Xv[]'
            }
            RKSIMD_izscl(&space->C[j], 1.0f / (RKFloat)(count + k), gateCount);         // E{Xh * Xv'}   (unbiased)
        } else {
            Xh = RKGetSplitComplexDataFromPulse(input[k], 0);
            Xv = RKGetSplitComplexDataFromPulse(input[0], 1);
            RKSIMD_zmul(&Xh, &Xv, &space->C[j], gateCount, 1);                          // C = Xh * Xv', flag 1 = conjugate
            for (n = k + 1; n < count; n++) {
                Xh = RKGetSplitComplexDataFromPulse(input[n    ], 0);
                Xv = RKGetSplitComplexDataFromPulse(input[n - k], 1);
                RKSIMD_zcma(&Xh, &Xv, &space->C[j], gateCount, 1);                      // C = C + Xh[] * Xv[]'
            }
            RKSIMD_izscl(&space->C[j], 1.0f / (RKFloat)(count - k), gateCount);         // E{Xh * Xv'}   (unbiased)
        }
        // Comment the following line to include the zero-Doppler signal
        RKSIMD_zabs(&space->C[j], space->aC[j], gateCount);                             // |E{Xh * Xv'} - E{Xh} * E{Xv}'| --> absC[ic]
        
        w = (3.0f * N * N + 3.0f * N - 1.0f - 5.0f * (RKFloat)(k * k));
        for (n = 0; n < gateCount; n++) {
            space->gC[n] += w * logf(space->aC[j][n]);
        }
    }
    
    /*  MATLAB can be very simple:
     
     g = 1; % gate 1
     C = xcorr(Xh(g, :), Xv(g, :), 4, 'unbiased') .'
     wn = 3 * N^2 + 3 * N - 1 - 5 * (-N + 1 : N - 1).^2;
     wd = 3 / ((2 * N - 1) * (2 * N + 1) * (2 * N + 3));
     gC = exp(wd * sum(wn(:) .* log(abs(C))))
     
     */
    w = 3.0f / ((2.0f * N - 1.0f) * (2.0f * N + 1.0f) * (2.0f * N + 3.0f));
    for (n = 0; n < gateCount; n++) {
        space->gC[n] = expf(w * space->gC[n]);
    }

    // Zero out the tails
    for (p = 0; p < 2; p++) {
        for (k = 0; k < lagCount; k++) {
            RKZeroTailFloat(space->aR[p][k], capacity, gateCount);
        }
        RKZeroTailIQZ(&space->vX[p], capacity, gateCount);
        for (j = 0; j < 2 * lagCount - 1; j++) {
            k = j - lagCount + 1;
            RKZeroTailFloat(&space->gC[j], capacity, gateCount);
        }
    }
    
    if (count < 8) {
        char variable[32];
        char line[2048];
        RKIQZ X[count];
        const int gateShown = 8;
        for (p = 0; p < 2; p++) {
            printf("\033[4mChannel %d (%s pol):\033[24m\n", p, p == 0 ? "H" : (p == 1 ? "V" : "X"));
            for (n = 0; n < count; n++) {
                X[n] = RKGetSplitComplexDataFromPulse(input[n], p);
            }
            
            j = sprintf(line, "  X%s = [", p == 0 ? "h" : "v");
            for (k = 0; k < gateCount; k++) {
                for (n = 0; n < count; n++) {
                    j += sprintf(line + j, " %.4f%+.4fj,", X[n].i[k], X[n].q[k]);
                }
                j += sprintf(line + j - 1, ";...\n") - 1;
            }
            sprintf(line + j - 5, "]\n");
            printf("%s\n", line);
            
            for (n = 0; n < count; n++) {
                sprintf(variable, "  X[%d] = ", n);
                RKShowVecIQZ(variable, &X[n], gateShown);
            }
            printf(RKEOL);
            RKShowVecIQZ("    mX = ", &space->mX[p], gateShown);                              // mean(X) in MATLAB
            RKShowVecIQZ("    vX = ", &space->vX[p], gateShown);                              // var(X, 1) in MATLAB
            printf(RKEOL);
            for (k = 0; k < lagCount; k++) {
                sprintf(variable, "  R[%d] = ", k);
                RKShowVecIQZ(variable, &space->R[p][k], gateShown);
            }
            printf(RKEOL);
            for (k = 0; k < lagCount; k++) {
                sprintf(variable, " aR[%d] = ", k);
                RKShowVecFloat(variable, space->aR[p][k], gateShown);
            }
            printf(RKEOL);
        }
        printf("\033[4mCross-channel:\033[24m\n");
        for (j = 0; j < 2 * lagCount - 1; j++) {
            k = j - lagCount + 1;
            sprintf(variable, " C[%2d] = ", k);
            RKShowVecIQZ(variable, &space->C[j], gateShown);                                 // xcorr(Xh, Xv, 'unbiased') in MATLAB
        }
        printf(RKEOL);
        RKShowVecFloat("    gC = ", space->gC, gateShown);
        printf(RKEOL);
    } else {
        RKLog("ERROR. Skipped printing a large array.\n");
    }

    //    gettimeofday(&toc, NULL);
    //    RKLog("Diff time = %.4f ms", 1.0e3 * RKTimevalDiff(toc, tic));
    
    return count;
    
}
