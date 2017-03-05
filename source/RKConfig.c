//
//  RKConfig.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/16/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKConfig.h>

void RKConfigAdvance(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, ...) {
    va_list   arg;
    uint32_t  c;
    char      *string;

    c = *configIndex;                           RKConfig *oldConfig = &configs[c];
    c = RKNextModuloS(c, configBufferDepth);    RKConfig *newConfig = &configs[c];

    // If a mutex is needed, here is the place to lock it.

    // Copy everything
    memcpy(newConfig, oldConfig, sizeof(RKConfig));

    va_start(arg, configBufferDepth);

    uint32_t key = va_arg(arg, RKConfigKey);

    // Modify the values based on the supplied keys
    while (key != RKConfigKeyNull) {
        switch (key) {
            case RKConfigKeySweepElevation:
                newConfig->sweepElevation = (float)va_arg(arg, double);
                break;
            case RKConfigKeySweepAzimuth:
                newConfig->sweepAzimuth = (float)va_arg(arg, double);
                break;
            case RKConfigPositionMarker:
                newConfig->startMarker = va_arg(arg, RKMarker);
                break;
            case RKConfigKeyPRF:
                newConfig->prf[0] = va_arg(arg, uint32_t);
                break;
            case RKConfigKeyDualPRF:
                newConfig->prf[0] = va_arg(arg, uint32_t);
                newConfig->prf[1] = va_arg(arg, uint32_t);
                break;
            case RKConfigKeyGateCount:
                newConfig->gateCount[0] = va_arg(arg, uint32_t);
                break;
            case RKConfigKeyWaveformId:
                // ???
                //RKParseCommaDelimitedValues(newConfig->waveformId, RKValueTypeUInt32, RKMaxFilterCount, string);
                break;
            case RKConfigKeyVCPDefinition:
                string = va_arg(arg, char *);
                RKLog(">string = %s", string);
                break;
            case RKConfigKeyZCal:
                newConfig->ZCal[0] = (RKFloat)va_arg(arg, double);
                newConfig->ZCal[1] = (RKFloat)va_arg(arg, double);
                RKLog(">ZCal = %.2f %.2f dB\n", newConfig->ZCal[0], newConfig->ZCal[1]);
                break;
            case RKConfigKeyNoise:
                newConfig->noise[0] = (RKFloat)va_arg(arg, double);
                newConfig->noise[1] = (RKFloat)va_arg(arg, double);
                RKLog(">Noise = %.2f %.2f ADU^2\n", newConfig->noise[0], newConfig->noise[1]);
                break;
            default:
                break;
        }
        // Get the next key
        key = va_arg(arg, RKConfigKey);
    }

    va_end(arg);

    // Update
    newConfig->i++;
    *configIndex = c;
}
