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
#include <RadarKit/RKPulseCompression.h>
#include <RadarKit/RKLocalCommandCenter.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

enum RKInitFlag {
    RKInitFlagAllocMomentBuffer = 1,
    RKInitFlagAllocRawIQBuffer  = 1 << 1,
    RKInitFlagAllocEverything   = RKInitFlagAllocMomentBuffer | RKInitFlagAllocRawIQBuffer
};

typedef RKEnum RKRadarState;
enum RKRadarState {
    RKRadarStateRayBufferInitiated                   = 1,
    RKRadarStateRawIQBufferInitiated                 = 1 << 1,
    RKRadarStateCompressedIQBufferInitiated          = 1 << 2,
    RKRadarStateFilteredCompressedIQBufferInitiated  = 1 << 3,
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
    RKInt16Pulse               *rawPulses;
    RKFloatPulse               *compressedPulses;
    RKFloatPulse               *filteredCompressedPulses;
    RKInt16Ray                 *rays;
    //
    uint32_t                   index;
    //
    //
    RKEnum                     initFlags;
    RKRadarState               state;
    bool                       active;
    //
    unsigned int               memoryUsage;
    //
    //
    //
    RKPulseCompressionEngine   *pulseCompressionEngine;
    RKServer                   *socketServer;

} RKRadar;

RKRadar *RKInit(void);
int RKFree(RKRadar *radar);

int RKGoLive(RKRadar *radar);
int RKStop(RKRadar *radar);

RKInt16Pulse *RKGetVacantPulse(RKRadar *radar);
void RKSetPulseReady(RKInt16Pulse *pulse);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKRadar__) */
