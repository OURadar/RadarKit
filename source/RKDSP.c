//
//  RKDSP.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKDSP.h>

float RKGetSignedMinorSectorInDegrees(const float angle1, const float angle2) {
    float delta = angle1 - angle2;
    if (delta > 180.0f) {
        delta -= 360.0f;
    } else if (delta < -180.0f) {
        delta += 360.0f;
    }
    return delta;
}

float RKGetMinorSectorInDegrees(const float angle1, const float angle2) {
    return fabs(RKGetSignedMinorSectorInDegrees(angle1, angle2));
}

// Linear interpololation : V_interp = V_before + alpha * (V_after - V_before), alpha in [0, 1]
float RKInterpolatePositiveAngles(const float angleBefore, const float angleAfter, const float alpha) {
    float value = RKGetSignedMinorSectorInDegrees(angleAfter, angleBefore);
    value = angleBefore + alpha * value;
    if (value > 360.0f) {
        value -= 360.0f;
    } else if (value < 0.0f) {
        value += 360.0f;
    }
    return value;
}

float RKInterpolateAngles(const float angleBefore, const float angleAfter, const float alpha) {
    float value = RKGetSignedMinorSectorInDegrees(angleAfter, angleBefore);
    value = angleBefore + alpha * value;
    return value;
}

int RKMeasureNoiseFromPulse(RKFloat *noise, RKPulse *pulse, const int origin) {
    int j = 0, k, p;
    k = 0;
    do {
        usleep(1000);
    } while (k++ < 100 && !(pulse->header.s & RKPulseStatusCompressed));
    if (k >= 100) {
        RKLog("Noise measurement failed.  k = %d  %x\n", k, pulse->header.s);
        return RKResultTimeout;
    }
    if (pulse->header.gateCount < 2 * origin) {
        RKLog("Error. Unable to measure noise at origin %s with %s gates.\n",
              RKIntegerToCommaStyleString(origin), RKIntegerToCommaStyleString(pulse->header.gateCount));
        return RKResultTooBig;
    }
    RKComplex *x;
    for (p = 0; p < 2; p++) {
        x = RKGetComplexDataFromPulse(pulse, p);
        // Add and subtract a few gates to avoid transcient efftects
        x += origin;
        noise[p] = 0.0f;
        for (j = 0; j < pulse->header.gateCount - 2 * origin; j++) {
            noise[p] += x->i * x->i + x->q * x->q;
            x++;
        }
        noise[p] /= (RKFloat)j;
    }
    return RKResultSuccess;
}

