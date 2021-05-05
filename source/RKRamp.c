//
//  RKRamp.c
//  RadarKit
//
//  Created by Boonleng Cheong on 5/5/2021.
//  Copyright (c) Boonleng Cheong. All rights reserved.
//

#include <RKRamp.h>

//
// Ones
//
static void _ones(double *r, const int n) {
    for (int i = 0; i < n; i++) {
        r[i] = 1.0;
    }
}

//
// Step
//
static void _step(double *r, const int n, double x) {
    int s = (int)ceil(x * n);
    for (int i = s; i < n; i++) {
        r[i] = 1.0;
    }
}

//
// Linear
//

static void _linear(double *r, const int n) {
    double slope = (double)1.0 / (n - 1);
    for (int i = 0; i < n; i++) {
        r[i] = slope * i;
    }
}

//
// Raised Cosine
//

static void _rcos(double *r, const int n) {
    double omega = M_PI / (n - 1);
    for (int i = 0; i < n; i++) {
        r[i] = 0.5 * (1.0 - cos(omega * i));
    }
}

#pragma mark - Methods

//
// RKRampMake(buffer, RKRampTypeLinear, 20) creates a linear ramp of length 20
// RKRampMake(buffer, RKRampTypeStep, 20, 0.2) creates a step ramp of length 20 with transition at 0.2
//
void RKRampMake(RKFloat *buffer, RKRampType type, const int length, ...) {
    int k;
    double param;
    va_list args;
    
    if (length == 0) {
        return;
    }
    
    double *ramp = (double *)malloc(length * sizeof(double));
    memset(ramp, 0, length * sizeof(double));

    va_start(args, length);

    switch (type) {
        
        case RKRampTypeOnes:
            _ones(ramp, length);
            break;
        
        case RKRampTypeStep:
            param = va_arg(args, double);
            _step(ramp, length, param);
            break;
        
        case RKRampTypeLinear:
            _linear(ramp, length);
            break;

        case RKRampTypeRaisedCosine:
            _rcos(ramp, length);
            break;

        case RKRampTypeZeros:
        default:
            break;
    }
    
    va_end(args);
    
    // Copy to RadarKit buffer
    for (k = 0; k < length; k++) {
        buffer[k] = (RKFloat)ramp[k];
    }
    
    free(ramp);
}
