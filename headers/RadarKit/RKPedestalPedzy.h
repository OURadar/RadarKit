//
//  RKPedestalPedzy.h
//  RadarKit
//
//  This is an example implementation of hardware interaction through RKPedestal
//  Many features here are specific the pedzy and may not apply to your situations
//
//  Created by Boonleng Cheong on 1/3/17.
//  Copyright © 2017-2023 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_PedestalPedzy__
#define __RadarKit_PedestalPedzy__

#include <RadarKit/RKRadar.h>

#define RKPedestalPedzyFeedbackDepth   8

typedef struct rk_pedzy {
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKPedestalPedzyFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumCommandLength];
    RKRadar                *radar;
    float                  headingOffset;
    pthread_t              tidBackground;
    RKByte                 dump[1024 * 1024];
} RKPedestalPedzy;

RKPedestal RKPedestalPedzyInit(RKRadar *, void *);
int RKPedestalPedzyExec(RKPedestal, const char *, char *);
int RKPedestalPedzyFree(RKPedestal);

#endif /* __RadarKit_RKPedestal__ */
