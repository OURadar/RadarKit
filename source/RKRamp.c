//
//  RKRamp.c
//  RadarKit
//
//  Created by Boonleng Cheong on 5/5/2021.
//  Copyright (c) Boonleng Cheong. All rights reserved.
//

#include <RKRamp.h>

//
// Hann window
//
static void step(double *r, const int n, double x) {
    int s = (int)ceil(x * n);
    for (int i = s; i < n; i++) {
        r[i] = 1.0;
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
            for (k = 0; k < length; k++) {
                ramp[k] = 1.0;
            }
            break;
        
        case RKRampTypeStep:
            param = va_arg(args, double);
            step(ramp, length, param);
            break;
        
        case RKRampTypeRaisedCosine:
            param = va_arg(args, double);
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
