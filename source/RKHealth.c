//
//  RKHealth.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//
//  █

#include <RadarKit/RKHealth.h>

void *healthConsolidator(void *in) {
    RKHealthEngine *engine = (RKHealthEngine *)in;
    RKRadarDesc *desc = engine->radarDescription;
    
    int k, j, s;
    
    RKHealth *health;
    char *valueString, *subString;
    bool led = false;
    float value = 0.0f;
    int type = 0;
    
    uint32_t *indices = (uint32_t *)malloc(desc->healthNodeCount * sizeof(uint32_t));
    memset(indices, 0, desc->healthNodeCount * sizeof(uint32_t));
    
    char keywords[][RKNameLength] = {"HVPS", "Body Current", "Cathode Voltage"};
    const int keywordsCount = sizeof(keywords) / RKNameLength;
    
    RKLog("%s Started.   mem = %s B   healthIndex = %d   keywordsCount = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->healthIndex, keywordsCount);
    
    engine->state = RKHealthEngineStateActive;
    
    bool allSame = true;
    
    k = 0;   // health index
    while (engine->state == RKHealthEngineStateActive) {
        // Get the latest health
        health = &engine->healthBuffer[k];
        // Wait until a newer status came in
        s = 0;
//        while (k == *engine->healthIndex && engine->state == RKHealthEngineStateActive) {
//            usleep(10000);
//            if (++s % 100 == 0 && engine->verbose > 1) {
//                RKLog("%s sleep 0/%.1f s   k = %d   health.s = 0x%02x\n",
//                      engine->name, (float)s * 0.01f, k, health->flag);
//            }
//        }
        allSame = true;
        while (allSame && engine->state == RKHealthEngineStateActive) {
            for (j = 0; j < desc->healthNodeCount; j++) {
                if (indices[j] != engine->healthNodes[j].index) {
                    indices[j] = engine->healthNodes[j].index;
                    allSame = false;
                }
            }
            if (allSame) {
                usleep(10000);
                if (++s % 100 == 0 && engine->verbose > 1) {
                    RKLog("%s sleep 0/%.1f s   k = %d   health.s = 0x%02x\n",
                          engine->name, (float)s * 0.01f, k, health->flag);
                }
            }
        }
        
        // Combine all the JSON strings
        RKLog("%s %s %s\n", engine->name, engine->healthNodes[0].healths[indices[0]], engine->healthNodes[2].healths[indices[2]]);

        if (engine->state != RKHealthEngineStateActive) {
            break;
        }
        
        // Look for certain keywords, extract some information
        if ((valueString = RKGetValueOfKey(health->string, "latitude")) != NULL) {
            desc->latitude = atof(valueString);
        }
        if ((valueString = RKGetValueOfKey(health->string, "longitude")) != NULL) {
            desc->longitude = atof(valueString);
        }
        
        for (j = 0; j < keywordsCount; j++) {
            subString = RKGetValueOfKey(health->string, keywords[j]);
            if (subString) {
                valueString = RKGetValueOfKey(subString, "value");
                if (valueString) {
                    if (!strcasecmp(valueString, "true")) {
                        type = 1;
                        led = true;
                    } else if (!strcasecmp(valueString, "false")) {
                        type = 1;
                        led = false;
                    } else {
                        type = 2;
                        value = atof(valueString);
                    }
                }
                valueString = RKGetValueOfKey(subString, "color");
                if (valueString) {
                    if (!strcasecmp(valueString, "green")) {
                        //printf("got green\n");
                    } else if (!strcasecmp(valueString, "orange")) {
                        //printf("got orange\n");
                    } else if (!strcasecmp(valueString, "red")) {
                        RKLog("%s Got red on HVPS\n", engine->name);
                        // Suspend the radar
                    }
                }
                if (engine->verbose > 1) {
                    if (type == 1) {
                        RKLog("%s %s -> %s / %s", engine->name, keywords[j], led == true ? "On" : "Off", valueString);
                    } else if (type == 2) {
                        RKLog("%s %s -> %.4f / %s", engine->name, keywords[j], value, valueString);
                    }
                }
            }
        }

        // Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->healthBufferDepth);
    }
    
    free(indices);
    
    return NULL;
}

#pragma mark - Life Cycle

RKHealthEngine *RKHealthEngineInit() {
    RKHealthEngine *engine = (RKHealthEngine *)malloc(sizeof(RKHealthEngine));
    memset(engine, 0, sizeof(RKHealthEngine));
    sprintf(engine->name, "%s<HealthAssistant>%s",
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
                                         RKRadarDesc *radarDescription,
                                         RKNodalHealth *healthNodes,
                                         RKHealth *healthBuffer, uint32_t *healthIndex, const uint32_t healthBufferDepth) {
    engine->radarDescription = radarDescription;
    engine->healthNodes = healthNodes;
    engine->healthBuffer = healthBuffer;
    engine->healthIndex = healthIndex;
    engine->healthBufferDepth = healthBufferDepth;
}

int RKHealthEngineStart(RKHealthEngine *engine) {
    RKLog("%s Starting ...\n", engine->name);
    if (pthread_create(&engine->threadId, NULL, healthConsolidator, engine)) {
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
