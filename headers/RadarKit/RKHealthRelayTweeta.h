//
//  RKHealthRelayTweeta.h
//  RadarKit
//
//  For relaying health information through a TCP/IP port
//  Messages must be in a valid JSON key-value dictionary
//
//  Created by Boonleng Cheong on 1/28/17.
//  Copyright © 2017-2023 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_HealthRelayTweeta__
#define __RadarKit_HealthRelayTweeta__

#include <RadarKit/RKRadar.h>

#define RKHealthRelayTweetaFeedbackDepth   8

typedef struct rk_tweeta {
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKHealthRelayTweetaFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumCommandLength];
    RKRadar                *radar;
    bool                   handlingEvent;
    bool                   toggleEvent;
    pthread_t              tidBackground;
    RKByte                 response[RKMaximumStringLength];
} RKHealthRelayTweeta;

RKHealthRelay RKHealthRelayTweetaInit(RKRadar *, void *);
int RKHealthRelayTweetaExec(RKHealthRelay, const char *, char *);
int RKHealthRelayTweetaFree(RKHealthRelay);

#endif /* __RadarKit_HealthRelayTweeta__ */
