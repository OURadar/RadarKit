//
//  RKPedestalPedzy.h
//  RadarKit
//
//  This is an example implementation of hardware interaction through RKPedestal
//  Many features here are specific the pedzy and may not apply to your situations
//
//  Created by Boonleng Cheong on 1/3/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_PedestalPedzy__
#define __RadarKit_PedestalPedzy__

#include <RadarKit/RKRadar.h>

#define RKPedestalPedzyFeedbackDepth   8

//#define RKPedestalVcpRepeat            1
//#define RKPedestalVcpNoRepeat          0
//#define RKPedestalVcpStatusPeriodMS    2000
//
//

typedef struct rk_pedzy {
    // User set variables
    RKClient               *client;
    uint32_t               responseIndex;
    char                   responses[RKPedestalPedzyFeedbackDepth][RKMaximumStringLength];
    char                   latestCommand[RKMaximumCommandLength];
    RKRadar                *radar;
    float                  headingOffset;
    bool                   vcpActive;
    RKPedestalVcpHandle    *vcpHandle;
    uint32_t               lastActionAge;
    char                   msg[4096];

    // Program set variables
    pthread_t              tidPedestalMonitor;
    //pthread_t              tidVcpEngine;

    // Status / health
    char                   statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    uint32_t               statusBufferIndex;
} RKPedestalPedzy;

RKPedestal RKPedestalPedzyInit(RKRadar *, void *);
int RKPedestalPedzyExec(RKPedestal, const char *, char *);
int RKPedestalPedzyFree(RKPedestal);

//RKPedestalVcpHandle *pedestalVcpInit(void);
//void pedestalVcpSendAction(int sd, char *,RKPedestalAction *);

#endif /* __RadarKit_RKPedestal__ */
