//
//  RKHealthRelayNaveen.h
//  RadarKit
//
//  Created by Boonleng Cheong on 6/9/2023.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_HealthRelayNaveen__
#define __RadarKit_HealthRelayNaveen__

#include <RadarKit/RKRadar.h>

#define RKHealthRelayNaveenFeedbackDepth   8

typedef struct rk_naveen {
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKHealthRelayNaveenFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumCommandLength];
    RKRadar                *radar;
    pthread_t              tidBackground;
    RKByte                 response[RKMaximumStringLength];
} RKHealthRelayNaveen;

RKHealthRelay RKHealthRelayNaveenInit(RKRadar *, void *);
int RKHealthRelayNaveenExec(RKHealthRelay, const char *, char *);
int RKHealthRelayNaveenFree(RKHealthRelay);

#endif /* __RadarKit_HealthRelayNaveen__ */
