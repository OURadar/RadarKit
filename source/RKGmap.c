//
//  RKGmap.c
//  RadarKit
//
//  Created by Skyler Garner in December 2025.
//  Copyright (c) 2026 Skyler Garner. All rights reserved.
//

#include <RadarKit/RKGmap.h>
#include "ext/mexDspGMAP.c"
// #include "ext/cosine_window.c"


static void fill_fCorDots(float *fCorDots, unsigned int M) {
    const float twoPiOverM = 2.0f * M_PI / (float) M;
    for (int n = 0; n < M; ++n) {
        float ang1 = twoPiOverM * n;      // 2π n / M
        float ang2 = 2.0f * ang1;          // 4π n / M

        /* Row 0: cos(2π n / M) */
        fCorDots[0 * M + n] = cosf(ang1);

        /* Row 1: cos(2π n / M + π/2) = -sin(2π n / M) */
        fCorDots[1 * M + n] = cosf(ang1 + M_PI_2);
        /* alternatively: fCorDots[1*M + n] = -sin(ang1); */

        /* Row 2: cos(4π n / M) */
        fCorDots[2 * M + n] = cosf(ang2);

        /* Row 3: cos(4π n / M + π/2) = -sin(4π n / M) */
        fCorDots[3 * M + n] = cosf(ang2 + M_PI_2);
        /* alternatively: fCorDots[3*M + n] = -sin(ang2); */
    }
}

