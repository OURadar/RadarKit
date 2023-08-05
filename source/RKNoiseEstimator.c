//
//  RKNoiseEstimator.h
//  RadarKit
//
//  Created by Min-Duan Tzeng on 7/26/2023.
//  Copyright (c) 2023 Min-Duan Tzeng. All rights reserved.
//

#include <RadarKit/RKNoiseEstimator.h>

const uint16_t minSampleCount = 800;
const uint16_t failureSampleCount = 200;

RKFloat TCN[] = {0, 246.342840, 59.019163, 27.284472, 17.386875, 12.822073,
    10.112010, 8.456563, 7.346106, 6.527433, 5.915182, 5.474314, 5.050479, 4.729339,
    4.449649, 4.224396, 4.028606, 3.860354, 3.697541, 3.570579, 3.397484, 3.304638,
    3.270436, 3.193755, 3.113686, 3.038384, 2.972167, 2.911041, 2.854365, 2.801666,
    2.751110, 2.704821, 2.664238, 2.625989, 2.588804, 2.552798, 2.518628, 2.487077,
    2.456474, 2.426187, 2.399409, 2.373835, 2.349386, 2.325986, 2.303568, 2.282072,
    2.261439, 2.241619, 2.222562, 2.204226, 2.186568, 2.169552, 2.153143, 2.137308,
    2.122016, 2.107240, 2.092954, 2.079133, 2.065755, 2.052797, 2.040241, 2.028067,
    2.016259, 2.004798, 1.993671, 1.982862, 1.972358, 1.962145, 1.952212, 1.942547,
    1.933139, 1.923978, 1.915053, 1.906357, 1.897879, 1.889612, 1.881548, 1.873679, 1.865998, 1.858499,
    1.851174, 1.844018, 1.837024, 1.830188, 1.823504, 1.816967, 1.810571, 1.804313, 1.798187, 1.792190,
    1.786317, 1.780565, 1.774930, 1.769407, 1.763995, 1.758689, 1.753486, 1.748384, 1.743378, 1.738468,
    1.733649, 1.728919, 1.724276, 1.719717, 1.715241, 1.710844, 1.706524, 1.702280, 1.698110, 1.694011,
    1.689982, 1.686020, 1.682125, 1.678295, 1.674527, 1.670821, 1.667174, 1.663586, 1.660055, 1.656579,
    1.653158, 1.649789, 1.646472, 1.643206, 1.639989, 1.636820, 1.633698, 1.630622, 1.627592, 1.624605,
    1.621661, 1.618760, 1.615899, 1.613079, 1.610299, 1.607557, 1.604852, 1.602185, 1.599554, 1.596959,
    1.594398, 1.591871, 1.589377, 1.586917, 1.584488, 1.582091, 1.579724, 1.577388, 1.575081, 1.572803,
    1.570554, 1.568333, 1.566139, 1.563972, 1.561831, 1.559717, 1.557627, 1.555563, 1.553523, 1.551507,
    1.549515, 1.547546, 1.545600, 1.543676, 1.541775, 1.539895, 1.538036, 1.536198, 1.534380, 1.532583,
    1.530806, 1.529048, 1.527309, 1.525589, 1.523888, 1.522205, 1.520540, 1.518892, 1.517262, 1.515649,
    1.514053, 1.512473, 1.510910, 1.509363, 1.507831, 1.506315, 1.504815, 1.503329, 1.501858, 1.500402,
    1.498960, 1.497533, 1.496119, 1.494719, 1.493333, 1.491960, 1.490600, 1.489253, 1.487919, 1.486598};

