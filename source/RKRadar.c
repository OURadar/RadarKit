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

#pragma mark -

RKTransceiver backgroundTransceiverInit(void *in) {
    RKRadar *radar = (RKRadar *)in;
    return radar->transceiverInit(radar, radar->transceiverInitInput);
}

RKPedestal backgroundPedestalInit(void *in) {
    //RKRadar *radar = (RKRadar *)in;
    return NULL;
}

#pragma mark -

RKRadar *RKInitWithDesc(const RKRadarInitDesc desc) {
    RKRadar *radar;
    size_t bytes;

    if (desc.initFlags & RKInitFlagVerbose) {
        RKLog("Initializing ... 0x%x", desc.initFlags);
    }
    // Allocate self
    bytes = sizeof(RKRadar);
    if (posix_memalign((void **)&radar, RKSIMDAlignSize, bytes)) {
        fprintf(stderr, "Error allocation memory for radar.\n");
        return NULL;
    }
    memset(radar, 0, bytes);

    // Set some non-zero variables
    radar->memoryUsage += bytes;
    radar->state |= RKRadarStateBaseAllocated;
    radar->active = true;

    // Copy over the input flags and constaint the capacity and depth to hard-coded limits
    radar->desc = desc;
    if (radar->desc.pulseBufferDepth > RKBuffer0SlotCount) {
        radar->desc.pulseBufferDepth = RKBuffer0SlotCount;
    }
    if (radar->desc.pulseCapacity > RKGateCount) {
        radar->desc.pulseCapacity = RKGateCount;
    }
    if (radar->desc.rayBufferDepth > RKBuffer2SlotCount) {
        radar->desc.rayBufferDepth = RKBuffer2SlotCount;
    }

    // Config buffer
    radar->state |= RKRadarStateConfigBufferAllocating;
    bytes = RKBufferCSlotCount * sizeof(RKOperatingParameters);
    radar->parameters = (RKOperatingParameters *)malloc(bytes);
    if (radar->parameters == NULL) {
        RKLog("Error. Unable to allocate memory for pulse parameters");
        return NULL;
    }
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Config buffer occupies %s B\n", RKIntegerToCommaStyleString(bytes));
    }
    memset(radar->parameters, 0, bytes);
    radar->memoryUsage += bytes;
    radar->state ^= RKRadarStateConfigBufferAllocating;
    radar->state |= RKRadarStateConfigBufferIntialized;

    // IQ buffer
    if (radar->desc.initFlags & RKInitFlagAllocRawIQBuffer) {
        radar->state |= RKRadarStateRawIQBufferAllocating;
        bytes = RKPulseBufferAlloc((void **)&radar->pulses, radar->desc.pulseCapacity, radar->desc.pulseBufferDepth);
        if (bytes == 0 || radar->pulses == NULL) {
            RKLog("Error. Unable to allocate memory for I/Q pulses");
            return NULL;
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Level I buffer occupies %s B  (%s gates x %s pulses)\n",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.pulseCapacity),
                  RKIntegerToCommaStyleString(radar->desc.pulseBufferDepth));
        }
        for (int i = 0; i < radar->desc.pulseBufferDepth; i++) {
            RKPulse *pulse = RKGetPulse(radar->pulses, i);
            size_t offset = (size_t)pulse->data - (size_t)pulse;
            if (offset != 256) {
                printf("Unexpected offset = %d != 256\n", (int)offset);
            }
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRawIQBufferAllocating;
        radar->state |= RKRadarStateRawIQBufferInitialized;
    }

    // Moment bufer
    if (radar->desc.initFlags & RKInitFlagAllocMomentBuffer) {
        radar->state |= RKRadarStateRayBufferAllocating;
        bytes = RKRayBufferAlloc((void **)&radar->rays, radar->desc.pulseCapacity / radar->desc.pulseRayRatio, radar->desc.rayBufferDepth);
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Level II buffer occupies %s B  (%s gates x %s rays)\n",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.pulseCapacity / radar->desc.pulseRayRatio),
                  RKIntegerToCommaStyleString(radar->desc.rayBufferDepth));
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRayBufferAllocating;
        radar->state |= RKRadarStateRayBufferInitialized;
    }

    // Pulse compression engine
    radar->pulseCompressionEngine = RKPulseCompressionEngineInit();
    RKPulseCompressionEngineSetInputOutputBuffers(radar->pulseCompressionEngine,
                                                  radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth);
    radar->state |= RKRadarStatePulseCompressionEngineInitialized;

    // Moment engine
    radar->momentEngine = RKMomentEngineInit();
    RKMomentEngineSetInputOutputBuffers(radar->momentEngine,
                                        radar->pulses, &radar->pulseIndex, radar->desc.pulseBufferDepth,
                                        radar->rays, &radar->rayIndex, radar->desc.rayBufferDepth);
    radar->state |= RKRadarStateMomentEngineInitialized;

    // TCP/IP socket server
    //
    // NOTE 2016/12/30 - Server should be on top of radar, managing a few radars.
    //
