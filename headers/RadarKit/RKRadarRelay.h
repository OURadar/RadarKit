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
#include <RadarKit/RKClient.h>

#define RKRadarRelayFeedbackDepth   200

typedef struct rk_radar_relay {
    // User defined variables
    char                   name[RKNameLength];
    char                   host[RKNameLength];
    RKConfig               *configBuffer;
    uint32_t               *configIndex;
    uint32_t               configBufferDepth;
    RKBuffer               pulseBuffer;                        // Buffer of raw pulses
    uint32_t               *pulseIndex;                        // The refence index to watch for
    uint32_t               pulseBufferDepth;                   // Size of the buffer
    RKBuffer               rayBuffer;
    uint32_t               *rayIndex;
    uint32_t               rayBufferDepth;
    uint8_t                verbose;
    
    // Program set variables
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKRadarRelayFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumStringLength];
    pthread_t              tidBackground;
    RKStream               streams;

    // Status / health
    char                   pulseStatusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    char                   rayStatusBuffer[RKBufferSSlotCount][RKMaximumStringLength];
    uint32_t               pulseStatusBufferIndex;
    uint32_t               rayStatusBufferIndex;
    RKEngineState          state;
    size_t                 memoryUsage;
} RKRadarRelay;

RKRadarRelay *RKRadarRelayInit(void);
void RKRadarRelayFree(RKRadarRelay *);

void RKRadarRelaySetVerbose(RKRadarRelay *, const int verbose);
void RKRadarRelaySetHost(RKRadarRelay *, const char *hostname);

#endif
