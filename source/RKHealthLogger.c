//
//  RKHealthLogger.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 4/26/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHealthLogger.h>

#pragma mark - Helper Functions

static void RKProcessHealthKeywords(RKHealthLogger *engine, const char *string) {

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
        fprintf(stderr, "RKProcessHealthKeywords() - Expected '{'.\n");
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

static void *healthLogger(void *in) {
    RKHealthLogger *engine = (RKHealthLogger *)in;
    RKRadarDesc *desc = engine->radarDescription;

    int i, k, s;
	struct timeval t0;

    RKHealth *health;
    char *stringValue, *stringEnum, *stringObject;
    int headingChangeCount = 0;
    int locationChangeCount = 0;

    char filename[RKMaximumPathLength] = "";

	time_t unixTime;
	struct tm *timeStruct;
	int min = -1;

	// Update the engine state
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;

    RKLog("%s Started.   mem = %s B   healthIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->healthIndex);

	// Increase the tic once to indicate the engine is ready
	engine->tic = 1;

    gettimeofday(&t0, NULL);

	// Here comes the busy loop
	// i  anonymous
    k = 0;   // health index
    while (engine->state & RKEngineStateActive) {
        // Get the latest health
        health = &engine->healthBuffer[k];

        // Wait until the engine index advances
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->healthIndex && engine->state & RKEngineStateActive) {
            usleep(100000);
            if (++s % 10 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   healthIndex = %d   flag = 0x%02x\n",
                      engine->name, (float)s * 0.1f, k , *engine->healthIndex, health->flag);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // Wait until the payload is properly filled
        s = 0;
        while (!(health->flag & RKHealthFlagReady) && engine->state & RKEngineStateActive) {
            usleep(100000);
            if (++s % 10 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   healthIndex = %d   flag = 0x%02x\n",
                      engine->name, (float)s * 0.1f, k , *engine->healthIndex, health->flag);
            }
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateActive)) {
            break;
        }

        // Log a copy
        char *keyValue = RKGetValueOfKey(health->string, "Log Time");
        if (keyValue == NULL) {
            RKLog("%s Error. No log time found.\n", engine->name);
            unixTime = (time_t)t0.tv_sec;
        } else {
            unixTime = (time_t)atol(keyValue);
        }
        timeStruct = gmtime(&unixTime);
        if (min != timeStruct->tm_min) {
            min = timeStruct->tm_min;
            if (engine->doNotWrite) {
                if (engine->verbose && strlen(filename)) {
                    RKLog("%s Skipped %s\n", engine->name, filename);
                }
            } else {
                if (engine->fid != NULL) {
                    fclose(engine->fid);
                    if (engine->verbose) {
                        RKLog("%s Recorded %s\n", engine->name, filename);
                    }
                    // Notify file manager of a new addition
                    RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeHealth);
                }
            }
            i = sprintf(filename, "%s%s%s/", desc->dataPath, desc->dataPath[0] == '\0' ? "" : "/", RKDataFolderHealth);
            i += strftime(filename + i, 10, "%Y%m%d", gmtime(&unixTime));
            i += sprintf(filename + i, "/%s-", desc->filePrefix);
            strftime(filename + i, 22, "%Y%m%d-%H%M%S.json", gmtime(&unixTime));
            if (!engine->doNotWrite) {
                RKPreparePath(filename);
                engine->fid = fopen(filename, "w");
                if (engine->fid == NULL) {
                    RKLog("%s Error. Unable to create file for health log.\n", engine->name);
                }
            }
        }
        if (engine->fid != NULL) {
            fprintf(engine->fid, "%s\n", health->string);
        }

        // Look for certain keywords with {"Value":x,"Enum":y} pairs, extract some information
        double latitude = NAN, longitude = NAN;
        float heading = NAN;
        
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
                heading = (float)atof(stringValue);
            }
        }
        if (isfinite(latitude) && isfinite(longitude) && isfinite(heading)) {
            if (engine->verbose > 1) {
                RKLog("%s GPS:  latitude = %.7f   longitude = %.7f   heading = %.2f\n", engine->name, latitude, longitude, heading);
            }
            // Only update if it is significant, GPS accuracy < 7.8 m ~ 7.0e-5 deg. Let's do half of that.
            if ((fabs(desc->latitude - latitude) > 3.5e-5 || fabs(desc->longitude - longitude) > 3.5e-5)) {
                if (locationChangeCount++ > 3) {
                    desc->latitude = latitude;
                    desc->longitude = longitude;
                    RKLog("%s GPS update.   latitude = %.7f   longitude = %.7f\n", engine->name, desc->latitude, desc->longitude);
                    locationChangeCount = 0;
                }
            } else {
                locationChangeCount = 0;
            }
            if (fabsf(desc->heading - heading) > 1.0f) {
                if (headingChangeCount++ > 3) {
                    desc->heading = heading;
                    RKLog("%s GPS update.   heading = %.2f degree\n", engine->name, desc->heading);
                    headingChangeCount = 0;
                }
            } else {
                headingChangeCount = 0;
            }
        } else {
            // Concatenate with latitude, longitude and heading values if GPS values are not reported
            sprintf(health->string + strlen(health->string) - 1, ", "
                    "\"GPS Valid\":{\"Value\":true,\"Enum\":0}, "
                    "\"GPS from File\":{\"Value\":true,\"Enum\":0}, "
                    "\"GPS Latitude\":{\"Value\":\"%.7f\",\"Enum\":0}, "
                    "\"GPS Longitude\":{\"Value\":\"%.7f\",\"Enum\":0}, "
                    "\"GPS Heading\":{\"Value\":\"%.2f\",\"Enum\":0}, "
                    "\"LocationFromDescriptor\":true}",
                    desc->latitude,
                    desc->longitude,
                    desc->heading);
        }
        
        RKProcessHealthKeywords(engine, health->string);

		engine->tic++;

		// Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->radarDescription->healthBufferDepth);
        if (k >= engine->radarDescription->healthBufferDepth) {
            RKLog("Error. k = %d\n", k);
            k = *engine->healthIndex;
            RKLog("Error. --> %d\n", k);
        }
    }

    if (engine->fid != NULL) {
        fclose(engine->fid);
        engine->fid = NULL;
    }
    
    return NULL;
}

