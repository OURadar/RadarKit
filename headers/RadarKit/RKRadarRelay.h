//
//  RKRadarRelay.h
//  RadarKit
//
//  This is really a virtual digital transceiver interface. Instead of receiving samples
//  from a hardware digital transceiver. It receives samples through a network socket,
//  which have been organized by another RadarKit.
//
//  Created by Boon Leng Cheong on 4/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RadarRelay__
#define __RadarKit_RadarRelay__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKFileManager.h>
#include <RadarKit/RKClient.h>

#define RKRadarRelayFeedbackDepth     200
#define RKRadarRelayFeedbackCapacity  8196

typedef struct rk_radar_relay {
    // User defined variables
    RKName                           name;
    RKName                           host;
    RKRadarDesc                      *radarDescription;
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    RKHealth                         *healthBuffer;
    uint32_t                         *healthIndex;
    RKStatus                         *statusBuffer;
    uint32_t                         *statusIndex;
    RKBuffer                         pulseBuffer;                        // Buffer of raw pulses
    uint32_t                         *pulseIndex;                        // The refence index to watch for
    RKBuffer                         rayBuffer;
    uint32_t                         *rayIndex;
    uint8_t                          verbose;
    RKFileManager                    *fileManager;

    // Program set variables
    RKClient                         *client;
    uint32_t                         responseIndex;
    char                             responses[RKRadarRelayFeedbackDepth][RKRadarRelayFeedbackCapacity];
    char                             latestCommand[RKMaximumStringLength];
    pthread_t                        tidBackground;
    RKStream                         streams;

    // For handling sweeps
    RKSweepHeader                    sweepHeaderCache;
	uint32_t                         sweepPacketCount;
    uint32_t                         sweepRayIndex;
    struct timeval                   sweepTic;
    struct timeval                   sweepToc;

    // Status / health
    char                             pulseStatusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    char                             rayStatusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t                         pulseStatusBufferIndex;
    uint32_t                         rayStatusBufferIndex;
    RKEngineState                    state;
	uint32_t                         tic;
    size_t                           memoryUsage;
} RKRadarRelay;

RKRadarRelay *RKRadarRelayInit(void);
void RKRadarRelayFree(RKRadarRelay *);

void RKRadarRelaySetVerbose(RKRadarRelay *, const int verbose);
void RKRadarRelaySetInputOutputBuffers(RKRadarRelay *, const RKRadarDesc *, RKFileManager *,
                                       RKStatus *statusBuffer, uint32_t *statusIndex,
                                       RKConfig *configBuffer, uint32_t *configIndex,
                                       RKHealth *healthBuffer, uint32_t *healthIndex,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex,
                                       RKBuffer rayBuffer,     uint32_t *rayIndex);
void RKRadarRelaySetHost(RKRadarRelay *, const char *hostname);

int RKRadarRelayStart(RKRadarRelay *);
int RKRadarRelayStop(RKRadarRelay *);

int RKRadarRelayExec(RKRadarRelay *, const char *command, char *response);
int RKRadarRelayUpdateStreams(RKRadarRelay *, RKStream);

#endif
