//
//  RKHealth.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHealth.h>

void *healthConsolidator(void *in) {
    RKHealthEngine *engine = (RKHealthEngine *)in;
    RKRadarDesc *desc = engine->radarDescription;
    
    int i, j, k, n, s;
    
    RKHealth *health;
    char *stringValue, *stringEnum, *stringObject;
    int type = 0;
    int valueEnum = 0;
    bool valueBool = false;
    float valueFloat = 0.0f;

    uint32_t *indices = (uint32_t *)malloc(desc->healthNodeCount * sizeof(uint32_t));
    memset(indices, 0xFF, desc->healthNodeCount * sizeof(uint32_t));
    
    char keywords[][RKNameLength] = {"HVPS", "Body Current", "Cathode Voltage", "FPGA Temp"};
    const int keywordsCount = sizeof(keywords) / RKNameLength;
    
    RKLog("%s Started.   mem = %s B   healthIndex = %d   keywordsCount = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->healthIndex, keywordsCount);
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    bool allTrue;
    char *string;
    struct timeval t0, t1;

    gettimeofday(&t1, NULL);

    k = 0;   // health index
    while (engine->state & RKEngineStateActive) {
        // Evaluate the nodal-health buffers every once in a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) < 0.5) {
            usleep(10000);
            continue;
        }
        t1 = t0;

        // Get the latest health
        health = &engine->healthBuffer[k];
        string = health->string;

        // Wait while all the indices are the same (wait when all the indices are the same)
        s = 0;
        allTrue = true;
        while (allTrue && engine->state & RKEngineStateActive) {
            for (j = 0; j < desc->healthNodeCount; j++) {
                if (indices[j] != engine->healthNodes[j].index) {
                    indices[j] = engine->healthNodes[j].index;
                    allTrue = false;
                }
            }
            if (allTrue) {
                usleep(100000);
                if (++s % 20 == 0 && engine->verbose) {
                    i = sprintf(string, "indices = [%02d", engine->healthNodes[0].active ? indices[0] : -1);
                    for (j = 1; j < desc->healthNodeCount; j++) {
                        i += sprintf(string + i,  ", %02d", engine->healthNodes[j].active ? indices[j] : -1);
                    }
                    i += sprintf(string + i, "]");
                    RKLog("%s sleep 0/%.1f s   %s   k = %d\n", engine->name, (float)s * 0.1f, string, k);
                }
            }
        }
        // Wait until all the flags are ready (wait when any flag is vacant)
        s = 0;
        while (engine->state & RKEngineStateActive) {
            allTrue = true;
            for (j = 0; j < desc->healthNodeCount; j++) {
                if (engine->healthNodes[j].active) {
                    allTrue &= engine->healthNodes[j].healths[indices[j]].flag == RKHealthFlagReady;
                }
            }
            if (allTrue) {
                break;
            } else {
                usleep(100000);
                if (++s % 20 == 0 && engine->verbose) {
                    n = indices[0];
                    i = sprintf(string, "flags = [%x", engine->healthNodes[0].healths[n].flag);
                    for (j = 1; j < desc->healthNodeCount; j++) {
                        n = indices[j];
                        i += sprintf(string + i,  ", %x", engine->healthNodes[j].healths[n].flag);
                    }
                    i += sprintf(string + i, "]");
                    RKLog("%s sleep 1/%.1f s   %s   k = %d\n", engine->name, (float)s * 0.1f, string, k);
                }
            }
        }
        if (!(engine->state & RKEngineStateActive)) {
            break;
        }

        if (engine->verbose > 1) {
            i = sprintf(string, "indices = [%02d", engine->healthNodes[0].active ? indices[0] : -1);
            for (j = 1; j < desc->healthNodeCount; j++) {
                i += sprintf(string + i,  ", %02d", engine->healthNodes[j].active ? indices[j] : -1);
            }
            i += sprintf(string + i, "]");
            RKLog("%s %s   k = %d\n", engine->name, string, k);
            n = indices[0];
            i = sprintf(string, "flags = [%x", engine->healthNodes[0].healths[n].flag);
            for (j = 1; j < desc->healthNodeCount; j++) {
                n = indices[j];
                i += sprintf(string + i,  ", %x", engine->healthNodes[j].healths[n].flag);
            }
            i += sprintf(string + i, "]");
            RKLog("%s %s   k = %d   s = %d\n", engine->name, string, k, s);
        }

        // Combine all the active JSON strings
        i = sprintf(string, "{");
        for (j = 0; j < desc->healthNodeCount; j++) {
            n = indices[j];
            if (engine->healthNodes[j].active && strlen(engine->healthNodes[j].healths[n].string) > 6) {   // {"k":0} is at least 7 chars
                i += sprintf(string + i, "%s", engine->healthNodes[j].healths[n].string + 1);              // Ignore the first "{"
                i -= RKStripTail(string);                                                                  // Strip away white spaces
                i--;                                                                                       // Ignore the last "}"
                i += sprintf(string + i, ", ");                                                            // Get ready to concatenante
            }
        }
        i += sprintf(string + i - 2, "}");                                                                 // Erase the last ", "
        health->flag = RKHealthFlagReady;

        //RKLog("%s", string);

        // Look for certain keywords, extract some information
        if ((stringValue = RKGetValueOfKey(health->string, "latitude")) != NULL) {
            desc->latitude = atof(stringValue);
        }
        if ((stringValue = RKGetValueOfKey(health->string, "longitude")) != NULL) {
            desc->longitude = atof(stringValue);
        }
        
        for (j = 0; j < keywordsCount; j++) {
            stringObject = RKGetValueOfKey(health->string, keywords[j]);
            //RKLog("%s %s subString = %s\n", engine->name, keywords[j], subString);
            if (stringObject) {
                stringValue = RKGetValueOfKey(stringObject, "value");
                if (stringValue) {
                    if (!strcasecmp(stringValue, "true")) {
                        type = 1;
                        valueBool = true;
                    } else if (!strcasecmp(stringValue, "false")) {
                        type = 1;
                        valueBool = false;
                    } else {
                        type = 2;
                        valueFloat = atof(stringValue);
                    }
                }
                stringEnum = RKGetValueOfKey(stringObject, "enum");
                if (stringEnum) {
                    valueEnum = atoi(stringEnum);
                    if (valueEnum > 1) {
                        RKLog("%s Warning. %s -> %s / %d --> Shutdown\n",
                              engine->name,
                              keywords[j],
                              type == 1 ? (valueBool ? "true" : "false") : (RKFloatToCommaStyleString(valueFloat)),
                              valueEnum);
                    }
                }
            }
        }

        // Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->healthBufferDepth);
        health = &engine->healthBuffer[k];
        health->string[0] = '\0';
        health->flag = RKHealthFlagVacant;
        *engine->healthIndex = k;
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
    engine->state = RKEngineStateAllocated;
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
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->threadId, NULL, healthConsolidator, engine)) {
        RKLog("Error. Unable to start health engine.\n");
        return RKResultFailedToStartHealthWorker;
    }
    while (!(engine->state & RKEngineStateActive)) {
        usleep(10000);
    }
    struct timeval t;
    gettimeofday(&t, NULL);
    engine->startTime = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    return RKResultSuccess;
}

int RKHealthEngineStop(RKHealthEngine *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose > 1) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    pthread_join(engine->threadId, NULL);
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    engine->state = RKEngineStateAllocated;
    return RKResultSuccess;
}

char *RKHealthEngineStatusString(RKHealthEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, engine->healthBufferDepth)];
}
