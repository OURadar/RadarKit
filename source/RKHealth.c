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
    
    int i, j, k, n, s;
    
    RKHealth *health;
    char *valueString, *enumString, *subString;
    int type = 0;
    int valueEnum = 0;
    bool valueBool = false;
    float valueFloat = 0.0f;

    uint32_t *indices = (uint32_t *)malloc(desc->healthNodeCount * sizeof(uint32_t));
    memset(indices, 0, desc->healthNodeCount * sizeof(uint32_t));
    
    char keywords[][RKNameLength] = {"HVPS", "Body Current", "Cathode Voltage", "FPGA Temp"};
    const int keywordsCount = sizeof(keywords) / RKNameLength;
    
    RKLog("%s Started.   mem = %s B   healthIndex = %d   keywordsCount = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->healthIndex, keywordsCount);
    
    engine->state = RKHealthEngineStateActive;
    
    bool allSame;
    char string[RKMaximumStringLength];
    struct timeval t0, t1;

    gettimeofday(&t1, NULL);

    k = 0;   // health index
    while (engine->state == RKHealthEngineStateActive) {
        // Evaluate the nodal-health buffers every once in a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) < 0.25) {
            usleep(10000);
            continue;
        }
        t1 = t0;

        // Get the latest health
        health = &engine->healthBuffer[k];

        // Wait until a newer status came in
        s = 0;
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
                if (++s % 100 == 0 && engine->verbose) {
                    j = sprintf(string, "indices = [%02d", indices[0]);
                    for (i = 1; i < RKHealthNodeCount; i++) {
                        j += sprintf(string + j,  ", %02d", indices[i]);
                    }
                    j += sprintf(string + j, "]");
                    RKLog("%s sleep 0/%.1f s   %s   k = %d\n", engine->name, (float)s * 0.01f, string, k);
                }
            }
        }

        j = sprintf(string, "active = [%02d", engine->healthNodes[0].active ? indices[0] : -1);
        for (i = 1; i < RKHealthNodeCount; i++) {
            j += sprintf(string + j,  ", %02d", engine->healthNodes[i].active ? indices[i] : -1);
        }
        j += sprintf(string + j, "]");
        RKLog("%s %s   k = %d\n", engine->name, string, k);

        // Combine all the active JSON strings
        j = sprintf(string, "{");
        for (i = 0; i < RKHealthNodeCount; i++) {
            if (engine->healthNodes[i].active) {
                n = RKPreviousModuloS(indices[i], desc->healthBufferDepth);
                j += sprintf(string + j, "%s", engine->healthNodes[i].healths[n].string + 1);  // Ignore the first "{"
                j -= RKStripTail(string);                                                      // Strip away white spaces
                j--;                                                                           // Ignore the last "}"
                j += sprintf(string + j, ", ");                                                // Get ready to concatenante
            }
        }
        j += sprintf(string + j - 2, "}");                                                     // Erase the last ", "

        RKLog("%s %s\n", engine->name, string);

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
            RKLog("%s subString = %s\n", engine->name, subString);
            if (subString) {
                valueString = RKGetValueOfKey(subString, "value");
                if (valueString) {
                    if (!strcasecmp(valueString, "true")) {
                        type = 1;
                        valueBool = true;
                    } else if (!strcasecmp(valueString, "false")) {
                        type = 1;
                        valueBool = false;
                    } else {
                        type = 2;
                        valueFloat = atof(valueString);
                    }
                }
                enumString = RKGetValueOfKey(subString, "enum");
                if (enumString) {
                    valueEnum = atoi(enumString);
                    RKLog("%s %s -> %s / %d\n",
                          engine->name,
                          keywords[j],
                          type == 1 ? (valueBool ? "true" : "false") : (RKFloatToCommaStyleString(valueFloat)),
                          valueEnum);
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
