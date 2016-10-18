//
//  RKRadar.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include <RadarKit/RKRadar.h>

RKRadar *RKInitWithFlags(const RKEnum flags);

//int RKSetPulseStatus(RKInt16Pulse *pulse, RKPulseStatus);

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

    // Copy over the input flags
    radar->initFlags = flags;

    // Set some non-zero variables
    radar->active = true;
    
    // Other allocatinos
    if (flags & RKInitFlagAllocMomentBuffer) {
        bytes = RKBuffer2SlotCount * sizeof(RKInt16Ray);
        if (posix_memalign((void **)&radar->rays, RKSIMDAlignSize, bytes)) {
            RKLog("ERROR. Unable to allocate memory for rays");
            return NULL;
        }
        radar->memoryUsage += bytes;
        radar->state |= RKRadarStateRayBufferInitiated;
    }
    
    if (flags & RKInitFlagAllocRawIQBuffer) {
        bytes = RKBuffer0SlotCount * sizeof(RKInt16Pulse);
        if (posix_memalign((void **)&radar->rawPulses, RKSIMDAlignSize, bytes)) {
            RKLog("ERROR. Unable to allocate memory for raw pulses");
            return NULL;
        }
        memset(radar->rawPulses, 0, bytes);
        radar->memoryUsage += bytes;
        radar->state |= RKRadarStateRawIQBufferInitiated;
        for (int i = 0; i < RKBuffer0SlotCount; i++) {
            radar->rawPulses[i].header.i = i - RKBuffer0SlotCount;
        }
        
        bytes = RKBuffer0SlotCount * sizeof(RKFloatPulse);
        if (posix_memalign((void **)&radar->compressedPulses, RKSIMDAlignSize, bytes)) {
            RKLog("ERROR. Unable to allocate memory for compressed pulses");
            return NULL;
        }
        radar->memoryUsage += bytes;
        radar->state |= RKRadarStateCompressedIQBufferInitiated;

        bytes = RKBuffer0SlotCount * sizeof(RKFloatPulse);
        if (posix_memalign((void **)&radar->filteredCompressedPulses, RKSIMDAlignSize, bytes)) {
            RKLog("ERROR. Unable to allocate memory for filtered compressed pulse");
            return NULL;
        }
        radar->memoryUsage += bytes;
        radar->state |= RKRadarStateFilteredCompressedIQBufferInitiated;
    }

    radar->pulseCompressionEngine = RKPulseCompressionEngineInit();
    RKPulseCompressionEngineSetInputOutputBuffers(radar->pulseCompressionEngine, radar->rawPulses, radar->compressedPulses, &radar->index, RKBuffer0SlotCount);

    radar->socketServer = RKServerInit();
    RKServerSetCommandHandler(radar->socketServer, &socketCommandHandler);
    RKServerSetStreamHandler(radar->socketServer, &socketStreamHandler);

    RKLog("Radar initialized\n");
    return radar;
}

int RKFree(RKRadar *radar) {
    if (radar-> state & RKRadarStateRawIQBufferInitiated) {
        free(radar->rawPulses);
        free(radar->compressedPulses);
        free(radar->filteredCompressedPulses);
    }
    if (radar->state & RKRadarStateRayBufferInitiated) {
        free(radar->rays);
    }
    free(radar);
    return EXIT_SUCCESS;
}

int RKGoLive(RKRadar *radar) {
    RKPulseCompressionEngineStart(radar->pulseCompressionEngine);
    RKServerActivate(radar->socketServer);

    return 0;
}

int RKStop(RKRadar *radar) {
    RKPulseCompressionEngineStop(radar->pulseCompressionEngine);
    RKServerStop(radar->socketServer);
    RKServerWait(radar->socketServer);
    radar->active = false;
    return 0;
}

RKInt16Pulse *RKGetVacantPulse(RKRadar *radar) {
    if (radar->rawPulses == NULL) {
        RKLog("Error. Buffer for raw pulses is not allocated.\n");
        exit(EXIT_FAILURE);
    }
    RKInt16Pulse *pulse = &radar->rawPulses[radar->index];
    pulse->header.s = RKPulseStatusVacant;
    pulse->header.i += RKBuffer0SlotCount;
    radar->index = RKNextBuffer0Slot(radar->index);
    return pulse;
}

void RKSetPulseReady(RKInt16Pulse *pulse) {
    pulse->header.s = RKPulseStatusReady;
}
