//
//  RKSweep.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKSweep.h>

#pragma mark - Helper Functions

void *sweepWriter(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    uint32_t n = engine->sweep.count;
    RKRay **rays = engine->sweep.rays;
    //RKLog(">%s %p %p %p ... %p\n", engine->name, rays[0], rays[1], rays[2], rays[n - 1]);

    RKRay *S = rays[0];
    RKRay *E = rays[n - 1];
    RKLog("%s Sweep   C%d-%d   E%4.2f-%.2f   A%6.2f-%6.2f   \033[32mM\033[0m%04x-%04x   %05lu...%05lu (%s%d%s)\n",
          engine->name,
          S->header.configIndex    , E->header.configIndex,
          S->header.startElevation , E->header.endElevation,
          S->header.startAzimuth   , E->header.endAzimuth,
          S->header.marker & 0xFFFF, E->header.marker & 0xFFFF,
          S->header.n              , E->header.n,
          n < 360 ? "\033[91m" : "",
          n,
          RKNoColor);
    
    
    
    return NULL;
}

#pragma mark - Threads

void *rayGatherer(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    int k, s, n;
    
    RKRay *ray, *S, *E;
    RKRay **rays = engine->sweep.rays;

    // Start and end indices of the input rays
    uint32_t is = 0;
    pthread_t tidSweepWriter = NULL;
    
    RKLog("%s started.   mem = %s B   rayIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->rayIndex);
    
    engine->state |= RKSweepEngineStateActive;
    
    k = 0;   // ray index
    while (engine->state & RKSweepEngineStateActive) {
        // The ray
        ray = RKGetRay(engine->rayBuffer, k);
        // Wait until the buffer is advanced
        s = 0;
        while (k == *engine->rayIndex && engine->state & RKSweepEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->rayIndex, ray->header.s);
            }
        }
        // Wait until the ray is ready
        s = 0;
        while (!(ray->header.s & RKRayStatusReady) && engine->state & RKSweepEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->rayIndex, ray->header.s);
            }
        }
        if (engine->state & RKSweepEngineStateActive) {
            // Lag of the engine
            engine->lag = fmodf(((float)*engine->rayIndex + engine->rayBufferDepth - k) / engine->rayBufferDepth, 1.0f);
            if (ray->header.marker & RKMarkerSweepEnd) {
                S = RKGetRay(engine->rayBuffer, is);
                E = ray;
                n = 0;
                do {
                    ray = RKGetRay(engine->rayBuffer, is);
                    ray->header.n = is;
                    rays[n++] = ray;
                    is = RKNextModuloS(is, engine->rayBufferDepth);
                } while (is != k && n < RKMaxRaysPerSweep);
                ray = RKGetRay(engine->rayBuffer, is);
                ray->header.n = is;
                rays[n++] = ray;
                engine->sweep.count = n;
                is = RKPreviousNModuloS(is, n + 1, engine->rayBufferDepth);

//                RKLog("%s Sweep   E%4.2f-%.2f   A%6.2f-%6.2f   \033[32mM\033[0m%04x-%04x   %05lu...%05lu (%d)\n",
//                      engine->name,
//                      S->header.startElevation , E->header.endElevation,
//                      S->header.startAzimuth   , E->header.endAzimuth,
//                      S->header.marker & 0xFFFF, E->header.marker & 0xFFFF,
//                      is, k, n);
                
                //RKLog(">%s %p %p %p ... %p\n", engine->name, engine->sweep.rays[0], engine->sweep.rays[1], engine->sweep.rays[2], engine->sweep.rays[n - 1]);
                
                if (tidSweepWriter) {
                    pthread_join(tidSweepWriter, NULL);
                }
                if (pthread_create(&tidSweepWriter, NULL, sweepWriter, engine)) {
                    RKLog("%s Error. Unable to launch a sweep writer.\n", engine->name);
                }

                is = k;
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

