//
//  RKTest.h
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

typedef int RKTestSIMDFlag;
enum RKTestSIMDFlag {
    RKTestSIMDFlagNull                       = 0,
    RKTestSIMDFlagShowNumbers                = 1,
    RKTestSIMDFlagPerformanceTestArithmetic  = 1 << 1,
    RKTestSIMDFlagPerformanceTestConversion  = 1 << 2,
    RKTestSIMDFlagPerformanceTestAll         = RKTestSIMDFlagPerformanceTestArithmetic | RKTestSIMDFlagPerformanceTestConversion
};

void RKTestModuloMath(void);
void RKTestSIMD(const RKTestSIMDFlag);
void RKTestParseCommaDelimitedValues(void);

RKTransceiver RKTestTransceiverInit(RKRadar *, void *);
int RKTestTransceiverExec(RKTransceiver, const char *);
int RKTestTransceiverFree(RKTransceiver);

void RKTestPulseCompression(RKRadar *, RKTestFlag);
void RKTestProcessorSpeed(void);
void RKTestOneRay(void);

#endif /* defined(__RadarKit_RKFile__) */
