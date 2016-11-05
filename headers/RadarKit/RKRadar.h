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
#include <RadarKit/RKMoment.h>

enum RKInitFlag {
    RKInitFlagNone              = 0,
    RKInitFlagVerbose           = 1,
    RKInitFlagAllocMomentBuffer = (1 << 1),
    RKInitFlagAllocRawIQBuffer  = (1 << 2),
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
    RKRadarStateMomentEngineInitialized              = (1 << 6),   // 0x40
    RKRadarStateSocketServerInitialized              = (1 << 7)
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
    RKRay                      *rays;
    //
    uint32_t                   index;
    uint32_t                   rayIndex;
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
    RKMomentEngine             *momentEngine;
    RKServer                   *socketServer;

} RKRadar;

RKRadar *RKInit(void);
int RKFree(RKRadar *radar);

int RKSetWaveform(RKRadar *radar, const char *filename, const int group, const int maxDataLength);
int RKSetWaveformToImpulse(RKRadar *radar);
int RKSetWaveformTo121(RKRadar *radar);
int RKSetProcessingCoreCounts(RKRadar *radar,
                              const unsigned int pulseCompressionCoreCount,
                              const unsigned int momentProcessorCoreCount);

int RKGoLive(RKRadar *radar);
int RKStop(RKRadar *radar);

RKPulse *RKGetVacantPulse(RKRadar *radar);
void RKSetPulseReady(RKPulse *pulse);

#endif /* defined(__RadarKit_RKRadar__) */
