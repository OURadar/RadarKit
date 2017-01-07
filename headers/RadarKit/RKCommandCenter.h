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

typedef struct rk_command_center {
    RKRadar   *radars;
    RKServer  *server;
} RKCommandCenter;

void RKCommandCenterAddRadar(RKCommandCenter *, RKRadar *);
void RKCommandCenterRemoveRadar(RKCommandCenter *, RKRadar *);

#endif /* __RadarKit_RKLocalCommandCenter__ */
