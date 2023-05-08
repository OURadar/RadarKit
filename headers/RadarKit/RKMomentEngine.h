//
//  RKMomentEngine.h
//  RadarKit
//
//  Created by Boonleng Cheong on 9/20/15.
//  Copyright (c) 2015-2022 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Moment_Engine__
#define __RadarKit_Moment_Engine__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKMoment.h>
#include <RadarKit/RKPulsePair.h>
#include <RadarKit/RKMultiLag.h>
#include <RadarKit/RKSpectralMoment.h>
#include <RadarKit/RKCalibrator.h>

#define RKMomentDFTPlanCount    16

typedef struct rk_moment_worker RKMomentWorker;
typedef struct rk_moment_engine RKMomentEngine;

struct rk_moment_worker {
    RKShortName                      name;
    int                              id;
    pthread_t                        tid;                                      // Thread ID
    RKMomentEngine                   *parent;                                  // Parent engine reference

    char                             semaphoreName[32];
    uint64_t                         tic;                                      // Tic count
    uint32_t                         pid;                                      // Latest processed index of pulses buffer
    double                           dutyBuff[RKWorkerDutyCycleBufferDepth];   // Duty cycle history
    double                           dutyCycle;                                // Latest duty cycle estimate
    float                            lag;                                      // Relative lag from the latest index
    sem_t                            *sem;
};

struct rk_moment_engine {
    // User set variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;
    RKConfig                         *configBuffer;
    uint32_t                         *configIndex;
    RKBuffer                         pulseBuffer;
    uint32_t                         *pulseIndex;
    RKBuffer                         rayBuffer;
    uint32_t                         *rayIndex;
    RKFFTModule                      *fftModule;
    uint8_t                          verbose;
    uint8_t                          coreCount;
    uint8_t                          coreOrigin;
    bool                             useSemaphore;
    int                              (*processor)(RKMomentScratch *, RKPulse **, const uint16_t);
    void                             (*calibrator)(RKMomentScratch *);

    // Program set variables
    RKModuloPath                     *momentSource;
    RKMomentWorker                   *workers;
    pthread_t                        tidPulseGatherer;
    pthread_mutex_t                  mutex;
    uint8_t                          processorLagCount;                        // Number of lags to calculate R[n]'s
    uint8_t                          processorFFTOrder;                        // Maximum number of FFT order (1 << order)
    uint8_t                          userLagChoice;                            // Lag parameter for multilag method

    // Status / health
    uint32_t                         processedPulseIndex;
    char                             statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    char                             rayStatusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    uint32_t                         statusBufferIndex;
    uint32_t                         rayStatusBufferIndex;
    RKEngineState                    state;
    uint64_t                         tic;
    float                            lag;
    uint32_t                         almostFull;
    size_t                           memoryUsage;
};

RKMomentEngine *RKMomentEngineInit(void);
void RKMomentEngineFree(RKMomentEngine *);

void RKMomentEngineSetVerbose(RKMomentEngine *, const int verbose);
void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *, const RKRadarDesc *,
                                         RKConfig *configBuffer, uint32_t *configIndex,
                                         RKBuffer pulseBuffer, uint32_t *pulseIndex,
                                         RKBuffer rayBuffer,   uint32_t *rayIndex);
void RKMomentEngineSetFFTModule(RKMomentEngine *, RKFFTModule *);
void RKMomentEngineSetCoreCount(RKMomentEngine *, const uint8_t);
void RKMomentEngineSetCoreOrigin(RKMomentEngine *, const uint8_t);

int RKMomentEngineStart(RKMomentEngine *);
int RKMomentEngineStop(RKMomentEngine *);

char *RKMomentEngineStatusString(RKMomentEngine *);

#endif /* defined(__RadarKit_Moment_Engine__) */
