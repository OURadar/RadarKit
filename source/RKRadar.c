//
//  RKRadar.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include <RadarKit/RKRadar.h>

RKRadar *RKInitWithFlags(const RKenum flags);

//
//
//

#pragma mark -

RKRadar *RKInit(void) {
    return RKInitWithFlags(RKInitFlagAllocEverything);
}

RKRadar *RKInitWithFlags(const RKenum flags) {
    RKRadar *radar;
    size_t bytes;

    // Allocate itself
    bytes = sizeof(RKRadar);
    if (posix_memalign((void **)&radar, RKSIMDAlignSize, bytes)) {
        fprintf(stderr, "Error allocation memory for radar.\n");
        return NULL;
    }
    radar->memoryUsage += bytes;

    // Copy over the input flags
    radar->initFlags = flags;
    
    // Other allocatinos
    if (flags & RKInitFlagAllocMomentBuffer) {
        bytes = RKBuffer1SlotCount * sizeof(RKInt16Ray);
        if (posix_memalign((void **)&radar->rays, RKSIMDAlignSize, bytes)) {
            fprintf(stderr, "Error allocation memory for raw pulse");
            return NULL;
        }
        radar->memoryUsage += bytes;
        radar->rayBuffersInitialized = TRUE;
    }
    
    if (flags & RKInitFlagAllocRawIQBuffer) {
        bytes = RKBuffer0SlotCount * sizeof(RKInt16Pulse);
        if (posix_memalign((void **)&radar->rawPulses, RKSIMDAlignSize, bytes)) {
            fprintf(stderr, "Error allocation memory for raw pulse");
            return NULL;
        }
        radar->memoryUsage += bytes;
        
        bytes = RKBuffer0SlotCount * sizeof(RKFloatPulse);
        if (posix_memalign((void **)&radar->compressedPulses, RKSIMDAlignSize, bytes)) {
            fprintf(stderr, "Error allocation memory for compressed pulse");
            return NULL;
        }
        radar->memoryUsage += bytes;
        
        bytes = RKBuffer0SlotCount * sizeof(RKFloatPulse);
        if (posix_memalign((void **)&radar->filteredCompressedPulses, RKSIMDAlignSize, bytes)) {
            fprintf(stderr, "Error allocation memory for filtered compressed pulse");
            return NULL;
        }
        radar->memoryUsage += bytes;
        radar->pulseBuffersInitialized = TRUE;
    }

    printf("Radar initialized\n");
    return radar;
}

int RKFree(RKRadar *radar) {
    printf("Freeing Radar\n");
    if (radar->pulseBuffersInitialized) {
        free(radar->rawPulses);
        free(radar->compressedPulses);
        free(radar->filteredCompressedPulses);
    }
    if (radar->rayBuffersInitialized) {
        free(radar->rays);
    }
    free(radar);
    return EXIT_SUCCESS;
}
