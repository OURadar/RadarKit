//
//  RKHealthRelayTweeta.h
//  RadarKit
//
//  Created by Boonleng Cheong on 1/28/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_HealthRelayTweeta__
#define __RadarKit_HealthRelayTweeta__

#include <RadarKit/RKRadar.h>

#define RKHealthRelayTweetaFeedbackDepth   8

typedef struct rk_tweeta {
    // User defined variables
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKHealthRelayTweetaFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumCommandLength];
    bool                   handlingEvent;
    bool                   toggleEvent;
    pthread_t              tidBackground;
    RKRadar                *radar;
} RKHealthRelayTweeta;

RKHealthRelay RKHealthRelayTweetaInit(RKRadar *, void *);
int RKHealthRelayTweetaExec(RKHealthRelay, const char *, char *);
int RKHealthRelayTweetaFree(RKHealthRelay);

#endif /* __RadarKit_HealthRelayTweeta__ */
