//
//  RKMultiLag.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/2/17.
//  Copyright (c) 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKMultiLag.h>

int RKMultiLag(RKScratch *space, RKPulse **input, const uint16_t pulseCount) {
    //    struct timeval tic, toc;
    //    gettimeofday(&tic, NULL);
        
    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W
    
    int n, j, k, p;
    
    // Get the start pulse to know the capacity
    RKPulse *pulse = input[0];
    const uint32_t capacity = pulse->header.capacity;
    const uint32_t gateCount = pulse->header.gateCount;
    const int lagCount = MIN(pulseCount, space->lagCount);
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
        } while (n != pulseCount);
        
        // Divide by n for the average
        RKSIMD_izscl(&space->mX[p], 1.0f / (float)n, gateCount);                         // mX /= n
        
        // ACF
        for (k = 0; k < lagCount; k++) {
            RKSIMD_izscl(&R[k], 1.0 / ((float)(n - k)), gateCount);                      // R[k] /= (n - k)   (unbiased)
            RKSIMD_zabs(&R[k], space->aR[p][k], gateCount);                              // aR[k] = abs(R[k])
        }
        
        // Mean and variance (2nd moment)
        RKSIMD_zsmul(&space->mX[p], &space->vX[p], gateCount, 1);                        // E{Xh} * E{Xh}' --> var  (step 1)
        RKSIMD_izsub(&space->R[p][0], &space->vX[p], gateCount);                         // Rh[] - var     --> var  (step 2)
    }
    
    // Cross-channel
    RKSIMD_zmul(&space->mX[0], &space->mX[1], &space->ts, gateCount, 1);                 // E{Xh} * E{Xv}'
    
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
            for (n = -k + 1; n < pulseCount; n++) {
                Xh = RKGetSplitComplexDataFromPulse(input[n + k], 0);
                Xv = RKGetSplitComplexDataFromPulse(input[n    ], 1);
                RKSIMD_zcma(&Xh, &Xv, &space->C[j], gateCount, 1);                      // C = C + Xh[] * Xv[]'
            }
            RKSIMD_izscl(&space->C[j], 1.0f / (RKFloat)(pulseCount + k), gateCount);    // E{Xh * Xv'}   (unbiased)
        } else {
            Xh = RKGetSplitComplexDataFromPulse(input[k], 0);
            Xv = RKGetSplitComplexDataFromPulse(input[0], 1);
            RKSIMD_zmul(&Xh, &Xv, &space->C[j], gateCount, 1);                          // C = Xh * Xv', flag 1 = conjugate
            for (n = k + 1; n < pulseCount; n++) {
                Xh = RKGetSplitComplexDataFromPulse(input[n    ], 0);
                Xv = RKGetSplitComplexDataFromPulse(input[n - k], 1);
                RKSIMD_zcma(&Xh, &Xv, &space->C[j], gateCount, 1);                      // C = C + Xh[] * Xv[]'
            }
            RKSIMD_izscl(&space->C[j], 1.0f / (RKFloat)(pulseCount - k), gateCount);    // E{Xh * Xv'}   (unbiased)
        }
        // Comment the following line to include the zero-Doppler signal
        RKSIMD_zabs(&space->C[j], space->aC[j], gateCount);                             // |E{Xh * Xv'} - E{Xh} * E{Xv}'| --> absC[ic]
        
        w = (3.0f * N * N + 3.0f * N - 1.0f - 5.0f * (RKFloat)(k * k));
        for (n = 0; n < gateCount; n++) {
            space->gC[n] += w * logf(space->aC[j][n]);
        }
    }
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
            RKZeroTailFloat(&space->gC[j], capacity, gateCount);
        }
    }
    
    if (space->showNumbers && gateCount < 16 && pulseCount < 32) {
        char variable[32];
        char line[2048];
        RKIQZ X[pulseCount];
        const int gateShown = 8;
        for (p = 0; p < 2; p++) {
            printf("\033[4mChannel %d (%s pol):\033[24m\n", p, p == 0 ? "H" : (p == 1 ? "V" : "X"));
            for (n = 0; n < pulseCount; n++) {
                X[n] = RKGetSplitComplexDataFromPulse(input[n], p);
            }
            
            /* A block ready for MATLAB
             
             - Copy and paste X = [ 0+2j, 0+1j, ...
            
            Then, all the previous calculations can be extremely easy.
             
             N = 4; % lag count 5 (max lag 4)
             g = 1; % gate 1
             C = xcorr(Xh(g, :), Xv(g, :), 4, 'unbiased') .';
             wn = 3 * N^2 + 3 * N - 1 - 5 * (-N + 1 : N - 1).^2;
             wd = 3 / ((2 * N - 1) * (2 * N + 1) * (2 * N + 3));
             gC = exp(wd * sum(wn(:) .* log(abs(C))))
             
             */
            j = sprintf(line, "  X%s = [", p == 0 ? "h" : "v");
            for (k = 0; k < gateCount; k++) {
                for (n = 0; n < pulseCount; n++) {
                    j += sprintf(line + j, " %.0f%+.0fj,", X[n].i[k], X[n].q[k]);
                }
                j += sprintf(line + j - 1, ";...\n") - 1;
            }
            sprintf(line + j - 5, "]\n");
            printf("%s\n", line);
            
            for (n = 0; n < pulseCount; n++) {
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
    }

    //    gettimeofday(&toc, NULL);
    //    RKLog("Diff time = %.4f ms", 1.0e3 * RKTimevalDiff(toc, tic));
    
    return pulseCount;
    
}
