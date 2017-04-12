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

#include <RadarKit/RKRadar.h>

#define RKRadarRelayFeedbackDepth   200

typedef struct rk_radar_relay {
    // User defined variables
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKRadarRelayFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumStringLength];
    pthread_t              tidBackground;
    RKStream               streams;
    RKRadar                *radar;
} RKRadarRelay;

#endif
