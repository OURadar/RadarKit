//
//  RKPulsePair.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/31/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulsePair.h>

int RKPulsePair(RKRay *output, RKPulse *inputBuffer, const RKModuloPath path, const char *name) {
    // Start and end indices of the I/Q data
    int is = path.origin;
    int ie = RKNextNModuloS(is, path.length - 1, path.modulo);
    RKPulse *pulseStart = RKGetPulse(inputBuffer, is);
    RKPulse *pulseEnd = RKGetPulse(inputBuffer, ie);
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
