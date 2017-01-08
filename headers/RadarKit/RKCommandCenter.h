//
//  RKCommandCenter.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKCommandCenter__
#define __RadarKit_RKCommandCenter__

#include <RadarKit/RKRadar.h>

#define RKCommandCenterMaxConnections 4

typedef struct rk_command_center {
    // User set variables
    char      name[64];
    int       verbose;
    RKRadar   *radars[4];
    
    // Program set variables
    RKServer  *server;
    int       radarCount;
    char      user[RKCommandCenterMaxConnections][64];
    bool      authorized[RKCommandCenterMaxConnections];
    int       radarSelection[RKCommandCenterMaxConnections];
    
} RKCommandCenter;

RKCommandCenter *RKCommandCenterInit(void);
void RKCommandCenterFree(RKCommandCenter *);

void RKCommandCenterSetVerbose(RKCommandCenter *, const int);
void RKCommandCenterAddRadar(RKCommandCenter *, RKRadar *);
void RKCommandCenterRemoveRadar(RKCommandCenter *, RKRadar *);

void RKCommandCenterStart(RKCommandCenter *);

#endif /* __RadarKit_RKCommandCenter__ */
