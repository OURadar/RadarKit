//
//  RKMonitorTweeta.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_MonitorTweeta__
#define __RadarKit_MonitorTweeta__

#include <RadarKit/RKRadar.h>

typedef struct rk_tweeta {
    // User defined variables
    RKClient                *client;
} RKHealthMonitorTweeta;

RKHealthMonitor RKMonitorTweetaInit(RKRadar *, void *);
int RKHealthMonitorTweetaExec(RKHealthMonitor, const char *);
int RKHealthMonitorTweetaFree(RKHealthMonitor);

#endif /* __RadarKit_MonitorTweeta__ */
