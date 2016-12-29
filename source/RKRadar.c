//
//  RKRadar.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include <RadarKit/RKRadar.h>

RKRadar *RKInitWithFlags(const RKEnum flags);

//
//
//

#pragma mark -

RKRadar *RKInit(void) {
    return RKInitWithFlags(RKInitFlagAllocEverything);
}

RKRadar *RKInitWithFlags(const RKEnum flags) {
    RKRadar *radar;
    size_t bytes;

    // Allocate itself
    bytes = sizeof(RKRadar);
    if (posix_memalign((void **)&radar, RKSIMDAlignSize, bytes)) {
        fprintf(stderr, "Error allocation memory for radar.\n");
        return NULL;
    }
    memset(radar, 0, bytes);
    radar->memoryUsage += bytes;
    radar->state |= RKRadarStateBaseAllocated;

    // Copy over the input flags
    radar->initFlags = flags;

    // Set some non-zero variables
    radar->active = true;

    // Config buffer
    radar->state |= RKRadarSTateConfigBufferAllocating;
    bytes = RKBufferCSlotCount * sizeof(RKRadarConfig);
    radar->config = (RKRadarConfig *)malloc(bytes);
    if (radar->config == NULL) {
        RKLog("ERROR. Unable to allocate memory for config");
        return NULL;
    }
    if (flags & RKInitFlagVerbose) {
        RKLog("Config buffer occupies %s B\n", RKIntegerToCommaStyleString(bytes));
    }
    memset(radar->config, 0, bytes);
    radar->memoryUsage += bytes;
    radar->state ^= RKRadarSTateConfigBufferAllocating;
    radar->state |= RKRadarSTateConfigBufferIntialized;

    // I/Q buffer
    if (flags & RKInitFlagAllocRawIQBuffer) {
        radar->state |= RKRadarStateRawIQBufferAllocating;
        bytes = RKBuffer0SlotCount * sizeof(RKPulse);
        if (posix_memalign((void **)&radar->pulses, RKSIMDAlignSize, bytes)) {
            RKLog("ERROR. Unable to allocate memory for I/Q pulses");
            return NULL;
        }
        if (flags & RKInitFlagVerbose) {
            RKLog("Level I buffer occupies %s B\n", RKIntegerToCommaStyleString(bytes));
        }
        memset(radar->pulses, 0, bytes);
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRawIQBufferAllocating;
        radar->state |= RKRadarStateRawIQBufferInitialized;
        for (int i = 0; i < RKBuffer0SlotCount; i++) {
            radar->pulses[i].header.i = i - RKBuffer0SlotCount;
        }
    }

    // Moment bufer
    if (flags & RKInitFlagAllocMomentBuffer) {
        radar->state |= RKRadarStateRayBufferAllocating;
        bytes = RKBuffer2SlotCount * sizeof(RKRay);
        if (posix_memalign((void **)&radar->rays, RKSIMDAlignSize, bytes)) {
            RKLog("ERROR. Unable to allocate memory for rays");
            return NULL;
        }
        if (flags & RKInitFlagVerbose) {
            RKLog("Level II buffer occupies %s B\n", RKIntegerToCommaStyleString(bytes));
        }
        memset(radar->rays, 0, bytes);
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRayBufferAllocating;
        radar->state |= RKRadarStateRayBufferInitialized;
    }

    // Pulse compression engine
    radar->pulseCompressionEngine = RKPulseCompressionEngineInit();
    RKPulseCompressionEngineSetInputOutputBuffers(radar->pulseCompressionEngine,
                                                  radar->pulses, &radar->pulseIndex, RKBuffer0SlotCount);
    radar->state |= RKRadarStatePulseCompressionEngineInitialized;

    // Moment engine
    radar->momentEngine = RKMomentEngineInit();
    RKMomentEngineSetInputOutputBuffers(radar->momentEngine,
                                        radar->pulses, &radar->pulseIndex, RKBuffer0SlotCount,
                                        radar->rays, &radar->rayIndex, RKBuffer2SlotCount);
    radar->state |= RKRadarStateMomentEngineInitialized;

    // TCP/IP socket server
    radar->socketServer = RKServerInit();
    RKServerSetCommandHandler(radar->socketServer, &socketCommandHandler);
    RKServerSetStreamHandler(radar->socketServer, &socketStreamHandler);
    radar->state |= RKRadarStateSocketServerInitialized;

    RKLog("Radar initialized\n");
    return radar;
}

