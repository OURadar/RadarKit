//
//  RKHealth.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHealth.h>

#pragma mark - Helper Functions

static void RKProcessHealthKeywords(RKHealthEngine *engine, const char *string) {

    char *str = (char *)malloc(strlen(string) + 1);
    char *key = (char *)malloc(RKNameLength);
    char *obj = (char *)malloc(RKMaximumPathLength);
    char *subKey = (char *)malloc(RKNameLength);
    char *subObj = (char *)malloc(RKMaximumPathLength);
    uint8_t type;
    uint8_t subType;
    RKStatusEnum componentStatus;
    
    if (str == NULL) {
        RKLog("%s Error allocating memory for str.\n", engine->name);
        return;
    }
    if (key == NULL) {
        RKLog("%s Error allocating memory for key.\n", engine->name);
        return;
    }
    if (obj == NULL) {
        RKLog("%s Error allocating memory for obj.\n", engine->name);
        return;
    }
    if (subKey == NULL) {
        RKLog("%s Error allocating memory for subKey.\n", engine->name);
        return;
    }
    if (subObj == NULL) {
        RKLog("%s Error allocating memory for subObj.\n", engine->name);
        return;
    }

    strcpy(str, string);

    char *ks;
    char *sks;
    if (*str != '{') {
        fprintf(stderr, "Expected '{'.\n");
    }

    ks = str + 1;
    while (*ks != '\0' && *ks != '}') {
        ks = RKExtractJSON(ks, &type, key, obj);
        if (type != RKJSONObjectTypeObject) {
            continue;
        }
        sks = obj + 1;
        while (*sks != '\0' && *sks != '}') {
            sks = RKExtractJSON(sks, &subType, subKey, subObj);
            if (strcmp("Enum", subKey)) {
                continue;
            }
            if ((componentStatus = atoi(subKey)) == RKStatusEnumCritical) {
                RKLog("%s Warning. '%s' registered critical.\n", engine->name, key);
            }
        }
    }

    free(str);
    free(key);
    free(obj);
    free(subKey);
    free(subObj);
}

#pragma mark - Delegate Workers