int RKBestStrideOfHopsV1(const int hopCount, const bool showNumbers) {
    int i, k, n;
    int n1, n2, n3;
    int s1, s2, s3;
    int m1, m2, m3;
    float score, maxScore = 0.0f;
    int stride = 1;
    const float a = 1.00f, b = 0.50f, c = 0.25f;
    bool used[hopCount];
    int u;
    for (i = 1; i < hopCount; i++) {
        m1 = hopCount;
        m2 = hopCount;
        m3 = hopCount;
        memset(used, false, hopCount * sizeof(bool));
        n = 0;
        for (k = 0; k < hopCount; k++) {
            used[n] = true;
            n = RKNextNModuloS(n, i, hopCount);
        }
        u = true;
        for (k = 0; k < hopCount; k++) {
            u &= used[k];
        }
        n = 0;
        for (k = 0; k < hopCount; k++) {
            n1 = (n + 1 * i) % hopCount;
            n2 = (n + 2 * i) % hopCount;
            n3 = (n + 3 * i) % hopCount;
            s1 = abs(n1 - n);
            s2 = abs(n2 - n);
            s3 = abs(n3 - n);
            if (m1 > s1) {
                m1 = s1;
            }
            if (m2 > s2) {
                m2 = s2;
            }
            if (m3 > s3) {
                m3 = s3;
            }
            //score += a * s1 + b * s2 + c * s3;
            n = RKNextNModuloS(n, i, hopCount);
        }
        //score += hopCount * ((a * m1 + b * m2 + c * m3) - 1.00f * ((m1 == 0) + (m2 == 0) + (m3 == 0)));
        score = a * (a * (m1 >= 2) + c * (m1 >= 3) + c * (m1 == 1))
              + b * (a * (m2 >= 2) + c * (m2 >= 3) + c * (m2 == 1))
              + c * (a * (m3 >= 2) + c * (m3 >= 3) + c * (m3 == 1))
              - 3.0f * ((m1 == 0) + (m2 == 0) + (m3 == 0))
              - 3.0f * (u == false);
        if (showNumbers) {
            printf("stride = %2d   m = %d %d %d   u = %5s   score = %.2f + %.2f + %.2f + ... = %+5.2f\n", i, m1, m2, m3,
                   u ? "true" : "false" ,
                   a * (a * (m1 >= 2) + c * (m1 >= 3) + c * (m1 == 1)),
                   b * (a * (m2 >= 2) + c * (m2 >= 3) + c * (m2 == 1)),
                   c * (a * (m3 >= 2) + c * (m3 >= 3) + c * (m3 == 1)),
                   score);
        }
        if (maxScore < score) {
            maxScore = score;
            stride = i;
        }
    }
    if (showNumbers) {
        n = 0;
        char sequence[1024];
        sequence[1023] = '\0';
        i = 0;
        for (k = 0; k < hopCount; k++) {
            i += snprintf(sequence + i, 1023 - i, " %d", n);
            n = RKNextNModuloS(n, stride, hopCount);
        }
        printf("    Best stride = %d  ==> %s\n", stride, sequence);
    }
    return stride;
}

// Calculate the best stride to produce an optimal hop sequence
// If bestStride is supplied, this function will do the calculation and show the intermediate parameters / values
// Otherwise, calculation is done quietly
static int _RKBestStrideOfHops(const int hopCount, const int bestStride) {
    int i, j, k, n;
    int h[hopCount], s[hopCount], m[hopCount];
    int count = 3;
    int stride = 1;
    
    float score, maxScore = 0.0f;
    const float a = 1.00f, b = 0.25f;
    bool used[hopCount];
    int u;
    
    if (hopCount == 0) {
        return 0;
    }
    
    for (i = 1; i < hopCount; i++) {
        // Figure out how many steps until the hop sequence is complete, i.e., return to the origin
        n = 0;
        count = 0;
        memset(used, false, hopCount * sizeof(bool));
        do {
            n = RKNextNModuloS(n, i, hopCount);
            used[n] = true;
            u = true;
            for (k = 0; k < hopCount; k++) {
                u &= used[k];
            }
            count++;
        } while (n != 0 && count < hopCount && !u);
        // Evaluate the initial position (n) to the next position (n + i * j) modulo hopCount
        memset(used, false, hopCount * sizeof(bool));
        for (j = 0; j < count; j++) {
            m[j] = hopCount;
        }
        n = 0;
        for (k = 0; k < hopCount; k++) {
            used[n] = true;
            for (j = 1; j < count; j++) {
                h[j] = (n + i * j) % hopCount;
                s[j] = abs(h[j] - n);
                m[j] = MIN(m[j], s[j]);
            }
            n = RKNextNModuloS(n, i, hopCount);
        }
        u = true;
        for (k = 0; k < hopCount; k++) {
            u &= used[k];
        }
        score = 0.0f;
        for (j = 1; j < count; j++) {
            score += (2.0 / (RKFloat)(1 << j)) * (a * (m[j] >= 2) + b * (m[j] >= 3) + b * (m[j] == 1));
            score -= count * (m[j] == 0);
        }
        score -= count * (u == false);
        if (bestStride > 0) {
            printf("%2d: m[%d] =", i, count);
            for (j = 1; j < MIN(9, count); j++) {
                printf(" %d", m[j]);
            }
            if (j < count) {
                printf(" ...");
            }
            printf(" %s   score = %.2f", u ? "Y" : "N", (a * (m[j] >= 2) + b * (m[j] >= 3) + b * (m[j] == 1)));
            bool hasNegative = false;
            for (j = 1; j < MIN(9, count); j++) {
                printf(" + %.2f", (2.0 / (RKFloat)(1 << j)) * (a * (m[j] >= 2) + b * (m[j] >= 3) + b * (m[j] == 1)));
                if (m[j] == 0) {
                    hasNegative = true;
                }
            }
            if (hasNegative) {
                printf(" - ...");
            }
            if (j < hopCount) {
                printf(" + ...");
            }
            if (i == bestStride) {
                printf(" = %s%+.2f%s *\n",
                       rkGlobalParameters.showColor ? RKGreenColor : "",
                       score,
                       rkGlobalParameters.showColor ? RKNoColor : "");
            } else {
                printf(" = %+.2f\n", score);
            }
        }
        if (maxScore < score) {
            maxScore = score;
            stride = i;
        }
    }
    if (bestStride) {
        n = 0;
        char sequence[1024];
        sequence[1023] = '\0';
        i = 0;
        for (k = 0; k < hopCount; k++) {
            i += snprintf(sequence + i, 1023 - i, " %d", n);
            n = RKNextNModuloS(n, stride, hopCount);
        }
        printf("    Best stride = %d  ==> %s\n", stride, sequence);
    }
    return stride;
}