#pragma mark - Life Cycle

RKHealthLogger *RKHealthLoggerInit() {
    RKHealthLogger *engine = (RKHealthLogger *)malloc(sizeof(RKHealthLogger));
    memset(engine, 0, sizeof(RKHealthLogger));
    sprintf(engine->name, "%s<HealthLogWriter>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHealthLogger) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->memoryUsage = sizeof(RKHealthLogger);
    engine->state = RKEngineStateAllocated;
    return engine;
}

void RKHealthLoggerFree(RKHealthLogger *engine) {
    if (engine->state & RKEngineStateActive) {
        RKHealthLoggerStop(engine);
    }
    free(engine);
}

#pragma mark - Properties

void RKHealthLoggerSetVerbose(RKHealthLogger *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKHealthLoggerSetInputOutputBuffers(RKHealthLogger *engine, RKRadarDesc *desc, RKFileManager *fileManager,
                                         RKHealth *healthBuffer, uint32_t *healthIndex) {
    engine->radarDescription  = desc;
    engine->fileManager       = fileManager;
    engine->healthBuffer      = healthBuffer;
    engine->healthIndex       = healthIndex;
    engine->state |= RKEngineStateProperlyWired;
}

void RKHealthLoggerSetDoNotWrite(RKHealthLogger *engine, const bool value) {
    engine->doNotWrite = value;
}

#pragma mark - Interactions

int RKHealthLoggerStart(RKHealthLogger *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    RKLog("%s Starting ...\n", engine->name);
	engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidBackground, NULL, healthLogger, engine)) {
        RKLog("Error. Unable to start health engine.\n");
        return RKResultFailedToStartHealthWorker;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKHealthLoggerStop(RKHealthLogger *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose > 1) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    if (!(engine->state & RKEngineStateActive)) {
        RKLog("%s Not active.\n", engine->name);
        return RKResultEngineDeactivatedMultipleTimes;
    }
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    pthread_join(engine->tidBackground, NULL);
    engine->state ^= RKEngineStateDeactivating;
    RKLog("%s Stopped.\n", engine->name);
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

char *RKHealthLoggerStatusString(RKHealthLogger *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, engine->radarDescription->healthBufferDepth)];
}
