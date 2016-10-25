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
    RKInitFlagAllocRawIQBuffer  = (1 << 1),
    RKInitFlagAllocEverything   = RKInitFlagAllocMomentBuffer | RKInitFlagAllocRawIQBuffer
};

typedef RKEnum RKRadarState;
enum RKRadarState {
    RKRadarStateBaseAllocated                        = 1,          // 0x01
    RKRadarStateRayBufferAllocating                  = (1 << 1),   // 0x02
    RKRadarStateRayBufferInitialized                 = (1 << 2),   // 0x04
    RKRadarStateRawIQBufferAllocating                = (1 << 3),   // 0x08
    RKRadarStateRawIQBufferInitialized               = (1 << 4),   // 0x10
    RKRadarStatePulseCompressionEngineInitialized    = (1 << 5),   // 0x20
    RKRadarStateSocketServerInitialized              = (1 << 6)    // 0x40
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