int RKFree(RKRadar *radar) {
    if (radar->active) {
        RKStop(radar);
    }
    RKPulseCompressionEngineFree(radar->pulseCompressionEngine);
    RKMomentEngineFree(radar->momentEngine);
    RKServerFree(radar->socketServer);
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

// Function incomplete
int RKSetWaveform(RKRadar *radar, const char *filename, const int group, const int maxDataLength) {
    // Load in the waveform
    // Call a transceiver delegate function to fill in the DAC
    RKComplex filter[] = {{1.0f, 0.0f}};
    return RKPulseCompressionSetFilter(radar->pulseCompressionEngine, filter, 1, 0, maxDataLength, group, 0);
}

int RKSetWaveformToImpulse(RKRadar *radar) {
    return RKPulseCompressionSetFilterToImpulse(radar->pulseCompressionEngine);
}

int RKSetWaveformTo121(RKRadar *radar) {
    return RKPulseCompressionSetFilterTo121(radar->pulseCompressionEngine);
}

int RKSetProcessingCoreCounts(RKRadar *radar,
                              const unsigned int pulseCompressionCoreCount,
                              const unsigned int momentProcessorCoreCount) {
    RKPulseCompressionEngineSetCoreCount(radar->pulseCompressionEngine, pulseCompressionCoreCount);
    RKMomentEngineSetCoreCount(radar->momentEngine, momentProcessorCoreCount);
    return 0;
}

int RKSetTransceiver(RKRadar *radar, RKTransceiver init(RKRadar *, void *), void *initInput) {
    radar->transceiverInit = init;
    radar->transceiverInitInput = initInput;
    return 0;
}

int RKSetPRF(RKRadar *radar, const float prf) {
    return 0;
}

#pragma mark -

RKTransceiver backgroundTransceiverInit(void *in) {
    RKRadar *radar = (RKRadar *)in;
    return radar->transceiverInit(radar, radar->transceiverInitInput);
}

RKPedestal backgroundPedestalInit(void *in) {
    //RKRadar *radar = (RKRadar *)in;
    return NULL;
}

int RKGoLive(RKRadar *radar) {
    RKPulseCompressionEngineStart(radar->pulseCompressionEngine);
    RKMomentEngineStart(radar->momentEngine);
    RKServerActivate(radar->socketServer);

    // Operation parameters
    if (radar->transceiverInit != NULL) {
        RKLog("Initializing transceiver ...");
        pthread_t transceiverThreadId;
        pthread_create(&transceiverThreadId, NULL, backgroundTransceiverInit, radar);
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
    if (radar->state & RKRadarStateSocketServerInitialized) {
        RKServerStop(radar->socketServer);
        RKServerWait(radar->socketServer);
    }
    return 0;
}

RKPulse *RKGetVacantPulse(RKRadar *radar) {
    if (radar->pulses == NULL) {
        RKLog("Error. Buffer for raw pulses has not been allocated.\n");
        exit(EXIT_FAILURE);
    }
    RKPulse *pulse = &radar->pulses[radar->pulseIndex];
    pulse->header.s = RKPulseStatusVacant;
    pulse->header.i += RKBuffer0SlotCount;
    radar->pulseIndex = RKNextBuffer0Slot(radar->pulseIndex);
    return pulse;
}

void RKSetPulseReady(RKPulse *pulse) {
    pulse->header.s = RKPulseStatusReady;
}
