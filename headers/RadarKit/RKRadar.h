//
//  RKRadar.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKRadar__
#define __RadarKit_RKRadar__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKLocalCommandCenter.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

enum RKInitFlag {
    RKInitFlagAllocMomentBuffer = 1,
    RKInitFlagAllocRawIQBuffer  = 1 << 1,
    RKInitFlagAllocEverything   = RKInitFlagAllocMomentBuffer | RKInitFlagAllocMomentBuffer
};

/*!
 * @typedef RKRadar
 * @brief The big structure that holds all the necessary buffers
 * @param rawPulse
 */
typedef struct RKRadar {
    //
    // Buffers aligned to SIMD requirements
    //
    RKInt16Pulse    *rawPulses;
    RKFloatPulse    *compressedPulses;
    RKFloatPulse    *filteredCompressedPulses;
    RKInt16Ray      *rays;
    //
    //
    //
    RKenum          initFlags;
    RKRadarState    state;
    bool            active;
    //
    //
    //
    unsigned int    memoryUsage;
    //
    //
    //
    unsigned int    pulseCompressionCoreCount;
    pthread_t       tidPulseCompression[64];
    sem_t           semPulseCompression[64];
    //
    //
    //
    RKServer        *socketServer;

} RKRadar;

RKRadar *RKInit(void);
int RKFree(RKRadar *radar);

int RKGoLive(RKRadar *radar);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKRadar__) */