RKFloat varThreshold[] = {0.0,12.501880,9.066873,6.978578,5.602964,3.184561,2.709687,2.352218,
    2.075309,1.855436,1.677084,1.529734,1.406049,1.300803,0.683347, 0.625970,
    0.592077, 0.560521, 0.531522, 0.505351, 0.481602, 0.459942, 0.439551, 0.420858,
    0.403674, 0.387825, 0.373164, 0.359562, 0.346910, 0.335113, 0.324087, 0.313760,
    0.304067, 0.292595, 0.284680, 0.277142, 0.269806, 0.262747, 0.256040, 0.249657,
    0.243574, 0.237771, 0.232228, 0.226885, 0.221765, 0.216871, 0.212188, 0.207704,
    0.203405, 0.199281, 0.195321, 0.191515, 0.187856, 0.184334, 0.180942, 0.177673,
    0.174515, 0.171464, 0.168519, 0.165675, 0.162927, 0.160270, 0.157700, 0.155213,
    0.152805, 0.150471, 0.148210, 0.146016, 0.143889, 0.141824, 0.139819, 0.137871,
    0.135979, 0.134139, 0.132349, 0.130609, 0.128915, 0.127265, 0.125659, 0.124094,
    0.122569, 0.121083, 0.119633, 0.118219, 0.116839, 0.115493, 0.114178, 0.112894,
    0.111640, 0.110415, 0.109218, 0.108047, 0.106903, 0.105783, 0.104688, 0.103617,
    0.102568, 0.101542, 0.100537, 0.099552, 0.098588, 0.097644, 0.096718, 0.095811,
    0.094921, 0.094049, 0.093194, 0.092355, 0.091059, 0.090263, 0.089482, 0.088715,
    0.087962, 0.087223, 0.086497, 0.085783, 0.085083, 0.084394, 0.083718, 0.083053,
    0.082399, 0.081757, 0.081125, 0.080504, 0.079893, 0.079292, 0.078700, 0.078119,
    0.077546, 0.076983, 0.076428, 0.075882, 0.075345, 0.074816, 0.074295, 0.073781,
    0.073276, 0.072778, 0.072287, 0.071804, 0.071327, 0.070858, 0.070395, 0.069939,
    0.069489, 0.069046, 0.068609, 0.068178, 0.067753, 0.067334, 0.066920, 0.066512,
    0.066110, 0.065713, 0.065321, 0.064934, 0.064553, 0.064176, 0.063804, 0.063437,
    0.063075, 0.062717, 0.062364, 0.062015, 0.061671, 0.061331, 0.060995, 0.060663,
    0.060335, 0.060011, 0.059691, 0.059375, 0.059063, 0.058754, 0.058449, 0.058148,
    0.057850, 0.057555, 0.057262, 0.056972, 0.056686, 0.056403, 0.056123, 0.055846,
    0.055573, 0.055302, 0.055034, 0.054770, 0.054508, 0.054249, 0.053993, 0.053740,
    0.053489, 0.053241, 0.052996, 0.052753, 0.052513, 0.052276, 0.052040, 0.051808};

// PFA = 1e-3
RKFloat SNRThreshold[] = {0, 4.616698, 3.742957, 3.265547, 2.958829, 2.742457, 2.580232,
    2.453271, 2.350688, 2.265737, 2.193985, 2.132440, 2.078920,
    2.031867, 1.990098, 1.952726, 1.919036, 1.888477, 1.860602,
    1.835049, 1.811518, 1.789762, 1.769572, 1.750774, 1.733216,
    1.716768, 1.701329, 1.686795, 1.673078, 1.660117, 1.647842,
    1.636193, 1.625116, 1.614578, 1.604528, 1.594931, 1.585759,
    1.576975, 1.568563, 1.560486, 1.552736, 1.545277, 1.538108,
    1.531194, 1.524537, 1.518106, 1.511904, 1.505904, 1.500106,
    1.494492, 1.489057, 1.483789, 1.478678, 1.473726, 1.468915,
    1.464243, 1.459702, 1.455285, 1.450992, 1.446812, 1.442737,
    1.438776, 1.434911, 1.431144, 1.427469, 1.423882, 1.420378,
    1.416964, 1.413625, 1.410359, 1.407173, 1.404054, 1.401002,
    1.398019, 1.395097, 1.392236, 1.389436, 1.386692, 1.384003,
    1.381367, 1.378785, 1.376252, 1.373767, 1.371327, 1.368937,
    1.366589, 1.364284, 1.362018, 1.359798, 1.357614, 1.355467,
    1.353357, 1.351286, 1.349248, 1.347244, 1.345272, 1.343334,
    1.341427, 1.339550, 1.337702, 1.335882, 1.334094, 1.332332,
    1.330596, 1.328885, 1.327199, 1.325543, 1.323908, 1.322296,
    1.320707, 1.319140, 1.317599, 1.316077, 1.314576, 1.313095,
    1.311633, 1.310195, 1.308774, 1.307371, 1.305987, 1.304620,
    1.303273, 1.301941, 1.300627, 1.299330, 1.298048, 1.296781,
    1.295533, 1.294298, 1.293078, 1.291873, 1.290682, 1.289504,
    1.288343, 1.287191, 1.286057, 1.284934, 1.283824, 1.282725,
    1.281641, 1.287659, 1.286709, 1.285770, 1.284841, 1.283924,
    1.283017, 1.282120, 1.281234, 1.280357, 1.279490, 1.278633, 1.277785,
    1.276947, 1.276117, 1.275297, 1.274485, 1.273682, 1.272888, 1.272102, 1.271324,
    1.270554, 1.269792, 1.269038, 1.268292, 1.267553, 1.266822, 1.266098, 1.265381,
    1.264671, 1.263969, 1.263273, 1.262584, 1.261901, 1.261226, 1.260556, 1.259893,
    1.259237, 1.258586, 1.257942, 1.257303, 1.256671, 1.256044, 1.255423, 1.254807,
    1.254198, 1.253593, 1.252994, 1.252401, 1.251812, 1.251229, 1.250651, 1.250078,
    1.249510, 1.248947, 1.248389, 1.247835, 1.247286, 1.246742, 1.246202, 1.245667};

