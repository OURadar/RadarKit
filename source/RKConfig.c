//
//  RKConfig.c
//  RadarKit
//
//  Created by Boonleng Cheong on 1/16/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKConfig.h>

void RKConfigAdvanceEllipsis(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, ...) {
    va_list args;
    va_start(args, configBufferDepth);
    return RKConfigAdvance(configs, configIndex, configBufferDepth, args);
}

void RKConfigAdvance(RKConfig *configs, uint32_t *configIndex, uint32_t configBufferDepth, va_list args) {
    uint32_t  j, k, n;
    char      *string;
    char      stringBuffer[RKMaximumFilterCount][RKStatusStringLength];
    char      format[RKStatusStringLength];
    int       w0 = 0, w1 = 0, w2 = 0;

    for (k = 0; k < RKMaximumFilterCount; k++) {
        memset(stringBuffer[k], 0, RKStatusStringLength * sizeof(char));
    }
    
    // Use exclusive access here to prevent multiple processes trying to change RKConfig too quickly
    pthread_mutex_lock(&rkGlobalParameters.mutex);

    RKConfig *newConfig = &configs[*configIndex];
    RKConfig *oldConfig = &configs[RKPreviousModuloS(*configIndex, configBufferDepth)];

    const RKIdentifier configId = newConfig->i + configBufferDepth;
    
    //RKLog("--- RKConfigAdvance()   Id = %llu ---\n", configId);

    RKWaveform *waveform;
    RKWaveformCalibration *waveformCal;

    // Copy everything
    memcpy(newConfig, oldConfig, sizeof(RKConfig));

    n = 0;
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
				sprintf(stringBuffer[0], "New Sweep   EL %.2f°   AZ %.2f°   %s%s%s   %s",
                        newConfig->sweepElevation,
                        newConfig->sweepAzimuth,
                        rkGlobalParameters.showColor ? RKDeepPinkColor : "",
                        RKMarkerScanTypeString(newConfig->startMarker),
                        rkGlobalParameters.showColor ? RKNoColor : "",
                        RKVariableInString("filterCount", &newConfig->waveform->count, RKValueTypeUInt8));
                break;
            case RKConfigKeyPRT:
                newConfig->prt[0] = (RKFloat)va_arg(args, double);
                printf("=== args = %.6f --> %.6f\n", newConfig->prt[0], 1.0 / (double)newConfig->prt[0]);
                if (newConfig->prt[0] != oldConfig->prt[0]) {
                    sprintf(stringBuffer[0], "PRT = %s ms", RKFloatToCommaStyleString(1.0e3f * newConfig->prt[0]));
                }
                break;
            case RKConfigKeyPRF:
                newConfig->prt[0] = (RKFloat)(1.0 / va_arg(args, double));
                if (newConfig->prt[0] != oldConfig->prt[0]) {
                    sprintf(stringBuffer[0], "PRF = %s Hz **", RKIntegerToCommaStyleString((int)roundf(1.0f / newConfig->prt[0])));
                }
                break;
            case RKConfigKeyDualPRF:
                newConfig->prt[0] = (RKFloat)(1.0 / va_arg(args, double));
                newConfig->prt[1] = (RKFloat)(1.0 / va_arg(args, double));
                if (newConfig->prt[0] != oldConfig->prt[0] || newConfig->prt[1] != oldConfig->prt[1]) {
                    sprintf(stringBuffer[0], "Dual PRF = %s / %s Hz", RKFloatToCommaStyleString(newConfig->prt[0]), RKFloatToCommaStyleString(newConfig->prt[1]));
                }
                break;
            case RKConfigKeyPulseGateCount:
                newConfig->pulseGateCount = va_arg(args, uint32_t);
                if (newConfig->pulseGateCount != oldConfig->pulseGateCount) {
                    sprintf(stringBuffer[0], "PulseGateCount = %s", RKIntegerToCommaStyleString(newConfig->pulseGateCount));
                }
                break;
            case RKConfigKeyPulseGateSize:
                newConfig->pulseGateSize = (RKFloat)va_arg(args, double);
                if (newConfig->pulseGateSize != oldConfig->pulseGateSize) {
                    sprintf(stringBuffer[0], "PulseGateSize = %s m", RKFloatToCommaStyleString(newConfig->pulseGateSize));
                }
                break;
            case RKConfigKeyVCPDefinition:
                string = va_arg(args, char *);
                if (string == NULL) {
                    sprintf(stringBuffer[0], "VCP = (NULL)\n");
                } else {
                    sprintf(stringBuffer[0], "VCP = %s\n", string);
                    strncpy(newConfig->vcpDefinition, string, RKMaximumCommandLength - 8);
                }
                break;
            case RKConfigKeySystemNoise:
                newConfig->noise[0] = (RKFloat)va_arg(args, double);
                newConfig->noise[1] = (RKFloat)va_arg(args, double);
                if (newConfig->noise[0] != oldConfig->noise[0] || newConfig->noise[1] != oldConfig->noise[1]) {
                    sprintf(stringBuffer[0], "SystemNoise = %.2f %.2f ADU^2", newConfig->noise[0], newConfig->noise[1]);
                }
                break;
            case RKConfigKeySystemZCal:
                newConfig->systemZCal[0] = (RKFloat)va_arg(args, double);
                newConfig->systemZCal[1] = (RKFloat)va_arg(args, double);
                if (newConfig->systemZCal[0] != oldConfig->systemZCal[0] || newConfig->systemZCal[1] != oldConfig->systemZCal[1]) {
                    sprintf(stringBuffer[0], "SystemZCal = %.2f %.2f dB", newConfig->systemZCal[0], newConfig->systemZCal[1]);
                }
                break;
			case RKConfigKeySystemDCal:
				newConfig->systemDCal = (RKFloat)va_arg(args, double);
                if (newConfig->systemDCal != oldConfig->systemDCal) {
                    sprintf(stringBuffer[0], "SystemDCal = %.2f dB", newConfig->systemDCal);
                }
				break;
            case RKConfigKeySystemPCal:
                newConfig->systemPCal = (RKFloat)va_arg(args, double);
                if (newConfig->systemPCal != oldConfig->systemPCal) {
                    sprintf(stringBuffer[0], "SystemPCal = %.2f rad", newConfig->systemPCal);
                }
                break;
            case RKConfigKeyWaveform:
                waveform = (RKWaveform *)va_arg(args, void *);
                if (waveform == NULL || waveform->filterCounts[0] == 0) {
                    RKLog("Bad input for RKConfigKeyWaveform\n");
                    return;
                }
                newConfig->waveform = waveform;
                strncpy(newConfig->waveformName, waveform->name, RKNameLength - 1);
                sprintf(stringBuffer[0], "Waveform = '%s'", waveform->name);
                break;
            case RKConfigKeyWaveformDecimate:
                waveform = (RKWaveform *)va_arg(args, void *);
                if (waveform == NULL || waveform->filterCounts[0] == 0) {
                    RKLog("Bad input for RKConfigKeyWaveformDecimate\n");
                    return;
                }
                newConfig->waveformDecimate = waveform;
                sprintf(stringBuffer[0], "WaveformDecimate = '%s'", waveform->name);
                break;
            case RKConfigKeyPulseWidth:
                newConfig->pw[0] = (RKFloat)va_arg(args, double);
                for (j = 1; j < RKMaximumFilterCount; j++) {
                    newConfig->pw[j] = newConfig->pw[0];
                }
                sprintf(stringBuffer[0], "PulseWidth = %s us", RKFloatToCommaStyleString(1.0e6 * newConfig->pw[0]));
                break;
            case RKConfigKeyWaveformName:
                strncpy(newConfig->waveformName, va_arg(args, char *), RKNameLength - 1);
                sprintf(stringBuffer[0], "Waveform = '%s'", newConfig->waveformName);
                break;
            case RKConfigKeyWaveformCalibration:
                // Calibration constants in [filterIndex][H/V] specified as N, ZCal[0][H], ZCal[0][V], ZCal[1][H], ZCal[1][V], ..., ZCal[N-1][H], ZCal[N-1][V]
                waveformCal = (RKWaveformCalibration *)va_arg(args, void *);
                if (waveformCal == NULL) {
                    if (newConfig->waveform) {
                        // Only use the first anchor count of the first tone, assume they are all the same
                        for (j = 0; j < newConfig->waveform->filterCounts[0]; j++) {
                            newConfig->ZCal[j][0] = 0.0f;
                            newConfig->ZCal[j][1] = 0.0f;
                            newConfig->DCal[j] = 0.0f;
                            newConfig->PCal[j] = 0.0f;
                            sprintf(stringBuffer[j], "WavCal[0/1] @ All zeroes");
                        }
                    } else {
                        for (j = 0; j < RKMaximumFilterCount; j++) {
                            newConfig->ZCal[j][0] = 0.0f;
                            newConfig->ZCal[j][1] = 0.0f;
                            newConfig->DCal[j] = 0.0f;
                            newConfig->PCal[j] = 0.0f;
                            sprintf(stringBuffer[j], "WavCal[0/1] @ All zeroes");
                        }
                    }
                    break;
                }
                w0 = 0;
                w1 = 0;
                w2 = 0;
                for (j = 0; j < waveformCal->count; j++) {
                    w0 = MAX(w0, (int)log10f(waveformCal->ZCal[j][0]));
                    w0 = MAX(w0, (int)log10f(waveformCal->ZCal[j][1]));
                    w1 = MAX(w1, (int)log10f(waveformCal->DCal[j]));
                    w2 = MAX(w1, (int)log10f(waveformCal->PCal[j]));
                }
                sprintf(format, "WavCal[%%%dd/%%%dd] @ Z = %%+%d.2f, %%+%d.2f dB   D = %%+%d.2f dB   P = %%+%d.2f rad",
                        (int)log10f((float)waveformCal->count) + 1,
                        (int)log10f((float)waveformCal->count) + 1,
                        w0 + 5,
                        w0 + 5,
                        w1 + 5,
                        w2 + 5);
                for (j = 0; j < waveformCal->count; j++) {
                    newConfig->ZCal[j][0] = waveformCal->ZCal[j][0];
                    newConfig->ZCal[j][1] = waveformCal->ZCal[j][1];
                    newConfig->DCal[j] = waveformCal->DCal[j];
                    newConfig->PCal[j] = waveformCal->PCal[j];
                    sprintf(stringBuffer[j], format, j, waveformCal->count, newConfig->ZCal[j][0], newConfig->ZCal[j][1], newConfig->DCal[j], newConfig->PCal[j]);
                }
                break;
            case RKConfigKeySNRThreshold:
                newConfig->SNRThreshold = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer[0], "SNRThreshold = %.2f dB", newConfig->SNRThreshold);
                break;
            case RKConfigKeySQIThreshold:
                newConfig->SQIThreshold = (RKFloat)va_arg(args, double);
                sprintf(stringBuffer[0], "SQIThreshold = %.2f", newConfig->SQIThreshold);
                break;
            case RKConfigKeyRingFilterGateCount:
                newConfig->ringFilterGateCount = va_arg(args, uint32_t);
                sprintf(stringBuffer[0], "RingFilterGateCount = %s", RKIntegerToCommaStyleString(newConfig->ringFilterGateCount));
                break;
            case RKConfigKeyTransitionGateCount:
                newConfig->transitionGateCount = va_arg(args, uint32_t);
                sprintf(stringBuffer[0], "TransitionGateCount = %s", RKIntegerToCommaStyleString(newConfig->transitionGateCount));
                break;
            default:
                sprintf(stringBuffer[0], "Key %d not understood.", key);
                break;
        }
        for (k = 0; k < RKMaximumFilterCount; k++) {
            if (strlen(stringBuffer[k])) {
                RKLog(n > 0 ? ">%s<ParameterKeeper>%s C%02d %s   %s\n" : "%s<ParameterKeeper>%s C%02d %s   %s\n",
                      rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorConfig) : "",
                      rkGlobalParameters.showColor ? RKNoColor : "",
                      *configIndex,
                      stringBuffer[k],
                      RKVariableInString("configId", &configId, RKValueTypeIdentifier));
                stringBuffer[k][0] = '\0';
                n++;
            }
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
