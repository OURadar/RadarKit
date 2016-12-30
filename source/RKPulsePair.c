//
//  RKPulsePair.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/31/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulsePair.h>

int RKPulsePair(RKScratch *space, RKPulse *input, const RKModuloPath path, const char *name) {
    // Start and end indices of the I/Q data
    int is = path.origin;
    int ie = RKNextNModuloS(is, path.length - 1, path.modulo);
    RKPulse *pulseStart = RKGetPulse(input, is);
    RKPulse *pulseEnd = RKGetPulse(input, ie);
    // Azimuth beamwidth
    float deltaAzimuth = pulseEnd->header.azimuthDegrees - pulseStart->header.azimuthDegrees;
    if (deltaAzimuth > 180.0f) {
        deltaAzimuth -= 360.0f;
    } else if (deltaAzimuth < -180.0f) {
        deltaAzimuth += 360.0f;
    }
    deltaAzimuth = fabsf(deltaAzimuth);
    // Elevation beamwidth
    float deltaElevation = pulseEnd->header.elevationDegrees - pulseStart->header.elevationDegrees;
    if (deltaElevation > 180.0f) {
        deltaElevation -= 360.0f;
    } else if (deltaElevation < -180.0f) {
        deltaElevation += 360.0f;
    }
    deltaElevation = fabsf(deltaElevation);
    
    #if defined(DEBUG_MM)
    RKLog("%s  %04u...%04u  %5u  E%4.2f-%.2f ^ %4.2f   A%6.2f-%6.2f ^ %4.2f\n",
          name, is, ie, output->header.i,
          pulseStart->header.elevationDegrees, pulseEnd->header.elevationDegrees, deltaElevation,
          pulseStart->header.azimuthDegrees,   pulseEnd->header.azimuthDegrees,   deltaAzimuth);
    #endif
    
    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W
    
    while (is != ie) {
        
        is = RKNextModuloS(is, path.modulo);
    }
    usleep(25 * 1000);
    
    return ie;

}

int RKPulsePairHop(RKScratch *space, RKPulse *input, const RKModuloPath path, const char *name) {
    // Start and end indices of the I/Q data
    int is = path.origin;
    int ie = RKNextNModuloS(is, path.length - 1, path.modulo);
    RKPulse *S = RKGetPulse(input, is);
    RKPulse *E = RKGetPulse(input, ie);
    // Azimuth beamwidth
    float deltaAzimuth = E->header.azimuthDegrees - S->header.azimuthDegrees;
    if (deltaAzimuth > 180.0f) {
        deltaAzimuth -= 360.0f;
    } else if (deltaAzimuth < -180.0f) {
        deltaAzimuth += 360.0f;
    }
    deltaAzimuth = fabsf(deltaAzimuth);
    // Elevation beamwidth
    float deltaElevation = E->header.elevationDegrees - S->header.elevationDegrees;
    if (deltaElevation > 180.0f) {
        deltaElevation -= 360.0f;
    } else if (deltaElevation < -180.0f) {
        deltaElevation += 360.0f;
    }
    deltaElevation = fabsf(deltaElevation);

#if defined(DEBUG_MM)
    RKLog("%s  %04u...%04u  %5u  E%4.2f-%.2f ^ %4.2f   A%6.2f-%6.2f ^ %4.2f\n",
          name, is, ie, output->header.i,
          pulseStart->header.elevationDegrees, pulseEnd->header.elevationDegrees, deltaElevation,
          pulseStart->header.azimuthDegrees,   pulseEnd->header.azimuthDegrees,   deltaAzimuth);
#endif

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

    int k, p;

    // Zero out the tail

    int n = 0;
    while (is != ie) {
        // Go through each lag
        for (k = 0; k < RKMaxLag; k++) {
            // Go through each polarization
            for (p = 0; p < 2; p++) {
                RKIQZ *R = &space->R[p][k];
                RKLog("R @ %p + %p", R->i, R->q);
            }
        }

        is = RKNextModuloS(is, path.modulo);
        n++;
    }
    usleep(25 * 1000);

    return ie;

}
