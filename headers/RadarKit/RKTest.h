//
//  RKFile.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKTest__
#define __RadarKit_RKTest__

#include <RadarKit/RKRadar.h>

typedef int RKTestFlag;
enum RKTestFlag {
    RKTestFlagNone         = 0,
    RKTestFlagShowResults  = 1
};

void RKTestModuloMath(void);

RKTransceiver RKTestSimulateDataStream(RKRadar *radar, void *input);

void RKTestPulseCompression(RKRadar *, RKTestFlag);

#endif /* defined(__RadarKit_RKFile__) */
