//
//  RKPulseRingFilter.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 11/11/17.
//  Copyright (c) 2015-2018 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulseRingFilter.h>

#pragma mark - Helper Functions

static void RKPulseRingFilterUpdateStatusString(RKPulseRingFilterEngine *engine) {
    int i, c;
    char *string = engine->statusBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';
    
    // Use RKStatusBarWidth characters to draw a bar
    i = *engine->pulseIndex * RKStatusBarWidth / engine->radarDescription->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'R';

    // Engine lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKMaximumStringLength - RKStatusBarWidth, " | %s%02.0f%s |",
                                    rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                                    99.9f * engine->lag,
                                    rkGlobalParameters.showColor ? RKNoColor : "");
    
    RKPulseRingFilterWorker *worker;

    // Lag from each core
    for (c = 0; c < engine->coreCount; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKMaximumStringLength - i, " %s%02.0f%s",
                      rkGlobalParameters.showColor ? RKColorLag(worker->lag) : "",
                      99.5f * worker->lag,
                      rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Put a separator
    i += snprintf(string + i, RKMaximumStringLength - i, " |");
    // Duty cycle of each core
    for (c = 0; c < engine->coreCount && i < RKMaximumStringLength - 13; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKMaximumStringLength - i, " %s%02.0f%s",
                      rkGlobalParameters.showColor ? RKColorDutyCycle(worker->dutyCycle) : "",
                      99.5f * worker->dutyCycle,
                      rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Almost full count
    i += snprintf(string + i, RKMaximumStringLength - i, " [%d]", engine->almostFull);
    if (i > RKMaximumStringLength - 13) {
        memset(string + i, '#', RKMaximumStringLength - i - 1);
    }
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

#pragma mark - Delegate Workers

static void *ringFilterCore(void *_in) {
    RKPulseRingFilterWorker *me = (RKPulseRingFilterWorker *)_in;
    RKPulseRingFilterEngine *engine = me->parentEngine;

    int k;
    struct timeval t0, t1, t2;

    const int c = me->id;
    
    // Find the semaphore
    sem_t *sem = sem_open(me->semaphoreName, O_RDWR);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve semaphore %d\n", c);
        return (void *)RKResultFailedToRetrieveSemaphore;
    };

    // Initiate a variable to store my name
    char name[RKNameLength];
    if (rkGlobalParameters.showColor) {
        pthread_mutex_lock(&engine->coreMutex);
        k = snprintf(name, RKNameLength - 1, "%s", rkGlobalParameters.showColor ? RKGetColor() : "");
        pthread_mutex_unlock(&engine->coreMutex);
    } else {
        k = 0;
    }
    if (engine->coreCount > 9) {
        k += sprintf(name + k, "P%02d", c);
    } else {
        k += sprintf(name + k, "P%d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(name + k, RKNoColor);
    }

    size_t mem = 0;
    
    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);

    // Log my initial state
    pthread_mutex_lock(&engine->coreMutex);
    engine->memoryUsage += mem;
    
    RKLog(">%s %s Started.   mem = %s B\n",
          engine->name, name, RKIntegerToCommaStyleString(mem));
    
    pthread_mutex_unlock(&engine->coreMutex);

    // Increase the tic once to indicate this processing core is created.
    me->tic++;
    
    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]
    //
    uint32_t tic = me->tic;

    while (engine->statusBufferIndex & RKEngineStateActive) {
        t2 = t0;
    }

    // Clean up
    if (engine->verbose > 1) {
        RKLog("%s %s Freeing reources ...\n", engine->name, name);
    }

    RKLog(">%s %s Stopped.\n", engine->name, name);

    return NULL;
}

#pragma mark - Life Cycle

RKPulseRingFilterEngine *RKPulseRingFilterEngineInit(void) {
    RKPulseRingFilterEngine *engine = (RKPulseRingFilterEngine *)malloc(sizeof(RKPulseRingFilterEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a pulse ring filter engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKPulseRingFilterEngine));
    sprintf(engine->name, "%s<PulseRingFilter>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPulseRingFilterEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->useSemaphore = true;
    engine->memoryUsage = sizeof(RKPulseRingFilterEngine);
    pthread_mutex_init(&engine->coreMutex, NULL);
    return engine;
}

void RKPulseRingFilterEngineFree(RKPulseRingFilterEngine *engine) {
    if (engine->state & RKEngineStateActive) {
        RKPulseRingFilterEngineStop(engine);
    }
    pthread_mutex_destroy(&engine->coreMutex);
    free(engine);
}

#pragma mark - Properties

void RKPulseRingFilterEngineSetVerbose(RKPulseRingFilterEngine *engine, const int verb) {
    engine->verbose = verb;
}

void RKPulseRingFilterEngineSetInputOutputBuffers(RKPulseRingFilterEngine *engine, const RKRadarDesc *desc,
                                                  RKConfig *configBuffer, uint32_t *configIndex,
                                                  RKBuffer pulseBuffer,   uint32_t *pulseIndex) {
    
}

void RKPulseRingFilterEngineSetCoreCount(RKPulseRingFilterEngine *engine, const uint8_t count) {
    if (engine->state & RKEngineStateActive) {
        RKLog("%s Error. Core count cannot change when the engine is active.\n", engine->name);
        return;
    }
    engine->coreCount = count;
}

void RKPulseRingFilterEngineSetCoreOrigin(RKPulseRingFilterEngine *engine, const uint8_t origin) {
    if (engine->state & RKEngineStateActive) {
        RKLog("%s Error. Core origin cannot change when the engine is active.\n", engine->name);
        return;
    }
    engine->coreOrigin = origin;
}

int RKPulseRingFilterEngineSetFilter(RKPulseRingFilterEngine *engine, RKIIRFilter *filter) {
    return RKResultSuccess;
}

int RKPulseRingFilterEngineStart(RKPulseRingFilterEngine *engine) {
    return RKResultSuccess;
}

int RKPulseRingFilterEngineStop(RKPulseRingFilterEngine *engine) {
    return RKResultSuccess;
}

char *RKPulseRingFilterEngineStatusString(RKPulseRingFilterEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}

#pragma mark - Interactions

