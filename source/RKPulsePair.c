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
    RKPulse *S = RKGetPulse(input, is);
    RKPulse *E = RKGetPulse(input, ie);
    
    // Beamwidths of azimuth & elevation
    float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees, E->header.azimuthDegrees);
    float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);
    
    //#if defined(DEBUG_MM)
    RKLog("%s  %04u...%04u  %5u  E%4.2f-%.2f %% %4.2f   A%6.2f-%6.2f %% %4.2f\n",
          name, is, ie, S->header.i,
          S->header.elevationDegrees, E->header.elevationDegrees, deltaElevation,
          S->header.azimuthDegrees,   E->header.azimuthDegrees,   deltaAzimuth);
    //#endif
    
    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W
    
    do {
        //
        RKPulse *pulse = RKGetPulse(input, is);
        is = RKNextModuloS(is, path.modulo);
    } while (is != ie);

    usleep(20 * 1000);
    
    return ie;

}

int RKPulsePairHop(RKScratch *space, RKPulse *input, const RKModuloPath path, const char *name) {
    // Start and end indices of the I/Q data
    int is = path.origin;
    int ie = RKNextNModuloS(is, path.length - 1, path.modulo);
    
    #if defined(DEBUG_MM)
    RKPulse *S = RKGetPulse(input, is);
    RKPulse *E = RKGetPulse(input, ie);
    // Beamwidths of azimuth & elevation
    float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees, E->header.azimuthDegrees);
    float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);
    
    RKLog("%s  %04u...%04u  %5u  E%4.2f-%.2f %% %4.2f   A%6.2f-%6.2f %% %4.2f\n",
          name, is, ie, S->header.i,
          S->header.elevationDegrees, E->header.elevationDegrees, deltaElevation,
          S->header.azimuthDegrees,   E->header.azimuthDegrees,   deltaAzimuth);
    #endif

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

    int k, p;

    // Zero out the tail

    int n = 0;
    do {
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
    } while (is != ie);
    
    usleep(15 * 1000);

    return ie;

}
