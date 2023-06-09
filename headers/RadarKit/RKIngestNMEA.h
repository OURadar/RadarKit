//
//  RKIngestNMEA.h
//  RadarKit
//
//  Created by Boonleng Cheong on 6/9/2023.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_IngestNMEA__
#define __RadarKit_IngestNMEA__

#include <RadarKit/RKRadar.h>

#define RKIngestNMEAFeedbackDepth   8

typedef struct rk_ingest_nmea {
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKIngestNMEAFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumCommandLength];
    RKRadar                *radar;
    pthread_t              tidBackground;
    RKByte                 response[RKMaximumStringLength];
} RKIngestNMEA;

RKIngestNMEA RKIngestNMEAInit(RKRadar *, void *);
int RKIngestNMEAExec(RKIngestNMEA, const char *, char *);
int RKIngestNMEAFree(RKIngestNMEA);

#endif /* __RadarKit_IngestNMEA__ */
