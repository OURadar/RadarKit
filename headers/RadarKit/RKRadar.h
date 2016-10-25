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

enum RKInitFlag {
    RKInitFlagAllocMomentBuffer = 1,
    RKInitFlagAllocRawIQBuffer  = 1 << 1,
    RKInitFlagAllocEverything   = RKInitFlagAllocMomentBuffer | RKInitFlagAllocRawIQBuffer
};

typedef RKEnum RKRadarState;
enum RKRadarState {
    RKRadarStateBaseAllocated                        = 1,
    RKRadarStateRayBufferInitialized                 = 1 << 1,
    RKRadarStateRawIQBufferInitialized               = 1 << 2,
    RKRadarStatePulseCompressionEngineInitialized    = 1 << 3,
    RKRadarStateSocketServerInitialized              = 1 << 4
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
    RKPulse                    *pulses;
    RKFloatRay                 *rays;
    //
    uint32_t                   index;
    //
    //
    RKEnum                     initFlags;
    RKRadarState               state;
    bool                       active;
    //
    size_t                     memoryUsage;
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

RKPulse *RKGetVacantPulse(RKRadar *radar);
void RKSetPulseReady(RKPulse *pulse);

int RKSetWaveformToImpulse(RKRadar *radar);
int RKSetWaveformTo121(RKRadar *radar);

#endif /* defined(__RadarKit_RKRadar__) */
