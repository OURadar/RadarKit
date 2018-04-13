//
//  RKHealth.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHealth.h>

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

static void *healthConsolidator(void *_in) {
    RKHealthEngine *engine = (RKHealthEngine *)_in;
    RKRadarDesc *desc = engine->radarDescription;
    
    int i, j, k, n, s;
	struct timeval t0, t1;

    RKHealth *health;

	bool allTrue;
	char *string;

    uint32_t *indices = (uint32_t *)malloc(desc->healthNodeCount * sizeof(uint32_t));
    memset(indices, 0xFF, desc->healthNodeCount * sizeof(uint32_t));
    
	// Update the engine state
	engine->state |= RKEngineStateActive;
	engine->state ^= RKEngineStateActivating;

    if (engine->verbose) {
        RKLog("%s Started.   mem = %s B   healthIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->healthIndex);
    }
    
	// Increase the tic once to indicate the engine is ready
	engine->tic = 1;

    gettimeofday(&t1, NULL);

    k = 0;   // health index
    while (engine->state & RKEngineStateActive) {
        // Evaluate the nodal-health buffers every once in a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) < 0.1) {
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
                    sprintf(string + i, "]");
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
                            n = RKPreviousModuloS(n, desc->healthBufferDepth);
                            RKHealth *h1 = &engine->healthNodes[j].healths[n];
                            // Copy over the previous health to current health, set all enums to old
                            h0->i += desc->healthBufferDepth;
                            strcpy(h0->string, h1->string);
                            RKReplaceKeyValue(h0->string, "Enum", RKStatusEnumOld);
                            h0->flag = RKHealthFlagReady;
                        }
                    }
                    if (engine->verbose > 1) {
                        n = indices[0];
                        i = sprintf(string, "ready = [%x", engine->healthNodes[0].healths[n].flag);
                        for (j = 1; j < desc->healthNodeCount; j++) {
                            n = indices[j];
                            i += sprintf(string + i,  " %s", engine->healthNodes[j].active ? (engine->healthNodes[j].healths[n].flag ? "1" : "0") : "-");
                        }
                        sprintf(string + i, "]");
                        RKLog("%s sleep 1/%.1f s   %s   k = %d\n", engine->name, (float)s * 0.1f, string, k);
                    }
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
            sprintf(string + i, "]");
            RKLog("%s %s   k = %d\n", engine->name, string, k);
            n = indices[0];
            i = sprintf(string, "flags   = [%02x", engine->healthNodes[0].healths[n].flag);
            for (j = 1; j < desc->healthNodeCount; j++) {
                n = indices[j];
                i += sprintf(string + i,  ", %02x", engine->healthNodes[j].healths[n].flag);
            }
            sprintf(string + i, "]");
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
        sprintf(string + i, "\"Log Time\":%zu}", t0.tv_sec);                                               // Add the log time as the last object
        health->flag = RKHealthFlagReady;

        if (engine->verbose > 2) {
            RKLog("%s", string);
        }

		engine->tic++;

        // Update index for the next watch
        k = RKNextModuloS(k, desc->healthBufferDepth);
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
    sprintf(engine->name, "%s<HealthCollector>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHealthEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
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

void RKHealthEngineSetInputOutputBuffers(RKHealthEngine *engine, const RKRadarDesc *desc,
                                         RKNodalHealth *healthNodes,
                                         RKHealth *healthBuffer, uint32_t *healthIndex) {
    engine->radarDescription  = (RKRadarDesc *)desc;
    engine->healthNodes       = healthNodes;
    engine->healthBuffer      = healthBuffer;
    engine->healthIndex       = healthIndex;
    engine->state |= RKEngineStateProperlyWired;
}

#pragma mark - Interactions

int RKHealthEngineStart(RKHealthEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidHealthConsolidator, NULL, healthConsolidator, engine)) {
        RKLog("Error. Unable to start health engine.\n");
        return RKResultFailedToStartHealthWorker;
    }
	while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKHealthEngineStop(RKHealthEngine *engine) {
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
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
	if (engine->tidHealthConsolidator) {
		pthread_join(engine->tidHealthConsolidator, NULL);
		engine->tidHealthConsolidator = (pthread_t)0;
	} else {
		RKLog("%s Invalid thread ID.\n", engine->name);
	}
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

char *RKHealthEngineStatusString(RKHealthEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, engine->radarDescription->healthBufferDepth)];
}