int RKBestStrideOfHops(const int hopCount, const bool showNumbers) {
    int bestStride = _RKBestStrideOfHops(hopCount, 0);
    if (showNumbers) {
        _RKBestStrideOfHops(hopCount, bestStride);
    }
    return bestStride;
}

// Technically, this is a function that generates X = Xi + j Xq from Xi
void RKHilbertTransform(RKFloat *w, RKComplex *b, const int n) {
    int i;
    const int nfft = (int)powf(2.0f, ceilf(log2f((float)n)));

    fftwf_complex *in  = (fftwf_complex *)fftwf_malloc(nfft * sizeof(fftwf_complex));
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(nfft * sizeof(fftwf_complex));
    fftwf_plan plan_fwd = fftwf_plan_dft_1d(nfft, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_plan plan_rev = fftwf_plan_dft_1d(nfft, out, in, FFTW_BACKWARD, FFTW_ESTIMATE);

    for (i = 0; i < n; i++) {
        in[i][0] = (float)w[i];
        in[i][1] = 0.0f;
    }
    memset(&in[i][0], 0, (nfft - n) * sizeof(fftwf_complex));

    fftwf_execute(plan_fwd);

    #if defined(DEBUG_HILBERT)
    for (i = 0; i < nfft; i++) {
        printf("i:%2d --> %8.4f%+8.4fi\n", i, in[i][0], in[i][1]);
    }
    printf("\n");

    for (i = 0; i < nfft; i++) {
        printf("i:%2d --> %8.4f%+8.4fi\n", i, out[i][0], out[i][1]);
    }
    printf("\n");
    #endif

    //
    // Coefficients for Hilbert Transform
    //
    //  h = [1 2 2 2 ... 2 1 0 0 ... 0];
    //       ~~~~~~~~~~~~^ ^~~~~~~~~~~
    //         1st half      2nd half
    //
    //

    float s = 1.0f / nfft;

    out[0][0] *= s;
    out[0][1] *= s;
    out[nfft >> 1][0] *= s;
    out[nfft >> 1][1] *= s;

    s = 2.0f / nfft;
    for (i = 1; i < nfft >> 1; i++) {
        out[i][0] *= s;
        out[i][1] *= s;
    }
    memset(&out[(nfft >> 1) + 1][0], 0, ((nfft >> 1) - 1) * sizeof(fftwf_complex));

    fftwf_execute(plan_rev);

    #if defined(DEBUG_HILBERT)
    for (i = 0; i < nfft; i++) {
        printf("H[%2d] --> %8.4f%+8.4fi\n", i, in[i][0], in[i][1]);
    }
    printf("\n");
    #endif

    memcpy(b, in, n * sizeof(RKComplex));

    // Destroy the plans
    fftwf_destroy_plan(plan_fwd);
    fftwf_destroy_plan(plan_rev);
    fftwf_free(in);
    fftwf_free(out);
}

//
// Original idea from Michael Baczynski
// http://lab.polygonal.de/2007/07/18/fast-and-accurate-sinecosine-approximation/
// These functions only works when x in [-PI, PI]
//
void RKFasterSineCosine(float x, float *sin, float *cos) {
    // sin(phi)
    if (x < 0.0f) {
        *sin = 1.27323954f * x + 0.405284735f * x * x;
    } else {
        *sin = 1.27323954f * x - 0.405284735f * x * x;
    }
    // cos(x) = sin(x + PI/2)
    x += 1.57079632f;
    if (x > 3.14159265f) {
        x -= 6.28318531f;
    }
    if (x < 0.0f) {
        *cos = 1.27323954f * x + 0.405284735f * x * x;
    } else {
        *cos = 1.27323954f * x - 0.405284735f * x * x;
    }
}


void RKFastSineCosine(float x, float *sin, float *cos) {
    // sin(phi)
    if (x < 0.0f) {
        *sin = 1.27323954f * x + 0.405284735f * x * x;
        if (*sin < 0.0f) {
            *sin = 0.225f * (*sin * -*sin - *sin) + *sin;
        } else {
            *sin = 0.225f * (*sin * *sin - *sin) + *sin;
        }
    } else {
        *sin = 1.27323954 * x - 0.405284735 * x * x;
        if (*sin < 0.0f) {
            *sin = 0.225f * (*sin * -*sin - *sin) + *sin;
        } else {
            *sin = 0.225f * (*sin * *sin - *sin) + *sin;
        }
    }
    if (*sin < 0) {
        *sin = MAX(*sin, -1.0f);
    } else {
        *sin = MIN(*sin, 1.0f);
    }
    // cos(x) = sin(x + PI/2)
    x += 1.57079632f;
    if (x >  3.14159265f)
        x -= 6.28318531f;
    if (x < 0.0f) {
        *cos = 1.27323954f * x + 0.405284735f * x * x;
        if (*cos < 0.0f) {
            *cos = 0.225f * (*cos * -*cos - *cos) + *cos;
        } else {
            *cos = 0.225f * (*cos * *cos - *cos) + *cos;
        }
    } else {
        *cos = 1.27323954 * x - 0.405284735 * x * x;
        if (*cos < 0.0f) {
            *cos = 0.225f * (*cos * -*cos - *cos) + *cos;
        } else {
            *cos = 0.225f * (*cos * *cos - *cos) + *cos;
        }
    }
    if (*cos < 0) {
        *cos = MAX(*cos, -1.0f);
    } else {
        *cos = MIN(*cos, 1.0f);
    }
}

void RKGetFilterCoefficients(RKIIRFilter *filter, const RKFilterType type) {
    filter->type = type;
    RKComplex *b = filter->B;
    RKComplex *a = filter->A;
    memset(b, 0, RKMaximumIIRFilterTaps * sizeof(RKComplex));
    memset(a, 0, RKMaximumIIRFilterTaps * sizeof(RKComplex));
    switch (type) {
        case RKFilterTypeNull:
            sprintf(filter->name, "Null");
            filter->bLength = 0;
            filter->aLength = 0;
            break;
        case RKFilterTypeElliptical1:
            //  0.1 radians / sample
            sprintf(filter->name, "Elliptical-0.1");
            filter->bLength = 6;
            filter->aLength = 6;
            b++->i = +0.852417f; b++->i = -4.259192f; b++->i = +8.515492f; b++->i = -8.515492f; b++->i = +4.259192f; b->i = -0.852417f;
            a++->i = +1.000000f; a++->i = -4.624806f; a++->i = +8.563920f; a++->i = -7.934279f; a++->i = +3.676572f; a->i = -0.681368f;
            break;
        case RKFilterTypeElliptical2:
            //  0.2 radians / sample
            sprintf(filter->name, "Elliptical-0.2");
            filter->bLength = 6;
            filter->aLength = 6;
            b++->i = +0.708621f; b++->i = -3.533447f; b++->i = +7.057262f; b++->i = -7.057262f; b++->i = +3.533447f; b->i = -0.708621f;
            a++->i = +1.000000f; a++->i = -4.256763f; a++->i = +7.289072f; a++->i = -6.260851f; a++->i = +2.690193f; a->i = -0.460609f;
            break;
        case RKFilterTypeElliptical3:
            //  0.3 radians / sample
            sprintf(filter->name, "Elliptical-0.3");
            filter->bLength = 6;
            filter->aLength = 6;
            b++->i = +0.590096f; b++->i = -2.932276f; b++->i = +5.846463f; b++->i = -5.846463f; b++->i = +2.932276f; b->i = -0.590096f;
            a++->i = +1.000000f; a++->i = -3.889502f; a++->i = +6.151535f; a++->i = -4.904427f; a++->i = +1.954838f; a->i = -0.305741f;
            break;
        case RKFilterTypeElliptical4:
            //  0.4 radians / sample
            sprintf(filter->name, "Elliptical-0.4");
            filter->bLength = 6;
            filter->aLength = 6;
            b++->i = +0.491560f; b++->i = -2.430619f; b++->i = +4.834367f; b++->i = -4.834367f; b++->i = +2.430619f; b->i = -0.491560f;
            a++->i = +1.000000f; a++->i = -3.520292f; a++->i = +5.139254f; a++->i = -3.808225f; a++->i = +1.409268f; a->i = -0.195916f;
            break;
        case RKFilterTypeImpulse:
            sprintf(filter->name, "Impulse");
            filter->bLength = 1;
            filter->aLength = 1;
            b->i = 1.0f;
            a->i = 1.0f;
            break;
        case RKFilterTypeTest1:
            sprintf(filter->name, "Test1");
            filter->bLength = 2;
            filter->aLength = 3;
            b++->i = 0.5f; b->i = 1.0f;
            a++->i = 1.0f; a++->i = 0.2f; a->i = 0.1f;
            break;
        default:
            filter->type = RKFilterTypeUserDefined;
            break;
    }
}

#pragma mark - Common DFT

RKFFTModule *RKFFTModuleInit(const uint32_t capacity, const int verbose) {
    int k;
    RKFFTModule *module = (RKFFTModule *)malloc(sizeof(RKFFTModule));
    if (module == NULL) {
        fprintf(stderr, "Error. Unable to allocate RKFFTModule.\n");
        exit(EXIT_FAILURE);
    }
    memset(module, 0, sizeof(RKFFTModule));
    sprintf(module->name, "%s<CommonFFTModule>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorFFTModule) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    module->verbose = verbose;
    
    // DFT Wisdom
    sprintf(module->wisdomFile, RKFFTWisdomFile);
    if (RKFilenameExists(module->wisdomFile)) {
        RKLog("%s Loading DFT wisdom ...\n", module->name);
        fftwf_import_wisdom_from_filename(module->wisdomFile);
    } else {
        RKLog("%s DFT wisdom file not found.\n", module->name);
        module->exportWisdom = true;
    }

    // Compute the maximum plan size
    uint32_t planCount = (int)ceilf(log2f((float)MIN(RKMaximumGateCount, capacity))) + 1;
    if (planCount >= RKCommonFFTPlanCount) {
        RKLog("%s Error. Unexpected planCount = %s.\n", module->name, RKIntegerToCommaStyleString(planCount));
        exit(EXIT_FAILURE);
    }

    // Temporary buffers
    fftwf_complex *in, *out;
    uint32_t internalCapacity = 1 << (planCount - 1);
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&in, RKSIMDAlignSize, internalCapacity * sizeof(fftwf_complex)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&out, RKSIMDAlignSize, internalCapacity * sizeof(fftwf_complex)))

    // Create FFT plans
    if (module->verbose) {
        RKLog("%s Allocating FFT resources with capacity %s ...\n", module->name, RKIntegerToCommaStyleString(internalCapacity));
    }
    struct timeval toc, tic;
    gettimeofday(&tic, NULL);
    for (k = 0; k < planCount; k++) {
        module->plans[k].size = 1 << k;
        if (module->verbose) {
            RKLog(">%s Setting up plan[%d] @ nfft = %s\n", module->name, k, RKIntegerToCommaStyleString(module->plans[k].size));
        }
        module->plans[k].forwardInPlace = fftwf_plan_dft_1d(module->plans[k].size, in, in, FFTW_FORWARD, FFTW_MEASURE);
        module->plans[k].forwardOutPlace = fftwf_plan_dft_1d(module->plans[k].size, in, out, FFTW_FORWARD, FFTW_MEASURE);
        module->plans[k].backwardInPlace = fftwf_plan_dft_1d(module->plans[k].size, out, out, FFTW_BACKWARD, FFTW_MEASURE);
        module->plans[k].backwardOutPlace = fftwf_plan_dft_1d(module->plans[k].size, out, in, FFTW_BACKWARD, FFTW_MEASURE);
        module->count++;
    }
    gettimeofday(&toc, NULL);
    if (RKTimevalDiff(toc, tic) > 0.5) {
        module->exportWisdom = true;
    }

    free(in);
    free(out);
    return module;
}

