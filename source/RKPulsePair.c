//
//  RKPulsePair.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/31/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulsePair.h>

int RKPulsePair(RKScratch *space, RKPulse **input, const uint16_t count, const char *name) {
    usleep(20 * 1000);
    
    return 0;

}

int RKPulsePairHop(RKScratch *space, RKPulse **input, const uint16_t count, const char *name) {
//    struct timeval tic, toc;
//    gettimeofday(&tic, NULL);
    
    #if defined(DEBUG_MM)
    RKPulse *S = input[0];
    RKPulse *E = input[count -1];
    // Beamwidths of azimuth & elevation
    float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees, E->header.azimuthDegrees);
    float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);
    
    RKLog("%s   %04u...%04u   E%4.2f-%.2f %% %4.2f   A%6.2f-%6.2f %% %4.2f\n",
          name, S->header.i, E->header.i,
          S->header.elevationDegrees, E->header.elevationDegrees, deltaElevation,
          S->header.azimuthDegrees,   E->header.azimuthDegrees,   deltaAzimuth);
    #endif

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

//    gettimeofday(&toc, NULL);
//    RKLog("Diff time = %.4f ms", 1.0e3 * RKTimevalDiff(toc, tic));

    return count;

}