RKFloat runSumPercentile[] = {0,0.004693,0.004658,0.004693,0.004693,0.004764,0.004800,0.004555,
    0.004555,0.004693,0.004872,0.004555,0.004909,0.004555,0.004872,0.004836,
    0.004946,0.004555,0.004909,0.004693,0.004555,0.004487,0.004487,0.004555,
    0.004693,0.004909,0.004259,0.004555,0.004946,0.004355,0.004836,0.004291,
    0.004872,0.004355,0.005058,0.004555,0.004103,0.004909,0.004454,0.004042,
    0.004983,0.004555,0.004165,0.005291,0.004872,0.004487,0.004134,0.005452,
    0.005058,0.004693,0.004355,0.004042,0.005576,0.005212,0.004872,0.004555,
    0.004259,0.003982,0.005789,0.005452,0.005134,0.004836,0.004555,0.004291,
    0.004042,0.003808,0.005922,0.005618,0.005330,0.005058,0.004800,0.004555,
    0.004323,0.004103,0.003895,0.003697,0.006242,0.005966,0.005703,0.005452,
    0.005212,0.004983,0.004764,0.004555,0.004355,0.004165,0.003982,0.003808,
    0.003642,0.003483,0.006580,0.006337,0.006103,0.005877,0.005660,0.005452,
    0.005251,0.005058,0.004872,0.004693,0.004521,0.004355,0.004196,0.004042,
    0.003895,0.003752,0.003615,0.003483,0.003356,0.003234,0.003117,0.006937,
    0.006731,0.006531,0.006337,0.006149,0.005966,0.005789,0.005618,0.005452,
    0.005291,0.005134,0.004983,0.004836,0.004693,0.004555,0.004421,0.004291,
    0.004165,0.004042,0.003924,0.003808,0.003697,0.003589,0.003483,0.003382,
    0.003283,0.003187,0.003094,0.003003,0.002916,0.002831,0.008011,0.007831,
    0.007654,0.007482,0.007314,0.007150,0.006990,0.006833,0.006680,0.006531,
    0.006385,0.006242,0.006103,0.005966,0.005833,0.005703,0.005576,0.005452,
    0.005330,0.005212,0.005096,0.004983,0.004872,0.004764,0.004658,0.004555,
    0.004454,0.004355,0.004259,0.004165,0.004073,0.003982,0.003895,0.003808,
    0.003724,0.003642,0.003562,0.003483,0.003407,0.003332,0.003258,0.003187,
    0.003117,0.003048,0.002981,0.002916,0.002852,0.002789,0.002728,0.002668,
    0.002610,0.002552,0.002497,0.002442,0.002389,0.002336,0.002285,0.002235};

RKFloat varf(RKFloat *astart, const uint16_t window) {
    int k;
    RKFloat m = 0.0f, v = 0.0f;
    RKFloat *x = astart;
    for (k = 0; k < window; k++) {
        v += *x * *x;
        m += *x;
        x++;
    }
    m = m / window;
    return v / window - m * m;
}

void arraySort(RKFloat *array , uint16_t n) {
    // declare some local variables
    int i = 0 , j = 0;
    RKFloat temp = 0;
    for (i = 0; i < n ; i++) {
        for (j = 0; j < n - 1; j++) {
            if (*(array + j) > *(array + j + 1)) {
                temp              = *(array + j);
                *(array + j)      = *(array + j + 1);
                *(array + j + 1)  = temp;
            }
        }
    }
}

// function to calculate the median of the array
float medianf(RKFloat *array , uint16_t n) {
    arraySort(array, n);
    RKFloat median = 0.0f;
    // if number of elements are even
    if (n % 2 == 0)
        median = (*(array + (n - 1) / 2) + *(array + n / 2)) / 2.0f;
    // if number of elements are odd
    else
        median = *(array + n / 2);
    return median;
}

