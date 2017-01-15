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

#pragma mark - Helper Functions

void *backgroundTransceiverInit(void *in) {
    RKRadar *radar = (RKRadar *)in;
    radar->transceiver = radar->transceiverInit(radar, radar->transceiverInitInput);
    return NULL;
}

void *backgroundPedestalInit(void *in) {
    RKRadar *radar = (RKRadar *)in;
    radar->pedestal = radar->pedestalInit(radar, radar->pedestalInitInput);
    return NULL;
}

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

RKRadar *RKInitWithDesc(const RKRadarInitDesc desc) {
    RKRadar *radar;
    size_t bytes;

    if (desc.initFlags & RKInitFlagVerbose) {
        RKLog("Initializing ... 0x%x", desc.initFlags);
    }
    // Allocate self
    bytes = sizeof(RKRadar);
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&radar, RKSIMDAlignSize, bytes))
    memset(radar, 0, bytes);

    // Set some non-zero variables
    strncpy(radar->name, "PX-10k", RKNameLength - 1);
    radar->state |= RKRadarStateBaseAllocated;
    radar->memoryUsage += bytes;

    // Copy over the input flags and constaint the capacity and depth to hard-coded limits
    radar->desc = desc;
    if (radar->desc.pulseBufferDepth > RKBuffer0SlotCount) {
        radar->desc.pulseBufferDepth = RKBuffer0SlotCount;
    }
    if (radar->desc.rayBufferDepth > RKBuffer2SlotCount) {
        radar->desc.rayBufferDepth = RKBuffer2SlotCount;
    }
    if (radar->desc.pulseCapacity > RKGateCount) {
        radar->desc.pulseCapacity = RKGateCount;
    }

    // Config buffer
    radar->state |= RKRadarStateConfigBufferAllocating;
    bytes = RKBufferCSlotCount * sizeof(RKOperatingParameters);
    radar->parameters = (RKOperatingParameters *)malloc(bytes);
    if (radar->parameters == NULL) {
        RKLog("Error. Unable to allocate memory for pulse parameters");
        exit(EXIT_FAILURE);
    }
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Config buffer occupies %s B  (%s sets)\n",
              RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(RKBufferCSlotCount));
    }
    memset(radar->parameters, 0, bytes);
    radar->memoryUsage += bytes;
    radar->state ^= RKRadarStateConfigBufferAllocating;
    radar->state |= RKRadarStateConfigBufferIntialized;

    // Position buffer
    if (radar->desc.initFlags & RKInitFlagAllocRawIQBuffer) {
        radar->state |= RKRadarStatePositionBufferAllocating;
        bytes = RKBufferPSlotCount * sizeof(RKPosition);
        radar->positions = (RKPosition *)malloc(bytes);
        if (radar->positions == NULL) {
            RKLog("Error. Unable to allocate memory for positions.");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Position buffer occupies %s B  (%s positions)\n",
                  RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(RKBufferPSlotCount));
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStatePositionBufferAllocating;
        radar->state |= RKRadarStatePositionBufferInitialized;
    }

    // IQ buffer
    if (radar->desc.initFlags & RKInitFlagAllocRawIQBuffer) {
        radar->state |= RKRadarStateRawIQBufferAllocating;
        bytes = RKPulseBufferAlloc(&radar->pulses, radar->desc.pulseCapacity, radar->desc.pulseBufferDepth);
        if (bytes == 0 || radar->pulses == NULL) {
            RKLog("Error. Unable to allocate memory for I/Q pulses.");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Level I buffer occupies %s B  (%s pulses x %s gates)\n",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.pulseBufferDepth),
                  RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
        }
        for (int i = 0; i < radar->desc.pulseBufferDepth; i++) {
            RKPulse *pulse = RKGetPulse(radar->pulses, i);
            size_t offset = (size_t)pulse->data - (size_t)pulse;
            if (offset != RKPulseHeaderSize) {
                printf("Unexpected offset = %d != %d\n", (int)offset, RKPulseHeaderSize);
            }
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRawIQBufferAllocating;
        radar->state |= RKRadarStateRawIQBufferInitialized;
    }

    // Moment bufer
    if (radar->desc.initFlags & RKInitFlagAllocMomentBuffer) {
        radar->state |= RKRadarStateRayBufferAllocating;
        bytes = RKRayBufferAlloc(&radar->rays, radar->desc.pulseCapacity / radar->desc.pulseToRayRatio, radar->desc.rayBufferDepth);
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Level II buffer occupies %s B  (%s rays x %d products of %s gates)\n",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.rayBufferDepth),
                  RKMaxProductCount,
                  RKIntegerToCommaStyleString(radar->desc.pulseCapacity / radar->desc.pulseToRayRatio));
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRayBufferAllocating;
        radar->state |= RKRadarStateRayBufferInitialized;
    }

    // Clocks
    radar->pulseClock = RKClockInitWithSize(5000, 1000);
    RKClockSetName(radar->pulseClock, "<pulseClock>");
    radar->memoryUsage += sizeof(RKClock);
    
    radar->positionClock = RKClockInit();
    RKClockSetName(radar->positionClock, "<positionClock>");
    RKClockSetOffset(radar->positionClock, -0.2);
    radar->memoryUsage += sizeof(RKClock);
    
    // Pulse compression engine
    radar->pulseCompressionEngine = RKPulseCompressionEngineInit();
    RKPulseCompressionEngineSetInputOutputBuffers(radar->pulseCompressionEngine,
                                                  radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth);
    radar->memoryUsage += sizeof(RKPulseCompressionEngine);
    radar->state |= RKRadarStatePulseCompressionEngineInitialized;

    // Position engine
    radar->positionEngine = RKPositionEngineInit();
    RKPositionEngineSetInputOutputBuffers(radar->positionEngine,
                                          radar->positions, &radar->positionIndex, RKBufferPSlotCount,
                                          radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth);
    radar->memoryUsage += sizeof(RKPositionEngine);
    radar->state |= RKRadarStatePositionEngineInitialized;
    
    // Moment engine
    radar->momentEngine = RKMomentEngineInit();
    RKMomentEngineSetInputOutputBuffers(radar->momentEngine,
                                        radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth,
                                        radar->rays, &radar->rayIndex, radar->desc.rayBufferDepth);
    radar->memoryUsage += sizeof(RKMomentEngine);
    radar->state |= RKRadarStateMomentEngineInitialized;

    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Radar initialized. Data buffers occupy %s B (%s GiB)\n",
              RKIntegerToCommaStyleString(radar->memoryUsage),
              RKFloatToCommaStyleString(1.0e-9f * radar->memoryUsage));
    }

    return radar;
}