void RKFFTModuleFree(RKFFTModule *module) {
    int k;
    if (module->count == 0) {
        fprintf(stderr, "FFT module has no plans.\n");
        return;
    }
    // Export wisdom
    if (module->exportWisdom) {
        if (module->verbose) {
            RKLog("%s Saving DFT wisdom ...\n", module->name);
        }
        fftwf_export_wisdom_to_filename(module->wisdomFile);
    }
    // Destroy DFT plans
    if (module->verbose) {
        RKLog("%s De-allocating FFT resources ...\n", module->name);
    }
    for (k = module->count - 1; k >= 0; k--) {
        if (module->verbose) {
            RKLog(">%s Destroying plan[%d] @ nfft = %s   useCount = %s\n", module->name, k,
                  RKIntegerToCommaStyleString(module->plans[k].size),
                  RKIntegerToCommaStyleString(module->plans[k].count));
        }
        fftwf_destroy_plan(module->plans[k].forwardInPlace);
        fftwf_destroy_plan(module->plans[k].forwardOutPlace);
        fftwf_destroy_plan(module->plans[k].backwardInPlace);
        fftwf_destroy_plan(module->plans[k].backwardOutPlace);
        module->plans[k].forwardInPlace = NULL;
        module->plans[k].forwardOutPlace = NULL;
        module->plans[k].backwardInPlace = NULL;
        module->plans[k].backwardOutPlace = NULL;
    }
    module->count = 0;
    free(module);
}

#pragma mark - SGFit

// Always assume the x-axis is in [0, 2 * M_PI) across count points
RKGaussian RKSGFit(RKFloat *x, RKComplex *y, const int count) {
    int k;
    RKFloat q, s, phi;
    RKComplex omega = {.i = 0.0f, .q = 0.0f};
    RKGaussian gauss = {.A = 0.0f, .mu = 0.0f, .sigma = 0.0f};
    const RKFloat twoPi = 1.0f / (RKFloat)count * 2.0f * M_PI;
    const int halfCount = count / 2;

    s = 0.0f;
    RKComplex *yy = y;
    for (k = 0; k < count; k++) {
        q = yy->i * yy->i + yy->q * yy->q;
        s += q;
        if (k >= halfCount) {
            phi = (RKFloat)(k - count) * twoPi;
        } else {
            phi = (RKFloat)k * twoPi;
        }
        gauss.A = sqrtf(q);
        omega.i += gauss.A * cosf(phi);
        omega.q += gauss.A * sinf(phi);
        y++;
    }

    return gauss;
}
