//
//  RKSpectralMoment.c
//  RadarKit
//
//  Created by Min-Duan Tzneg on 11/2/23.
//  Copyright (c) 2023- Min-Duan Tzneg. All rights reserved.
//

#include <RadarKit/RKSpectralMoment.h>

//
// NOTE: This function is incomplete
//
int RKSpectralMoment(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount) {

    int g, j, k, p;

    RKPulse *pulse;

    // Always choose an order that is slightly higher
    int offt = MIN(space->fftModule->count, (int)ceilf(log2f((float)pulseCount * 1.2f)));
    int planSize = space->fftModule->plans[offt].size;

    //RKLog("%s -> %s",
    //      RKVariableInString("offt", &offt, RKValueTypeInt),
    //      RKVariableInString("planSize", &planSize, RKValueTypeInt));

    fftwf_complex *in, *Xh, *Xv;
    RKFloat A, phi, q, omegaI, omegaQ, omegasqI, omegasqQ, gA;
    RKFloat s;
    // RKFloat sumW2, sumW4, sumY2;
    // RKFloat sumW2Y2, sumW4Y2;
    RKFloat omega;
    // RKFloat a, b, c, d;

    const RKFloat sGain = ((RKFloat)pulseCount * (RKFloat)planSize);
    // const RKFloat sNoise[2] = {(RKFloat)space->noise[0] / (RKFloat)planSize, (RKFloat)space->noise[1] / (RKFloat)planSize};
    const RKFloat unitOmega = 2.0f * M_PI / (RKFloat)planSize;
    // const RKFloat twoPi = 2.0f * M_PI;

    // Call conventional pulse pair to get the first-pass velocity estimate
    //RKPulsePair(space, pulses, pulseCount);

    for (p = 0; p < 2; p++) {
        // I know, there are other ways to get the data in. Intuitively, one would expect Method 1,
        // which has less number of RKGetComplexDataFromPulse() calls would be beneficial but through
        // some real world tests, it has been validated that Method 2 is the more efficient despite
        // repeated calls of RKGetComplexDataFromPulse().
        //
        // Method 1:
        //
        //    for (k = 0; k < pulseCount; k++) {
        //        pulse = pulses[k];
        //        RKComplex *X = RKGetComplexDataFromPulse(pulse, 0);
        //        for (g = 0; g < space->gateCount; g++) {
        //            space->inBuffer[g][k][0] = X[g].i;
        //            space->inBuffer[g][k][1] = X[g].q;
        //        }
        //    }
        //    for (g = 0; g < space->gateCount; g++) {
        //        memset(space->inBuffer[g][pulseCount], 0, (planSize - k) * sizeof(fftwf_complex));
        //    }
        //
        // Method 2:
        //
        //    for (g = 0; g < space->gateCount; g++) {
        //        in = space->inBuffer[g];
        //        for (k = 0; k < pulseCount; k++) {
        //            pulse = pulses[k];
        //            RKComplex *X = RKGetComplexDataFromPulse(pulse, 1);
        //            in[k][0] = X[g].i;
        //            in[k][1] = X[g].q;
        //        }
        //        memset(in[k], 0, (planSize - k) * sizeof(fftwf_complex));
        //    }
        //
        for (g = 0; g < space->gateCount; g++) {
            // in = space->inBuffer[g];
            in = space->fS[p][g];
            for (k = 0; k < pulseCount; k++) {
                pulse = pulses[k];
                RKComplex *X = RKGetComplexDataFromPulse(pulse, p);
                in[k][0] = X[g].i;
                in[k][1] = X[g].q;
            }
            memset(in[k], 0, (planSize - k) * sizeof(fftwf_complex));

#ifdef DEBUG_SPECTRAL_MOMENT

            RKShowVecComplex("X = ", (RKComplex *)in, space->fftModule->plans[offt].size);

#endif

            fftwf_execute_dft(space->fftModule->plans[offt].forwardInPlace, in, in);
        }

#ifdef DEBUG_SPECTRAL_MOMENT

        printf("\n");

#endif

        //
        // Do some more tinkering
        //

        // Represent spectra in A exp (j phi) form
//         for (g = 0; g < space->gateCount; g++) {
//             s = 0.0f;
//             omegaI = 0.0f;
//             omegaQ = 0.0f;
//             in = space->fS[p][g];
//             // Go through the spectrum
//             for (k = 0; k < planSize; k++) {
//                 q = in[k][0] * in[k][0] + in[k][1] * in[k][1];
//                 s += q;
//                 phi = (RKFloat)k * unitOmega;
//                 A = sqrtf(q);
//                 omegaI += A * cosf(phi);
//                 omegaQ += A * sinf(phi);
//             }
//             omega = atan2(omegaQ, omegaI);
//             // Forward fft has a gain of sqrtf(planSize) ==> S has a gain of (planSize)
//             space->S[p][g] = s / sGain - space->noise[p];
//             space->SNR[p][g] = space->S[p][g] / space->noise[p];
//             space->Q[p][g] = MIN(1.0f, space->SNR[p][g]);
//             space->Z[p][g] = 10.0f * log10f(space->S[p][g]) + space->S2Z[p][g];
//             space->V[p][g] = space->velocityFactor * omega;

//             // // Gaussian fitting around the new x-axis
//             // //
//             // //        omega
//             // //          |
//             // //     +----+----+
//             // //     |    |    |
//             // //  :--|-o--x-:--|-o----: phi
//             // //  0    1    2    3
//             // //       PI   PI   PI
//             // //
//             // sumW2 = 0.0f;
//             // sumW4 = 0.0f;
//             // sumY2 = 0.0f;
//             // //sumW2Y2 = 0.0f;
//             // //sumW4Y2 = 0.0f;
//             // // Use omeage in range [0, 2 * PI)
//             // if (omega < 0.0f) {
//             //     omega += twoPi;
//             // }
//             // for (k = 0; k < planSize; k++) {
//             //     phi = (RKFloat)k * unitOmega;
//             //     q = phi - omega;
//             //     if (q < -M_PI) {
//             //         phi += twoPi;
//             //     } else if (q > M_PI) {
//             //         phi -= twoPi;
//             //     }
//             //     q = phi * phi;
//             //     sumW2 += q;
//             //     sumW4 += q * q;
//             //     s = in[k][0] * in[k][0] + in[k][1] * in[k][1];
//             //     //sumW2Y2 += q * s;
//             //     //sumW4Y2 += q * q * s;
//             // }
//             // a = (RKFloat)planSize;
//             // b = sumW2;
//             // c = b;
//             // d = sumW4;
//             // if (a == b || b == c || c == d || sumY2 == 0.0f) {
//             //     fprintf(stderr, "Some random code to avoid compiler warnings.\n");
//             // }
//             // // M = 1.0 / (a * d - b * c) * np.array([[d, -b], [-c, a]])
//         }

// //        g = 1;
// //        s = 0.0f;
// //        for (k = 0; k < pulseCount; k++) {
// //            pulse = pulses[k];
// //            RKIQZ X = RKGetSplitComplexDataFromPulse(pulse, p);
// //            s += (X.i[g] * X.i[g] + X.q[g] * X.q[g]);
// //        }
// //        s -= space->noise[p];
// //        printf("s= %.4f   S = %.4f   r = %.4f\n", s, space->S[p][g], s / space->S[p][g]);

//         // Censoring
//         for (g = 0; g < space->gateCount; g++) {
//             if (space->SNR[p][g] < space->config->SNRThreshold) {
//                 space->Z[p][g] = NAN;
//                 space->V[p][g] = NAN;
//             }
//         }

//        for (g = 0; g < space->gateCount; g++) {
//            fftwf_complex *in = space->inBuffer[g];
//            fftwf_complex *out = space->outBuffer[g];
//            fftwf_execute_dft(space->fftModule->plans[offt].backwardOutPlace, in, out);
//
//            // Scaling due to a net gain of planSize from forward + backward DFT, plus the waveform gain
//            RKSIMD_iyscl((RKComplex *)out, 1.0f / space->fftModule->plans[offt].size, space->fftModule->plans[offt].size);
//
//#ifdef DEBUG_SPECTRAL_MOMENT
//
//            RKShowVecComplex("X = ", (RKComplex *)out, space->fftModule->plans[offt].size);
//
//#endif
//
//        }
    }
    // Update the use count and selected order
    space->fftModule->plans[offt].count += 2 * space->gateCount;
    space->fftOrder = offt;

    for (g = 0; g < space->gateCount; g++) {
        Xh = space->fS[0][g];
        Xv = space->fS[1][g];
        in = space->fC[g];
        // Go through the spectrum
        for (k = 0; k < planSize; k++) {
            // Xh[] * Xv[]'
            in[k][0] = Xh[k][0] * Xv[k][0] + Xh[k][1] * Xv[k][1];       // C.i = Xh.i * Xv.i + Xh.q * Xv.q
            in[k][1] = Xh[k][1] * Xv[k][0] - Xh[k][0] * Xv[k][1];       // C.q = Xh.q * Xv.i - Xh.i * Xv.q
        }
    }
    // We have fS and fC calculated here let's do some spectral based filtering process
    // notice that fS and fC never been scaled and assumed to be scaled while summarizing moment
    // remeber to edit moment estimation if move the scaling here in future

    // Summarize spectral to moment
    RKFloat *Ci = space->C[0].i;
    RKFloat *Cq = space->C[0].q;
    for (g = 0; g < space->gateCount; g++) {
        for (p = 0; p < 2; p++) {
            s = 0.0f;
            omegaI = 0.0f;
            omegaQ = 0.0f;
            omegasqI = 0.0f;
            omegasqQ = 0.0f;
            gA = 0.0f;
            in = space->fS[p][g];
            // Go through the spectrum
            for (k = 0; k < planSize; k++) {
                q = in[k][0] * in[k][0] + in[k][1] * in[k][1];
                s += q;
                phi = (RKFloat)k * unitOmega;
                A = sqrtf(q);
                omegaI += A * cosf(phi);
                omegaQ += A * sinf(phi);
                gA += A;
                omegasqI += A * cosf(phi) * cosf(phi);
                omegasqQ += A * sinf(phi) * sinf(phi);
            }
            omega = atan2(omegaQ, omegaI);
            // Forward fft has a gain of sqrtf(planSize) ==> S has a gain of (planSize)
            space->aR[p][0][g] = s / sGain;
            space->S[p][g] = space->aR[p][0][g] - space->noise[p];
            space->SNR[p][g] = space->S[p][g] / space->noise[p];
            space->Q[p][g] = MIN(1.0f, space->SNR[p][g]);
            q = ( omegasqI + omegasqQ ) / gA - ( omegaI * omegaI + omegaQ * omegaQ ) / gA / gA;
            // if (space->SNR[p][g] < space->config->SNRThreshold) {
            if (0) {
                space->Z[p][g] = NAN;
                space->V[p][g] = NAN;
                space->W[p][g] = NAN;
            } else{
                space->Z[p][g] = 10.0f * log10f(space->S[p][g]) + space->S2Z[p][g];
                space->V[p][g] = space->velocityFactor * omega;
                space->W[p][g] = space->velocityFactor * q;
            }
        }
        in = space->fC[g];
        Ci[g] = 0.0f;
        Cq[g] = 0.0f;
        for (k = 0; k < planSize; k++) {
            Ci[g] += in[k][0];
            Cq[g] += in[k][1];
        }
        Ci[g] = Ci[g] / sGain;
        Cq[g] = Cq[g] / sGain;
    }

    for (g = 0; g < space->gateCount; g++) {
        // if (space->SNR[0][g] < space->config->SNRThreshold || space->SNR[1][g] < space->config->SNRThreshold) {
        if (0) {
            space->ZDR[g] = NAN;
            space->PhiDP[g] = NAN;
            space->RhoHV[g] = NAN;
        } else {
            space->ZDR[g] = 10.0f * log10f(space->S[0][g] / space->S[1][g]) + space->dcal[g];
            space->RhoHV[g] = sqrtf(( Ci[g] * Ci[g] + Cq[g] * Cq[g] ) / (space->aR[0][0][g] * space->aR[1][0][g]));
            // space->RhoHV[g] = sqrtf(( Ci[g] * Ci[g] + Cq[g] * Cq[g] ) / ((1.0f + 1.0f/space->SNR[0][g]) * (1.0f + 1.0f/space->SNR[1][g])) / (space->aR[0][0][g] * space->aR[1][0][g]));
            space->PhiDP[g] = atan2(Cq[g], Ci[g]) + space->pcal[g];
            if (g > 1) {
                space->KDP[g] = space->PhiDP[g] - space->PhiDP[g - 1];
                space->KDP[g] = RKSingleWrapTo2PI(space->KDP[g]);
                space->KDP[g] = space->KDPFactor * space->KDP[g];
            }
        }
    }
    // Show and Tell
    if (space->verbose && pulseCount < 50 && space->gateCount < 50) {
        char variable[RKNameLength];
        char line[RKMaximumStringLength];
        RKIQZ *X = (RKIQZ *)malloc(RKMaximumPulsesPerRay * sizeof(RKIQZ));
        const int gateShown = 8;

        // Go through both polarizations
        for (p = 0; p < 2; p++) {
            printf((rkGlobalParameters.showColor ? UNDERLINE("Channel %d (%s pol):") "\n" : "Channel %d (%s pol):\n"),
                   p, p == 0 ? "H" : (p == 1 ? "V" : "X"));
            for (k = 0; k < pulseCount; k++) {
                X[k] = RKGetSplitComplexDataFromPulse(pulses[k], p);
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
            for (g = 0; g < space->gateCount; g++) {
                for (k = 0; k < pulseCount; k++) {
                    j += sprintf(line + j, " %.0f%+.0fj,", X[g].i[k], X[g].q[k]);
                }
                j += sprintf(line + j - 1, ";...\n") - 1;
            }
            sprintf(line + j - 5, "]\n");
            printf("%s\n", line);

            // for (k = 0; k < pulseCount; k++) {
            //     sprintf(variable, "  X[%d] = ", k);
            //     RKShowVecIQZ(variable, &X[k], gateShown);
            // }
            // printf(RKEOL);
            // RKShowVecIQZ("    mX = ", &space->mX[p], gateShown);                                                 // mean(X) in MATLAB
            // RKShowVecIQZ("    vX = ", &space->vX[p], gateShown);                                                 // var(X, 1) in MATLAB
            // printf(RKEOL);
            // for (k = 0; k < 3; k++) {
            //     sprintf(variable, "  R[%d] = ", k);
            //     RKShowVecIQZ(variable, &space->R[p][k], gateShown);
            // }
            // printf(RKEOL);
            // for (k = 0; k < 3; k++) {
            //     sprintf(variable, " aR[%d] = ", k);
            //     RKShowVecFloat(variable, space->aR[p][k], gateShown);
            // }
            // printf(RKEOL);
            sprintf(variable, "  S2Z = ");
            RKShowVecFloat(variable, space->S2Z[p], gateShown);
            printf(RKEOL);
            sprintf(variable, "    Z%s = ", p == 0 ? "h" : "v");
            RKShowVecFloat(variable, space->Z[p], gateShown);
            printf(RKEOL);
        }
        // printf(rkGlobalParameters.showColor ? UNDERLINE("Cross-channel:") "\n" : "Cross-channel:\n");
        // RKShowVecIQZ("  C[0] = ", &space->C[0], gateShown);                                                      // xcorr(Xh, Xv, 'unbiased') in MATLAB
        // printf(RKEOL);
        RKShowVecFloat("   ZDR = ", space->ZDR, gateShown);
        RKShowVecFloat(" PhiDP = ", space->PhiDP, gateShown);
        RKShowVecFloat(" RhoHV = ", space->RhoHV, gateShown);

        printf(RKEOL);
        fflush(stdout);

        free(X);
    }
    // Mark the calculated products, exclude K here since it is not ready
    space->calculatedProducts = RKBaseProductListFloatZVWDPR;
    return pulseCount;
}

int RKSpectralMoment2(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount) {

    int n, k, p;

    // Get the start pulse to know the capacity
    RKPulse *pulse = pulses[0];
    const uint32_t gateCount = pulse->header.downSampledGateCount;
    const int lagCount = 2;

    //
    //  ACF
    //

    // Go through each polarization
    for (p = 0; p < 2; p++) {

        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], space->capacity);
        for (k = 0; k < lagCount; k++) {
            RKZeroOutIQZ(&space->R[p][k], space->capacity);
        }

        RKIQZ *R = &space->R[p][0];

        // Go through all pulses
        n = 0;
        do {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], p);

            RKSIMD_izadd(&Xn, &space->mX[p], gateCount);                                 // mX += X
            // Go through each lag
            for (k = 0; k < lagCount; k++) {
                //RKLog(">Lag %d\n", k);
                if (n >= k) {
                    RKIQZ Xk = RKGetSplitComplexDataFromPulse(pulses[n - k], p);
                    RKSIMD_zcma(&Xn, &Xk, &R[k], gateCount, 1);                          // R[k] += X[n] * X[n - k]'
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

    return pulseCount;
}
