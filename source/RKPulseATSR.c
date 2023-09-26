//
//  RKPulseATSR.c
//  RadarKit
//
//  Created by Min-Duan Tzeng on 9/14/23.
//  Copyright (c) 2023 Min-Duan Tzeng. All rights reserved.
//

#include <RadarKit/RKPulseATSR.h>

void RKUpdateATSRProductsInScratchSpace(RKMomentScratch *space, const int gateCount) {
    //  ACF & CCF to L RX PX
    const RKFloat va = space->velocityFactor;
    const RKFloat wa = space->widthFactor;
    const RKFloat ten = 10.0f;
    const RKFloat one = 1.0f;
    const RKFloat two = 2.0f;
    const RKFloat zero = 0.0f;
    const RKFloat tiny = 1.0e-6;
    const RKVec va_pf = _rk_mm_set1(va);
    const RKVec wa_pf = _rk_mm_set1(wa);
    const RKVec ten_pf = _rk_mm_set1(ten);
    const RKVec one_pf = _rk_mm_set1(one);
    const RKVec two_pf = _rk_mm_set1(two);
    const RKVec zero_pf = _rk_mm_set1(zero);
    //const RKVec dcal_pf = _rk_mm_set1(space->dcal);
    RKFloat n;
    RKVec n_pf;
    RKFloat *s;
    RKFloat *z;
    RKFloat *v;
    RKFloat *w;
    RKFloat *l;
    RKFloat *x;
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
    RKVec *l_pf;
    RKFloat *ri;
    RKFloat *rq;
    int p, k, K = (gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    for (p = 0; p < 2; p++) {
        n = MAX(tiny, space->noise[p]);
        n_pf = _rk_mm_set1(n);
        s_pf = (RKVec *)space->S[p];
        r_pf = (RKVec *)space->aR[p][0];
        p_pf = (RKVec *)space->aR[p][1];
        w_pf = (RKVec *)space->W[p];
        a_pf = (RKVec *)space->SNR[p];
        q_pf = (RKVec *)space->Q[p];
        // Packed single math
        for (k = 0; k < K; k++) {
            // S: R[0] - N
            *s_pf = _rk_mm_max(zero_pf, _rk_mm_sub(*r_pf, n_pf));
            // SNR: S / N
            *a_pf = _rk_mm_div(*s_pf, n_pf);
            // SQI: |R(1)| / |R(0)|
            *q_pf = _rk_mm_div(*p_pf, *r_pf);
            // W: S / R(1)
            *w_pf = _rk_mm_max(one_pf, _rk_mm_div(*s_pf, *p_pf));
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
        x = space->aRX[p][0];
        if (p ==0){
            l = space->L[1];
        } else {
            l = space->L[0];
        }
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
            *l++ = log10f(*x++);
            w++;
        }
    }
    for (p = 0; p < 2; p++) {
        z_pf = (RKVec *)space->Z[p];
        v_pf = (RKVec *)space->V[p];
        w_pf = (RKVec *)space->W[p];
        r_pf = (RKVec *)space->S2Z[p];
        l_pf = (RKVec *)space->L[p];
        // Packed single math
        for (k = 0; k < K; k++) {
            // Z:  10 * (previous) + rcr =  10 * log10(S) + rangeCorrection;
            *z_pf = _rk_mm_add(_rk_mm_mul(ten_pf, *z_pf), *r_pf);
            // V: V = va * (previous) = va * angle(R1)
            *v_pf = _rk_mm_mul(va_pf, *v_pf);
            // W: w = wa * sqrt(previous) = wa * sqrt(ln(S / R[1]))
            *w_pf = _rk_mm_mul(wa_pf, _rk_mm_sqrt(*w_pf));
            // L(num): 10 * (previous) + rcr =  10 * log10(S) + rangeCorrection;
            *l_pf = _rk_mm_add(_rk_mm_mul(ten_pf, *l_pf), *r_pf);
            z_pf++;
            r_pf++;
            v_pf++;
            w_pf++;
            l_pf++;
        }
    }

    // L[H], L[V], RhoXP[H], RhoXP[V]
    for (p = 0; p < 2; p++) {
        p_pf = (RKVec *)space->L[p];
        q_pf = (RKVec *)space->Z[p];
        if (p ==0){
            h_pf = (RKVec *)space->aR[0][0];
            v_pf = (RKVec *)space->aRX[1][0];
        } else {
            h_pf = (RKVec *)space->aRX[0][0];
            v_pf = (RKVec *)space->aR[1][0];
        }
        w_pf = (RKVec *)space->aCX[p][0];
        r_pf = (RKVec *)space->RhoXP[p];
        for (k = 0; k < K; k++) {
            // L: Zcx - Zco
            *p_pf = _rk_mm_sub(*p_pf, *q_pf);
            // R: |C[0]| * sqrt( R[H][0] * R[V][0])
            *r_pf = _rk_mm_rcp(_rk_mm_sqrt(_rk_mm_mul(*h_pf, *v_pf)));
            *r_pf = _rk_mm_mul(*w_pf, *r_pf);
            //*r_pf = *s_pf;
            p_pf++;
            q_pf++;
            r_pf++;
            h_pf++;
            v_pf++;
            w_pf++;
        }
    }
    // PhiXP[H], PhiXP[V]
    for (p = 0; p < 2; p++) {
        s = space->PhiXP[p];
        ri = (RKFloat *)space->CX[p][0].i;
        rq = (RKFloat *)space->CX[p][0].q;
        s[0] = atan2f(*rq++, *ri++);
        for (k = 1; k < gateCount; k++) {
            s[k] = atan2f(*rq++, *ri++);
            if (s[k] < -M_PI) {
                s[k] += 2.0f * M_PI;
            } else if (s[k] >= M_PI) {
                s[k] -= 2.0f * M_PI;
            }
        }
    }

    // D P R K
    z_pf = (RKVec *)space->ZDR;
    r_pf = (RKVec *)space->RhoHV;
    s_pf = (RKVec *)space->aC[0];
    l_pf = (RKVec *)space->aC[1];
    h_pf = (RKVec *)space->aR[0][0];
    v_pf = (RKVec *)space->aR[1][0];
    a_pf = (RKVec *)space->Z[0];
    w_pf = (RKVec *)space->Z[1];
    d_pf = (RKVec *)space->dcal;
    for (k = 0; k < K; k++) {
        // D: Zh - Zv + DCal
        //*z_pf = _rk_mm_add(_rk_mm_sub(*a_pf, *w_pf), dcal_pf);
        *z_pf = _rk_mm_add(_rk_mm_sub(*a_pf, *w_pf), *d_pf);
        // R: |C[0]| * sqrt( R[H][0] * R[V][0])
        *r_pf = _rk_mm_rcp(_rk_mm_sqrt(_rk_mm_mul(*h_pf, *v_pf)));
        *r_pf = _rk_mm_mul(_rk_mm_div(_rk_mm_add(*s_pf, *l_pf), two_pf), *r_pf);
        //*r_pf = *s_pf;
        z_pf++;
        r_pf++;
        s_pf++;
        l_pf++;
        h_pf++;
        v_pf++;
        a_pf++;
        w_pf++;
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

int RKPulseATSR(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount) {

    //
    // alternative transmit and simultaneous receive processing
    //
    //  h  v  h  v  h  v
    //  1  2  1  2  1  2
    //
    //  o  o  o  o  o  o
    //  |  |  |  |  |  |
    //  +--+--+--+--+--+-
    //
    // Properties:
    //   - ACF
    //   - R[0][lag] hc (h-copolar) from odd pulses H channel receive
    //   - R[1][lag] vc (v-copolar) from even pulses V channel receive
    //   - RX[0][lag] hx (h-cross-polar) from even pulses H channel receive
    //   - RX[1][lag] vx (v-cross-polar) from odd pulses V channel receive
    //
    //   - CCF
    //   - C[0] Ca               hc[n] * vc[n+1]'
    //   - C[1] Cb               hc[n] * vc[n-1]'
    //   - CX[0][lag] hcvx       hc[] * vx[]'
    //   - CX[1][lag] vchx       vc[] * hx[]'

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

    int n, j, k, p;

    // const uint32_t gateCount = space->gateCount;
    // const int K = (gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);

    RKIQZ Xn;
    RKIQZ Xm;
    RKIQZ Xk;

    //
    //  ACF (R,RX), CCF (hcvx, vchx)
    //

    // Get the start pulse
    RKPulse *pulse = pulses[0];
    const uint32_t gateCount = pulse->header.downSampledGateCount;

    // Go through each polarization
    for (p = 0; p < 2; p++) {

        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], gateCount);
        RKZeroOutIQZ(&space->R[p][0], gateCount);
        RKZeroOutIQZ(&space->R[p][1], gateCount);
        RKZeroOutIQZ(&space->RX[p][0], gateCount);
        RKZeroOutIQZ(&space->RX[p][1], gateCount);
    }
    // Go through the odd pulses for hc and vx
    n = 0;
    if (!(pulse->header.i % 2)) {
        // Ignore the first pulse if it is an even-pulse
        n = 1;
    }

    Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);            //hc
    RKSIMD_zcma(&Xn, &Xn, &space->R[0][0], gateCount, 1);
    Xm = RKGetSplitComplexDataFromPulse(pulses[n], 1);            //vx
    RKSIMD_zcma(&Xm, &Xm, &space->RX[1][0], gateCount, 1);

    RKSIMD_zcma(&Xn, &Xm, &space->CX[0][0], gateCount, 1);

    n += 2;
    j = 1;
    for (; n < pulseCount; n += 2) {
        Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        Xk = RKGetSplitComplexDataFromPulse(pulses[n - 2], 0);
        RKSIMD_zcma(&Xn, &Xn, &space->R[0][0], gateCount, 1);                                                // R[0] += X[n] * X[n]'
        RKSIMD_zcma(&Xn, &Xk, &space->R[0][1], gateCount, 1);                                                // R[k] += X[n] * X[n- k]'
        Xm = RKGetSplitComplexDataFromPulse(pulses[n], 1);
        Xk = RKGetSplitComplexDataFromPulse(pulses[n - 2], 1);
        RKSIMD_zcma(&Xm, &Xm, &space->RX[1][0], gateCount, 1);
        RKSIMD_zcma(&Xm, &Xk, &space->RX[1][1], gateCount, 1);

        RKSIMD_zcma(&Xn, &Xm, &space->CX[0][0], gateCount, 1);
        j++;
    }
    RKSIMD_izscl(&space->R[0][0], 1.0f / (float)(j), gateCount);                                             // R[0] /= j   (unbiased)
    RKSIMD_izscl(&space->RX[1][0], 1.0f / (float)(j), gateCount);
    RKSIMD_izscl(&space->R[0][1], 1.0f / (float)(j-1), gateCount);                                           // R[1] /= (j-1)   (unbiased)
    RKSIMD_izscl(&space->RX[1][1], 1.0f / (float)(j-1), gateCount);

    RKSIMD_izscl(&space->CX[0][0], 1.0f / (float)(j), gateCount);

    RKSIMD_zabs(&space->R[0][0], space->aR[0][0], gateCount);
    RKSIMD_zabs(&space->RX[1][0], space->aRX[1][0], gateCount);
    RKSIMD_zabs(&space->R[0][1], space->aR[0][1], gateCount);
    RKSIMD_zabs(&space->RX[1][1], space->aRX[1][1], gateCount);

    RKSIMD_zabs(&space->CX[0][0], space->aCX[0][0], gateCount);

    // Go through the even pulses for vc and hx
    n = 1;
    if (!(pulse->header.i % 2)) {
        // Take first pulse if it is an even-pulse
        n = 0;
    }

    Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);            //hx
    RKSIMD_zcma(&Xn, &Xn, &space->RX[0][0], gateCount, 1);
    Xm = RKGetSplitComplexDataFromPulse(pulses[n], 1);            //vc
    RKSIMD_zcma(&Xm, &Xm, &space->R[1][0], gateCount, 1);

    RKSIMD_zcma(&Xm, &Xn, &space->CX[1][0], gateCount, 1);

    n += 2;
    j = 1;
    for (; n < pulseCount; n += 2) {
        Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        Xk = RKGetSplitComplexDataFromPulse(pulses[n - 2], 0);
        RKSIMD_zcma(&Xn, &Xn, &space->RX[0][0], gateCount, 1);                                                // R[0] += X[n] * X[n]'
        RKSIMD_zcma(&Xn, &Xk, &space->RX[0][1], gateCount, 1);                                                // R[k] += X[n] * X[n- k]'
        Xm = RKGetSplitComplexDataFromPulse(pulses[n], 1);
        Xk = RKGetSplitComplexDataFromPulse(pulses[n - 2], 1);
        RKSIMD_zcma(&Xm, &Xm, &space->R[1][0], gateCount, 1);
        RKSIMD_zcma(&Xm, &Xk, &space->R[1][1], gateCount, 1);

        RKSIMD_zcma(&Xm, &Xn, &space->CX[1][0], gateCount, 1);
        j++;
    }
    RKSIMD_izscl(&space->RX[0][0], 1.0f / (float)(j), gateCount);                                             // R[0] /= j   (unbiased)
    RKSIMD_izscl(&space->R[1][0], 1.0f / (float)(j), gateCount);
    RKSIMD_izscl(&space->RX[0][1], 1.0f / (float)(j-1), gateCount);                                           // R[1] /= (j-1)   (unbiased)
    RKSIMD_izscl(&space->R[1][1], 1.0f / (float)(j-1), gateCount);

    RKSIMD_izscl(&space->CX[1][0], 1.0f / (float)(j), gateCount);

    RKSIMD_zabs(&space->RX[0][0], space->aRX[0][0], gateCount);
    RKSIMD_zabs(&space->R[1][0], space->aR[1][0], gateCount);
    RKSIMD_zabs(&space->RX[0][1], space->aRX[0][1], gateCount);
    RKSIMD_zabs(&space->R[1][1], space->aR[1][1], gateCount);

    RKSIMD_zabs(&space->CX[1][0], space->aCX[1][0], gateCount);

    //
    //  CCF ( hcvc [ Ca & Cb ] )
    //

    n = 0;
    if (!(pulse->header.i % 2)) {
        // Ignore the first pulse if it is an even-pulse
        n = 1;
    }

    j = 0;
    for (; n < pulseCount-1; n += 2) {
        Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        Xk = RKGetSplitComplexDataFromPulse(pulses[n + 1], 1);
        RKSIMD_zcma(&Xn, &Xk, &space->C[0], gateCount, 1);                                                  // Ca = C[0] += H[n] * V[n+1]'
        j++;
    }
    RKSIMD_izscl(&space->C[0], 1.0f / (float)(j), gateCount);                                             // Ca /= j   (unbiased)
    RKSIMD_zabs(&space->C[0], space->aC[0], gateCount);

    n = 2;
    if (!(pulse->header.i % 2)) {
        // Ignore the first pulse if it is an even-pulse
        n = 1;
    }
    j = 0;
    for (; n < pulseCount; n += 2) {
        Xn = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        Xk = RKGetSplitComplexDataFromPulse(pulses[n - 1], 1);
        RKSIMD_zcma(&Xn, &Xk, &space->C[1], gateCount, 1);                                                  // Cb = C[0] += H[n] * V[n-1]'
        j++;
    }
    RKSIMD_izscl(&space->C[1], 1.0f / (float)(j), gateCount);                                             // Cb /= j   (unbiased)
    RKSIMD_zabs(&space->C[1], space->aC[1], gateCount);

    // Mark the calculated moments
    space->calculatedMoments = RKMomentListHR0
                             | RKMomentListVR0
                             | RKMomentListHR1
                             | RKMomentListVR1
                             | RKMomentListCa0
                             | RKMomentListCb0
                             | RKMomentListChcvx0
                             | RKMomentListCvchx0;

    //
    //  ACF & CCF to moments
    //
    RKUpdateATSRProductsInScratchSpace(space, gateCount);
    return pulseCount;
}