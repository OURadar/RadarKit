//
//  RKMultiLag.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/2/17.
//  Copyright (c) 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKMultiLag.h>

enum RKMomentMask {
	RKMomentMaskCensored = -1,
	RKMomentMaskNormal = 1,
	RKMomentMaskLag1 = 1,
	RKMomentMaskLag2 = 2,
	RKMomentMaskLag3 = 3,
	RKMomentMaskLag4 = 4
};

int RKMultiLag(RKScratch *space, RKPulse **input, const uint16_t pulseCount) {
//    struct timeval tic, toc;
//    gettimeofday(&tic, NULL);
    
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
    // so S & R are biased unless the filter is turned off. This is a downside of clutter filter using mean removal
    
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

    // Show and tell
    if (space->showNumbers && gateCount <= 16 && pulseCount <= 32) {
        char variable[RKNameLength];
        char *line = (char *)malloc(RKMaximumStringLength);
        RKIQZ *X = (RKIQZ *)malloc(pulseCount * sizeof(RKIQZ));
        if (line == NULL || X == NULL) {
            fprintf(stderr, "Error allocating pulse buffer.\n");
            exit(EXIT_FAILURE);
        }
        const int gateShown = 8;
        for (p = 0; p < 2; p++) {
			printf((rkGlobalParameters.showColor ? UNDERLINE("Channel %d (%s pol):") "\n" : "Channel %d (%s pol):\n"), p, p == 0 ? "H" : (p == 1 ? "V" : "X"));
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
		printf(rkGlobalParameters.showColor ? UNDERLINE("Cross-channel:") "\n" : "Cross-channel:\n");
        for (j = 0; j < 2 * lagCount - 1; j++) {
            k = j - lagCount + 1;
            sprintf(variable, " C[%2d] = ", k);
            RKShowVecIQZ(variable, &space->C[j], gateShown);                                 // xcorr(Xh, Xv, 'unbiased') in MATLAB
        }
        printf(RKEOL);
        RKShowVecFloat("    gC = ", space->gC, gateShown);
        printf(RKEOL);
        fflush(stdout);

        free(line);
        free(X);
    }

    //
    //  ACF & CCF to moments
    //
    
	RKFloat num, den;

    for (p = 0; p < 2; p++) {
		for (k = 0; k < gateCount; k++) {
            // Derive some criteria for censoring and lag selection
			space->SNR[p][k] = powf(space->aR[p][1][k], 4.0f / 3.0f) / powf(space->aR[p][2][k], 1.0f / 3.0f) / space->noise[p];
			if (space->SNR[0][k] < space->SNRThreshold || space->SNR[1][k] < space->SNRThreshold) {
				space->mask[k] = RKMomentMaskCensored;
			} else {
				space->mask[k] = space->userLagChoice;
			}
			if (space->mask[k] == RKMomentMaskCensored) {
				space->Z[p][k] = NAN;
				space->V[p][k] = NAN;
				space->W[p][k] = NAN;
				continue;
			}
			switch (space->mask[k]) {
				default:
				case RKMomentMaskNormal:
					space->S[p][k] = space->aR[p][0][k] - space->noise[p];
					space->SNR[p][k] = space->S[p][k] / space->noise[p];
					num = logf(space->S[p][k]);
					den = logf(space->aR[p][1][k]);
					break;
				case RKMomentMaskLag2:
					space->S[p][k] = powf(space->aR[p][1][k], 4.0f / 3.0f)
					/ powf(space->aR[p][2][k], 1.0f / 3.0f);
					space->SNR[p][k] = space->S[p][k] / space->noise[p];
					num = logf(space->aR[p][1][k]);
					den = logf(space->aR[p][2][k]);
					break;
				case RKMomentMaskLag3:
					space->S[p][k] = powf(space->aR[p][1][k], 6.0f / 7.0f) * powf(space->aR[p][2][k], 3.0f / 7.0f)
					/ powf(space->aR[p][3][k], 2.0f / 7.0f);
					space->SNR[p][k] = space->S[p][k] / space->noise[p];
					num = 11.0f * logf(space->aR[p][1][k]) + 2.0f * logf(space->aR[p][2][k]);
					den = 13.0f * logf(space->aR[p][3][k]);
					break;
				case RKMomentMaskLag4:
					space->S[p][k] = powf(space->aR[p][1][k], 54.0f / 86.0f) * powf(space->aR[p][2][k], 39.0f / 86.0f) * powf(space->aR[p][3][k], 14.0f / 86.0f)
					/ powf(space->aR[p][4][k], 21.0f / 86.0f);
					space->SNR[p][k] = space->S[p][k] / space->noise[p];
					num = 13.0f * logf(space->aR[p][1][k]) + 7.0f * logf(space->aR[p][2][k]);
					den = 3.0f * logf(space->aR[p][3][k]) + 17.0f * logf(space->aR[p][4][k]);
					break;
			}
			space->Z[p][k] = 10.0f * log10f(space->S[p][k]) + space->rcor[p][k];
			space->V[p][k] = space->velocityFactor * atan2f(space->R[p][1].i[k], space->R[p][1].q[k]);
			if (num < den) {
				space->W[p][k] = 0.0f;
			} else {
				space->W[p][k] = space->widthFactor * sqrtf(num - den);
			}
		} // for (k = 0; k < gateCount ...)
    }
	// Note (k = j - lagCount + 1) was used for C[j] = lag k; So, lag-0 is stored at index (lagCount), e.g., For lagCount = 3, C in [-2, -1, 0, 1, 2]
	RKFloat *Ci = space->C[space->lagCount].i;
	RKFloat *Cq = space->C[space->lagCount].q;
    for (k = 0; k < gateCount; k++) {
		if (space->mask[k] == RKMomentMaskCensored) {
			space->ZDR[k] = NAN;
			space->PhiDP[k] = NAN;
			space->RhoHV[k] = NAN;
			space->KDP[k] = NAN;
			continue;
		}
		switch (space->mask[k]) {
			default:
			case RKMomentMaskNormal:
				space->ZDR[k] = 10.0f * log10f(space->S[0][k] / space->S[1][k])
				+ space->dcal;
				space->RhoHV[k] = space->gC[k] / sqrtf(space->aR[0][0][k] * space->aR[1][0][k]);
				break;
			case RKMomentMaskLag2:
				space->ZDR[k] = 10.0f * log10f( powf(space->aR[0][1][k], 4.0f / 3.0f) * powf(space->aR[1][2][k], 1.0f / 3.0f) /
											   (powf(space->aR[1][1][k], 4.0f / 3.0f) * powf(space->aR[0][2][k], 1.0f / 3.0f)))
				+ space->dcal;
				space->RhoHV[k] = space->gC[k]
				* powf(space->aR[0][2][k] * space->aR[1][2][k], 1.0f / 6.0f)
				/ powf(space->aR[0][1][k] * space->aR[1][1][k], 2.0f / 3.0f);
				break;
			case RKMomentMaskLag3:
				space->ZDR[k] = 10.0f * log10f( powf(space->aR[0][1][k], 6.0f / 7.0f) * powf(space->aR[0][2][k], 3.0f / 7.0f) * powf(space->aR[1][3][k], 2.0f / 7.0f) /
											   (powf(space->aR[1][1][k], 6.0f / 7.0f) * powf(space->aR[1][2][k], 3.0f / 7.0f) * powf(space->aR[0][3][k], 2.0f / 7.0f)))
				+ space->dcal;
				space->RhoHV[k] = space->gC[k]
				* powf(space->aR[0][3][k] * space->aR[1][3][k], 1.0f / 7.0f)
				/ (powf(space->aR[0][1][k] * space->aR[1][1][k], 3.0 / 7.0f) * powf(space->aR[0][2][k] * space->aR[1][2][k], 3.0f / 14.0f));
				break;
			case RKMomentMaskLag4:
				space->ZDR[k] = 10.0f * log10f( powf(space->aR[0][1][k], 54.0f / 86.0f) * powf(space->aR[0][2][k], 39.0f / 86.0f) * powf(space->aR[0][3][k], 14.0f / 86.0f) * powf(space->aR[1][4][k], 21.0f / 86.0f) /
											   (powf(space->aR[1][1][k], 54.0f / 86.0f) * powf(space->aR[1][2][k], 39.0f / 86.0f) * powf(space->aR[1][3][k], 14.0f / 86.0f) * powf(space->aR[0][4][k], 21.0f / 86.0f)) )
				+ space->dcal;
				space->RhoHV[k] = space->gC[k]
				* powf(space->aR[0][4][k] * space->aR[1][4][k], 21.0f / 172.0f)
				/ (powf(space->aR[0][1][k] * space->aR[1][1][k], 27.0f / 86.0f) * powf(space->aR[0][2][k] * space->aR[1][2][k], 39.0 / 172.0f) * powf(space->aR[0][3][k] * space->aR[1][3][k], 7.0f / 86.0f));
				break;
		}
		space->PhiDP[k] = -atan2(Ci[k], Cq[k]) + space->pcal;
		RKSingleWrapTo2PI(space->PhiDP[k]);
		if (k > 1) {
			space->KDP[k] = space->PhiDP[k] - space->PhiDP[k - 1];
			RKSingleWrapTo2PI(space->KDP[k]);
			space->KDP[k] = space->KDPFactor * space->KDP[k];
		}
    }

//    gettimeofday(&toc, NULL);
//    RKLog("Diff time = %.4f ms", 1.0e3 * RKTimevalDiff(toc, tic));
    
    return pulseCount;
    
}
