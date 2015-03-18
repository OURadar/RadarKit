//
//  RKRadar.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include <RadarKit/RKRadar.h>
RKRadar *RKInitWithFlags(RKenum flags);

RKRadar *RKInit(void) {
    return RKInitWithFlags(RKInitFlatWantEverything);
}

RKRadar *RKInitWithFlags(RKenum flags) {
    RKRadar *radar;
    size_t bytes;

    radar->initFlags = flags;
    
    if (flags & RKInitFlagWantMomentBuffer) {
        bytes = RKBuffer1SlotCount * sizeof(RKInt16Ray);
        if (posix_memalign((void **)&radar->rays, RKSIMDAlignSize, bytes)) {
            fprintf(stderr, "Error allocation memory for raw pulse");
            return NULL;
        }
        radar->memoryUsage += bytes;
        radar->rayBuffersInitialized = TRUE;
    }
    
    if (flags & RKInitFlagWantRawIQBuffer) {
        bytes = sizeof(RKRadar);
        if (posix_memalign((void **)&radar, RKSIMDAlignSize, bytes)) {
            return NULL;
        }
        radar->memoryUsage += bytes;
        
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
