//
//  RKHealthLogger.c
//  RadarKit
//
//  Created by Boonleng Cheong on 4/26/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKHealthLogger.h>

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

static void *healthLogger(void *in) {
    RKHealthLogger *engine = (RKHealthLogger *)in;
    RKRadarDesc *desc = engine->radarDescription;

    int i, k, s;
	struct timeval t0;

    RKHealth *health;
//    char *stringValue, *stringEnum, *stringObject;
//    int headingChangeCount = 0;
//    int locationChangeCount = 0;

    char filename[RKMaximumPathLength] = "";

	time_t unixTime;
	struct tm *timeStruct;
	int min = -1;

	// Update the engine state
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;

    RKLog("%s Started.   mem = %s B   healthIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->healthIndex);

	// Increase the tic once to indicate the engine is ready
	engine->tic = 1;

    engine->state |= RKEngineStateActive;

    gettimeofday(&t0, NULL);

	// Here comes the busy loop
	// i  anonymous
    k = 0;   // health index
    while (engine->state & RKEngineStateWantActive) {
        // Get the latest health
        health = &engine->healthBuffer[k];

        // Wait until the engine index advances
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->healthIndex && engine->state & RKEngineStateWantActive) {
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
        while (!(health->flag & RKHealthFlagReady) && engine->state & RKEngineStateWantActive) {
            usleep(100000);
            if (++s % 10 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   healthIndex = %d   flag = 0x%02x\n",
                      engine->name, (float)s * 0.1f, k , *engine->healthIndex, health->flag);
            }
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateWantActive)) {
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
            if (engine->record && engine->fid) {
                fclose(engine->fid);
                if (engine->verbose) {
                    RKLog("%s %sRecorded%s %s\n",
                          engine->name,
                          rkGlobalParameters.showColor ? RKGreenColor : "",
                          rkGlobalParameters.showColor ? RKNoColor : "",
                          filename);
                }
                // Notify file manager of a new addition
                if (engine->fileManager) {
                    RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeHealth);
                }
            } else {
                if (engine->verbose && strlen(filename)) {
                    RKLog("%s %sSkipped%s %s\n",
                        engine->name,
                        rkGlobalParameters.showColor ? RKLightOrangeColor : "",
                        rkGlobalParameters.showColor ? RKNoColor : "",
                        filename);
                }
            }
            i = sprintf(filename, "%s%s%s/", desc->dataPath, desc->dataPath[0] == '\0' ? "" : "/", RKDataFolderHealth);
            i += strftime(filename + i, 10, "%Y%m%d", gmtime(&unixTime));
            i += sprintf(filename + i, "/%s-", desc->filePrefix);
            strftime(filename + i, 22, "%Y%m%d-%H%M%S.json", gmtime(&unixTime));
            if (engine->record) {
                RKPreparePath(filename);
                engine->fid = fopen(filename, "w");
                if (engine->fid == NULL) {
                    RKLog("%s Error. Unable to create file for health log.\n", engine->name);
                }
            }
        }
        if (engine->fid) {
            fprintf(engine->fid, "%s\n", health->string);
        }

//        RKProcessHealthKeywords(engine, health->string);

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

    engine->state ^= RKEngineStateActive;
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
    if (engine->state & RKEngineStateWantActive) {
        RKHealthLoggerStop(engine);
    }
    free(engine);
}

#pragma mark - Properties

void RKHealthLoggerSetVerbose(RKHealthLogger *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKHealthLoggerSetEssentials(RKHealthLogger *engine, RKRadarDesc *desc, RKFileManager _Nullable *fileManager,
                                         RKHealth *healthBuffer, uint32_t *healthIndex) {
    engine->radarDescription  = desc;
    engine->fileManager       = fileManager;
    engine->healthBuffer      = healthBuffer;
    engine->healthIndex       = healthIndex;
    engine->state |= RKEngineStateProperlyWired;
}

void RKHealthLoggerSetRecord(RKHealthLogger *engine, const bool value) {
    engine->record = value;
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
    if (!(engine->state & RKEngineStateWantActive)) {
        RKLog("%s Not active.\n", engine->name);
        return RKResultEngineDeactivatedMultipleTimes;
    }
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateWantActive;
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