//    radar->socketServer = RKServerInit();
//    RKServerSetCommandHandler(radar->socketServer, &socketCommandHandler);
//    RKServerSetStreamHandler(radar->socketServer, &socketStreamHandler);
//    radar->state |= RKRadarStateSocketServerInitialized;

    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Radar initialized. Data buffers occupiy %s B (%s GiB)\n",
              RKIntegerToCommaStyleString(radar->memoryUsage),
              RKFloatToCommaStyleString(1.0e-9f * radar->memoryUsage));
    }

    return radar;
}

RKRadar *RKInitQuiet(void) {
    RKRadarInitDesc desc;
    desc.initFlags = RKInitFlagAllocEverythingQuiet;
    desc.pulseCapacity = RKGateCount;
    desc.pulseRayRatio = 1;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitLean(void) {
    RKRadarInitDesc desc;
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = 2048;
    desc.pulseRayRatio = 1;
    desc.pulseBufferDepth = 5000;
    desc.rayBufferDepth = 1000;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitMean(void) {
    RKRadarInitDesc desc;
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = RKGateCount / 2;
    desc.pulseRayRatio = 2;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitFull(void) {
    RKRadarInitDesc desc;
    desc.initFlags = RKInitFlagAllocEverything;
    desc.pulseCapacity = RKGateCount;
    desc.pulseRayRatio = 1;
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
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Freeing radar ...\n");
    }
    RKPulseCompressionEngineFree(radar->pulseCompressionEngine);
    RKMomentEngineFree(radar->momentEngine);
    //RKServerFree(radar->socketServer);
    while (radar->state & RKRadarStateRawIQBufferAllocating) {
        usleep(1000);
    }
    if (radar->state & RKRadarStateRawIQBufferInitialized) {
        free(radar->pulses);
    }
    while (radar->state & RKRadarStateRayBufferAllocating) {
        usleep(1000);
    }
    if (radar->state & RKRadarStateRayBufferInitialized) {
        free(radar->rays);
    }
    free(radar);
    return EXIT_SUCCESS;
}

#pragma mark -

int RKSetVerbose(RKRadar *radar, const int verbose) {
    if (verbose == 1) {
        radar->desc.initFlags |= RKInitFlagVerbose;
    } else if (verbose == 2) {
        radar->desc.initFlags |= RKInitFlagVeryVerbose;
    } else if (verbose == 3) {
        radar->desc.initFlags |= RKInitFlagVeryVeryVerbose;
    }
    RKPulseCompressionEngineSetVerbose(radar->pulseCompressionEngine, verbose);
    RKMomentEngineSetVerbose(radar->momentEngine, verbose);
    return RKResultNoError;
}

// Function incomplete
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

int RKSetTransceiver(RKRadar *radar, RKTransceiver init(RKRadar *, void *), void *initInput) {
    radar->transceiverInit = init;
    radar->transceiverInitInput = initInput;
    return RKResultNoError;
}

int RKSetPRF(RKRadar *radar, const float prf) {
    return RKResultNoError;
}

size_t RKGetPulseCapacity(RKRadar *radar) {
    if (radar->pulses == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    return radar->pulses[0].header.capacity;
}

#pragma mark -

int RKGoLive(RKRadar *radar) {
    RKPulseCompressionEngineStart(radar->pulseCompressionEngine);
    RKMomentEngineStart(radar->momentEngine);
    //RKServerActivate(radar->socketServer);

    // Operation parameters
    if (radar->transceiverInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Initializing transceiver ...");
        }
        pthread_create(&radar->transceiverThreadId, NULL, backgroundTransceiverInit, radar);
    }

    if (radar->pedestalInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Initializing pedestal ...");
        }
        pthread_create(&radar->pedestalThreadId, NULL, backgroundPedestalInit, radar);
    }

    return 0;
}

int RKWaitWhileActive(RKRadar *radar) {
    while (radar->active) {
        sleep(1);
    }
    return 0;
}

int RKStop(RKRadar *radar) {
    radar->active = false;
    if (radar->state & RKRadarStatePulseCompressionEngineInitialized) {
        RKPulseCompressionEngineStop(radar->pulseCompressionEngine);
    }
    if (radar->state & RKRadarStateMomentEngineInitialized) {
        RKMomentEngineStop(radar->momentEngine);
    }
//    if (radar->state & RKRadarStateSocketServerInitialized) {
//        RKServerStop(radar->socketServer);
//        RKServerWait(radar->socketServer);
//    }
    if (radar->transceiverInit != NULL) {
        pthread_join(radar->transceiverThreadId, NULL);
    }
    if (radar->pedestalInit != NULL) {
        pthread_join(radar->pedestalThreadId, NULL);
    }
    return 0;
}

RKPulse *RKGetVacantPulse(RKRadar *radar) {
    if (radar->pulses == NULL) {
        RKLog("Error. Buffer for raw pulses has not been allocated.\n");
        exit(EXIT_FAILURE);
    }
    RKPulse *pulse = RKGetPulse(radar->pulses, radar->pulseIndex);
    pulse->header.s = RKPulseStatusVacant;
    pulse->header.i += radar->desc.pulseBufferDepth;
    radar->pulseIndex = RKNextModuloS(radar->pulseIndex, radar->desc.pulseBufferDepth);
    return pulse;
}

void RKSetPulseReady(RKPulse *pulse) {
    pulse->header.s = RKPulseStatusReady;
}
