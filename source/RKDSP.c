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

int RKBestStrideOfHops(const int hopCount, const bool showNumbers) {
    int i, k, n;
    int n1, n2, n3;
    int s1, s2, s3;
    int m1, m2, m3;
    float score, maxScore = 0.0f;
    int stride = 1;
    const float a = 1.00f, b = 0.75, c = 0.50f;
    for (i = 1; i < hopCount; i++) {
        n = 0;
        score = 0.0f;
        m1 = hopCount;
        m2 = hopCount;
        m3 = hopCount;
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
            score += a * s1 + b * s2 + c * s3;
            n = RKNextNModuloS(n, i, hopCount);
        }
        score += hopCount * ((a * m1 + b * m2 + c * m3) - 1.00f * ((m1 == 0) + (m2 == 0) + (m3 == 0)));
        if (showNumbers) {
            if (hopCount > 10) {
                printf("stride = %2d   m = %d %d %d   score = %.2f\n", i, m1, m2, m3, score);
            } else {
                printf("stride = %d   m = %d %d %d   score = %.2f\n", i, m1, m2, m3, score);
            }
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
    RKComplex *b = filter->B;
    RKComplex *a = filter->A;
    switch (type) {
        case RKFilterTypeElliptical1:
            //  0.1 radians / sample
            sprintf(filter->name, "Elliptical-0.1");
            filter->bLength = 6;
            filter->aLength = 6;
            memset(b, 0, filter->bLength * sizeof(RKComplex));
            memset(a, 0, filter->aLength * sizeof(RKComplex));
            b++->i = +0.852417f; b++->i = -4.259192f; b++->i = +8.515492f; b++->i = -8.515492f; b++->i = +4.259192f; b->i = -0.852417f;
            a++->i = +1.000000f; a++->i = -4.624806f; a++->i = +8.563920f; a++->i = -7.934279f; a++->i = +3.676572f; a->i = -0.681368f;
            break;
        case RKFilterTypeElliptical2:
            //  0.2 radians / sample
            sprintf(filter->name, "Elliptical-0.2");
            filter->bLength = 6;
            filter->aLength = 6;
            memset(b, 0, filter->bLength * sizeof(RKComplex));
            memset(a, 0, filter->aLength * sizeof(RKComplex));
            b++->i = +0.708621f; b++->i = -3.533447f; b++->i = +7.057262f; b++->i = -7.057262f; b++->i = +3.533447f; b->i = -0.708621f;
            a++->i = +1.000000f; a++->i = -4.256763f; a++->i = +7.289072f; a++->i = -6.260851f; a++->i = +2.690193f; a->i = -0.460609f;
            break;
        case RKFilterTypeElliptical3:
            //  0.3 radians / sample
            sprintf(filter->name, "Elliptical-0.3");
            filter->bLength = 6;
            filter->aLength = 6;
            memset(b, 0, filter->bLength * sizeof(RKComplex));
            memset(a, 0, filter->aLength * sizeof(RKComplex));
            b++->i = +0.590096f; b++->i = -2.932276f; b++->i = +5.846463f; b++->i = -5.846463f; b++->i = +2.932276f; b->i = -0.590096f;
            a++->i = +1.000000f; a++->i = -3.889502f; a++->i = +6.151535f; a++->i = -4.904427f; a++->i = +1.954838f; a->i = -0.305741f;
            break;
        case RKFilterTypeElliptical4:
            //  0.4 radians / sample
            sprintf(filter->name, "Elliptical-0.4");
            filter->bLength = 6;
            filter->aLength = 6;
            memset(b, 0, filter->bLength * sizeof(RKComplex));
            memset(a, 0, filter->aLength * sizeof(RKComplex));
            b++->i = +0.491560f; b++->i = -2.430619f; b++->i = +4.834367f; b++->i = -4.834367f; b++->i = +2.430619f; b->i = -0.491560f;
            a++->i = +1.000000f; a++->i = -3.520292f; a++->i = +5.139254f; a++->i = -3.808225f; a++->i = +1.409268f; a->i = -0.195916f;
        case RKFilterTypeImpulse:
            sprintf(filter->name, "Impulse");
            filter->bLength = 1;
            filter->aLength = 1;
            memset(b, 0, 8 * sizeof(RKComplex));
            memset(a, 0, 8 * sizeof(RKComplex));
            b->i = 1.0f;
            a->i = 1.0f;
            break;
        case RKFilterTypeTest1:
            sprintf(filter->name, "Test1");
            filter->bLength = 2;
            filter->aLength = 3;
            memset(b, 0, 8 * sizeof(RKComplex));
            memset(a, 0, 8 * sizeof(RKComplex));
            b++->i = 0.5f; b->i = 1.0f;
            a++->i = 1.0f; a++->i = 0.2f; a->i = 0.1f;
            break;
        default:
            break;
    }
}
