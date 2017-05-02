//
//  RKConfig.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/16/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKConfig.h>

void RKConfigAdvanceEllipsis(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, ...) {
    va_list args;
    va_start(args, configBufferDepth);
    return RKConfigAdvance(configs, configIndex, configBufferDepth, args);
}

void RKConfigAdvance(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, va_list args) {
    uint32_t  c;
    char      *string;

    c = *configIndex;                           RKConfig *oldConfig = &configs[c];
    c = RKNextModuloS(c, configBufferDepth);    RKConfig *newConfig = &configs[c];

    // If a mutex is needed, here is the place to lock it.

    // Copy everything
    memcpy(newConfig, oldConfig, sizeof(RKConfig));

    uint32_t key = va_arg(args, RKConfigKey);

    // Modify the values based on the supplied keys
    while (key != RKConfigKeyNull) {
        switch (key) {
            case RKConfigKeySweepElevation:
                newConfig->sweepElevation = (float)va_arg(args, double);
                break;
            case RKConfigKeySweepAzimuth:
                newConfig->sweepAzimuth = (float)va_arg(args, double);
                break;
            case RKConfigPositionMarker:
                newConfig->startMarker = va_arg(args, RKMarker);
                break;
            case RKConfigKeyPRF:
                newConfig->prf[0] = va_arg(args, uint32_t);
                RKLog(">PRF = %s Hz", RKIntegerToCommaStyleString(newConfig->prf[0]));
                break;
            case RKConfigKeyDualPRF:
                newConfig->prf[0] = va_arg(args, uint32_t);
                newConfig->prf[1] = va_arg(args, uint32_t);
                break;
            case RKConfigKeyGateCount:
                newConfig->gateCount[0] = va_arg(args, uint32_t);
                break;
            case RKConfigKeyWaveformId:
                // ???
                //RKParseCommaDelimitedValues(newConfig->waveformId, RKValueTypeUInt32, RKMaxFilterCount, string);
                break;
            case RKConfigKeyVCPDefinition:
                string = va_arg(args, char *);
                if (string == NULL) {
                    RKLog(">VCP = (NULL)\n");
                } else {
                    RKLog(">VCP = %s\n", string);
                    strncpy(newConfig->vcpDefinition, string, RKNameLength - 1);
                }
                break;
            case RKConfigKeyNoise:
                newConfig->noise[0] = (RKFloat)va_arg(args, double);
                newConfig->noise[1] = (RKFloat)va_arg(args, double);
                RKLog(">Noise = %.2f %.2f ADU^2\n", newConfig->noise[0], newConfig->noise[1]);
                break;
            case RKConfigKeyZCal:
                newConfig->ZCal[0][0] = (RKFloat)va_arg(args, double);
                newConfig->ZCal[1][0] = (RKFloat)va_arg(args, double);
                RKLog(">ZCal = %.2f %.2f dB\n", newConfig->ZCal[0][0], newConfig->ZCal[1][0]);
                break;
            case RKConfigKeyDCal:
                newConfig->DCal[0] = (RKFloat)va_arg(args, double);
                RKLog(">DCal = %.2f dB\n", newConfig->DCal[0]);
                break;
            case RKConfigKeyPCal:
                newConfig->PCal[0] = (RKFloat)va_arg(args, double);
                RKLog(">PCal = %.2f rad\n", newConfig->PCal[0]);
                break;
            case RKConfigKeySNRThreshold:
                newConfig->SNRThreshold = (RKFloat)va_arg(args, double);
                RKLog(">SNRThreshold = %.2f dB\n", newConfig->SNRThreshold);
                break;
            case RKConfigKeyWaveform:
                strncpy(newConfig->waveform, va_arg(args, char *), RKNameLength - 1);
                RKLog(">Waveform = '%s'\n", newConfig->waveform);
                break;
            default:
                RKLog(">Key %d not understood.\n", key);
                break;
        }
        // Get the next key
        key = va_arg(args, RKConfigKey);
    }

    va_end(args);

    // Update the identifier and the buffer index
    newConfig->i++;
    *configIndex = c;
}

RKConfig *RKConfigWithId(RKConfig *configs, uint32_t configBufferDepth, uint64_t id) {
    int k = configBufferDepth;
    while (k > 0) {
        k--;
        if (configs[k].i == id) {
            return &configs[k];
        }
    }
    return NULL;
}
