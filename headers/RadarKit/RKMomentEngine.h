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
#include <RadarKit/RKScratch.h>
#include <RadarKit/RKPulsePair.h>
#include <RadarKit/RKMultiLag.h>
#include <RadarKit/RKNoiseEstimator.h>
#include <RadarKit/RKSpectralMoment.h>
#include <RadarKit/RKCalibrator.h>
#include <RadarKit/RKPulseATSR.h>

#define RKMomentDFTPlanCount    16

typedef struct rk_moment_worker RKMomentWorker;
typedef struct rk_moment_engine RKMomentEngine;

struct rk_moment_worker {
    RKChildName                      name;
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
    uint32_t                         doneIndex;                                // Last retrieved ray index that's processed
    RKFFTModule                      *fftModule;
    RKUserModule                     userModule;
    uint8_t                          verbose;
    uint8_t                          coreCount;
    uint8_t                          coreOrigin;
    bool                             useSemaphore;
    bool                             excludeBoundaryPulses;
    int                              (*noiseEstimator)(RKMomentScratch *, RKPulse **, const uint16_t);
    int                              (*momentProcessor)(RKMomentScratch *, RKPulse **, const uint16_t);
    void                             (*calibrator)(RKUserModule, RKMomentScratch *);

    // Program set variables
    RKModuloPath                     *momentSource;
    RKMomentWorker                   *workers;
    pthread_t                        tidPulseGatherer;
    pthread_mutex_t                  mutex;
    uint8_t                          processorLagCount;                        // Number of lags to calculate R[n]'s
    uint8_t                          processorFFTOrder;                        // FFT order used in spectral processing
    uint8_t                          userLagChoice;                            // Lag parameter for multilag method
    uint32_t                         business;

    // Status / health
    uint32_t                         processedPulseIndex;
    char                             statusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    char                             rayStatusBuffer[RKBufferSSlotCount][RKStatusStringLength];
    uint32_t                         statusBufferIndex;
    uint32_t                         rayStatusBufferIndex;
    RKEngineState                    state;
    uint64_t                         tic;
    float                            lag;
    float                            minWorkerLag;
    float                            maxWorkerLag;
    uint32_t                         almostFull;
    size_t                           memoryUsage;
};

RKMomentEngine *RKMomentEngineInit(void);
void RKMomentEngineFree(RKMomentEngine *);

void RKMomentEngineSetVerbose(RKMomentEngine *, const int verbose);
void RKMomentEngineSetEssentials(RKMomentEngine *, const RKRadarDesc *, RKFFTModule *,
                                 RKConfig *configBuffer, uint32_t *configIndex,
                                 RKBuffer pulseBuffer,   uint32_t *pulseIndex,
                                 RKBuffer rayBuffer,     uint32_t *rayIndex);
void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *, const RKRadarDesc *,
                                         RKConfig *configBuffer, uint32_t *configIndex,
                                         RKBuffer pulseBuffer,   uint32_t *pulseIndex,
                                         RKBuffer rayBuffer,     uint32_t *rayIndex)
                                         __attribute__ ((deprecated));
void RKMomentEngineSetFFTModule(RKMomentEngine *, RKFFTModule *) __attribute__ ((deprecated));
void RKMomentEngineSetCalibrator(RKMomentEngine *, void (*)(RKUserModule, RKMomentScratch *), RKUserModule);
void RKMomentEngineUnsetCalibrator(RKMomentEngine *);
void RKMomentEngineSetCoreCount(RKMomentEngine *, const uint8_t);
void RKMomentEngineSetCoreOrigin(RKMomentEngine *, const uint8_t);
void RKMomentEngineSetExcludeBoundaryPulses(RKMomentEngine *, const bool);
void RKMomentEngineSetNoiseEstimator(RKMomentEngine *, int (*)(RKMomentScratch *, RKPulse **, const uint16_t));
void RKMomentEngineSetMomentProcessor(RKMomentEngine *, int (*)(RKMomentScratch *, RKPulse **, const uint16_t));

int RKMomentEngineStart(RKMomentEngine *);
int RKMomentEngineStop(RKMomentEngine *);

RKRay *RKMomentEngineGetProcessedRay(RKMomentEngine *, const bool);
void RKMomentEngineFlush(RKMomentEngine *);
void RKMomentEngineWaitWhileBusy(RKMomentEngine *);

char *RKMomentEngineStatusString(RKMomentEngine *);

int RKNoiseFromConfig(RKMomentScratch *, RKPulse **, const uint16_t);

#endif /* defined(__RadarKit_Moment_Engine__) */
