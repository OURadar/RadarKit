//
//  RKHealth.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHealth.h>

void *healthRelay(void *in) {
    RKHealthEngine *engine = (RKHealthEngine *)in;
    
    int k, s;
    
    RKHealth *health;
    
    RKLog("%s started.   mem = %s B   pulseIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->healthIndex);
    
    engine->state = RKHealthEngineStateActive;
    
    k = 0;   // health index
    while (engine->state == RKHealthEngineStateActive) {
        // Get the latest health
        health = &engine->healthBuffer[*engine->healthIndex];
        // Wait until a newer status came in
        s = 0;
        while (k == *engine->healthIndex && engine->state == RKHealthEngineStateActive) {
            
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 0/%.1f s   k = %d   health.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, health->flag);
            }
        }
        k = RKNextModuloS(k, engine->healthBufferDepth);
    }

    return (void *)NULL;
}

#pragma mark - Life Cycle

RKHealthEngine *RKHealthEngineInit() {
    RKHealthEngine *engine = (RKHealthEngine *)malloc(sizeof(RKHealthEngine));
    memset(engine, 0, sizeof(RKHealthEngine));
    sprintf(engine->name, "%s<TweetaRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->memoryUsage = sizeof(RKHealthEngine);
    engine->state = RKHealthEngineStateAllocated;
    return engine;
}

void RKHealthEngineFree(RKHealthEngine *engine) {
    free(engine);
}

#pragma mark - Properties

void RKHealthEngineSetVerbose(RKHealthEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKHealthEngineSetInputOutputBuffers(RKHealthEngine *engine,
                                         RKHealth *healthBuffer, uint32_t *healthIndex, const uint32_t healthBufferDepth) {
    engine->healthBuffer = healthBuffer;
    engine->healthIndex = healthIndex;
    engine->healthBufferDepth = healthBufferDepth;
}

int RKHealthEngineStart(RKHealthEngine *engine) {
    RKLog("%s starting ...\n", engine->name);
    if (pthread_create(&engine->threadId, NULL, healthRelay, engine)) {
        RKLog("Error. Unable to start health engine.\n");
        return RKResultFailedToStartHealthWorker;
    }
    while (engine->state < RKHealthEngineStateActive) {
        usleep(10000);
    }
    struct timeval t;
    gettimeofday(&t, NULL);
    engine->startTime = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    return RKResultSuccess;
}

int RKHealthEngineStop(RKHealthEngine *engine) {
    if (engine->verbose > 1) {
        RKLog("%s stopping ...\n", engine->name);
    }
    engine->state = RKHealthEngineStateDeactivating;
    pthread_join(engine->threadId, NULL);
    RKLog("%s stopped.\n", engine->name);
    engine->state = RKHealthEngineStateNull;
    return RKResultSuccess;
}

char *RKHealthEngineStatusString(RKHealthEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, engine->healthBufferDepth)];
}