int RKGmapRun(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount) {
	// in-place application of Gaussian model adaptive processing.
    int k, g, p;

    // Get the start pulse to know the capacity
    RKPulse *pulse = pulses[0];
    const uint32_t gateCount = pulse->header.downSampledGateCount;
	const int lagCount = space->userLagChoice == 0 ? MIN(pulseCount, RKMaximumLagCount) : MIN(space->userLagChoice + 1, RKMaximumLagCount);

    const int i = (int)ceilf(log2f((float)pulseCount));
    const fftwf_plan planForwardInPlace = space->fftModule->plans[i].forwardInPlace;
    const fftwf_plan planBackwardInPlace = space->fftModule->plans[i].backwardInPlace;
    const unsigned int planSize = space->fftModule->plans[i].size;

	if (lagCount > pulseCount) {
		RKLog("WARNING. Memory leak in RKGmap.\n");
	}

	// Step 0: Inputs needed: noise power, clutter width

	// based on:
	// Gaussian model adaptive processing (GMAP) for improved ground clutter cancellation and moment calculation
	// (A. D. Siggia and R. E. Passarelli, Jr., ERAD 2004)

	// Step 1: window the time series data and FFT.
	// 1.1 Hamming window first! Can recalulate with rectangular window for very low clutter (CSR<2.5dB) or blackmann if a lot of clutter (CSR>40dB)

	// Step 2: Determine noise power.
	// We do this before! Done.

	// Step 3: Remove clutter points.
	// 3.1 sum the three central spectrum components (DC+1) and compare to a gaussian of the given clutter spectrum width to normalize the gaussian
	// 3.2 extend the gaussian to the noise level and remove these points. I think to the noise level. The power removed is the clutter power.
	// Note if there is no clutter, ie clutter power is lower than the noise power, we pass it on to moment generation without the window applied.
	// And it is good if there is little clutter to only remove DC!

	// Step 4: Replace clutter points.
	// With the fixed noise case, they don't talk about it in the paper.
	// But I am thinking we could just subtract the fixed noise value, and then continue with the gaussian model.

	// Step 5: Check window (CSR) and recalculate moments if necessary.
	//



	// Step 0: Inputs needed: noise power, clutter width

	// TODO: Where do I get thess info?
	// compute clutter width; need beamwidth, angular velocity, elevation angle, and wavelength
	// float elevation = 0;
	// float angular_velocity = 15 * PI / 180;
	// float beamwidth = 3 * PI/180;
	// float tx_frequency = 3e9;
	// float wavelength = 3e8 / tx_frequency;
	// float calculated_clutterwidth = angular_velocity * wavelength * cos(elevation) / (2 * PI * beamwidth) * sqrt(log(2));

	// TODO: set to calculated clutterwidth, e.g., 0.08, 0.4, 2.0. Probably with a floor value for wind speed
	const float clutterFactor = 0.4;
	const float aliasingVelocity = (float)space->velocityFactor * M_PI;
	const float clutterWidth = clutterFactor / aliasingVelocity;

    // This is used in GMAP, but we want to init it outside of the loop.
	// wgcm is set in the function and will keep it's values as long as iSpecSize, iWinType, and fClutterWidth are not changed.
	tWGCM wgcm = {
		.iWinType = 0,
		.iSpecSize = planSize,
		.fClutterWidth = 0,
		.iMainLobePts = 0,
		.fModelPwr = space->user1
	};
	memset(wgcm.fModelPwr, 0, planSize * sizeof(float));

	// same as above, but this one just shouldn't change in the loop.
	tDftConf dftConf = {
		.iSpecSize = planSize,
		.iWinType = WIN_BLACKMAN,
		.window = space->user2,
		.fCorDots = space->user3
	};
	// const RKFloat coeff[3] = {0.42, -0.5, 0.08};
	// cosine_window(dftConf.window, pulseCount, coeff, 3, true);

	// TODO: Could use a better way of setting which window to use.
	// hamming window
	// dftConf.iWinType = (int) WIN_HAMMING;
	// float coeff[2] = {0.54, -0.46};

	RKWindowMake(dftConf.window, RKWindowTypeBlackman, pulseCount);
	fill_fCorDots(dftConf.fCorDots, planSize);

	// Normzalized window, and zero pad to planSize if needed.
	RKFloat s = 0.0f;
	for (k = 0; k < pulseCount; k++) {
		s += dftConf.window[k] * dftConf.window[k];
	}
	for (k = 0; k < pulseCount; k++) {
		dftConf.window[k] *= sqrt(pulseCount / s);
	}
	if (planSize > pulseCount) {
		memset(dftConf.window + pulseCount, 0, (planSize - pulseCount) * sizeof(float));
	}

	// Outputs of dspGMAP function
	float gapPoints = 0;
	float powerRemoved = 0;

	// Go through both polarizations and all range gates
	float *freqPower = space->user4;
    for (p = 0; p < 2; p++) {
		// for each range gate
		fftwf_complex *in = space->inBuffer[p];
		for (g = 0; g < gateCount; g++){
			// Step 1: Window the IQ data and FFT: Rectangular (CSR<2.5dB) or Blackmann (CSR>40dB)
			for (k = 0; k < pulseCount; k++) {
				RKComplex *src = RKGetComplexDataFromPulse(pulses[k], p);
				in[k][0] = src[g].i * dftConf.window[k];
				in[k][1] = src[g].q * dftConf.window[k];
			}

			// FFT windowed data: zero pad the input; a filter is always zero-padded in the setter function.
			if (planSize > pulseCount) {
				memset(in + pulseCount, 0, (planSize - pulseCount) * sizeof(fftwf_complex));
			}
			fftwf_execute_dft(planForwardInPlace, in, in);
			space->fftModule->plans[i].count++;
			RKSIMD_iyscl((RKComplex *)in, 1.0f / (RKFloat)planSize, planSize);

			// Normalize the amptlitude of the spectrum to 1, and save the power in freqPower for later use.
			for (k = 0; k < planSize; k++) {
				freqPower[k] = in[k][0] * in[k][0] + in[k][1] * in[k][1];
				s = sqrtf(freqPower[k]);
				in[k][0] /= s;
				in[k][1] /= s;
			}

			// Show input spectrum for testing
			if (space->verbose && gateCount == 1) {
				RKShowVecFloat("GMAP Input Spectrum:\n", freqPower, planSize);
			}

			// Step 2: Determine noise power: We do this before! Done.
			dspGMAP(freqPower, clutterWidth, space->noise[p], &dftConf, &wgcm, 1, &powerRemoved, &gapPoints);

			// Show output spectrum for testing
			if (space->verbose && gateCount == 1) {
				RKLog("GMAP: Power removed: %.4f, Gap points: %.1f\n", powerRemoved, gapPoints);
				RKShowVecFloat("GMAP Output Spectrum:\n", freqPower, planSize);
			}

			// Recover the amplitude of the spectrum after GMAP, and save it back to the input buffer for IFFT.
			for (k = 0; k < planSize; k++) {
				s = sqrtf(freqPower[k]);
				in[k][0] *= s;
				in[k][1] *= s;
			}

			// IFFT back to IQ data
			fftwf_execute_dft(planBackwardInPlace, in, in);
			space->fftModule->plans[i].count++;

			// overwrite buffer
			for(k = 0; k < pulseCount; k++){
				RKComplex* bufferValue = RKGetComplexDataFromPulse(pulses[k], p);
				bufferValue[g].i = in[k][0];
				bufferValue[g].q = in[k][1];
			}
		}
	}

	// when finished need to update the other kind of IQ in the pulse
	for(k = 0; k < pulseCount; k++){
		RKPulseDuplicateSplitComplex(pulses[k]);
	}

    return pulseCount;

}


RKGmap *RKGMapInit(void) {
    RKGmap *gmap = (RKGmap *)malloc(sizeof(RKGmap));
    if (gmap == NULL) {
        RKLog("Error. Unable to allocate gmap.\n");
        return NULL;
    }
	return NULL;
}

void RKGmapFree(RKGmap *gmap) {
	// do we need to check state before we stop? If we multithread...
    free(gmap);
}
