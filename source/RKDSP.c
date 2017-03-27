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
        RKLog("Noise measurement may be invalid.  k = %d  %x\n", k, pulse->header.s);
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