int RKRayNoiseEstimator(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount) {
    int n, j, k, p;
    RKFloat f, x;
    uint16_t u;
    uint16_t noiseGateCount, runSumThreshold;
    RKFloat scaleS, varInLog, medianP, powerThreshold;
    RKPulse *pulse = pulses[0];
    bool flat;
    bool failed = false;
    const uint16_t persist = 10;
    const uint32_t gateCount = pulse->header.downSampledGateCount;
    const uint32_t M = pulseCount < 199 ? pulseCount : 199;                                        // M = np.min([pulseCount, len(fTCN)])
    const uint32_t K = (uint32_t)ceilf(8.e3f / space->gateSizeMeters);                             // running window of size K in step 2
    const uint32_t runSumLength = (uint32_t)round(500.f / M);
    const uint32_t downSampledPulseWidthSampleCount = pulse->header.pulseWidthSampleCount
                                                    * pulse->header.downSampledGateCount
                                                    / pulse->header.gateCount;
    const RKFloat adjustedRunSumLength = 1.12f * (RKFloat)runSumLength;

    for (p = 0; p < 2; p++) {

        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], space->capacity);
        RKZeroOutIQZ(&space->R[p][0], space->capacity);

        RKIQZ *R = &space->R[p][0];

        // Go through all pulses
        n = 0;
        do {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], p);

            // Go through each lag
            RKIQZ Xk = RKGetSplitComplexDataFromPulse(pulses[n], p);
            RKSIMD_zcma(&Xn, &Xk, &R[0], gateCount, 1);                                            // R[k] += X[n] * X[n - k]'
            n++;
        } while (n != pulseCount);


        RKSIMD_izscl(&R[0], 1.0 / ((float)(n)), gateCount);                                        // R[k] /= (n - k)   (unbiased)
        RKSIMD_zabs(&R[0], space->aR[p][0], gateCount);                                            // aR[k] = abs(R[k])

        // // Mean and variance (2nd moment)
        // RKSIMD_zsmul(&space->mX[p], &space->vX[p], gateCount, 1);                               // E{Xh} * E{Xh}' --> var  (step 1)
        // RKSIMD_izsub(&space->R[p][0], &space->vX[p], gateCount);                                // Rh[] - var     --> var  (step 2)
    }

    for (p = 0; p < 2; p++) {
        scaleS = RKSIMD_sum(space->aR[p][0], gateCount) / gateCount;

        noiseGateCount = 0;
        for (k = downSampledPulseWidthSampleCount + 1; k < gateCount; k++) {
            space->S[p][noiseGateCount++] = space->aR[p][0][k] / scaleS;                           // iq_power
        }

        noiseGateCount = 0;
        for (k = 2; k < gateCount - 2; k++) {
            if ((space->S[p][k] < TCN[M] * space->S[p][k - 2]) &&
                (space->S[p][k] < TCN[M] * space->S[p][k + 2])) {
                space->S[p][noiseGateCount++] = space->S[p][k];                                    // P_noise
            }
        }
        for (k = 0; k < noiseGateCount; k++) {
            space->Z[p][k] = log10f(space->S[p][k]);                                               // log_P
            space->mask[k] = 0;
        }

        for (k = 0; k < noiseGateCount - K + 1; k++) {
            varInLog = varf(&space->Z[p][k], K);                                                   // Var_dB
            if (varInLog < varThreshold[M] ) {
                for (j = 0; j < K; j++) {
                    space->mask[k + j] = 1;                                                        // flat_P
                }
            }
        }
        f = 99999.0f;
        flat = false;
        for (k = 0; k < noiseGateCount - K + 1; k++) {
            if (space->mask[k] && !flat) {
                x = space->S[p][k];
                flat = true;
                u = 1;
            } else if (space->mask[k] && flat) {
                x += space->S[p][k];
                u++;
                if (k == (noiseGateCount - K)) {
                    x = x / u;
                    flat = false;
                    if (x < f) {
                        f = x;
                        // RKLog("End intermediateGateCount %d, mS %f (ADU^2)\n", intermediateGateCount, mS);
                    }
                }
            } else if (!space->mask[k] && flat) {
                x /= u;
                flat = false;
                if (x < f) {
                    f = x;
                    // RKLog("intermediateGateCount %d, mS %f (ADU^2)\n", intermediateGateCount, mS);
                }
            }
        }

        if (f > 99998.0f) {
            // print noise estimate fail here
            failed = true;
            break;
        }
        // RKLog("< NoiseEngine > Info. before noiseGateCount = %d\n", noiseGateCount);
        n = 0;
        f = SNRThreshold[M] * f;
        // RKLog("< NoiseEngine > Info. intermediate_power = %f\n", intermediate_power);
        for (k = 0; k < noiseGateCount; k++) {
            if (space->S[p][k] < f) {
                space->V[p][n] = space->S[p][k];
                space->S[p][n++] = space->S[p][k];                                                 // P_noise = P_noise[P_noise < fSNRThr[M] * intermediate_power]
            }
        }
        noiseGateCount = n;
        // RKLog("< NoiseEngine > Info. flat noiseGateCount = %d\n", noiseGateCount);
        medianP = medianf(&space->V[p][0], noiseGateCount);
        // RKLog("I am here noiseGateCount %d, median %f \n", noiseGateCount, median_P);
        for (k = 0; k < noiseGateCount; k++) {
            space->mask[k] = 0;
        }

        for (k = 0; k < noiseGateCount; k++) {
            for (j = -persist / 2; j < persist / 2; j++) {
                if (k + j < 0) {
                    continue;
                } else if (k + j >= noiseGateCount) {
                    break;
                }else if (space->S[p][k+j] < medianP) {
                    // RKLog("I am here k %d, k+j %d \n", k, k+j);
                    space->mask[k] = 1;
                    break;
                }
            }
        }
        u = 0;
        x = 0.0f;
        for (k = 0; k < noiseGateCount; k++) {
            if (space->mask[k]) {
                x += space->S[p][k];
                space->S[p][u++] = space->S[p][k];                                                 //  P_noise = P_noise[ P_mask ]
            }
        }
        noiseGateCount = u;
        f = SNRThreshold[M] * x / noiseGateCount;
        // RKLog("I am here before n, noiseGateCount %d, intermediate_power %f (ADU^2)\n", noiseGateCount, intermediate_power);
        u = 0;
        x = 0.0f;
        for (k = 0; k < noiseGateCount; k++) {
            if (space->S[p][k] < f) {
                x += space->S[p][k];
                space->S[p][u++] = space->S[p][k];                                                 // P_noise = P_noise[ P_noise < fSNRThr[M]*mean_power ]
            }
        }
        noiseGateCount = u;
        f = x / noiseGateCount;
        // RKLog("I am here before n, noise %f (ADU^2)\n", intermediate_power * iq_pm);

        for (n = 0; n < 10; n++) {
            // RKLog("< NoiseEngine > Info. init noiseGateCount = %d\n", noiseGateCount);
            for (k = 0; k < noiseGateCount; k++) {
                space->mask[k] = 0;
            }
            powerThreshold = f * adjustedRunSumLength;
            runSumThreshold = 0;
            x = 0.0f;
            for (k = 0; k < runSumLength; k++) {
                x += space->S[p][k];
            }
            if (x > powerThreshold) {
                runSumThreshold++;
                for (j = 0; j < runSumLength; j++) {
                    space->mask[j] = 1;
                }
            }
            for (k = 0; k < noiseGateCount - runSumLength; k++) {
                x += (space->S[p][k + runSumLength] - space->S[p][k]);
                if (x > powerThreshold) {
                    runSumThreshold++;
                    for (j = 1; j < runSumLength + 1; j++) {
                        space->mask[k + j] = 1;
                    }
                }
            }
            u = 0;
            x = 0.0f;
            for (k = 0; k < noiseGateCount; k++) {
                if (!space->mask[k]) {
                    x += space->S[p][k];
                    space->S[p][u++] = space->S[p][k];
                }
            }
            if (runSumThreshold < runSumPercentile[M] * (noiseGateCount - runSumLength + 1)) {
                RKLog("< NoiseEngine > Info. good break. n = %d, noiseGateCount = %d\n", n, noiseGateCount);
                break;
            } else if (u * M < minSampleCount) {
                break;
            }
            noiseGateCount = u;
            f = x / noiseGateCount;
        }
        if (noiseGateCount * M < failureSampleCount) {
            failed = true;
            space->noise[p] = space->config->noise[p];
            // RKLog("< NoiseEngine > Info. Skipped a ray/channel %d. noiseGateCount*M = %d < %d iEndMinSampleSize\n", p, noiseGateCount*M, iEndMinSampleSize);
        } else {
            // space->noise[p] = intermediatePower * iq_pm;
            space->noise[p] = f * scaleS;
            // RKLog("< NoiseEngine > Info. channel %d. noiseGateCount*M = %d, noise %f (ADU^2)\n", p, noiseGateCount*M, space->noise[p]);
        }
    }
    if (failed) {
        RKLog("< NoiseEngine > Info. ray/channel-F.\n");
        return RKResultFailedToEstimateNoise;
    } else {
        RKLog("< NoiseEngine > Info. ray/channel-S.\n");
        return RKResultSuccess;
    }
}
