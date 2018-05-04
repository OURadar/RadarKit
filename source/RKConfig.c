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
    uint32_t  j, k, s;
    char      *string;
    char      stringBuffer[RKMaximumStringLength] = "";
    
    // Use exclusive access here to prevent multiple processes trying to change RKConfig too quickly
    pthread_mutex_lock(&rkGlobalParameters.mutex);

//    RKConfig *newConfig = &configs[RKNextModuloS(*configIndex, configBufferDepth)];
//    RKConfig *oldConfig = &configs[*configIndex];

    RKConfig *newConfig = &configs[*configIndex];
    RKConfig *oldConfig = &configs[RKPreviousModuloS(*configIndex, configBufferDepth)];

    const uint64_t configId = newConfig->i + configBufferDepth;
    
    //RKLog("--- RKConfigAdvance()   Id = %llu ---\n", configId);

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
            case RKConfigKeyPositionMarker:
                newConfig->startMarker = va_arg(args, RKMarker);
				sprintf(stringBuffer, "New Sweep   EL %.2f°   AZ %.2f°  %s   filterCount = %d",
                        newConfig->sweepElevation,
                        newConfig->sweepAzimuth,
                        newConfig->startMarker & RKMarkerPPIScan ? "PPI" : (newConfig->startMarker & RKMarkerRHIScan ? "RHI" : "UNK"),
                        newConfig->filterCount);
                break;
            case RKConfigKeyPRF:
                newConfig->prf[0] = va_arg(args, uint32_t);
                sprintf(stringBuffer, "PRF = %s Hz", RKIntegerToCommaStyleString(newConfig->prf[0]));
                break;
            case RKConfigKeyDualPRF:
                newConfig->prf[0] = va_arg(args, uint32_t);
                newConfig->prf[1] = va_arg(args, uint32_t);
				sprintf(stringBuffer, "Dual PRF = %s / %s Hz", RKIntegerToCommaStyleString(newConfig->prf[0]), RKIntegerToCommaStyleString(newConfig->prf[1]));
                break;
            case RKConfigKeyGateCount:
                newConfig->gateCount[0] = va_arg(args, uint32_t);
				sprintf(stringBuffer, "GateCount = %s", RKIntegerToCommaStyleString(newConfig->gateCount[0]));
                break;
            case RKConfigKeyWaveformId:
                // ???
                //RKParseCommaDelimitedValues(newConfig->waveformId, RKValueTypeUInt32, RKMaxFilterCount, string);
                break;
            case RKConfigKeyVCPDefinition:
                string = va_arg(args, char *);
                if (string == NULL) {
                    sprintf(stringBuffer, "VCP = (NULL)\n");
                } else {
                    sprintf(stringBuffer, "VCP = %s\n", string);
                    strncpy(newConfig->vcpDefinition, string, RKNameLength - 1);
                }
                break;
            case RKConfigKeyNoise:
                newConfig->noise[0] = (RKFloat)va_arg(args, double);
                newConfig->noise[1] = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "Noise = %.2f %.2f ADU^2", newConfig->noise[0], newConfig->noise[1]);
                break;
            case RKConfigKeySystemZCal:
                newConfig->systemZCal[0] = (RKFloat)va_arg(args, double);
                newConfig->systemZCal[1] = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "SystemZCal = %.2f %.2f dB", newConfig->systemZCal[0], newConfig->systemZCal[1]);
                break;
			case RKConfigKeySystemDCal:
				newConfig->systemDCal = (RKFloat)va_arg(args, double);
				sprintf(stringBuffer, "SystemDCal = %.2f dB", newConfig->systemDCal);
				break;
            case RKConfigKeyZCal:
                newConfig->ZCal[0][0] = (RKFloat)va_arg(args, double);
                newConfig->ZCal[1][0] = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "ZCal = %.2f %.2f dB", newConfig->ZCal[0][0], newConfig->ZCal[1][0]);
                break;
            case RKConfigKeyDCal:
                newConfig->DCal[0] = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "DCal = %.2f dB", newConfig->DCal[0]);
                break;
            case RKConfigKeyPCal:
                newConfig->PCal[0] = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "PCal = %.2f rad", newConfig->PCal[0]);
                break;
            case RKConfigKeyZCal2:
                newConfig->ZCal[0][1] = (RKFloat)va_arg(args, double);
                newConfig->ZCal[1][1] = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "ZCal[2] = %.2f %.2f %.2f %.2f dB", newConfig->ZCal[0][0], newConfig->ZCal[0][1], newConfig->ZCal[1][0], newConfig->ZCal[1][1]);
                break;
            case RKConfigKeyDCal2:
                newConfig->DCal[1] = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "DCal[2] = %.2f %.2f dB", newConfig->DCal[0], newConfig->DCal[1]);
                break;
            case RKConfigKeyPCal2:
                newConfig->PCal[1] = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "PCal[2] = %.2f %.2f rad", newConfig->PCal[0], newConfig->PCal[1]);
                break;
            case RKConfigKeyZCals:
                k = va_arg(args, int);
                s = sprintf(stringBuffer, "ZCals =");
                for (j = 0; j < k; j++) {
                    newConfig->ZCal[0][j] = (RKFloat)va_arg(args, double);
                    newConfig->ZCal[1][j] = (RKFloat)va_arg(args, double);
                    s += sprintf(stringBuffer + s, "%s (%.2f, %.2f)", j > 0 ? "," : "", newConfig->ZCal[0][j], newConfig->ZCal[1][j]);
                }
                sprintf(stringBuffer + s, " dB");
                break;
            case RKConfigKeySNRThreshold:
                newConfig->SNRThreshold = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer, "SNRThreshold = %.2f dB", newConfig->SNRThreshold);
                break;
            case RKConfigKeyWaveform:
                strncpy(newConfig->waveform, va_arg(args, char *), RKNameLength - 1);
                sprintf(stringBuffer, "Waveform = '%s'", newConfig->waveform);
                break;
            case RKConfigKeyFilterCount:
                newConfig->filterCount = (uint8_t)va_arg(args, int);
                sprintf(stringBuffer, "Filter Count = %d", newConfig->filterCount);
                break;
            case RKConfigKeyFilterAnchor:
                memcpy(&newConfig->filterAnchors[0], va_arg(args, void *), sizeof(RKFilterAnchor));
                sprintf(stringBuffer, "Filter1 @ i:%d, o:%d, d:%d   %.2f dB",
                        newConfig->filterAnchors[0].inputOrigin,
                        newConfig->filterAnchors[0].outputOrigin,
                        newConfig->filterAnchors[0].maxDataLength,
                        newConfig->filterAnchors[0].sensitivityGain);
                break;
            case RKConfigKeyFilterAnchor2:
                memcpy(&newConfig->filterAnchors[1], va_arg(args, void *), sizeof(RKFilterAnchor));
                sprintf(stringBuffer, "Filter2 @ i:%d, o:%d, d:%d   %.2f dB",
                        newConfig->filterAnchors[1].inputOrigin,
                        newConfig->filterAnchors[1].outputOrigin,
                        newConfig->filterAnchors[1].maxDataLength,
                        newConfig->filterAnchors[1].sensitivityGain);
                break;
            default:
                sprintf(stringBuffer, "Key %d not understood.", key);
                break;
        }
        if (strlen(stringBuffer)) {
			RKLog("%s<ParameterKeeper>%s C%02d %s   ConfigId = %s\n",
				  rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorConfig) : "",
				  rkGlobalParameters.showColor ? RKNoColor : "",
                  *configIndex,
                  stringBuffer,
                  RKIntegerToCommaStyleString(configId));
        }
        // Get the next key
        key = va_arg(args, RKConfigKey);
    }

    va_end(args);

    // Update the identifier and the buffer index
    newConfig->i = configId;
    *configIndex = RKNextModuloS(*configIndex, configBufferDepth);
    
    pthread_mutex_unlock(&rkGlobalParameters.mutex);
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