RKRadar *RKInitQuiet(void) {
    RKRadarInitDesc desc;
    desc.initFlags = RKInitFlagAllocEverythingQuiet;
    desc.pulseCapacity = RKGateCount;
    desc.pulseToRayRatio = 1;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitLean(void) {
    RKRadarInitDesc desc;
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = 2048;
    desc.pulseToRayRatio = 1;
    desc.pulseBufferDepth = 10000;
    desc.rayBufferDepth = 1080;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitMean(void) {
    RKRadarInitDesc desc;
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = 8192;
    desc.pulseToRayRatio = 2;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitFull(void) {
    RKRadarInitDesc desc;
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = RKGateCount;
    desc.pulseToRayRatio = 1;
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
    RKMomentEngineFree(radar->momentEngine);
    RKPositionEngineFree(radar->positionEngine);
    RKPulseCompressionEngineFree(radar->pulseCompressionEngine);
    if (radar->pedestal) {
        radar->pedestalFree(radar->pedestal);
    }
    if (radar->transceiver) {
        radar->transceiverFree(radar->transceiver);
    }
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Freeing radar ...\n");
    }
    if (radar->state & RKRadarStateRawIQBufferInitialized) {
        free(radar->pulses);
    }
    if (radar->state & RKRadarStateRayBufferInitialized) {
        free(radar->rays);
    }
    if (radar->state & RKRadarStateConfigBufferIntialized) {
        free(radar->parameters);
    }
    if (radar->state & RKRadarStatePositionBufferInitialized) {
        free(radar->positions);
    }
    RKClockFree(radar->pulseClock);
    RKClockFree(radar->positionClock);
    free(radar);
    return EXIT_SUCCESS;
}

#pragma mark - Hardware Hooks

int RKSetTransceiver(RKRadar *radar, RKTransceiver routine(RKRadar *, void *), void *initInput) {
    radar->transceiverInit = routine;
    radar->transceiverInitInput = initInput;
    return RKResultNoError;
}

int RKSetTransceiverExec(RKRadar *radar, int routine(RKTransceiver, const char *)) {
    radar->transceiverExec = routine;
    return RKResultNoError;
}

int RKSetTransceiverFree(RKRadar *radar, int routine(RKTransceiver)) {
    radar->transceiverFree = routine;
    return RKResultNoError;
}

int RKSetPedestal(RKRadar *radar, RKPedestal routine(RKRadar *, void *), void *initInput) {
    radar->pedestalInit = routine;
    radar->pedestalInitInput = initInput;
    return RKResultNoError;
}

int RKSetPedestalExec(RKRadar *radar, int routine(RKPedestal, const char *)) {
    radar->pedestalExec = routine;
    return RKResultNoError;
}

int RKSetPedestalFree(RKRadar *radar, int routine(RKPedestal)) {
    radar->pedestalFree = routine;
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
    RKPulseCompressionEngineSetVerbose(radar->pulseCompressionEngine, verbose);
    RKPositionEngineSetVerbose(radar->positionEngine, verbose);
    RKMomentEngineSetVerbose(radar->momentEngine, verbose);
    return RKResultNoError;
}

//
// NOTE: Function incomplete, need to define file format
// ingest the samples, convert, etc.
//
int RKSetWaveform(RKRadar *radar, const char *filename, const int group, const int maxDataLength) {
    if (radar->pulseCompressionEngine == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    // Load in the waveform
    // Call a transceiver delegate function to fill in the DAC
    RKComplex filter[] = {{1.0f, 0.0f}};
    return RKPulseCompressionSetFilter(radar->pulseCompressionEngine, filter, 1, 0, maxDataLength, group, 0);
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

int RKSetPRF(RKRadar *radar, const float prf) {
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

#pragma mark - Interaction / State Change

int RKGoLive(RKRadar *radar) {
    radar->active = true;
    RKPulseCompressionEngineStart(radar->pulseCompressionEngine);
    RKPositionEngineStart(radar->positionEngine);
    RKMomentEngineStart(radar->momentEngine);

    // Pedestal
    if (radar->pedestalInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Initializing pedestal ...");
        }
        if (radar->pedestalFree == NULL) {
            RKLog("Error. Pedestal incomplete.");
            exit(EXIT_FAILURE);
        }
        pthread_create(&radar->pedestalThreadId, NULL, backgroundPedestalInit, radar);
        radar->state ^= RKRadarStatePedestalInitialized;
    }

    // Transceiver
    if (radar->transceiverInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Initializing transceiver ...");
        }
        if (radar->transceiverFree == NULL) {
            RKLog("Error. Transceiver incomplete.");
            exit(EXIT_FAILURE);
        }
        pthread_create(&radar->transceiverThreadId, NULL, backgroundTransceiverInit, radar);
        radar->state |= RKRadarStateTransceiverInitialized;
    }

    // Launch a co-pilot to monitor status of various engines
//    if (radar->momentEngine != NULL && radar->desc.initFlags & RKInitFlagVerbose) {
//        RKLog("Initializing status monitor ...");
//        pthread_create(&radar->monitorThreadId, NULL, radarCoPilot, radar);
//    }
    
    radar->state |= RKRadarStateLive;
    return 0;
}

int RKWaitWhileActive(RKRadar *radar) {
    while (radar->active) {
        usleep(100000);
    }
    return 0;
}

int RKStop(RKRadar *radar) {
    radar->active = false;

    if (radar->state & RKRadarStatePedestalInitialized) {
        radar->pedestalExec(radar->pedestal, "disconnect");
        if (pthread_join(radar->pedestalThreadId, NULL)) {
            RKLog("Error. Failed at the pedestal return.   errno = %d\n", errno);
        }
        radar->state ^= RKRadarStatePedestalInitialized;
    }
    if (radar->state & RKRadarStateTransceiverInitialized) {
        if (pthread_join(radar->transceiverThreadId, NULL)) {
            RKLog("Error. Failed at the transceiver return.   errno = %d\n", errno);
        }
        radar->state ^= RKRadarStateTransceiverInitialized;
    }
    if (radar->state & RKRadarStateMomentEngineInitialized) {
        RKMomentEngineStop(radar->momentEngine);
        radar->state ^= RKRadarStateMomentEngineInitialized;
    }
    if (radar->state & RKRadarStatePositionEngineInitialized) {
        RKPositionEngineStop(radar->positionEngine);
        radar->state ^= RKRadarStatePositionEngineInitialized;
    }
    if (radar->state & RKRadarStatePulseCompressionEngineInitialized) {
        RKPulseCompressionEngineStop(radar->pulseCompressionEngine);
        radar->state ^= RKRadarStatePulseCompressionEngineInitialized;
    }
    return 0;
}

#pragma mark -
#pragma mark Pulses and Rays

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
    radar->pulseIndex = RKNextModuloS(radar->pulseIndex, radar->desc.pulseBufferDepth);
    return pulse;
}

void RKSetPulseHasData(RKRadar *radar, RKPulse *pulse) {
    if (pulse->header.timeDouble == 0.0 && pulse->header.time.tv_sec == 0) {
        pulse->header.timeDouble = RKClockGetTime(radar->pulseClock, (double)pulse->header.t, &pulse->header.time);
    }
    if (pulse->header.gateCount > pulse->header.capacity) {
        RKLog("Error. gateCount should not be larger than the capacity %s > %s",
              RKIntegerToCommaStyleString(pulse->header.gateCount), RKIntegerToCommaStyleString(pulse->header.capacity));
        pulse->header.gateCount = pulse->header.capacity;
    }
    pulse->header.s = RKPulseStatusHasIQData;
    return;
}

void RKSetPulseReady(RKRadar *radar, RKPulse *pulse) {
    pulse->header.s = RKPulseStatusHasIQData | RKPulseStatusHasPosition;
}

RKPosition *RKGetVacantPosition(RKRadar *radar) {
    if (radar->positionEngine == NULL) {
        RKLog("Error. Pedestal engine has not started.\n");
        exit(EXIT_FAILURE);
    }
    RKPosition *position = &radar->positions[radar->positionIndex];
    position->c = 0;
    position->tic = 0;
    position->flag = RKPositionFlagVacant;
    return position;
}

void RKSetPositionReady(RKRadar *radar, RKPosition *position) {
    if (position->flag & ~RKPositionFlagHardwareMask) {
        RKLog("Error. Ingested a position with a flag (0x%08x) outside of allowable value.\n", position->flag);
    }
    position->flag |= RKPositionFlagReady;
    position->timeDouble = RKClockGetTime(radar->positionClock, (double)position->c, &position->time);
    radar->positionIndex = RKNextModuloS(radar->positionIndex, RKBufferPSlotCount);
    return;
}
