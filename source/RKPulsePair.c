//
//  RKPulsePair.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/31/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulsePair.h>

int RKPulsePair(RKFloatRay *output, RKPulse *inputBuffer, const RKModuloPath path, const char *name) {
    // Start and end indices of the I/Q data
    int is = path.origin;
    int ie = RKNextNModuloS(is, path.length - 1, path.modulo);
    // Azimuth beamwidth
    float deltaAzimuth = inputBuffer[ie].header.azimuthDegrees - inputBuffer[is].header.azimuthDegrees;
    if (deltaAzimuth > 180.0f) {
        deltaAzimuth -= 360.0f;
    } else if (deltaAzimuth < -180.0f) {
        deltaAzimuth += 360.0f;
    }
    deltaAzimuth = fabsf(deltaAzimuth);
    // Elevation beamwidth
    float deltaElevation = inputBuffer[ie].header.elevationDegrees - inputBuffer[is].header.elevationDegrees;
    if (deltaElevation > 180.0f) {
        deltaElevation -= 360.0f;
    } else if (deltaElevation < -180.0f) {
        deltaElevation += 360.0f;
    }
    deltaElevation = fabsf(deltaElevation);
    
    #if defined(DEBUG_MM)
    RKLog("%s  %04u...%04u  %5u  E%4.2f-%.2f ^ %4.2f   A%6.2f-%6.2f ^ %4.2f\n",
          name, is, ie, output->header.i,
          inputBuffer[is].header.elevationDegrees, inputBuffer[ie].header.elevationDegrees, deltaElevation,
          inputBuffer[is].header.azimuthDegrees,   inputBuffer[ie].header.azimuthDegrees,   deltaAzimuth);
    #endif
    
    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W
    
    usleep(25 * 1000);
    
    return ie;

}
