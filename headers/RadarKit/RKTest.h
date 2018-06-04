//
//  RKTest.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Test__
#define __RadarKit_Test__

#include <RadarKit/RKRadar.h>

typedef int RKTestFlag;
enum RKTestFlag {
    RKTestFlagNone         = 0,
    RKTestFlagVerbose      = 1,
    RKTestFlagShowResults  = 1 << 1
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
    RKName         name;
    int            verbose;
    int            sleepInterval;
    int            gateCount;
    float          gateSizeMeters;
    long           counter;
    double         fs;
    double         prt;
    RKByte         sprt;
    RKWaveform     *waveformCache[2];
    unsigned int   waveformCacheIndex;
	RKName         defaultWaveform;
	RKName         defaultPedestalMode;
	char           customCommand[RKNameLength + 16];
    pthread_t      tidRunLoop;
    RKEngineState  state;
    RKRadar        *radar;
    size_t         memoryUsage;
    bool           simFault;
    bool           transmitting;
} RKTestTransceiver;

typedef struct rk_test_pedestal {
    RKName         name;
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

typedef struct rk_test_health_relay {
    RKName         name;
    int            verbose;
    long           counter;
    pthread_t      tidRunLoop;
    RKEngineState  state;
    RKRadar        *radar;
    size_t         memoryUsage;
} RKTestHealthRelay;

#pragma mark - Basic Tests

void RKTestTerminalColors(void);
void RKTestPrettyStrings(void);
void RKTestModuloMath(void);
void RKTestParseCommaDelimitedValues(void);
void RKTestParseJSONString(void);
void RKTestFileManager(void);
void RKTestPreferenceReading(void);
void RKTestCountFiles(void);
void RKTestFileMonitor(void);
void RKTestHostMonitor(void);
void RKTestInitializingRadar(void);
void RKTestTemperatureToStatus(void);
void RKTestGetCountry(void);
void RKTestReadSweep(const char *);
void RKTestWaveformProperties(void);
void RKTestBufferOverviewText(void);

#pragma mark - Test Transceiver

RKTransceiver RKTestTransceiverInit(RKRadar *, void *);
int RKTestTransceiverExec(RKTransceiver, const char *, char *);
int RKTestTransceiverFree(RKTransceiver);

#pragma mark - Test Pedestal

RKPedestal RKTestPedestalInit(RKRadar *, void *);
int RKTestPedestalExec(RKPedestal, const char *, char *);
int RKTestPedestalFree(RKPedestal);

#pragma mark - Test Health Relay

RKHealthRelay RKTestHealthRelayInit(RKRadar *, void *);
int RKTestHealthRelayExec(RKHealthRelay, const char *, char *);
int RKTestHealthRelayFree(RKHealthRelay);

#pragma mark -

void RKTestPulseCompression(RKTestFlag);
void RKTestOneRay(int method(RKScratch *, RKPulse **, const uint16_t), const int);

void RKTestCacheWrite(void);
void RKTestWindow(void);

//void RKTestSingleCommand(void);

void RKTestMakeHops(void);

void RKTestWriteWaveform(void);
void RKTestWriteFFTWisdom(void);
void RKTestWaveformTFM(void);
void RKTestHilbertTransform(void);

void RKTestPulseCompressionSpeed(void);
void RKTestMomentProcessorSpeed(void);

void RKTestSIMD(const RKTestSIMDFlag);

#endif /* defined(__RadarKit_RKFile__) */
