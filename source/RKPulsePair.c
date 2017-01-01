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
    //#if defined(DEBUG_MM)
    RKPulse *S = input[0];
    RKPulse *E = input[count -1];
    // Beamwidths of azimuth & elevation
    float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees, E->header.azimuthDegrees);
    float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);
    
    RKLog("%s   %04u...%04u   E%4.2f-%.2f %% %4.2f   A%6.2f-%6.2f %% %4.2f\n",
          name, S->header.i, E->header.i,
          S->header.elevationDegrees, E->header.elevationDegrees, deltaElevation,
          S->header.azimuthDegrees,   E->header.azimuthDegrees,   deltaAzimuth);
    //#endif

    // Process
    // Identify odd pulses and even pulses
    // Calculate R0, R1
    // R0, R1 --> Z, V, W

    int n, k, p;

    // Get the start pulse to know the capacity
    RKPulse *pulse = input[0];
    const uint32_t capacity = pulse->header.capacity;
    const uint32_t gateCount = pulse->header.gateCount;

#if defined(DEBUG_PULSE_PAIR)
    char varname[20];
#endif
    
    //
    //  ACF
    //

    // Go through each polarization
    for (p = 0; p < 2; p++) {
        
        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], capacity);
        for (k = 0; k < RKMaxLag; k++) {
            RKZeroOutIQZ(&space->R[p][k], capacity);
        }

        RKIQZ *R = &space->R[p][0];

        // Go through all pulses
        n = 0;
        do {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(input[n], p);

#if defined(DEBUG_PULSE_PAIR)
            sprintf(varname, "X[%d](%d) = ", p, n);
            showvz(varname, &Xn, 9);
#endif

            RKSIMD_izadd(&Xn, &space->mX[p], pulse->header.gateCount);                                // mX += X
            // Go through each lag
            for (k = 0; k < RKMaxLag; k++) {
                //RKLog(">Lag %d\n", k);
                if (n >= k) {
                    RKIQZ Xk = RKGetSplitComplexDataFromPulse(input[n - k], p);
                    RKSIMD_zcma(&Xn, &Xk, &R[k], pulse->header.gateCount, 1);                         // R[k] += X[n] * X[n - k]'
                }
            }
            n++;
        } while (n != count);
        
        // Divide by n for the average
        RKSIMD_izscl(&space->mX[p], 1.0f / (float)n, gateCount);                                      // mX /= n
        
        // ACF
        for (k = 0; k < MIN(count, RKMaxLag); k++) {
            RKSIMD_izscl(&R[k], 1.0 / ((float)(n - k)), gateCount);                                   // R[k] /= (n - k)
            RKSIMD_zabs(&R[k], space->aR[p][k], gateCount);                                           // aR[k] = abs(R[k])

#if defined(DEBUG_PULSE_PAIR)
            sprintf(varname, "R[%d][%d] = ", p, k);
            showvz(varname, &space->R[p][k], 8);
            sprintf(varname, "R[%d][%d] = ", p, k);
            showv(varname, space->aR[p][k], 8);
#endif
        }
    }
    
    usleep(10 * 1000);

    return count;

}
