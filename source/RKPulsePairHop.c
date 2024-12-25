#include <RadarKit/RKPulsePairHop.h>

void RKUpdateRadarProductsInScratchSpace(RKMomentScratch *space, const int gateCount);

int RKPulsePairHop(RKMomentScratch *space, RKPulse **pulses, const uint16_t count) {

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
                *mi = _rk_mm_add(*mi, *si);                                                        // mX += X
                *mq = _rk_mm_add(*mq, *sq);                                                        // mX += X
                *r0 = _rk_mm_add(*r0, _rk_mm_add(_rk_mm_mul(*si, *si), _rk_mm_mul(*sq, *sq)));     // R[0] += X[n] * X[n]'  (I += I1 * I2 + Q1 * Q2)
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
        RKVec ss = _rk_mm_set1(c);
        RKVec *mi = (RKVec *)space->mX[p].i;
        RKVec *mq = (RKVec *)space->mX[p].q;
        RKVec *si = (RKVec *)space->R[p][0].i;
        RKVec *r0 = (RKVec *)space->aR[p][0];
        for (k = 0; k < K; k++) {
            *mi = _rk_mm_mul(*mi, ss);                                                             // mX /= j
            *mq = _rk_mm_mul(*mq, ss);                                                             // mX /= j
            *si = _rk_mm_mul(*si, ss);                                                             // R[0] /= j
            *r0++ = *si++;                                                                         // aR[0] = abs(R[0]) = real(R[0])
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
            RKSIMD_zcma(&Xn, &Xk, &space->R[p][1], gateCount, 1);                                  // R[k] += X[n] * X[n - k]'
            j++;
        }
        RKSIMD_izscl(&space->R[p][1], 1.0f / (float)(j), gateCount);                               // R[1] /= j   (unbiased)
        RKSIMD_zabs(&space->R[p][1], space->aR[p][1], gateCount);                                  // aR[1] = abs(R[1])

        // Mean and variance (2nd moment)
        RKSIMD_zsmul(&space->mX[p], &space->vX[p], gateCount, 1);                                  // E{Xh} * E{Xh}' --> var  (step 1)
        RKSIMD_izsub(&space->R[p][0], &space->vX[p], gateCount);                                   // Rh[] - var     --> var  (step 2)
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
    RKSIMD_zmul(&Xh, &Xv, &space->C[0], gateCount, 1);                                             // C = Xh * Xv', flag 1 = conjugate
    j = 1;
    n += 2;
    for (; n < count; n += 2) {
        Xh = RKGetSplitComplexDataFromPulse(pulses[n], 0);
        Xv = RKGetSplitComplexDataFromPulse(pulses[n], 1);
        RKSIMD_zcma(&Xh, &Xv, &space->C[0], gateCount, 1);                                         // C += Xh[] * Xv[]'
        j++;
    }
    RKSIMD_izrmrm(&space->C[0], space->aC[0], space->aR[0][0],
                  space->aR[1][0], 1.0f / (float)(j), gateCount);                                  // aC = |C| / sqrt(|Rh(0)*Rv(0)|)

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

    space->calculatedProducts = RKProductListFloatZVWDPR;

    if (space->verbose && count < 50 && gateCount < 50) {
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
            RKShowVecIQZ("    mX = ", &space->mX[p], gateShown);                                   // mean(X) in MATLAB
            RKShowVecIQZ("    vX = ", &space->vX[p], gateShown);                                   // var(X, 1) in MATLAB
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
        RKShowVecIQZ("  C[0] = ", &space->C[0], gateShown);                                        // xcorr(Xh, Xv, 'unbiased') in MATLAB
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
