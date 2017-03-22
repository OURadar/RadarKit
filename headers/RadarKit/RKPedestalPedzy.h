//
//  RKPedestalPedzy.h
//  RadarKit
//
//  This is an example implementation of hardware interaction through RKPedestal
//
//  Created by Boon Leng Cheong on 1/3/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_PedestalPedzy__
#define __RadarKit_PedestalPedzy__

#include <RadarKit/RKRadar.h>

#define RKPedestalPedzyFeedbackDepth   8

typedef struct rk_pedzy {
    // User set variables
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKPedestalPedzyFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumStringLength];
    RKRadar                *radar;

    // Program set variables
    pthread_t              tidPedestalMonitor;
} RKPedestalPedzy;

RKPedestal RKPedestalPedzyInit(RKRadar *, void *);
int RKPedestalPedzyExec(RKPedestal, const char *, char *);
int RKPedestalPedzyFree(RKPedestal);

#endif /* __RadarKit_RKPedestal__ */
