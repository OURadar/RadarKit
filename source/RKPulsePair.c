//
//  RKPulsePair.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/31/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulsePair.h>


void RKUpdateRadarProductsInScratchSpace(RKScratch *space, const int gateCount) {
    const RKFloat va = space->velocityFactor;
    const RKFloat wa = space->widthFactor;
    const RKVec va_pf = _rk_mm_set1_pf(va);
    const RKVec wa_pf = _rk_mm_set1_pf(wa);
    const RKVec ten_pf = _rk_mm_set1_pf(10.0f);
    const RKVec one_pf = _rk_mm_set1_pf(1.0f);
    const RKVec tiny_pf = _rk_mm_set1_pf(1.0e-6f);
    const RKVec dcal_pf = _rk_mm_set1_pf(space->dcal);
    RKVec n_pf;
    RKFloat *s;
    RKFloat *z;
    RKFloat *v;
    RKFloat *w;
    RKVec *s_pf;
    RKVec *z_pf;
    RKVec *h_pf;
    RKVec *v_pf;
    RKVec *w_pf;
    RKVec *r_pf;
    RKVec *q_pf;
    RKVec *a_pf;
    RKFloat *ri;
    RKFloat *rq;
    int p, k, K = (gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    // S Z V W
    for (p = 0; p < 2; p++) {
        n_pf = _rk_mm_set1_pf(space->noise[p]);
        s_pf = (RKVec *)space->S[p];
        r_pf = (RKVec *)space->aR[p][0];
        q_pf = (RKVec *)space->aR[p][1];
        w_pf = (RKVec *)space->W[p];
        a_pf = (RKVec *)space->SNR[p];
        // Packed single math
        for (k = 0; k < K; k++) {
            // S: R[0] - N
            *s_pf = _rk_mm_max_pf(tiny_pf, _rk_mm_sub_pf(*r_pf, n_pf));
            // SNR: S / N
            *a_pf = _rk_mm_div_pf(*s_pf, n_pf);
            // W: S / R(1)
            *w_pf = _rk_mm_div_pf(*s_pf, *q_pf);
            s_pf++;
            r_pf++;
            a_pf++;
            w_pf++;
            q_pf++;
        }
        // log10(S) --> Z (temp)
        s = space->S[p];
        z = space->Z[p];
        w = space->W[p];
        v = (RKFloat *)space->V[p];
        ri = (RKFloat *)space->R[p][1].i;
        rq = (RKFloat *)space->R[p][1].q;
        // Regular math, no intrinsic options
        for (k = 0; k < gateCount; k++) {
            // Z: log10(previous) = log10(R[0] - N)
            *z++ = log10f(*s++);
            // V: angle(R[1])
            *v++ = atan2f(*rq++, *ri++);
            // W: ln(previous) = ln(S / R[1])
            *w = logf(*w);
            w++;
        }
        z_pf = (RKVec *)space->Z[p];
        v_pf = (RKVec *)space->V[p];
        w_pf = (RKVec *)space->W[p];
        r_pf = (RKVec *)space->rcor[p];
        // Packed single math
        for (k = 0; k < K; k++) {
            // Z:  10 * (previous) + rcr =  10 * log10(S) + rangeCorrection;
            *z_pf = _rk_mm_add_pf(_rk_mm_mul_pf(ten_pf, *z_pf), *r_pf);
            // V: V = va * (previous) = va * angle(R1)
            *v_pf = _rk_mm_mul_pf(va_pf, *v_pf);
            // W: w = wa * sqrt(previous) = wa * sqrt(log10(S / R[1]))
            *w_pf = _rk_mm_mul_pf(wa_pf, _rk_mm_sqrt_pf(*w_pf));
            z_pf++;
            r_pf++;
            v_pf++;
            w_pf++;
        }
    }
    // D P R K
    z_pf = (RKVec *)space->ZDR;
    r_pf = (RKVec *)space->RhoHV;
    s_pf = (RKVec *)space->aC[0];
    h_pf = (RKVec *)space->SNR[0];
    v_pf = (RKVec *)space->SNR[1];
    a_pf = (RKVec *)space->Z[0];
    w_pf = (RKVec *)space->Z[1];
    for (k = 0; k < K; k++) {
        // D: Zh - Zv + DCal
        *z_pf = _rk_mm_add_pf(_rk_mm_sub_pf(*a_pf, *w_pf), dcal_pf);
        // R: |C[0]| * sqrt((1 + 1 / SNR-h) * (1 + 1 / SNR-v))
        *r_pf = _rk_mm_mul_pf(_rk_mm_add_pf(one_pf, _rk_mm_rcp_pf(*h_pf)), _rk_mm_add_pf(one_pf, _rk_mm_rcp_pf(*v_pf)));
        *r_pf = _rk_mm_mul_pf(*s_pf, _rk_mm_sqrt_pf(*r_pf));
        //*r_pf = *s_pf;

        z_pf++;
        a_pf++;
        w_pf++;
        h_pf++;
        v_pf++;
        r_pf++;
        s_pf++;
    }
    s = space->PhiDP;
    v = space->KDP;
    ri = space->C[0].i;
    rq = space->C[0].q;
    s[0] = atan2f(*rq++, *ri++);
    for (k = 1; k < gateCount; k++) {
        s[k] = atan2f(*rq++, *ri++) + space->pcal;
        if (s[k] < -M_PI) {
            s[k] += 2.0f * M_PI;
        } else if (s[k] >= M_PI) {
            s[k] -= 2.0f * M_PI;
        }
        *v++ = space->KDPFactor * (s[k] - s[k - 1]);
    }
}

int RKPulsePair(RKScratch *space, RKPulse **input, const uint16_t count) {
    usleep(20 * 1000);

    return 0;

}

int RKPulsePairHop(RKScratch *space, RKPulse **pulses, const uint16_t count) {

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

    int n, j, k, p;

    // Get the start pulse to know the capacity
    RKPulse *pulse = pulses[0];
    const uint32_t capacity = pulse->header.capacity;
    const uint32_t gateCount = pulse->header.gateCount;
    const int K = (gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);

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
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], p);
//            RKSIMD_izadd(&Xn, &space->mX[p], pulse->header.gateCount);                   // mX += X
//            RKSIMD_zcma(&Xn, &Xn, &space->R[p][0], pulse->header.gateCount, 1);          // R[0] += X[n] * X[n]'
            RKVec *si = (RKVec *)Xn.i;
            RKVec *sq = (RKVec *)Xn.q;
            RKVec *mi = (RKVec *)space->mX[p].i;
            RKVec *mq = (RKVec *)space->mX[p].q;
            RKVec *r0 = (RKVec *)space->R[p][0].i;
            for (k = 0; k < K; k++) {
                *mi = _rk_mm_add_pf(*mi, *si);
                *mq = _rk_mm_add_pf(*mq, *sq);
                *r0 = _rk_mm_add_pf(*r0, _rk_mm_add_pf(_rk_mm_mul_pf(*si, *si), _rk_mm_mul_pf(*sq, *sq))); // I += I1 * I2 + Q1 * Q2
                si++;
                sq++;
                mi++;
                mq++;
                r0++;
            }
            j++;
        }
        // Divide by n for the average
        //        RKSIMD_izscl(&space->mX[p], 1.0f / (float)(j), gateCount);                       // mX /= j
        //        RKSIMD_izscl(&space->R[p][0], 1.0f / (float)(j), gateCount);                     // R[0] /= j   (unbiased)
        //        RKSIMD_zabs(&space->R[p][0], space->aR[p][0], gateCount);                        // aR[0] = abs(R[0])
        RKVec ss = _rk_mm_set1_pf(1.0f / (float)j);
        RKVec *mi = (RKVec *)space->mX[p].i;
        RKVec *mq = (RKVec *)space->mX[p].q;
        RKVec *si = (RKVec *)space->R[p][0].i;
        RKVec *r0 = (RKVec *)space->aR[p][0];
        for (k = 0; k < K; k++) {
            *mi = _rk_mm_mul_pf(*mi, ss);                                                 // mX /= j
            *mq = _rk_mm_mul_pf(*mq, ss);                                                 // mX /= j
            *si = _rk_mm_mul_pf(*si, ss);                                                 // R[0] /= j
            *r0++ = *si++;                                                                // aR[0] = abs(R[0]) = real(R[0])
            mi++;
            mq++;
        }

        // Now we get the odd pulses for R(1)
        n = 1;
        if (pulse->header.i % 2) {
            // Ignore the first pulse if it is an odd-pulse
            n = 2;
        }
        j = 0;
        for (; n < count; n += 2) {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], p);
            RKIQZ Xk = RKGetSplitComplexDataFromPulse(pulses[n - 1], p);
            RKSIMD_zcma(&Xn, &Xk, &space->R[p][1], pulse->header.gateCount, 1);          // R[k] += X[n] * X[n - k]'
            j++;
        }
        RKSIMD_izscl(&space->R[p][1], 1.0f / (float)(j), gateCount);                     // R[1] /= j   (unbiased)
        RKSIMD_zabs(&space->R[p][1], space->aR[p][1], gateCount);                        // aR[1] = abs(R[1])

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
    Xh = RKGetSplitComplexDataFromPulse(pulses[n], 0);
    Xv = RKGetSplitComplexDataFromPulse(pulses[n], 1);
    RKSIMD_zmul(&Xh, &Xv, &space->C[0], gateCount, 1);                                   // C = Xh * Xv', flag 1 = conjugate
    j = 1;
    n += 2;
    for (; n < count; n += 2) {
        Xh = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        Xv = RKGetSplitComplexDataFromPulse(pulses[n], 1);
        RKSIMD_zcma(&Xh, &Xv, &space->C[0], gateCount, 1);                               // C += Xh[] * Xv[]'
        j++;
    }
    //    RKSIMD_izscl(&space->C[0], 1.0f / (float)(j), gateCount);                            // C /= j   (unbiased)
    RKSIMD_izrmrm(&space->C[0], space->aC[0], space->aR[0][0], space->aR[1][0], 1.0f / (float)(j), gateCount);  // aC = |C| / sqrt(|Rh(0)*Rv(0)|)

    //
    //  ACF & CCF to S Z V W D P R K
    //
    RKUpdateRadarProductsInScratchSpace(space, gateCount);

    if (space->showNumbers && count < 50 && gateCount < 50) {
        char variable[32];
        char line[4096];
        RKIQZ *X = (RKIQZ *)malloc(RKMaxPulsesPerRay * sizeof(RKIQZ));
        const int gateShown = 8;
        
        // Go through both polarizations
        for (p = 0; p < 2; p++) {
            printf("\%sChannel %d (%s pol):%s\n",
                   rkGlobalParameters.showColor ? "\033[4m" : "",
                   p, p == 0 ? "H" : (p == 1 ? "V" : "X"),
                   rkGlobalParameters.showColor ? "\033[24m" : "");
            for (n = 0; n < count; n++) {
                X[n] = RKGetSplitComplexDataFromPulse(pulses[n], p);
            }

            /* A block ready for MATLAB

             - Copy and paste X = [ 0+2j, 0+1j, ...

             Then, all the previous calculations can be extremely easy.

             g = 1; % gate 1
             k = 1; % even is correct, change to 2 for even is 1-off
             mXh = mean(Xh(:, k:2:end), 2).'
             mXv = mean(Xv(:, k:2:end), 2).'
             R0h = mean(Xh(:, k:2:end) .* conj(Xh(:, k:2:end)), 2).'
             R1h = mean(Xh(:, k+1:2:end) .* conj(Xh(:, k:2:end)), 2).'
             R0v = mean(Xv(:, k:2:end) .* conj(Xv(:, k:2:end)), 2).'
             R1v = mean(Xv(:, k+1:2:end) .* conj(Xv(:, k:2:end)), 2).'
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
            sprintf(variable, "  rcor = ");
            RKShowVecFloat(variable, space->rcor[p], gateShown);
            printf(RKEOL);
            sprintf(variable, "    Z%s = ", p == 0 ? "h" : "v");
            RKShowVecFloat(variable, space->Z[p], gateShown);
            printf(RKEOL);
        }
        printf("%sCross-channel:%s\n",
               rkGlobalParameters.showColor ? "\033[4m" : "",
               rkGlobalParameters.showColor ? "\033[24m" : "");
        RKShowVecIQZ("  C[0] = ", &space->C[0], gateShown);                                  // xcorr(Xh, Xv, 'unbiased') in MATLAB
        printf(RKEOL);

        free(X);
    }


    return count;

}
