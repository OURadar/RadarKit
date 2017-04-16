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

int RKMeasureNoiseFromPulse(RKFloat *noise, RKPulse *pulse) {
    int j = 0, k, p;
    k = 0;
    do {
        usleep(1000);
    } while (k++ < 100 && !(pulse->header.s & RKPulseStatusCompressed));
    if (k >= 100) {
        RKLog("Noise measurement failed.  k = %d  %x\n", k, pulse->header.s);
        return RKResultTimeout;
    }
    RKComplex *x;
    for (p = 0; p < 2; p++) {
        x = RKGetComplexDataFromPulse(pulse, p);
        noise[p] = 0.0f;
        for (j = 0; j < pulse->header.gateCount; j++) {
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
