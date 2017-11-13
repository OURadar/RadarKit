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

typedef int RKTestPedestalScanMode;
enum RKTestPedestalScanMode {
    RKTestPedestalScanModeNull,
    RKTestPedestalScanModePPI,
    RKTestPedestalScanModeRHI,
    RKTestPedestalScanModeBadPedestal
};

typedef struct rk_test_transceiver {
    char           name[RKNameLength];
    int            verbose;
    int            sleepInterval;
    int            gateCount;
    float          gateSizeMeters;
    long           counter;
    double         fs;
    double         prt;
    RKByte         sprt;
    pthread_t      tidRunLoop;
    RKEngineState  state;
    RKRadar        *radar;
    size_t         memoryUsage;
} RKTestTransceiver;

typedef struct rk_test_pedestal {
    char           name[RKNameLength];
    int            verbose;
    long           counter;
    int            scanMode;
    int            commandCount;
    float          scanElevation;
    float          scanAzimuth;
    float          speedAzimuth;
    float          speedElevation;
    float          rhiElevationStart;
    float          rhiElevationEnd;
    pthread_t      tidRunLoop;
    RKEngineState  state;
    RKRadar        *radar;
    size_t         memoryUsage;
} RKTestPedestal;

void RKTestModuloMath(void);
void RKTestSIMD(const RKTestSIMDFlag);
void RKTestParseCommaDelimitedValues(void);

RKTransceiver RKTestTransceiverInit(RKRadar *, void *);
int RKTestTransceiverExec(RKTransceiver, const char *, char *);
int RKTestTransceiverFree(RKTransceiver);

RKPedestal RKTestPedestalInit(RKRadar *, void *);
int RKTestPedestalExec(RKTransceiver, const char *, char *);
int RKTestPedestalFree(RKTransceiver);

void RKTestPulseCompression(RKRadar *, RKTestFlag);
void RKTestProcessorSpeed(void);
void RKTestOneRay(int method(RKScratch *, RKPulse **, const uint16_t), const int);

void RKTestCacheWrite(void);
void RKTestWindow(void);
void RKTestJSON(void);
void RKTestShowColors(void);
void RKTestSingleEngine(void);
void RKTestSingleCommand(void);
void RKTestMakeHops(void);
void RKTestPreferenceReading(void);
void RKTestCountFiles(void);
void RKTestFileMonitor(void);
void RKTestWriteWaveform(void);
void RKTestWaveformTFM(void);
void RKTestHilbertTransform(void);

#endif /* defined(__RadarKit_RKFile__) */
