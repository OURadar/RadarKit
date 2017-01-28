//
//  RKHealthRelayTweeta.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_HealthRelayTweeta__
#define __RadarKit_HealthRelayTweeta__

#include <RadarKit/RKRadar.h>

typedef struct rk_tweeta {
    // User defined variables
    RKClient                *client;
} RKHealthRelayTweeta;

RKHealthRelay RKHealthRelayTweetaInit(RKRadar *, void *);
int RKHealthRelayTweetaExec(RKHealthRelay, const char *);
int RKHealthRelayTweetaFree(RKHealthRelay);

#endif /* __RadarKit_HealthRelayTweeta__ */
