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

typedef struct rk_ingest_nmea {
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKHealthRelayNaveenFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumCommandLength];
    RKRadar                *radar;
    pthread_t              tidBackground;
    RKByte                 response[RKMaximumStringLength];
} RKHealthRelayNaveen;

RKHealthRelayNaveen RKHealthRelayNaveenInit(RKRadar *, void *);
int RKHealthRelayNaveenExec(RKHealthRelayNaveen, const char *, char *);
int RKHealthRelayNaveenFree(RKHealthRelayNaveen);

#endif /* __RadarKit_HealthRelayNaveen__ */
