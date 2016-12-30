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
    RKRadarStateConfigBufferAllocating               = (1 << 5),   // 0x20
    RKRadarStateConfigBufferIntialized               = (1 << 6),   // 0x40
    RKRadarStatePulseCompressionEngineInitialized    = (1 << 7),
    RKRadarStateMomentEngineInitialized              = (1 << 8),
    RKRadarStateSocketServerInitialized              = (1 << 9),
    RKRadarStateTransceiverInitialized               = (1 << 10),
    RKRadarStatePedestalInitialized                  = (1 << 11)
};

typedef struct rk_radar_desc {
    RKEnum                   initFlags;
    uint32_t                 pulseCapacity;
    uint32_t                 pulseRayRatio;
    uint32_t                 pulseBufferDepth;
    uint32_t                 rayBufferDepth;
} RKRadarInitDesc;

/*!
 * @typedef RKRadar
 * @brief The big structure that holds all the necessary buffers
 * @param rawPulse
 */
typedef struct rk_radar RKRadar;
struct rk_radar {
    //
    // General buffers
    //
    RKOperatingParameters      *parameters;
    //
    // Special buffers, aligned to SIMD requirements
    //
    RKPulse                    *pulses;
    RKRay                      *rays;
    //
    uint32_t                   parameterIndex;
    uint32_t                   pulseIndex;
    uint32_t                   rayIndex;
    //
    //
    RKRadarInitDesc            desc;
    RKRadarState               state;
    bool                       active;
    //
    size_t                     memoryUsage;
    //
    // Internal engines
    //
    RKPulseCompressionEngine   *pulseCompressionEngine;
    RKMomentEngine             *momentEngine;
    RKServer                   *socketServer;
    //
    // Hardware protocols for controls
    //
    RKTransceiver              transceiver;
    RKTransceiver              (*transceiverInit)(RKRadar *, void *);
    int                        (*transceiverExec)(RKTransceiver, const char *);
    int                        (*transceiverRead)(RKTransceiver, const char *, void *);
    int                        (*transceiverFree)(RKTransceiver);
    void                       *transceiverInitInput;
    RKPedestal                 pedestal;
    RKPedestal                 (*pedestalInit)(RKRadar *, void *);
    int                        (*pedestalExec)(RKPedestal, const char *);
    int                        (*pedestalRead)(RKPedestal, const char *, void *);
    int                        (*pedestalFree)(RKPedestal);
    void                       *pedestalInitInput;
};

RKRadar *RKInitWithFlags(RKRadarInitDesc);
RKRadar *RKInitLean(void);
RKRadar *RKInit(void);
int RKFree(RKRadar *radar);

int RKSetTransceiver(RKRadar *radar, RKTransceiver (RKRadar *, void *), void *);

int RKSetWaveform(RKRadar *radar, const char *filename, const int group, const int maxDataLength);
int RKSetWaveformToImpulse(RKRadar *radar);
int RKSetWaveformTo121(RKRadar *radar);
int RKSetProcessingCoreCounts(RKRadar *radar,
                              const unsigned int pulseCompressionCoreCount,
                              const unsigned int momentProcessorCoreCount);

int RKGoLive(RKRadar *radar);
int RKWaitWhileActive(RKRadar *radar);
int RKStop(RKRadar *radar);

RKPulse *RKGetVacantPulse(RKRadar *radar);
void RKSetPulseReady(RKPulse *pulse);

#endif /* defined(__RadarKit_RKRadar__) */
