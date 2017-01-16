//
//  RKSweep.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKSweep.h>

#pragma mark - Threads

void *rayGatherer(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    int k, s, n;
//    int i, j;
    
    RKRay *ray, *S, *E;
    
    // Start and end indices of the input rays
    uint32_t is = 0;
    uint32_t ie = 0;

    RKLog("%s started.   mem = %s B   engine->index = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->rayIndex);
    
    engine->state |= RKSweepEngineStateActive;
    
    k = 0;   // ray index
    s = 0;
    while (engine->state & RKSweepEngineStateActive) {
        ray = RKGetRay(engine->rayBuffer, k);
        while (k == *engine->rayIndex && engine->state & RKSweepEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose) {
                RKLog("%s sleep 1/%.1f s\n", engine->name, (float)s * 0.01f);
            }
        }
        if (engine->state & RKSweepEngineStateActive) {
            // Lag of the engine
            engine->lag = fmodf(((float)*engine->rayIndex + engine->rayBufferDepth - k) / engine->rayBufferDepth, 1.0f);
            if (ray->header.marker & RKMarkerSweepEnd) {
                ie = k;
                E = ray;
                S = RKGetRay(engine->rayBuffer, is);
                n = ie - is + 1;
                if (n < 0) {
                    n += engine->rayBufferDepth;
                }
                RKLog("%s Sweep   E%4.2f-%.2f   A%6.2f-%6.2f  %08x  %05lu...%05lu (%d)\n",
                      engine->name,
                      S->header.startElevation, E->header.endElevation,
                      S->header.startAzimuth, E->header.endAzimuth, ray->header.marker,
                      is, ie, n);
                
//                if (n > 357 && n < 360) {
//                    n = is;
//                    j = 0;
//                    i = RKNextModuloS(ie, engine->rayBufferDepth);
//                    do {
//                        j++;
//                        S = RKGetRay(engine->rayBuffer, n);
//                        RKLog(">%s  %3d  %04d    A%6.2f-%6.2f %d  %08x", engine->name, j, n, S->header.startAzimuth, S->header.endAzimuth, S->header.n, S->header.marker);
//                        n = RKNextModuloS(n, engine->rayBufferDepth);
//                    } while (n != i);
//                }
                
                is = ie;
            }
        }
        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->rayBufferDepth);
    }
    return NULL;
}

#pragma mark - Life Cycle

RKSweepEngine *RKSweepEngineInit(void) {
    RKSweepEngine *engine = (RKSweepEngine *)malloc(sizeof(RKSweepEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a sweep engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKSweepEngine));
    sprintf(engine->name, "%s<SweepProducer>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKSweepEngineStateAllocated;
    engine->memoryUsage = sizeof(RKSweepEngine);
    return engine;
}

void RKSweepEngineFree(RKSweepEngine *engine) {
    if (engine->state & RKSweepEngineStateActive) {
        engine->state ^= RKSweepEngineStateActive;
    }
    
}

#pragma mark - Properties

void RKSweepEngineSetVerbose(RKSweepEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKSweepEngineSetInputBuffer(RKSweepEngine *engine, RKBuffer rayBuffer, uint32_t *rayIndex, const uint32_t rayBufferDepth) {
    engine->rayBuffer = rayBuffer;
    engine->rayIndex = rayIndex;
    engine->rayBufferDepth = rayBufferDepth;
}

#pragma mark - Interactions

int RKSweepEngineStart(RKSweepEngine *engine) {
    RKLog("%s starting ...\n", engine->name);
    if (pthread_create(&engine->tidRayGatherer, NULL, rayGatherer, engine) != 0) {
        RKLog("Error. Failed to start a ray gatherer.\n");
        return RKResultFailedToStartRayGatherer;
    }
    while (!(engine->state & RKSweepEngineStateActive)) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKSweepEngineStop(RKSweepEngine *engine) {
    if (engine->state & RKSweepEngineStateActive) {
        engine->state ^= RKSweepEngineStateActive;
    }
    pthread_join(engine->tidRayGatherer, NULL);
    return RKResultSuccess;
}

