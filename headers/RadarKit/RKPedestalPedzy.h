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

typedef struct rk_pedzy {
    // User set variables
    RKClient               *client;
} RKPedestalPedzy;

RKPedestal RKPedestalPedzyInit(RKRadar *, void *);
int RKPedestalPedzyExec(RKPedestal, const char *);
int RKPedestalPedzyFree(RKPedestal);

#endif /* __RadarKit_RKPedestal__ */
