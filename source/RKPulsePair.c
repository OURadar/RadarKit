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
    
    RKPulse *S = input[0];
    RKPulse *E = input[count -1];
    // Beamwidths of azimuth & elevation
    float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees, E->header.azimuthDegrees);
    float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);
    
    RKLog("%s   %04u...%04u   E%4.2f-%.2f %% %4.2f   A%6.2f-%6.2f %% %4.2f\n",
          name, S->header.i, E->header.i,
          S->header.elevationDegrees, E->header.elevationDegrees, deltaElevation,
          S->header.azimuthDegrees,   E->header.azimuthDegrees,   deltaAzimuth);

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

    int n, j, k, p;

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
        RKZeroOutIQZ(&space->R[p][0], capacity);
        RKZeroOutIQZ(&space->R[p][1], capacity);

        // Go through the even pulses for mX and R(0)
        n = 0;
        if (pulse->header.i % 2) {
            // Ignore the first pulse if it is an odd-pulse
            n = 1;
        }
        j = 0;
        for (; n < count; n += 2) {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(input[n], p);
            RKSIMD_izadd(&Xn, &space->mX[p], pulse->header.gateCount);                   // mX += X
            RKSIMD_zcma(&Xn, &Xn, &space->R[p][0], pulse->header.gateCount, 1);          // R[0] += X[n] * X[n]'
            j++;
        }
        // Divide by n for the average
        RKSIMD_izscl(&space->mX[p], 1.0f / (float)(j), gateCount);                       // mX /= j
        RKSIMD_izscl(&space->R[p][0], 1.0f / (float)(j), gateCount);                     // R[0] /= j   (unbiased)
        RKSIMD_zabs(&space->R[p][0], space->aR[p][0], gateCount);                        // aR[0] = abs(R[0])

        // Now we get the odd pulses for R(1)
        n = 1;
        if (pulse->header.i % 2) {
            // Ignore the first pulse if it is an odd-pulse
            n = 2;
        }
        j = 0;
        for (; n < count; n += 2) {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(input[n], p);
            RKIQZ Xk = RKGetSplitComplexDataFromPulse(input[n - 1], p);
            RKSIMD_zcma(&Xn, &Xk, &space->R[p][1], pulse->header.gateCount, 1);          // R[k] += X[n] * X[n - k]'
            j++;
        }
        RKSIMD_izscl(&space->R[p][1], 1.0f / (float)(j), gateCount);                     // R[1] /= j   (unbiased)
        RKSIMD_zabs(&space->R[p][1], space->aR[p][1], gateCount);                        // aR[0] = abs(R[0])

        // Mean and variance (2nd moment)
        RKSIMD_zsmul(&space->mX[p], &space->vX[p], gateCount, 1);                        // E{Xh} * E{Xh}' --> var  (step 1)
        RKSIMD_izsub(&space->R[p][0], &space->vX[p], gateCount);                         // Rh[] - var     --> var  (step 2)
    }

    // Cross-channel
    RKZeroOutIQZ(&space->C[0], capacity);

    //
    //  CCF
    //

    RKIQZ Xh, Xv;

    // Go through the even pulses for mX and R(0)
    n = 0;
    if (pulse->header.i % 2) {
        // Ignore the first pulse if it is an odd-pulse
        n = 1;
    }
    Xh = RKGetSplitComplexDataFromPulse(input[n], 0);
    Xv = RKGetSplitComplexDataFromPulse(input[n], 1);
    RKSIMD_zmul(&Xh, &Xv, &space->C[0], gateCount, 1);                                   // C = Xh * Xv', flag 1 = conjugate
    j = 1;
    n += 2;
    for (; n < count; n += 2) {
        Xh = RKGetSplitComplexDataFromPulse(input[n], 0);
        Xv = RKGetSplitComplexDataFromPulse(input[n], 1);
        RKSIMD_zcma(&Xh, &Xv, &space->C[0], gateCount, 1);                               // C += Xh[] * Xv[]'
        j++;
    }
    RKSIMD_izscl(&space->C[0], 1.0f / (float)(j), gateCount);                            // C /= j   (unbiased)

    if (space->showNumbers) {
        char variable[32];
        char line[2048];
        RKIQZ X[count];
        const int gateShown = 8;
        for (p = 0; p < 2; p++) {
            printf("\%sChannel %d (%s pol):%s\n",
                   rkGlobalParameters.showColor ? "\033[4m" : "",
                   p, p == 0 ? "H" : (p == 1 ? "V" : "X"),
                   rkGlobalParameters.showColor ? "\033[24m" : "");
            for (n = 0; n < count; n++) {
                X[n] = RKGetSplitComplexDataFromPulse(input[n], p);
            }

            /* A block ready for MATLAB

             - Copy and paste X = [ 0+2j, 0+1j, ...

             Then, all the previous calculations can be extremely easy.

             g = 1; % gate 1
             k = 1; % even is correct, change to 2 for even is 1-off
             mX = mean(Xh(:, k:2:end), 2).'
             for g = 1:6, C(g) = xcorr(Xh(g, k:2:end), Xv(g, k:2:end), 0, 'unbiased'); end; disp(C)

             */
            j = sprintf(line, "  X%s = [", p == 0 ? "h" : "v");
            for (k = 0; k < gateCount; k++) {
                for (n = 0; n < count; n++) {
                    j += sprintf(line + j, " %.0f%+.0fj,", X[n].i[k], X[n].q[k]);
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
            for (k = 0; k < 2; k++) {
                sprintf(variable, "  R[%d] = ", k);
                RKShowVecIQZ(variable, &space->R[p][k], gateShown);
            }
            printf(RKEOL);
            for (k = 0; k < 2; k++) {
                sprintf(variable, " aR[%d] = ", k);
                RKShowVecFloat(variable, space->aR[p][k], gateShown);
            }
            printf(RKEOL);
        }
        printf("%sCross-channel:%s\n",
               rkGlobalParameters.showColor ? "\033[4m" : "",
               rkGlobalParameters.showColor ? "\033[24m" : "");
        RKShowVecIQZ("  C[0] = ", &space->C[0], gateShown);                                 // xcorr(Xh, Xv, 'unbiased') in MATLAB
        printf(RKEOL);
    }
    
    //    gettimeofday(&toc, NULL);
//    RKLog("Diff time = %.4f ms", 1.0e3 * RKTimevalDiff(toc, tic));

    return count;

}
