//
//  RKPulsePair.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/31/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulsePair.h>


void RKUpdateZVWInScratchSpace(RKScratch *space, const int gateCount) {
    const RKFloat va = 15.0f;
    const RKFloat wa = 0.3 / (2.0f * sqrt(2.0) * M_PI) * 2000.0f;
    const RKVec va_pf = _rk_mm_set1_pf(va);
    const RKVec wa_pf = _rk_mm_set1_pf(wa);
    const RKVec ten = _rk_mm_set1_pf(10.0f);
    const RKVec cal = _rk_mm_set1_pf(-30.0f);
    RKFloat *s;
    RKFloat *z;
    RKFloat *v;
    RKFloat *w;
    RKVec *s_pf;
    RKVec *z_pf;
    RKVec *v_pf;
    RKVec *w_pf;
    RKFloat *r1i;
    RKFloat *r1q;
    int p, k, K = (gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    for (p = 0; p < 2; p++) {
        // log10(S) --> Z (temp)
        s = space->S[p];
        z = space->Z[p];
        v = (RKFloat *)space->V[p];
        r1i = (RKFloat *)space->R[p][1].i;
        r1q = (RKFloat *)space->R[p][1].q;
        // Regular math, no intrinsic options
        for (k = 0; k < gateCount; k++) {
#if !defined(_rk_mm_log10_pf)
            *z++ = log10f(*s++);
#endif
            *v++ = atan2f(*r1q++, *r1i++);
        }
        z_pf = (RKVec *)space->Z[p];
        v_pf = (RKVec *)space->V[p];
        w_pf = (RKVec *)space->W[p];
        s_pf = (RKVec *)space->S[p];
        // Packed single math
        for (k = 0; k < K; k++) {
            // Z:  10 * (previous) + rcr --> Z; =>  Z = 10 * log10(S) + rangeCorrection + ZCal;
#if defined(_rk_mm_log10_pf)
            *z_pf = _rk_mm_add_pf(_rk_mm_mul_pf(ten, _rk_mm_log10_ps(*z_pf), cal));
#else
            *z_pf = _rk_mm_add_pf(_rk_mm_mul_pf(ten, *z_pf), cal);
#endif
            z_pf++;
            // V: V = Va * (previous) --> V; => V = angle(R1)
            *v_pf = _rk_mm_mul_pf(va_pf, *v_pf);
            v_pf++;
            // W: w = S / W
            *w_pf = _rk_mm_div_pf(*s_pf, *w_pf);
            s_pf++;
            w_pf++;
        }
        w = space->W[p];
        for (k = 0; k < gateCount; k++) {
            // W: = (log10f(previous))
            *w = log10f(*w);
            w++;
        }
        w_pf = (RKVec *)space->W[p];
        for (k = 0; k < K; k++) {
            *w_pf = _rk_mm_mul_pf(wa_pf, _rk_mm_sqrt_pf(*w_pf));
            w_pf++;
        }
    }
}

int RKPulsePair(RKScratch *space, RKPulse **input, const uint16_t count) {
    usleep(20 * 1000);

    return 0;

}

int RKPulsePairHop(RKScratch *space, RKPulse **input, const uint16_t count) {
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

    if (space->showNumbers && count < 50 && gateCount < 50) {
        char variable[32];
        char line[4096];
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

    
    //
    //  ACF & CCF to S Z V W D P R K
    //

    RKUpdateZVWInScratchSpace(space, gateCount);

    return count;

}
