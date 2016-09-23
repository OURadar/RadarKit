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
        bytes = RKBuffer0SlotCount * sizeof(RKInt16Ray);
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
        radar->memoryUsage += bytes;
        
        bytes = RKBuffer0SlotCount * sizeof(RKFloatPulse);
        if (posix_memalign((void **)&radar->compressedPulses, RKSIMDAlignSize, bytes)) {
            RKLog("ERROR. Unable to allocate memory for compressed pulses");
            return NULL;
        }
        radar->memoryUsage += bytes;
        
        bytes = RKBuffer0SlotCount * sizeof(RKFloatPulse);
        if (posix_memalign((void **)&radar->filteredCompressedPulses, RKSIMDAlignSize, bytes)) {
            RKLog("ERROR. Unable to allocate memory for filtered compressed pulse");
            return NULL;
        }
        radar->memoryUsage += bytes;
        radar->state |= RKRadarStatePulseBufferInitiated;
    }

    radar->pulseCompressionEngine = RKPulseCompressionEngineInit();
    RKPulseCompressionEngineSetInputOutputBuffers(radar->pulseCompressionEngine, radar->rawPulses, radar->compressedPulses, RKBuffer0SlotCount);

    radar->socketServer = RKServerInit();
    RKServerSetCommandHandler(radar->socketServer, &socketCommandHandler);
    RKServerSetStreamHandler(radar->socketServer, &socketStreamHandler);

    printf("Radar initialized\n");
    return radar;
}

int RKFree(RKRadar *radar) {
    if (radar-> state & RKRadarStatePulseBufferInitiated) {
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



