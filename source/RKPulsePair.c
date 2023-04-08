//
//  RKPulsePair.c
//  RadarKit
//
//  Created by Boonleng Cheong on 10/31/16.
//  Copyright (c) 2016 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKPulsePair.h>

void RKUpdateRadarProductsInScratchSpace(RKScratch *space, const int gateCount) {
    const RKFloat va = space->velocityFactor;
    const RKFloat wa = space->widthFactor;
    const RKFloat ten = 10.0f;
    const RKFloat one = 1.0f;
    const RKFloat zero = 0.0f;
    const RKFloat tiny = 1.0e-6;
    const RKVec va_pf = _rk_mm_set1_pf(va);
    const RKVec wa_pf = _rk_mm_set1_pf(wa);
    const RKVec ten_pf = _rk_mm_set1_pf(ten);
    const RKVec one_pf = _rk_mm_set1_pf(one);
    const RKVec zero_pf = _rk_mm_set1_pf(zero);
    //const RKVec dcal_pf = _rk_mm_set1_pf(space->dcal);
    RKFloat n;
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
    RKVec *p_pf;
    RKVec *a_pf;
    RKVec *d_pf;
    RKFloat *ri;
    RKFloat *rq;
    int p, k, K = (gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    // S Z V W
    for (p = 0; p < 2; p++) {
        n = MAX(tiny, space->noise[p]);
        n_pf = _rk_mm_set1_pf(n);
        s_pf = (RKVec *)space->S[p];
        r_pf = (RKVec *)space->aR[p][0];
        p_pf = (RKVec *)space->aR[p][1];
        w_pf = (RKVec *)space->W[p];
        a_pf = (RKVec *)space->SNR[p];
        q_pf = (RKVec *)space->Q[p];
        // Packed single math
        for (k = 0; k < K; k++) {
            // S: R[0] - N
            *s_pf = _rk_mm_max_pf(zero_pf, _rk_mm_sub_pf(*r_pf, n_pf));
            // SNR: S / N
            *a_pf = _rk_mm_div_pf(*s_pf, n_pf);
            // SQI: |R(1)| / |R(0)|
            *q_pf = _rk_mm_div_pf(*p_pf, *r_pf);
            // W: S / R(1)
            *w_pf = _rk_mm_max_pf(one_pf, _rk_mm_div_pf(*s_pf, *p_pf));
            s_pf++;
            r_pf++;
            a_pf++;
            w_pf++;
            q_pf++;
            p_pf++;
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
            // Z: log10(previous) = log10(S)
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
        r_pf = (RKVec *)space->S2Z[p];
        // Packed single math
        for (k = 0; k < K; k++) {
            // Z:  10 * (previous) + rcr =  10 * log10(S) + rangeCorrection;
            *z_pf = _rk_mm_add_pf(_rk_mm_mul_pf(ten_pf, *z_pf), *r_pf);
            // V: V = va * (previous) = va * angle(R1)
            *v_pf = _rk_mm_mul_pf(va_pf, *v_pf);
            // W: w = wa * sqrt(previous) = wa * sqrt(ln(S / R[1]))
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
    d_pf = (RKVec *)space->dcal;
    for (k = 0; k < K; k++) {
        // D: Zh - Zv + DCal
        //*z_pf = _rk_mm_add_pf(_rk_mm_sub_pf(*a_pf, *w_pf), dcal_pf);
        *z_pf = _rk_mm_add_pf(_rk_mm_sub_pf(*a_pf, *w_pf), *d_pf);
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
        d_pf++;
    }
    s = space->PhiDP;
    v = space->KDP;
    w = space->pcal;
    ri = space->C[0].i;
    rq = space->C[0].q;
    s[0] = atan2f(*rq++, *ri++);
    for (k = 1; k < gateCount; k++) {
        s[k] = atan2f(*rq++, *ri++) + *w++;
        if (s[k] < -M_PI) {
            s[k] += 2.0f * M_PI;
        } else if (s[k] >= M_PI) {
            s[k] -= 2.0f * M_PI;
        }
        *v++ = space->KDPFactor * (s[k] - s[k - 1]);
    }
}

int RKPulsePair(RKScratch *space, RKPulse **pulses, const uint16_t count) {

    //
    // Pulse-pair processing
    //
    //  o o o o o o
    //  | | | | | |
    //  +-+-+-+-+-+--
    //   t t t t t
    //   1 1 1 1 1
    //
    // Properties:
    //   - Reflectvity from all pulses
    //   - Velocity folding due to PRT1
    //

    int n, j, k, p;
    const uint32_t gateCount = space->gateCount;
    const int K = (gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    //printf("gateCount = %d   K = %d\n", gateCount, K);

    const RKFloat zero = 0.0f;
    const RKVec zero_pf = _rk_mm_set1_pf(zero);

    //
    //  ACF
    //

    RKIQZ Xl;
    RKIQZ Xm;
    RKIQZ Xn;
    RKVec *s0i = NULL;
    RKVec *s0q = NULL;
    RKVec *s1i = NULL;
    RKVec *s1q = NULL;
    RKVec *s2i = NULL;
    RKVec *s2q = NULL;
    RKVec *mi = NULL;
    RKVec *mq = NULL;
    RKVec *vi = NULL;
    RKVec *vq = NULL;
    RKVec *r0i = NULL;
    RKVec *r0a = NULL;
    RKVec *r1i = NULL;
    RKVec *r1q = NULL;
    RKVec *r1a = NULL;
    RKVec *r2i = NULL;
    RKVec *r2q = NULL;
    RKVec *r2a = NULL;

    // Go through each polarization
    for (p = 0; p < 2; p++) {

        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], space->capacity);
        RKZeroOutIQZ(&space->vX[p], space->capacity);
        RKZeroOutIQZ(&space->R[p][0], space->capacity);
        RKZeroOutIQZ(&space->R[p][1], space->capacity);
        RKZeroOutIQZ(&space->R[p][2], space->capacity);

        // The first samples
        Xn = RKGetSplitComplexDataFromPulse(pulses[0], p);
        s0i = (RKVec *)Xn.i;
        s0q = (RKVec *)Xn.q;
        mi = (RKVec *)space->mX[p].i;
        mq = (RKVec *)space->mX[p].q;
        r0i = (RKVec *)space->R[p][0].i;
        for (k = 0; k < K; k++) {
            *mi = _rk_mm_add_pf(*mi, *s0i);                                                                      // mX += X
            *mq = _rk_mm_add_pf(*mq, *s0q);                                                                      // mX += X
            *r0i = _rk_mm_add_pf(*r0i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s0i), _rk_mm_mul_pf(*s0q, *s0q)));     // R[0] += X[n] * X[n]'  (I += I1 * I2 + Q1 * Q2)
            s0i++;
            s0q++;
            mi++;
            mq++;
            r0i++;
        }

        // The second samples
        Xm = Xn;
        Xn = RKGetSplitComplexDataFromPulse(pulses[1], p);
        s1i = (RKVec *)Xm.i;
        s1q = (RKVec *)Xm.q;
        s0i = (RKVec *)Xn.i;
        s0q = (RKVec *)Xn.q;
        mi = (RKVec *)space->mX[p].i;
        mq = (RKVec *)space->mX[p].q;
        r0i = (RKVec *)space->R[p][0].i;
        r1i = (RKVec *)space->R[p][1].i;
        r1q = (RKVec *)space->R[p][1].q;
        for (k = 0; k < K; k++) {
            *mi = _rk_mm_add_pf(*mi, *s0i);                                                                      // mX += X
            *mq = _rk_mm_add_pf(*mq, *s0q);                                                                      // mX += X
            *r0i = _rk_mm_add_pf(*r0i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s0i), _rk_mm_mul_pf(*s0q, *s0q)));     // R[0].i += X[n] * X[n]'  (I += I1 * I2 + Q1 * Q2)
            *r1i = _rk_mm_add_pf(*r1i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s1i), _rk_mm_mul_pf(*s0q, *s1q)));     // R[1].i += X[n] * X[n-1]'  (I += I1 * I2 + Q1 * Q2)
            *r1q = _rk_mm_add_pf(*r1q, _rk_mm_sub_pf(_rk_mm_mul_pf(*s0q, *s1i), _rk_mm_mul_pf(*s0i, *s1q)));     // R[1].q += X[n] * X[n-1]'  (Q += Q1 * I2 - I1 * Q2)
            s0i++;
            s0q++;
            s1i++;
            s1q++;
            mi++;
            mq++;
            r0i++;
            r1i++;
            r1q++;
        }

        // The third samples
        Xl = Xm;
        Xm = Xn;
        Xn = RKGetSplitComplexDataFromPulse(pulses[2], p);
        s2i = (RKVec *)Xl.i;
        s2q = (RKVec *)Xl.q;
        s1i = (RKVec *)Xm.i;
        s1q = (RKVec *)Xm.q;
        s0i = (RKVec *)Xn.i;
        s0q = (RKVec *)Xn.q;
        mi = (RKVec *)space->mX[p].i;
        mq = (RKVec *)space->mX[p].q;
        r0i = (RKVec *)space->R[p][0].i;
        r1i = (RKVec *)space->R[p][1].i;
        r1q = (RKVec *)space->R[p][1].q;
        r2i = (RKVec *)space->R[p][2].i;
        r2q = (RKVec *)space->R[p][2].q;
        for (k = 0; k < K; k++) {
            *mi = _rk_mm_add_pf(*mi, *s0i);                                                                      // mX += X
            *mq = _rk_mm_add_pf(*mq, *s0q);                                                                      // mX += X
            *r0i = _rk_mm_add_pf(*r0i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s0i), _rk_mm_mul_pf(*s0q, *s0q)));     // R[0].i += X[n] * X[n]'  (I += I1 * I2 + Q1 * Q2)
            *r1i = _rk_mm_add_pf(*r1i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s1i), _rk_mm_mul_pf(*s0q, *s1q)));     // R[1].i += X[n] * X[n-1]'  (I += I1 * I2 + Q1 * Q2)
            *r1q = _rk_mm_add_pf(*r1q, _rk_mm_sub_pf(_rk_mm_mul_pf(*s0q, *s1i), _rk_mm_mul_pf(*s0i, *s1q)));     // R[1].q += X[n] * X[n-1]'  (Q += Q1 * I2 - I1 * Q2)
            *r2i = _rk_mm_add_pf(*r2i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s2i), _rk_mm_mul_pf(*s0q, *s2q)));     // R[2].i += X[n] * X[n-2]'  (I += I1 * I2 + Q1 * Q2)
            *r2q = _rk_mm_add_pf(*r2q, _rk_mm_sub_pf(_rk_mm_mul_pf(*s0q, *s2i), _rk_mm_mul_pf(*s0i, *s2q)));     // R[2].q += X[n] * X[n-2]'  (Q += Q1 * I2 - I1 * Q2)
            s0i++;
            s0q++;
            s1i++;
            s1q++;
            s2i++;
            s2q++;
            mi++;
            mq++;
            r0i++;
            r1i++;
            r1q++;
            r2i++;
            r2q++;
        }

        // Go through the rest of the pulses for mX and R(0)
        for (n = 3; n < count; n++) {
            Xl = Xm;
            Xm = Xn;
            Xn = RKGetSplitComplexDataFromPulse(pulses[n], p);
            s2i = (RKVec *)Xl.i;
            s2q = (RKVec *)Xl.q;
            s1i = (RKVec *)Xm.i;
            s1q = (RKVec *)Xm.q;
            s0i = (RKVec *)Xn.i;
            s0q = (RKVec *)Xn.q;
            mi = (RKVec *)space->mX[p].i;
            mq = (RKVec *)space->mX[p].q;
            r0i = (RKVec *)space->R[p][0].i;
            r1i = (RKVec *)space->R[p][1].i;
            r1q = (RKVec *)space->R[p][1].q;
            r2i = (RKVec *)space->R[p][2].i;
            r2q = (RKVec *)space->R[p][2].q;
            for (k = 0; k < K; k++) {
                *mi = _rk_mm_add_pf(*mi, *s0i);                                                                  // mX += X
                *mq = _rk_mm_add_pf(*mq, *s0q);                                                                  // mX += X
                *r0i = _rk_mm_add_pf(*r0i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s0i), _rk_mm_mul_pf(*s0q, *s0q))); // R[0].i += X[n] * X[n]'  (I += I1 * I2 + Q1 * Q2)
                *r1i = _rk_mm_add_pf(*r1i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s1i), _rk_mm_mul_pf(*s0q, *s1q))); // R[1].i += X[n] * X[n-1]'  (I += I1 * I2 + Q1 * Q2)
                *r1q = _rk_mm_add_pf(*r1q, _rk_mm_sub_pf(_rk_mm_mul_pf(*s0q, *s1i), _rk_mm_mul_pf(*s0i, *s1q))); // R[1].q += X[n] * X[n-1]'  (Q += Q1 * I2 - I1 * Q2)
                *r2i = _rk_mm_add_pf(*r2i, _rk_mm_add_pf(_rk_mm_mul_pf(*s0i, *s2i), _rk_mm_mul_pf(*s0q, *s2q))); // R[2].i += X[n] * X[n-2]'  (I += I1 * I2 + Q1 * Q2)
                *r2q = _rk_mm_add_pf(*r2q, _rk_mm_sub_pf(_rk_mm_mul_pf(*s0q, *s2i), _rk_mm_mul_pf(*s0i, *s2q))); // R[2].q += X[n] * X[n-2]'  (Q += Q1 * I2 - I1 * Q2)
                s0i++;
                s0q++;
                s1i++;
                s1q++;
                s2i++;
                s2q++;
                mi++;
                mq++;
                r0i++;
                r1i++;
                r1q++;
                r2i++;
                r2q++;
            }
        }
        // Divide by n for the average
        const float rc0 = 1.0f / (float)count;
        const float rc1 = 1.0f / (float)(count - 1);
        const float rc2 = 1.0f / (float)(count - 2);
        RKVec n0 = _rk_mm_set1_pf(rc0);
        RKVec n1 = _rk_mm_set1_pf(rc1);
        RKVec n2 = _rk_mm_set1_pf(rc2);
        mi = (RKVec *)space->mX[p].i;
        mq = (RKVec *)space->mX[p].q;
        r0i = (RKVec *)space->R[p][0].i;
        r0a = (RKVec *)space->aR[p][0];
        r1i = (RKVec *)space->R[p][1].i;
        r1q = (RKVec *)space->R[p][1].q;
        r1a = (RKVec *)space->aR[p][1];
        r2i = (RKVec *)space->R[p][2].i;
        r2q = (RKVec *)space->R[p][2].q;
        r2a = (RKVec *)space->aR[p][2];
        for (k = 0; k < K; k++) {
            *mi = _rk_mm_mul_pf(*mi, n0);                                                                        // mX /= n
            *mq = _rk_mm_mul_pf(*mq, n0);                                                                        // mX /= n
            *r0i = _rk_mm_mul_pf(*r0i, n0);                                                                      // R[0] /= n
            *r0a = *r0i;                                                                                         // aR[0] = abs(R[0]) = real(R[0])
            *r1i = _rk_mm_mul_pf(*r1i, n1);                                                                      // R[1].i /= (n - 1)
            *r1q = _rk_mm_mul_pf(*r1q, n1);                                                                      // R[1].q /= (n - 1)
            *r1a = _rk_mm_sqrt_pf(_rk_mm_add_pf(_rk_mm_mul_pf(*r1i, *r1i), _rk_mm_mul_pf(*r1q, *r1q)));          // aR[1] = sqrt(R[1].i ^ 2 + R[1].q ^ 2)
            *r2i = _rk_mm_mul_pf(*r2i, n2);                                                                      // R[1].i /= (n - 2)
            *r2q = _rk_mm_mul_pf(*r2q, n2);                                                                      // R[1].q /= (n - 2)
            *r2a = _rk_mm_sqrt_pf(_rk_mm_add_pf(_rk_mm_mul_pf(*r2i, *r2i), _rk_mm_mul_pf(*r2q, *r2q)));          // aR[2] = sqrt(R[2].i ^ 2 + R[2].q ^ 2)
            mi++;
            mq++;
            r0i++;
            r0a++;
            r1i++;
            r1q++;
            r1a++;
            r2i++;
            r2q++;
            r2a++;
        }

        // Variance (2nd moment)
        mi = (RKVec *)space->mX[p].i;
        mq = (RKVec *)space->mX[p].q;
        vi = (RKVec *)space->vX[p].i;
        vq = (RKVec *)space->vX[p].q;
        r0a = (RKVec *)space->aR[p][0];
        for (k = 0; k < K; k++) {
            *vi = _rk_mm_sub_pf(*r0a, _rk_mm_add_pf(_rk_mm_mul_pf(*mi, *mi), _rk_mm_mul_pf(*mq, *mq)));
            *vq = zero_pf;
            mi++;
            mq++;
            vi++;
            vq++;
            r0a++;
        }
    }

    // Cross-channel
    RKZeroOutIQZ(&space->C[0], space->capacity);

    //
    //  CCF
    //

    RKIQZ Xh, Xv;

    for (n = 0; n < count; n++) {
        Xh = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        Xv = RKGetSplitComplexDataFromPulse(pulses[n], 1);
        RKSIMD_zcma(&Xh, &Xv, &space->C[0], gateCount, 1);                                                       // C += Xh[] * Xv[]'
    }
    RKSIMD_izrmrm(&space->C[0], space->aC[0], space->aR[0][0],
                  space->aR[1][0], 1.0f / (float)(count), gateCount);                                            // aC = |C| / sqrt(|Rh(0)*Rv(0)|)

    // Mark the calculated moments
    space->calculatedMoments = RKMomentListHm
                             | RKMomentListVm
                             | RKMomentListHR0
                             | RKMomentListVR0
                             | RKMomentListHR1
                             | RKMomentListVR1
                             | RKMomentListHR2
                             | RKMomentListVR2
                             | RKMomentListC0;

    //
    //  ACF & CCF to S Z V W D P R K
    //
    RKUpdateRadarProductsInScratchSpace(space, gateCount);

    if (space->showNumbers && count < 50 && gateCount < 50) {
        char variable[RKNameLength];
        char line[RKMaximumStringLength];
        RKIQZ *X = (RKIQZ *)malloc(RKMaximumPulsesPerRay * sizeof(RKIQZ));
        const int gateShown = 8;

        // Go through both polarizations
        for (p = 0; p < 2; p++) {
            printf((rkGlobalParameters.showColor ?
                    UNDERLINE("Channel %d (%s pol):") "\n" :
                    "Channel %d (%s pol):\n"),
                   p, p == 0 ? "H" : (p == 1 ? "V" : "X"));
            for (n = 0; n < count; n++) {
                X[n] = RKGetSplitComplexDataFromPulse(pulses[n], p);
            }

            /* A block ready for MATLAB

             - Copy and paste X = [ 0+2j, 0+1j, ...

             Then, all the previous calculations can be extremely easy.

             g = 1; % gate 1
             mXh = mean(Xh, 2).'
             mXv = mean(Xv, 2).'
             R0h = mean(Xh .* conj(Xh), 2).'
             R1h = mean(Xh(:, 2:end) .* conj(Xh(:, 1:end-1)), 2).'
             R2h = mean(Xh(:, 3:end) .* conj(Xh(:, 1:end-2)), 2).'
             R0v = mean(Xv .* conj(Xv), 2).'
             R1v = mean(Xv(:, 2:end) .* conj(Xv(:, 1:end-1)), 2).'
             R2v = mean(Xv(:, 3:end) .* conj(Xv(:, 1:end-2)), 2).'
             vXh = R0h - mXh .* conj(mXh)
             vXv = R0v - mXv .* conj(mXv)
             for g = 1:6, C(g) = xcorr(Xh(g, :), Xv(g, :), 0, 'unbiased'); end; disp(C)

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
            RKShowVecIQZ("    mX = ", &space->mX[p], gateShown);                                                 // mean(X) in MATLAB
            RKShowVecIQZ("    vX = ", &space->vX[p], gateShown);                                                 // var(X, 1) in MATLAB
            printf(RKEOL);
            for (k = 0; k < 3; k++) {
                sprintf(variable, "  R[%d] = ", k);
                RKShowVecIQZ(variable, &space->R[p][k], gateShown);
            }
            printf(RKEOL);
            for (k = 0; k < 3; k++) {
                sprintf(variable, " aR[%d] = ", k);
                RKShowVecFloat(variable, space->aR[p][k], gateShown);
            }
            printf(RKEOL);
            sprintf(variable, "  S2Z = ");
            RKShowVecFloat(variable, space->S2Z[p], gateShown);
            printf(RKEOL);
            sprintf(variable, "    Z%s = ", p == 0 ? "h" : "v");
            RKShowVecFloat(variable, space->Z[p], gateShown);
            printf(RKEOL);
        }
        printf(rkGlobalParameters.showColor ? UNDERLINE("Cross-channel:") "\n" : "Cross-channel:\n");
        RKShowVecIQZ("  C[0] = ", &space->C[0], gateShown);                                                      // xcorr(Xh, Xv, 'unbiased') in MATLAB
        printf(RKEOL);
        RKShowVecFloat("   ZDR = ", space->ZDR, gateShown);
        RKShowVecFloat(" PhiDP = ", space->PhiDP, gateShown);
        RKShowVecFloat(" RhoHV = ", space->RhoHV, gateShown);

        printf(RKEOL);
        fflush(stdout);

        free(X);
    }

    return count;
}

int RKPulsePairStaggeredPRT(RKScratch *space, RKPulse **pulses, const uint16_t count) {

    //
    // Staggered PRT processing
    //
    //  o o   o o   o o
    //  | |   | |   | |
    //  +-+---+-+---+-+---
    //   t  t  t  t  t
    //   1  2  1  2  1
    //
    // Properties:
    //   - Reflectivity from odd pulses
    //   - Velocity from PRT1 + PRT2
    //       - Unfold using ratio
    //   - Spectrum width?
    //   - Polarimetric variables from even pulses
    //
    // Important tasks:
    //   - Calculate ACF (prt 1 & prt 2)
    //

    // Mark the calculated moments
    space->calculatedMoments = RKMomentListHm
                             | RKMomentListVm
                             | RKMomentListHR0
                             | RKMomentListVR0
                             | RKMomentListHR1
                             | RKMomentListVR1
                             | RKMomentListC0;

    return 0;
}

int RKPulsePairHop(RKScratch *space, RKPulse **pulses, const uint16_t count) {

    //
    // Frequency Hopping processing
    //
    //  f f   f f   f f
    //  1 1   2 2   3 3
    //
    //  o o   o o   o o
    //  | |   | |   | |
    //  +-+---+-+---+-+---
    //
    // Properties:
    //   - Reflectivity from odd pulses
    //   - Velocity from PRT1 + PRT2
    //       - Unfold using ratio
    //   - Spectrum width?
    //   - Polarimetric variables from even pulses
    //
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

        // Go through the even pulses for mX and R(0)
        n = 0;
        if (pulse->header.i % 2) {
            // Ignore the first pulse if it is an odd-pulse
            n = 1;
        }
        j = 0;
        for (; n < count; n += 2) {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], p);
            RKVec *si = (RKVec *)Xn.i;
            RKVec *sq = (RKVec *)Xn.q;
            RKVec *mi = (RKVec *)space->mX[p].i;
            RKVec *mq = (RKVec *)space->mX[p].q;
            RKVec *r0 = (RKVec *)space->R[p][0].i;
            for (k = 0; k < K; k++) {
                *mi = _rk_mm_add_pf(*mi, *si);                                                                   // mX += X
                *mq = _rk_mm_add_pf(*mq, *sq);                                                                   // mX += X
                *r0 = _rk_mm_add_pf(*r0, _rk_mm_add_pf(_rk_mm_mul_pf(*si, *si), _rk_mm_mul_pf(*sq, *sq)));       // R[0] += X[n] * X[n]'  (I += I1 * I2 + Q1 * Q2)
                si++;
                sq++;
                mi++;
                mq++;
                r0++;
            }
            j++;
        }
        // Divide by n for the average
        RKFloat c = 1.0f / (float)j;
        RKVec ss = _rk_mm_set1_pf(c);
        RKVec *mi = (RKVec *)space->mX[p].i;
        RKVec *mq = (RKVec *)space->mX[p].q;
        RKVec *si = (RKVec *)space->R[p][0].i;
        RKVec *r0 = (RKVec *)space->aR[p][0];
        for (k = 0; k < K; k++) {
            *mi = _rk_mm_mul_pf(*mi, ss);                                                                        // mX /= j
            *mq = _rk_mm_mul_pf(*mq, ss);                                                                        // mX /= j
            *si = _rk_mm_mul_pf(*si, ss);                                                                        // R[0] /= j
            *r0++ = *si++;                                                                                       // aR[0] = abs(R[0]) = real(R[0])
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
            RKSIMD_zcma(&Xn, &Xk, &space->R[p][1], gateCount, 1);                                                // R[k] += X[n] * X[n - k]'
            j++;
        }
        RKSIMD_izscl(&space->R[p][1], 1.0f / (float)(j), gateCount);                                             // R[1] /= j   (unbiased)
        RKSIMD_zabs(&space->R[p][1], space->aR[p][1], gateCount);                                                // aR[1] = abs(R[1])

        // Mean and variance (2nd moment)
        RKSIMD_zsmul(&space->mX[p], &space->vX[p], gateCount, 1);                                                // E{Xh} * E{Xh}' --> var  (step 1)
        RKSIMD_izsub(&space->R[p][0], &space->vX[p], gateCount);                                                 // Rh[] - var     --> var  (step 2)
    }

    // Cross-channel
    RKZeroOutIQZ(&space->C[0], gateCount);

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
    RKSIMD_zmul(&Xh, &Xv, &space->C[0], gateCount, 1);                                                           // C = Xh * Xv', flag 1 = conjugate
    j = 1;
    n += 2;
    for (; n < count; n += 2) {
        Xh = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        Xv = RKGetSplitComplexDataFromPulse(pulses[n], 1);
        RKSIMD_zcma(&Xh, &Xv, &space->C[0], gateCount, 1);                                                       // C += Xh[] * Xv[]'
        j++;
    }
    RKSIMD_izrmrm(&space->C[0], space->aC[0], space->aR[0][0],
                  space->aR[1][0], 1.0f / (float)(j), gateCount);                                                // aC = |C| / sqrt(|Rh(0)*Rv(0)|)

    // Mark the calculated moments
    space->calculatedMoments = RKMomentListHm
                             | RKMomentListVm
                             | RKMomentListHR0
                             | RKMomentListVR0
                             | RKMomentListHR1
                             | RKMomentListVR1
                             | RKMomentListC0;

    //
    //  ACF & CCF to S Z V W D P R K
    //
    RKUpdateRadarProductsInScratchSpace(space, gateCount);

    if (space->showNumbers && count < 50 && gateCount < 50) {
        char variable[RKNameLength];
        char line[RKMaximumStringLength];
        RKIQZ *X = (RKIQZ *)malloc(RKMaximumPulsesPerRay * sizeof(RKIQZ));
        const int gateShown = 8;

        // Go through both polarizations
        for (p = 0; p < 2; p++) {
            printf((
                    rkGlobalParameters.showColor ?
                    UNDERLINE("Channel %d (%s pol):") "\n" :
                    "Channel %d (%s pol):\n"),
                   p, p == 0 ? "H" : (p == 1 ? "V" : "X"));
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
            RKShowVecIQZ("    mX = ", &space->mX[p], gateShown);                                                 // mean(X) in MATLAB
            RKShowVecIQZ("    vX = ", &space->vX[p], gateShown);                                                 // var(X, 1) in MATLAB
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
            sprintf(variable, "  S2Z = ");
            RKShowVecFloat(variable, space->S2Z[p], gateShown);
            printf(RKEOL);
            sprintf(variable, "    Z%s = ", p == 0 ? "h" : "v");
            RKShowVecFloat(variable, space->Z[p], gateShown);
            printf(RKEOL);
        }
        printf(rkGlobalParameters.showColor ? UNDERLINE("Cross-channel:") "\n" : "Cross-channel:\n");
        RKShowVecIQZ("  C[0] = ", &space->C[0], gateShown);                                                      // xcorr(Xh, Xv, 'unbiased') in MATLAB
        printf(RKEOL);
        RKShowVecFloat("   ZDR = ", space->ZDR, gateShown);
        RKShowVecFloat(" PhiDP = ", space->PhiDP, gateShown);
        RKShowVecFloat(" RhoHV = ", space->RhoHV, gateShown);

        printf(RKEOL);
        fflush(stdout);

        free(X);
    }

    return count;
}
