//
//  RKHealthRelayNaveen.h
//  RadarKit
//
//  For relaying NMEA messages through a TCP/IP port
//
//  Supported NMEA 0183 input:
//  - GPGGA - 3D position
//  - GPHDT - heading
//  - GPVTG - 2D velocity
//  - GPRMC - 2D position, 2D velocity and coarse time
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