static void *healthConsolidator(void *in) {
    RKHealthEngine *engine = (RKHealthEngine *)in;
    RKRadarDesc *desc = engine->radarDescription;
    
    int i, j, k, n, s;
    
    RKHealth *health;
    char *stringValue, *stringEnum, *stringObject;
    int headingChangeCount = 0;
    int locationChangeCount = 0;

    char filename[RKMaximumStringLength];
    
    uint32_t *indices = (uint32_t *)malloc(desc->healthNodeCount * sizeof(uint32_t));
    memset(indices, 0xFF, desc->healthNodeCount * sizeof(uint32_t));
    
    RKLog("%s Started.   mem = %s B   healthIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->healthIndex);
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    bool allTrue;
    char *string;
    struct timeval t0, t1;

    gettimeofday(&t1, NULL);

    time_t unixTime;
    struct tm *timeStruct;
    int min = -1;

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
        engine->state |= RKEngineStateSleep1;
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
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // Wait until all the flags are ready (wait when any flag is still vacant)
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
                if (++s == 10) {
                    // Waited too long
                    for (j = 1; j < desc->healthNodeCount; j++) {
                        if (engine->healthNodes[j].active && engine->healthNodes[j].healths[indices[j]].flag != RKHealthFlagReady) {
                            RKLog("%s Replacing enum in node %d @ %d ...\n", engine->name, j, indices[j]);
                            // This node has been disconnected, duplicate the latest reading, set all enum to old
                            n = indices[j];
                            RKHealth *h0 = &engine->healthNodes[j].healths[n];
                            n = RKPreviousModuloS(n, engine->radarDescription->healthBufferDepth);
                            RKHealth *h1 = &engine->healthNodes[j].healths[n];
                            // Copy over the previous health to current health, set all enums to old
                            h0->i += engine->radarDescription->healthBufferDepth;
                            strcpy(h0->string, h1->string);
                            RKReplaceKeyValue(h0->string, "Enum", RKStatusEnumOld);
                            h0->flag = RKHealthFlagReady;
                        }
                    }
                } else if (engine->verbose > 1) {
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
        engine->state ^= RKEngineStateSleep2;
        
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
            i = sprintf(string, "flags   = [%02x", engine->healthNodes[0].healths[n].flag);
            for (j = 1; j < desc->healthNodeCount; j++) {
                n = indices[j];
                i += sprintf(string + i,  ", %02x", engine->healthNodes[j].healths[n].flag);
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
        i += sprintf(string + i, "\"Log Time\":%zu}", t0.tv_sec);                                          // Add the log time as the last object
        health->flag = RKHealthFlagReady;

        //RKLog("%s", string);

        // Look for certain keywords with {"Value":x,"Enum":y} pairs, extract some information
        float latitude = NAN, longitude = NAN, heading = NAN;
        
        if ((stringObject = RKGetValueOfKey(health->string, "latitude")) != NULL) {
            stringValue = RKGetValueOfKey(stringObject, "value");
            stringEnum = RKGetValueOfKey(stringObject, "enum");
            if (stringValue != NULL && stringEnum != NULL && atoi(stringEnum) == RKStatusEnumNormal) {
                latitude = atof(stringValue);
            }
        }
        if ((stringObject = RKGetValueOfKey(health->string, "longitude")) != NULL) {
            stringValue = RKGetValueOfKey(stringObject, "value");
            stringEnum = RKGetValueOfKey(stringObject, "enum");
            if (stringValue != NULL && stringEnum != NULL && atoi(stringEnum) == RKStatusEnumNormal) {
                longitude = atof(stringValue);
            }
        }
        if ((stringObject = RKGetValueOfKey(health->string, "heading")) != NULL) {
            stringValue = RKGetValueOfKey(stringObject, "value");
            stringEnum = RKGetValueOfKey(stringObject, "enum");
            if (stringValue != NULL && stringEnum != NULL && atoi(stringEnum) == RKStatusEnumNormal) {
                heading = atof(stringValue);
            }
        }
        if (isfinite(latitude) && isfinite(longitude && isfinite(heading))) {
            if (engine->verbose > 1) {
                RKLog("%s GPS:  latitude = %.7f   longitude = %.7f   heading = %.2f\n", engine->name, latitude, longitude, heading);
            }
            // Only update if it is significant, GPS accuracy < 7.8 m ~ 7.0e-5 deg. Let's do half.
            if (locationChangeCount++ > 3 && (fabs(engine->radarDescription->latitude - latitude) > 3.5e-5 || fabs(engine->radarDescription->longitude - longitude) > 3.5e-5)) {
                engine->radarDescription->latitude = latitude;
                engine->radarDescription->longitude = longitude;
                RKLog("%s GPS update.   latitude = %.7f   longitude = %.7f\n", engine->name, engine->radarDescription->latitude, engine->radarDescription->longitude);
            }
            if (headingChangeCount++ > 3 && fabs(engine->radarDescription->heading - heading) > 1.0) {
                engine->radarDescription->heading = heading;
                RKLog("%s GPS update.   heading = %.2f degree\n", engine->name, engine->radarDescription->heading);
                headingChangeCount = 0;
            }
        }

        RKProcessHealthKeywords(engine, health->string);

        // Log a copy
        unixTime = (time_t)t0.tv_sec;
        timeStruct = gmtime(&unixTime);
        if (min != timeStruct->tm_min) {
            min = timeStruct->tm_min;
            if (engine->healthLog != NULL) {
                fclose(engine->healthLog);
                RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeHealth);
            }
            i = sprintf(filename, "%s%s%s/", engine->radarDescription->dataPath, engine->radarDescription->dataPath[0] == '\0' ? "" : "/", RKDataFolderHealth);
            i += strftime(filename + i, 10, "%Y%m%d", gmtime(&unixTime));
            i += sprintf(filename + i, "/%s-", engine->radarDescription->filePrefix);
            i += strftime(filename + i, 22, "%Y%m%d-%H%M%S.json", gmtime(&unixTime));
            RKPreparePath(filename);
            RKLog("%s %s\n", engine->name, filename);
            engine->healthLog = fopen(filename, "w");
        }
        if (engine->healthLog != NULL) {
            fprintf(engine->healthLog, "%s\n", string);
        }

        // Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->healthBufferDepth);
        health = &engine->healthBuffer[k];
        health->string[0] = '\0';
        health->flag = RKHealthFlagVacant;
        *engine->healthIndex = k;
    }

    if (engine->healthLog != NULL) {
        fclose(engine->healthLog);
        engine->healthLog = NULL;
    }

    free(indices);
    
    return NULL;
}

#pragma mark - Life Cycle

RKHealthEngine *RKHealthEngineInit() {
    RKHealthEngine *engine = (RKHealthEngine *)malloc(sizeof(RKHealthEngine));
    memset(engine, 0, sizeof(RKHealthEngine));
    sprintf(engine->name, "%s<HealthAssistant>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(5) : "", rkGlobalParameters.showColor ? RKNoColor : "");
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

void RKHealthEngineSetInputOutputBuffers(RKHealthEngine *engine, RKRadarDesc *desc, RKFileManager *fileManager,
                                         RKNodalHealth *healthNodes,
                                         RKHealth *healthBuffer, uint32_t *healthIndex, const uint32_t healthBufferDepth) {
    engine->radarDescription  = desc;
    engine->fileManager       = fileManager;
    engine->healthNodes       = healthNodes;
    engine->healthBuffer      = healthBuffer;
    engine->healthIndex       = healthIndex;
    engine->healthBufferDepth = healthBufferDepth;
    engine->state |= RKEngineStateProperlyWired;
}

int RKHealthEngineStart(RKHealthEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
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
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->state);
    }
    return RKResultSuccess;
}

char *RKHealthEngineStatusString(RKHealthEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, engine->healthBufferDepth)];
}
