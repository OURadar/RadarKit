//
//  RKRadar.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include <RadarKit/RKRadar.h>

//
//
//

// More function definitions


#pragma mark - Helper Functions

void *radarCoPilot(void *in) {
    RKRadar *radar = (RKRadar *)in;
    RKMomentEngine *productGenerator = radar->momentEngine;
    int k = 0;
    int s = 0;

    RKLog("CoPilot started.  %d\n", productGenerator->rayStatusBufferIndex);
    
    while (radar->active) {
        s = 0;
        while (k == productGenerator->rayStatusBufferIndex && radar->active) {
            usleep(1000);
            if (++s % 200 == 0) {
                RKLog("Nothing ...\n");
            }
        }
        RKLog(productGenerator->rayStatusBuffer[k]);
        k = RKNextModuloS(k, RKBufferSSlotCount);
    }
    return NULL;
}

#pragma mark - Life Cycle

RKRadar *RKInitWithDesc(const RKRadarDesc desc) {
    RKRadar *radar;
    size_t bytes;
    int i, k;
    char name[RKNameLength];

    if (desc.initFlags & RKInitFlagVerbose) {
        RKLog("Initializing ... 0x%x", desc.initFlags);
    }
    // Allocate self
    bytes = sizeof(RKRadar);
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&radar, RKSIMDAlignSize, bytes))
    memset(radar, 0, bytes);

    // Set some non-zero variables
    sprintf(radar->name, "%s<MasterController>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(1) : "", rkGlobalParameters.showColor ? RKNoColor : "");
    radar->state |= RKRadarStateBaseAllocated;
    radar->memoryUsage += bytes;

    // Copy over the input flags and constaint the capacity and depth to hard-coded limits
    radar->desc = desc;
    if (radar->desc.pulseBufferDepth > RKBuffer0SlotCount) {
        radar->desc.pulseBufferDepth = RKBuffer0SlotCount;
        RKLog("Pulse buffer clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.pulseBufferDepth));
    } else if (radar->desc.pulseBufferDepth == 0) {
        radar->desc.pulseBufferDepth = 1000;
    }
    if (radar->desc.rayBufferDepth > RKBuffer2SlotCount) {
        radar->desc.rayBufferDepth = RKBuffer2SlotCount;
        RKLog("Ray buffer clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.rayBufferDepth));
    } else if (radar->desc.rayBufferDepth == 0) {
        radar->desc.rayBufferDepth = 720;
    }
    if (radar->desc.pulseCapacity > RKGateCount) {
        radar->desc.pulseCapacity = RKGateCount;
        RKLog("Pulse capacity clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
    } else if (radar->desc.pulseCapacity == 0) {
        radar->desc.pulseCapacity = 256;
    }
    radar->desc.pulseCapacity = (radar->desc.pulseCapacity * sizeof(RKFloat) / RKSIMDAlignSize) * RKSIMDAlignSize / sizeof(RKFloat);
    if (radar->desc.pulseCapacity != desc.pulseCapacity) {
        RKLog("Pulse capacity changed from %s to %s\n", RKIntegerToCommaStyleString(desc.pulseCapacity), RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
    }
    
    // Read in preference file here, override some values
    if (!strlen(radar->desc.name)) {
        sprintf(radar->desc.name, "Radar");
    }
    if (!strlen(radar->desc.filePrefix)) {
        sprintf(radar->desc.filePrefix, "RK");
    }
    RKLog("Radar name = '%s'  prefix = '%s'", radar->desc.name, radar->desc.filePrefix);
    radar->desc.latitude = 35.2550320;
    radar->desc.longitude = -97.4227810;

    // Config buffer
    radar->state |= RKRadarStateConfigBufferAllocating;
    if (radar->desc.configBufferDepth == 0 || radar->desc.configBufferDepth > RKBufferCSlotCount) {
        radar->desc.configBufferDepth = RKBufferCSlotCount;
    }
    bytes = radar->desc.configBufferDepth * sizeof(RKConfig);
    radar->configs = (RKConfig *)malloc(bytes);
    if (radar->configs == NULL) {
        RKLog("Error. Unable to allocate memory for pulse parameters");
        exit(EXIT_FAILURE);
    }
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Config buffer occupies %s B  (%s sets)\n",
              RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(radar->desc.configBufferDepth));
    }
    memset(radar->configs, 0, bytes);
    radar->memoryUsage += bytes;
    for (i = 0; i < radar->desc.configBufferDepth; i++) {
        radar->configs[i].i = i - radar->desc.configBufferDepth;
    }
    radar->state ^= RKRadarStateConfigBufferAllocating;
    radar->state |= RKRadarStateConfigBufferInitialized;
    
    // Health buffer
    radar->state |= RKRadarStateHealthBufferAllocating;
    if (radar->desc.healthBufferDepth == 0) {
        radar->desc.healthBufferDepth = RKBufferHSlotCount;
    }
    bytes = radar->desc.healthBufferDepth * sizeof(RKHealth);
    radar->healths = (RKHealth *)malloc(bytes);
    if (radar->healths == NULL) {
        RKLog("Error. Unable to allocate memory for health status");
        exit(EXIT_FAILURE);
    }
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Health buffer occupies %s B  (%s sets)\n",
              RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(radar->desc.healthBufferDepth));
    }
    memset(radar->healths, 0, bytes);
    radar->memoryUsage += bytes;
    for (i = 0; i < radar->desc.healthBufferDepth; i++) {
        radar->healths[i].i = i - radar->desc.healthBufferDepth;
    }
    if (radar->desc.healthNodeCount == 0 || radar->desc.healthNodeCount > RKHealthNodeCount) {
        radar->desc.healthNodeCount = RKHealthNodeCount;
    }
    bytes = radar->desc.healthNodeCount * sizeof(RKNodalHealth);
    radar->healthNodes = (RKNodalHealth *)malloc(bytes);
    memset(radar->healthNodes, 0, bytes);
    radar->memoryUsage += bytes;
    bytes = radar->desc.healthBufferDepth * sizeof(RKHealth);
    for (i = 0; i < radar->desc.healthNodeCount; i++) {
        radar->healthNodes[i].healths = (RKHealth *)malloc(bytes);
        memset(radar->healthNodes[i].healths, 0, bytes);
    }
    radar->memoryUsage += radar->desc.healthNodeCount * bytes;
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Nodal-health buffers occupy %s B  (%d nodes x %s sets)\n",
              RKIntegerToCommaStyleString(radar->desc.healthNodeCount * bytes), radar->desc.healthNodeCount, RKIntegerToCommaStyleString(radar->desc.healthBufferDepth));
    }
    for (k = 0; k < radar->desc.healthNodeCount; k++) {
        for (i = 0; i < radar->desc.healthBufferDepth; i++) {
            radar->healthNodes[k].healths[i].i = i - radar->desc.healthBufferDepth;
        }
    }
    radar->state ^= RKRadarStateHealthBufferAllocating;
    radar->state |= RKRadarStateHealthBufferInitialized;

    // Position buffer
    if (radar->desc.initFlags & RKInitFlagAllocRawIQBuffer) {
        radar->state |= RKRadarStatePositionBufferAllocating;
        if (radar->desc.positionBufferDepth == 0 || radar->desc.positionBufferDepth > RKBufferPSlotCount) {
            radar->desc.positionBufferDepth = RKBufferPSlotCount;
        }
        bytes = radar->desc.positionBufferDepth * sizeof(RKPosition);
        radar->positions = (RKPosition *)malloc(bytes);
        if (radar->positions == NULL) {
            RKLog("Error. Unable to allocate memory for positions.");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Position buffer occupies %s B  (%s positions)\n",
                  RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(radar->desc.positionBufferDepth));
        }
        radar->memoryUsage += bytes;
        for (i = 0; i < radar->desc.positionBufferDepth; i++) {
            radar->positions[i].i = i - radar->desc.positionBufferDepth;
        }
        radar->state ^= RKRadarStatePositionBufferAllocating;
        radar->state |= RKRadarStatePositionBufferInitialized;
    }

    // IQ buffer
    if (radar->desc.initFlags & RKInitFlagAllocRawIQBuffer) {
        radar->state |= RKRadarStateRawIQBufferAllocating;
        bytes = RKPulseBufferAlloc(&radar->pulses, radar->desc.pulseCapacity, radar->desc.pulseBufferDepth);
        if (bytes == 0 || radar->pulses == NULL) {
            RKLog("Error. Unable to allocate memory for I/Q pulses.\n");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Level I buffer occupies %s B  (%s pulses x %s gates)\n",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.pulseBufferDepth),
                  RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
        }
        for (i = 0; i < radar->desc.pulseBufferDepth; i++) {
            RKPulse *pulse = RKGetPulse(radar->pulses, i);
            size_t offset = (size_t)pulse->data - (size_t)pulse;
            if (offset != RKPulseHeaderPaddedSize) {
                printf("Unexpected offset = %d != %d\n", (int)offset, RKPulseHeaderPaddedSize);
            }
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRawIQBufferAllocating;
        radar->state |= RKRadarStateRawIQBufferInitialized;
    }

    // Moment bufer
    if (radar->desc.initFlags & RKInitFlagAllocMomentBuffer) {
        radar->state |= RKRadarStateRayBufferAllocating;
        k = ((int)ceilf((float)(radar->desc.pulseCapacity / radar->desc.pulseToRayRatio) / (float)RKSIMDAlignSize)) * RKSIMDAlignSize;
        bytes = RKRayBufferAlloc(&radar->rays, k, radar->desc.rayBufferDepth);
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Level II buffer occupies %s B  (%s rays x %d products of %s gates)\n",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.rayBufferDepth),
                  RKMaxProductCount,
                  RKIntegerToCommaStyleString(k));
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRayBufferAllocating;
        radar->state |= RKRadarStateRayBufferInitialized;
    }

    // Controls
    if (radar->desc.controlCount > RKControlCount) {
        radar->desc.controlCount = RKControlCount;
        RKLog("Info. Control count limited to %s\n", RKControlCount);
    } else if (radar->desc.controlCount == 0) {
        radar->desc.controlCount = RKControlCount;
    }
    if (radar->desc.controlCount) {
        radar->state |= RKRadarStateControlsAllocating;
        bytes = radar->desc.controlCount * sizeof(RKControl);
        radar->controls = (RKControl *)malloc(bytes);
        if (radar->controls == NULL) {
            RKLog("Error. Unable to allocate memory for controls.\n");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < radar->desc.controlCount; i++) {
            RKControl *control = &radar->controls[i];
            control->uid = i;
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Controls occupy %s B (%d)",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.controlCount));
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateControlsAllocating;
        radar->state |= RKRadarStateControlsInitialized;
    }

    // Clocks
    radar->pulseClock = RKClockInitWithSize(15000, 10000);
    sprintf(name, "%s<PulseClock>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(10) : "", RKNoColor);
    RKClockSetName(radar->pulseClock, name);
    radar->memoryUsage += sizeof(RKClock);
    
    radar->positionClock = RKClockInit();
    sprintf(name, "%s<PositionClock>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(10) : "", RKNoColor);
    RKClockSetName(radar->positionClock, name);
    RKClockSetOffset(radar->positionClock, -0.02);
    radar->memoryUsage += sizeof(RKClock);
    
    // Health engine
    radar->healthEngine = RKHealthEngineInit();
    RKHealthEngineSetInputOutputBuffers(radar->healthEngine, &radar->desc, radar->healthNodes,
                                        radar->healths, &radar->healthIndex, radar->desc.healthBufferDepth);
    radar->memoryUsage += radar->healthEngine->memoryUsage;
    radar->state |= RKRadarStateHealthEngineInitialized;
    
    // Pulse compression engine
    radar->pulseCompressionEngine = RKPulseCompressionEngineInit();
    RKPulseCompressionEngineSetInputOutputBuffers(radar->pulseCompressionEngine,
                                                  radar->configs, &radar->configIndex, radar->desc.configBufferDepth,
                                                  radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth);
    radar->memoryUsage += radar->pulseCompressionEngine->memoryUsage;
    radar->state |= RKRadarStatePulseCompressionEngineInitialized;

    // Position engine
    radar->positionEngine = RKPositionEngineInit();
    RKPositionEngineSetInputOutputBuffers(radar->positionEngine,
                                          radar->positions, &radar->positionIndex, radar->desc.positionBufferDepth,
                                          radar->configs, &radar->configIndex, radar->desc.configBufferDepth,
                                          radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth);
    radar->memoryUsage += radar->positionEngine->memoryUsage;
    radar->state |= RKRadarStatePositionEngineInitialized;
    
    // Moment engine
    radar->momentEngine = RKMomentEngineInit();
    RKMomentEngineSetInputOutputBuffers(radar->momentEngine, &radar->desc,
                                        radar->configs, &radar->configIndex, radar->desc.configBufferDepth,
                                        radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth,
                                        radar->rays, &radar->rayIndex, radar->desc.rayBufferDepth);
    radar->memoryUsage += radar->momentEngine->memoryUsage;
    radar->state |= RKRadarStateMomentEngineInitialized;
    
    // File engine
    radar->fileEngine = RKFileEngineInit();
    RKFileEngineSetInputOutputBuffers(radar->fileEngine, &radar->desc,
                                      radar->configs, &radar->configIndex, radar->desc.configBufferDepth,
                                      radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth);
    radar->memoryUsage +=  radar->fileEngine->memoryUsage;
    radar->state |= RKRadarStateFileEngineInitialized;

    // Sweep engine
    radar->sweepEngine = RKSweepEngineInit();
    RKSweepEngineSetInputOutputBuffer(radar->sweepEngine, &radar->desc,
                                      radar->configs, &radar->configIndex, radar->desc.configBufferDepth,
                                      radar->rays, &radar->rayIndex, radar->desc.rayBufferDepth);
    radar->memoryUsage += radar->sweepEngine->memoryUsage;
    radar->state |= RKRadarStateSweepEngineInitialized;

    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Radar initialized. Data buffers occupy \033[4m%s B\033[24m (%s GiB)\n",
              RKIntegerToCommaStyleString(radar->memoryUsage),
              RKFloatToCommaStyleString(1.0e-9f * radar->memoryUsage));
    }

    return radar;
}

RKRadar *RKInitQuiet(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocEverythingQuiet;
    desc.pulseCapacity = 4096;
    desc.pulseToRayRatio = 1;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.positionBufferDepth = RKBufferPSlotCount;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitLean(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = 2048;
    desc.pulseToRayRatio = 1;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.positionBufferDepth = RKBufferPSlotCount;
    desc.pulseBufferDepth = 10000;
    desc.rayBufferDepth = 1080;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitMean(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = 8192;
    desc.pulseToRayRatio = 2;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.positionBufferDepth = RKBufferPSlotCount;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitFull(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = RKGateCount;
    desc.pulseToRayRatio = 8;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.positionBufferDepth = RKBufferPSlotCount;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

RKRadar *RKInit(void) {
    return RKInitFull();
}

int RKFree(RKRadar *radar) {
    if (radar->active) {
        RKStop(radar);
    }
    RKSweepEngineFree(radar->sweepEngine);
    RKFileEngineFree(radar->fileEngine);
    RKMomentEngineFree(radar->momentEngine);
    RKHealthEngineFree(radar->healthEngine);
    RKPositionEngineFree(radar->positionEngine);
    RKPulseCompressionEngineFree(radar->pulseCompressionEngine);
    if (radar->pedestal) {
        radar->pedestalFree(radar->pedestal);
    }
    if (radar->transceiver) {
        radar->transceiverFree(radar->transceiver);
    }
    if (radar->healthRelay) {
        radar->healthRelayFree(radar->healthRelay);
    }
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Freeing radar '%s' ...\n", radar->desc.name);
    }
    if (radar->state & RKRadarStateRawIQBufferInitialized) {
        free(radar->pulses);
    }
    if (radar->state & RKRadarStateRayBufferInitialized) {
        free(radar->rays);
    }
    if (radar->state & RKRadarStateConfigBufferInitialized) {
        free(radar->configs);
    }
    if (radar->state & RKRadarStateHealthBufferInitialized) {
        free(radar->healths);
    }
    if (radar->state & RKRadarStatePositionBufferInitialized) {
        free(radar->positions);
    }
    if (radar->state & RKRadarStateControlsInitialized) {
        free(radar->controls);
    }
    RKClockFree(radar->pulseClock);
    RKClockFree(radar->positionClock);
    free(radar);
    return EXIT_SUCCESS;
}

#pragma mark - Hardware Hooks

int RKSetTransceiver(RKRadar *radar,
                     void *initInput,
                     RKTransceiver initRoutine(RKRadar *, void *),
                     int execRoutine(RKTransceiver, const char *, char *),
                     int freeRoutine(RKTransceiver)) {
    radar->transceiverInitInput = initInput;
    radar->transceiverInit = initRoutine;
    radar->transceiverExec = execRoutine;
    radar->transceiverFree = freeRoutine;
    return RKResultNoError;
}

int RKSetPedestal(RKRadar *radar,
                  void *initInput,
                  RKPedestal initRoutine(RKRadar *, void *),
                  int execRoutine(RKPedestal, const char *, char *),
                  int freeRoutine(RKPedestal)) {
    radar->pedestalInitInput = initInput;
    radar->pedestalInit = initRoutine;
    radar->pedestalExec = execRoutine;
    radar->pedestalFree = freeRoutine;
    return RKResultNoError;
}

int RKSetHealthRelay(RKRadar *radar,
                     void *initInput,
                     RKHealthRelay initRoutine(RKRadar *, void *),
                     int execRoutine(RKHealthRelay, const char *, char *),
                     int freeRoutine(RKHealthRelay)) {
    radar->healthRelayInitInput = initInput;
    radar->healthRelayInit = initRoutine;
    radar->healthRelayExec = execRoutine;
    radar->healthRelayFree = freeRoutine;
    return RKResultNoError;
}

#pragma mark - Properties

int RKSetVerbose(RKRadar *radar, const int verbose) {
    if (verbose) {
        RKLog("Setting verbose level to %d ...\n", verbose);
    }
    if (verbose == 1) {
        radar->desc.initFlags |= RKInitFlagVerbose;
    } else if (verbose == 2) {
        radar->desc.initFlags |= RKInitFlagVeryVerbose;
    } else if (verbose >= 3) {
        radar->desc.initFlags |= RKInitFlagVeryVeryVerbose;
    }
    RKClockSetVerbose(radar->pulseClock, verbose);
    RKClockSetVerbose(radar->positionClock, verbose);
    RKHealthEngineSetVerbose(radar->healthEngine, verbose);
    RKPulseCompressionEngineSetVerbose(radar->pulseCompressionEngine, verbose);
    RKPositionEngineSetVerbose(radar->positionEngine, verbose);
    RKMomentEngineSetVerbose(radar->momentEngine, verbose);
    RKFileEngineSetVerbose(radar->fileEngine, verbose);
    RKSweepEngineSetVerbose(radar->sweepEngine, verbose);
    return RKResultNoError;
}

int RKSetDoNotWrite(RKRadar *radar, const bool doNotWrite) {
    RKSweepEngineSetDoNotWrite(radar->sweepEngine, doNotWrite);
    RKFileEngineSetDoNotWrite(radar->fileEngine, doNotWrite);
    return RKResultNoError;
}

int RKSetWaveform(RKRadar *radar, RKWaveform *waveform, const int origin, const int maxDataLength) {
    int k;
    RKPulseCompressionResetFilters(radar->pulseCompressionEngine);
    RKFilterAnchor anchor;
    anchor.origin = origin;
    anchor.length = waveform->depth;
    anchor.maxDataLength = maxDataLength;
    for (k = 0; k < waveform->count; k++) {
        anchor.name = waveform->name[k];
        anchor.subCarrierFrequency = waveform->omega[k];
        RKPulseCompressionSetFilter(radar->pulseCompressionEngine,
                                    waveform->samples[k],
                                    anchor,
                                    k,
                                    0);
    }
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKPulseCompressionFilterSummary(radar->pulseCompressionEngine);
    }
    RKClockReset(radar->pulseClock);
    return RKResultNoError;
}

//
// NOTE: Function incomplete, need to define file format
// ingest the samples, convert, etc.
//
int RKSetWaveformByFilename(RKRadar *radar, const char *filename, const int group, const int maxDataLength) {
    if (radar->pulseCompressionEngine == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    // Load in the waveform
    // Call a transceiver delegate function to fill in the DAC
    // Advance operating parameter, add in the newest set
    RKComplex filter[] = {{1.0f, 0.0f}};
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    anchor.maxDataLength = maxDataLength;
    return RKPulseCompressionSetFilter(radar->pulseCompressionEngine, filter, anchor, group, 0);
}

int RKSetWaveformToImpulse(RKRadar *radar) {
    if (radar->pulseCompressionEngine == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    return RKPulseCompressionSetFilterToImpulse(radar->pulseCompressionEngine);
}

int RKSetWaveformTo121(RKRadar *radar) {
    if (radar->pulseCompressionEngine == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    return RKPulseCompressionSetFilterTo121(radar->pulseCompressionEngine);
}

int RKSetProcessingCoreCounts(RKRadar *radar,
                              const unsigned int pulseCompressionCoreCount,
                              const unsigned int momentProcessorCoreCount) {
    if (radar->state & RKRadarStateLive) {
        return RKResultUnableToChangeCoreCounts;
    }
    RKPulseCompressionEngineSetCoreCount(radar->pulseCompressionEngine, pulseCompressionCoreCount);
    RKMomentEngineSetCoreCount(radar->momentEngine, momentProcessorCoreCount);
    return RKResultNoError;
}

int RKSetPRF(RKRadar *radar, const uint32_t prf) {
    RKAddConfig(radar, RKConfigKeyPRF, prf, RKConfigKeyNull);
    return RKResultNoError;
}

uint32_t RKGetPulseCapacity(RKRadar *radar) {
    if (radar->pulses == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    RKPulse *pulse = RKGetPulse(radar->pulses, 0);
    return pulse->header.capacity;
}

void RKSetPulseTicsPerSeconds(RKRadar *radar, const double delta) {
    RKClockSetDxDu(radar->pulseClock, 1.0 / delta);
}

void RKSetPositionTicsPerSeconds(RKRadar *radar, const double delta) {
    RKClockSetDxDu(radar->positionClock, 1.0 / delta);
}

void RKAddControl(RKRadar *radar, const char *label, const char *command) {
    uint8_t index = radar->controlIndex++;
    if (index >= radar->desc.controlCount) {
        RKLog("Cannot add anymore controls.\n");
        return;
    }
    RKUpdateControl(radar, index, label, command);
}

void RKUpdateControl(RKRadar *radar, uint8_t index, const char *label, const char *command) {
    if (index >= radar->desc.controlCount) {
        RKLog("Error. Unable to update control.\n");
        return;
    }
    RKControl *control = &radar->controls[index];
    strncpy(control->label, label, RKNameLength - 1);
    strncpy(control->command, command, RKMaximumStringLength - 1);
}

#pragma mark - Interaction / State Change

int RKGoLive(RKRadar *radar) {
    radar->active = true;
    
    radar->memoryUsage -= radar->pulseCompressionEngine->memoryUsage;
    radar->memoryUsage -= radar->positionEngine->memoryUsage;
    radar->memoryUsage -= radar->healthEngine->memoryUsage;
    radar->memoryUsage -= radar->momentEngine->memoryUsage;
    radar->memoryUsage -= radar->fileEngine->memoryUsage;
    radar->memoryUsage -= radar->sweepEngine->memoryUsage;
    
    RKPulseCompressionEngineStart(radar->pulseCompressionEngine);
    RKPositionEngineStart(radar->positionEngine);
    RKHealthEngineStart(radar->healthEngine);
    RKMomentEngineStart(radar->momentEngine);
    RKFileEngineStart(radar->fileEngine);
    RKSweepEngineStart(radar->sweepEngine);

    radar->memoryUsage += radar->pulseCompressionEngine->memoryUsage;
    radar->memoryUsage += radar->positionEngine->memoryUsage;
    radar->memoryUsage += radar->healthEngine->memoryUsage;
    radar->memoryUsage += radar->momentEngine->memoryUsage;
    radar->memoryUsage += radar->fileEngine->memoryUsage;
    radar->memoryUsage += radar->sweepEngine->memoryUsage;

    // Add a dummy config to get things started if there hasn't been one
    if (radar->configIndex == 0) {
        RKAddConfig(radar,
                    RKConfigKeyPRF, 1000,
                    RKConfigKeyZCal, -43.0, -43.0,
                    RKConfigKeyNoise, 90.0, 90.0,
                    RKConfigKeyNull);
    }

    // Pedestal
    if (radar->pedestalInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
            RKLog("Initializing pedestal ...");
        }
        if (radar->pedestalFree == NULL || radar->pedestalExec == NULL) {
            RKLog("Error. Pedestal incomplete.");
            exit(EXIT_FAILURE);
        }
        radar->pedestal = radar->pedestalInit(radar, radar->pedestalInitInput);
        radar->state |= RKRadarStatePedestalInitialized;
    }

    // Health Relay
    if (radar->healthRelayInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
            RKLog("Initializing health relay ...");
        }
        if (radar->healthRelayFree == NULL || radar->healthRelayExec == NULL) {
            RKLog("Error. Health relay incomplete.");
            exit(EXIT_FAILURE);
        }
        radar->healthRelay = radar->healthRelayInit(radar, radar->healthRelayInitInput);
        radar->state |= RKRadarStateHealthRelayInitialized;
    }
    
    // Transceiver
    if (radar->transceiverInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
            RKLog("Initializing transceiver ...");
        }
        if (radar->transceiverFree == NULL || radar->transceiverExec == NULL) {
            RKLog("Error. Transceiver incomplete.");
            exit(EXIT_FAILURE);
        }
        radar->transceiver = radar->transceiverInit(radar, radar->transceiverInitInput);
        radar->state |= RKRadarStateTransceiverInitialized;
    }

    // For now, the transceiver is the master controller
    radar->masterController = radar->transceiver;
    radar->masterControllerExec = radar->transceiverExec;
    
    // Update memory usage
    if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
        RKLog("Radar Live. Memory usage = %s B (%s GiB)\n",
              RKIntegerToCommaStyleString(radar->memoryUsage),
              RKFloatToCommaStyleString(1.0e-9f * radar->memoryUsage));
    }

    radar->state |= RKRadarStateLive;
    return 0;
}

int RKWaitWhileActive(RKRadar *radar) {
    uint32_t pulseIndex = radar->pulseIndex;
    uint32_t positionIndex = radar->positionIndex;
    uint32_t healthIndex = radar->healthNodes[RKHealthNodeTweeta].index;
    bool transceiverOkay;
    bool pedestalOkay;
    bool healthOkay;
    int s = 0;
    while (radar->active) {
        if (s++ == 3) {
            s = 0;
            transceiverOkay = pulseIndex == radar->pulseIndex ? false : true;
            pedestalOkay = positionIndex == radar->positionIndex ? false : true;
            healthOkay = healthIndex == radar->healthNodes[RKHealthNodeTweeta].index ? false : true;
            RKHealth *health = RKGetVacantHealth(radar, RKHealthNodeRadarKit);
            sprintf(health->string, "{"
                    "\"Transceiver\":{\"Value\":%s,\"Enum\":%d},"
                    "\"Pedestal\":{\"Value\":%s,\"Enum\":%d},"
                    "\"Health Relay\":{\"Value\":%s,\"Enum\":%d},"
                    "\"Network\":{\"Value\":true,\"Enum\":0},"
                    "\"Recorder\":{\"Value\":true,\"Enum\":3}"
                    "}",
                    transceiverOkay ? "true" : "false", transceiverOkay ? 0 : 2,
                    pedestalOkay ? "true" : "false", pedestalOkay ? 0 : 2,
                    healthOkay ? "true" : "false", healthOkay ? 0 : 2
                    );
            RKSetHealthReady(radar, health);
            pulseIndex = radar->pulseIndex;
            positionIndex = radar->positionIndex;
            healthIndex = radar->healthNodes[RKHealthNodeTweeta].index;
        }
        usleep(100000);
    }
    return 0;
}

int RKStop(RKRadar *radar) {
    radar->active = false;

    if (radar->state & RKRadarStatePedestalInitialized) {
        if (radar->pedestalExec != NULL) {
            radar->pedestalExec(radar->pedestal, "disconnect", radar->pedestalResponse);
        }
        radar->state ^= RKRadarStatePedestalInitialized;
    }
    if (radar->state & RKRadarStateTransceiverInitialized) {
        if (radar->transceiverExec != NULL) {
            radar->transceiverExec(radar->transceiver, "disconnect", radar->transceiverResponse);
        }
        radar->state ^= RKRadarStateTransceiverInitialized;
    }
    if (radar->state & RKRadarStateHealthRelayInitialized) {
        if (radar->healthRelayExec != NULL) {
            radar->healthRelayExec(radar->healthRelay, "disconnect", radar->healthRelayResponse);
        }
        radar->state ^= RKRadarStateHealthRelayInitialized;
    }
    if (radar->state & RKRadarStateFileEngineInitialized) {
        RKFileEngineStop(radar->fileEngine);
        radar->state ^= RKRadarStateFileEngineInitialized;
    }
    if (radar->state & RKRadarStateSweepEngineInitialized) {
        RKSweepEngineStop(radar->sweepEngine);
        radar->state ^= RKRadarStateSweepEngineInitialized;
    }
    if (radar->state & RKRadarStateMomentEngineInitialized) {
        RKMomentEngineStop(radar->momentEngine);
        radar->state ^= RKRadarStateMomentEngineInitialized;
    }
    if (radar->state & RKRadarStateHealthEngineInitialized) {
        RKHealthEngineStop(radar->healthEngine);
        radar->state ^= RKRadarStateHealthEngineInitialized;
    }
    if (radar->state & RKRadarStatePositionEngineInitialized) {
        RKPositionEngineStop(radar->positionEngine);
        radar->state ^= RKRadarStatePositionEngineInitialized;
    }
    if (radar->state & RKRadarStatePulseCompressionEngineInitialized) {
        RKPulseCompressionEngineStop(radar->pulseCompressionEngine);
        radar->state ^= RKRadarStatePulseCompressionEngineInitialized;
    }
    return RKResultSuccess;
}

int RKResetEngines(RKRadar *radar) {
    RKClockReset(radar->pulseClock);
    return RKResultSuccess;
}

#pragma mark - Healths

RKHealth *RKGetVacantHealth(RKRadar *radar, const RKHealthNode node) {
    if (radar->healthEngine == NULL) {
        RKLog("Error. Health engine has not started.\n");
        //exit(EXIT_FAILURE);
        return NULL;
    }
    radar->healthNodes[node].active = true;
    uint32_t index = radar->healthNodes[node].index;
    RKHealth *health = &radar->healthNodes[node].healths[index];
    health->i += radar->desc.healthBufferDepth;
    index = RKNextModuloS(index, radar->desc.healthBufferDepth);
    radar->healthNodes[node].healths[index].flag = RKHealthFlagVacant;
    radar->healthNodes[node].healths[index].string[0] = '\0';
    radar->healthNodes[node].index = index;
    return health;
}

void RKSetHealthReady(RKRadar *radar, RKHealth *health) {
    gettimeofday(&health->time, NULL);
    health->timeDouble = (double)health->time.tv_sec + 1.0e-6 * (double)health->time.tv_usec;
    health->flag = RKHealthFlagReady;
}

RKHealth *RKGetLatestHealth(RKRadar *radar) {
    uint32_t index = RKPreviousModuloS(radar->healthIndex, radar->desc.healthBufferDepth);
    return &radar->healths[index];
}

int RKGetEnumFromLatestHealth(RKRadar *radar, const char *keyword) {
    RKHealth *health = RKGetLatestHealth(radar);
    char *stringObject = RKGetValueOfKey(health->string, keyword);
    if (stringObject) {
        char *stringEnum = RKGetValueOfKey(stringObject, "enum");
        return atoi(stringEnum);
    }
    return -1;
}

#pragma mark - Positions

RKPosition *RKGetVacantPosition(RKRadar *radar) {
    if (radar->positionEngine == NULL) {
        RKLog("Error. Pedestal engine has not started.\n");
        //exit(EXIT_FAILURE);
        return NULL;
    }
    RKPosition *position = &radar->positions[radar->positionIndex];
    position->i += radar->desc.positionBufferDepth;
    position->flag = RKPositionFlagVacant;
    return position;
}

void RKSetPositionReady(RKRadar *radar, RKPosition *position) {
    if (position->flag & ~RKPositionFlagHardwareMask) {
        RKLog("Error. Ingested a position with a flag (0x%08x) outside of allowable value.\n", position->flag);
    }
    position->timeDouble = RKClockGetTime(radar->positionClock, (double)position->tic, &position->time);
    position->flag |= RKPositionFlagReady;
    radar->positionIndex = RKNextModuloS(radar->positionIndex, radar->desc.positionBufferDepth);
    return;
}

#pragma mark - Pulses

RKPulse *RKGetVacantPulse(RKRadar *radar) {
    if (radar->pulses == NULL) {
        RKLog("Error. Buffer for raw pulses has not been allocated.\n");
        exit(EXIT_FAILURE);
    }
    RKPulse *pulse = RKGetPulse(radar->pulses, radar->pulseIndex);
    pulse->header.s = RKPulseStatusVacant;
    pulse->header.i += radar->desc.pulseBufferDepth;
    pulse->header.timeDouble = 0.0;
    pulse->header.time.tv_sec = 0;
    pulse->header.time.tv_usec = 0;
    pulse->header.configIndex = radar->configIndex;
    radar->pulseIndex = RKNextModuloS(radar->pulseIndex, radar->desc.pulseBufferDepth);
    return pulse;
}

void RKSetPulseHasData(RKRadar *radar, RKPulse *pulse) {
    if (pulse->header.i == 0) {
        RKClockReset(radar->pulseClock);
    }
    if (pulse->header.timeDouble == 0.0 && pulse->header.time.tv_sec == 0) {
        pulse->header.timeDouble = RKClockGetTime(radar->pulseClock, (double)pulse->header.t, &pulse->header.time);
    }
    if (pulse->header.gateCount > pulse->header.capacity) {
        RKLog("Error. gateCount = %s > capacity = %s\n",
              RKIntegerToCommaStyleString(pulse->header.gateCount), RKIntegerToCommaStyleString(pulse->header.capacity));
        pulse->header.gateCount = pulse->header.capacity;
    }
    pulse->header.s = RKPulseStatusHasIQData;
    return;
}

void RKSetPulseReady(RKRadar *radar, RKPulse *pulse) {
    pulse->header.s = RKPulseStatusHasIQData | RKPulseStatusHasPosition;
}

#pragma mark - Rays

void RKGetVacanRay(RKRadar *radar) {
    if (radar->rays == NULL) {
        RKLog("Error. Buffer for rays has not been allocated.\n");
        exit(EXIT_FAILURE);
    }
    RKRay *ray = RKGetRay(radar->rays, radar->rayIndex);
    ray->header.s = RKRayStatusVacant;
    ray->header.i += radar->desc.rayBufferDepth;
    ray->header.startTime.tv_sec = 0;
    ray->header.startTime.tv_usec = 0;
    ray->header.endTime.tv_sec = 0;
    ray->header.endTime.tv_usec = 0;
    radar->rayIndex = RKNextModuloS(radar->rayIndex, radar->desc.rayBufferDepth);
}

void RKSetRayReady(RKRadar *radar, RKRay *ray) {
    ray->header.s |= RKRayStatusReady;
}

#pragma mark - Internal Functions

// Users normally don't have to deal with these

void RKAddConfig(RKRadar *radar, ...) {
    va_list args;
    va_start(args, radar);
    if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
        RKLog("RKAddConfig() ...\n");
    }
    return RKConfigAdvance(radar->configs, &radar->configIndex, radar->desc.configBufferDepth, args);
}

RKConfig *RKGetLatestConfig(RKRadar *radar) {
    return &radar->configs[radar->configIndex];
}
