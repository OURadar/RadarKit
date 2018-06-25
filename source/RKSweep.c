//
//  RKSweep.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/15/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKSweep.h>

#pragma mark - Helper Functions

static void *rayReleaser(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;

    int i, s;
    RKRay *ray;

    // Grab the anchor reference as soon as possible
    const uint8_t scratchSpaceIndex = engine->scratchSpaceIndex;

    // Notify the thread creator that I have grabbed the parameter
    engine->tic++;

    // Wait for a few seconds
    s = 0;
    do {
        usleep(100000);
    } while (++s < 50 && engine->state & RKEngineStateActive);

    if (engine->verbose > 1) {
        RKLog("%s rayReleaser()  scratchSpaceIndex = %d\n", engine->name, scratchSpaceIndex);
    }
    // Set them free
    for (i = 0; i < engine->scratchSpaces[scratchSpaceIndex].rayCount; i++) {
        ray = engine->scratchSpaces[scratchSpaceIndex].rays[i];
        ray->header.s = RKRayStatusVacant;
    }

    return NULL;
}

static void *sweepManager(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;

    int i, j, p, s;

    // Grab the anchor reference as soon as possible
    const uint8_t scratchSpaceIndex = engine->scratchSpaceIndex;

    // Notify the thread creator that I have grabbed the parameter
    engine->tic++;

    // Collect rays that belong to a sweep to a scratch space
    RKSweep *sweep = RKSweepCollect(engine, scratchSpaceIndex);
    if (sweep == NULL) {
        if (engine->verbose > 1) {
            RKLog("%s Empty sweep   scratchSpaceIndex = %d\n", scratchSpaceIndex);
        }
        return NULL;
    }
    if (engine->verbose) {
        RKRay *S = sweep->rays[0];
        RKRay *E = sweep->rays[sweep->header.rayCount - 1];
        RKLog("%s C%02d E%5.2f/%5.2f-%5.2f   A%6.2f-%6.2f   M%02x-%02x   (%s x %s%d%s, %.1f km)\n",
              engine->name,
              S->header.configIndex,
              sweep->header.config.sweepElevation,
              S->header.startElevation , E->header.endElevation,
              S->header.startAzimuth   , E->header.endAzimuth,
              S->header.marker & 0xFF, E->header.marker & 0xFF,
              RKIntegerToCommaStyleString(sweep->header.gateCount),
              rkGlobalParameters.showColor && sweep->header.rayCount != 360 ? RKGetColorOfIndex(1) : "",
              sweep->header.rayCount,
              rkGlobalParameters.showColor ? RKNoColor : "",
              1.0e-3f * S->header.gateCount * S->header.gateSizeMeters);
    }

    // Mark the rays being used by user algorithms
    for (j = 0; j < sweep->header.rayCount; j++) {
        sweep->rays[j]->header.s |= RKRayStatusBeingConsumed;
    }

    // Localize the scratch space storage
    char *filename = engine->scratchSpaces[scratchSpaceIndex].filename;
    char *filelist = engine->scratchSpaces[scratchSpaceIndex].filelist;
    char *summary = engine->scratchSpaces[scratchSpaceIndex].summary;

    int productCount = __builtin_popcount(sweep->header.baseMomentList & engine->baseMomentList);

    // Base products
    for (p = 0; p < productCount; p++) {
        if (engine->baseMomentProductIds[p]) {
            RKProduct *product = RKSweepEngineGetVacantProduct(engine, sweep, engine->baseMomentProductIds[p]);
            if (product == NULL) {
                RKLog("Error. Unable to get a product slot   p = %d   pid = %d.\n", p, engine->baseMomentProductIds[p]);
                continue;
            }
            RKProductInitFromSweep(product, sweep);
            RKSweepEngineSetProductComplete(engine, sweep, product);
        }
    }

    // Other clients may report products at the same time here, so we wait
    s = 0;
    bool allReported = true;
    for (i = 0; i < engine->radarDescription->productBufferDepth; i++) {
        if (engine->productBuffer[i].flag == RKProductStatusVacant) {
            continue;
        }
        pthread_mutex_lock(&engine->productMutex);
        engine->productBuffer[i].flag |= RKProductStatusSleep0;
        pthread_mutex_unlock(&engine->productMutex);
        while (engine->productBuffer[i].i != sweep->header.config.i &&
               engine->userProductTimeoutSeconds * 100 > s &&
               engine->state & RKEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 0/%.1f s\n", engine->name, (float)s * 0.01f);
            };
        }
        if (engine->verbose > 2) {
            RKLog("%s %s @ i = %zu ==? %zu\n", engine->name,
                  RKVariableInString("productId", &engine->productBuffer[i].pid, RKValueTypeProductId), engine->productBuffer[i].i, sweep->header.config.i);
        }
        pthread_mutex_lock(&engine->productMutex);
        engine->productBuffer[i].flag ^= RKProductStatusSleep0;
        pthread_mutex_unlock(&engine->productMutex);
        if (engine->productBuffer[i].i != sweep->header.config.i) {
            allReported = false;
        }
    }
    if (!(engine->state & RKEngineStateActive)) {
        return NULL;
    }

    j = 0;
    summary[0] = '\0';
    for (i = 0; i < engine->radarDescription->productBufferDepth; i++) {
        if (engine->productBuffer[i].flag == RKProductStatusVacant) {
            continue;
        }
        j += snprintf(summary + j, RKMaximumCommandLength - j - 1,
                      rkGlobalParameters.showColor ? " " RKLimeColor "%d" RKNoColor "/%1x" : " %d/%1x",
                      engine->productBuffer[i].pid, engine->productBuffer[i].flag & 0x07);
    }
    RKLog("%s Concluding sweep.   %s   %s\n", engine->name, RKVariableInString("allReported", &allReported, RKValueTypeBool), summary);

    // Mark the state
    engine->state |= RKEngineStateWritingFile;

    // Initiate a system command with handleFileScript
    if (engine->hasHandleFilesScript) {
        strncpy(filelist, engine->handleFilesScript, RKMaximumPathLength);
    }

    int summarySize = 0;

    // Product recording
    for (p = 0; p < engine->radarDescription->productBufferDepth; p++) {
        if (engine->productBuffer[p].flag == RKProductStatusVacant) {
            continue;
        }
        RKProduct *product = &engine->productBuffer[p];
        if (engine->verbose > 1) {
            RKLog(">%s Gathering product %s (%s) ...\n", engine->name, product->desc.name,  product->desc.symbol);
            RKLog(">%s %s   %s\n",
                  engine->name,
                  RKVariableInString("unit", product->desc.unit, RKValueTypeString),
                  RKVariableInString("colormap", product->desc.colormap, RKValueTypeString));
            RKLog(">%s %s   %s\n",
                  engine->name,
                  RKVariableInString("rayCount", &product->header.rayCount, RKValueTypeUInt32),
                  RKVariableInString("gateCount", &product->header.gateCount, RKValueTypeUInt32));
            RKShowArray(product->data, product->desc.symbol, product->header.gateCount, product->header.rayCount);
        }
        // Full filename with symbol and extension
        sprintf(product->header.suggestedFilename, "%s-%s.%s", sweep->header.filename, product->desc.symbol, engine->productFileExtension);
        strncpy(filename, product->header.suggestedFilename, RKMaximumPathLength);

        // Keep concatenating the filename into filelist
        if (engine->hasHandleFilesScript) {
            sprintf(filelist + strlen(filelist), " %s", filename);
        }

        // Call a product writer only if the engine is set to record and the is a valid product recorder
        if (engine->productRecorder && !engine->doNotWrite) {
            if (engine->verbose > 1) {
                RKLog("%s Creating %s ...\n", engine->name, filename);
            }
            RKPreparePath(filename);
            i = engine->productRecorder(product, filename);
            if (i != RKResultSuccess) {
                RKLog("%s Error creating %s\n", engine->name, filename);
            }
            // Notify file manager of a new addition
            RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeMoment);
        } else {
            if (engine->verbose > 1) {
                RKLog("%s Skipping %s ...\n", engine->name, filename);
            }
        }
        
        // Make a summary for logging
        if (p == 0) {
            // There are at least two '/'s in the filename: ...rootDataFolder/moment/YYYYMMDD/RK-YYYYMMDD-HHMMSS-Enn.n-Z.nc
            summarySize = sprintf(summary, rkGlobalParameters.showColor ? "%s ...%s-" RKYellowColor "%s" RKNoColor ".%s" : "%s ...%s-%s.%s",
                                  engine->doNotWrite ? "Skipped" : "Created", RKLastTwoPartsOfPath(sweep->header.filename), product->desc.symbol, engine->productFileExtension);
        } else {
            summarySize += sprintf(summary + summarySize, rkGlobalParameters.showColor ? ", " RKYellowColor "%s" RKNoColor : ", %s", product->desc.symbol);
        }
    }

    // Unmark the state
    engine->state ^= RKEngineStateWritingFile;

    // We are done with the sweep
    RKSweepFree(sweep);

    // Show a summary of all the files created
    if (engine->verbose && summarySize > 0) {
        RKLog("%s %s", engine->name, summary);
    }

    if (!engine->doNotWrite && engine->hasHandleFilesScript) {
        //printf("CMD: '%s'\n", filelist);
        j = system(filelist);
        if (j) {
            RKLog("Error. Failed using system() -> %d   errno = %d\n", j, errno);
        }
        // Potential filenames that may be generated by the custom command. Need to notify file manager about them.
        RKReplaceFileExtension(filename, strrchr(filename, '-'), ".__");
        if (engine->handleFilesScriptProducesTgz) {
            RKReplaceFileExtension(filename, ".__", ".tgz");
            RKLog("%s %s", engine->name, filename);
            if (RKFilenameExists(filename)) {
                RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeMoment);
            }
            RKReplaceFileExtension(filename, ".tgz", ".__");
        }
        if (engine->handleFilesScriptProducesZip) {
            RKReplaceFileExtension(filename, ".__", ".zip");
            RKLog("%s %s", engine->name, filename);
            if (RKFilenameExists(filename)) {
                RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeMoment);
            }
            RKReplaceFileExtension(filename, "zip", ".__");
        }
    }

    return NULL;
}

#pragma mark - Delegate Workers

static void *rayGatherer(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;
    
    int j, n, p, s;

    uint32_t is = 0;   // Start index
    uint64_t tic = 0;  // Local copy of engine tic

    pthread_t tidSweepManager = (pthread_t)0;
    pthread_t tidRayReleaser = (pthread_t)0;

    RKRay *ray = RKGetRay(engine->rayBuffer, 0);
    RKRay **rays = engine->scratchSpaces[engine->scratchSpaceIndex].rays;

    // Update the engine state
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;

    RKName name;
    RKName unit;
    RKName symbol;
    RKName colormap;
    RKFloat lhma[4];
    RKBaseMomentIndex momentIndex = 0;
    RKBaseMomentList momentList = engine->baseMomentList;
    int productCount = __builtin_popcount(momentList);
    for (p = 0; p < productCount; p++) {
        // Get the symbol, name, unit, colormap, etc. from the product list
        RKGetNextProductDescription(symbol, name, unit, colormap, &momentIndex, &momentList);
        // Build a product description
        RKProductDesc productDescription;
        strcpy(productDescription.name, name);
        strcpy(productDescription.unit, unit);
        strcpy(productDescription.symbol, symbol);
        strcpy(productDescription.colormap, colormap);
        // Special treatment for RhoHV
        if (momentIndex == RKBaseMomentIndexR) {
            productDescription.pieceCount = 3;
            productDescription.w[0] = 1000.0f;
            productDescription.b[0] = -824.0f;
            productDescription.l[0] = 0.93f;
            productDescription.w[1] = 300.0f;
            productDescription.b[1] = -173.0f;
            productDescription.l[1] = 0.7f;
            productDescription.w[2] = 52.8571f;
            productDescription.b[2] = 0.0f;
            productDescription.l[2] = 0.0f;
            productDescription.mininimumValue = 0.0f;
            productDescription.maximumValue = 1.05f;
        } else {
            switch (momentIndex) {
                case RKBaseMomentIndexZ:
                    RKZLHMAC
                    break;
                case RKBaseMomentIndexV:
                    RKV2LHMAC
                    break;
                case RKBaseMomentIndexW:
                    RKWLHMAC
                    break;
                case RKBaseMomentIndexD:
                    RKDLHMAC
                    break;
                case RKBaseMomentIndexP:
                    RKPLHMAC
                    break;
                case RKBaseMomentIndexK:
                    RKKLHMAC
                    break;
                default:
                    break;
            }
            productDescription.pieceCount = 1;
            productDescription.mininimumValue = lhma[0];
            productDescription.maximumValue = lhma[1];
            productDescription.w[0] = lhma[2];
            productDescription.b[0] = lhma[3];
            productDescription.l[0] = 0.0f;
        }
        engine->baseMomentProductIds[p] = RKSweepEngineRegisterProduct(engine, productDescription);
    }

    RKLog("%s Started.   mem = %s B   rayIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->rayIndex);
    RKLog(">%s Handle files using '%s'   expectTgz = %s\n", engine->name, engine->handleFilesScript, engine->handleFilesScriptProducesTgz ? "true" : "false");

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    j = 0;   // ray index
    while (engine->state & RKEngineStateActive) {
        // The ray
        ray = RKGetRay(engine->rayBuffer, j);
        
        // Wait until the buffer is advanced
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (j == *engine->rayIndex && engine->state & RKEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, j, *engine->rayIndex, ray->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // Wait until the ray is ready. This can never happen right? Because rayIndex only advances after the ray is ready
        s = 0;
        while (!(ray->header.s & RKRayStatusReady) && engine->state & RKEngineStateActive) {
            RKLog("%s I can happen.   j = %d   is = %d\n", engine->name, j, is);
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, j, *engine->rayIndex, ray->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateActive)) {
            break;
        }
        
        // Lag of the engine
        engine->lag = fmodf(((float)*engine->rayIndex + engine->radarDescription->rayBufferDepth - j) / engine->radarDescription->rayBufferDepth, 1.0f);

        // A sweep is complete
        if (ray->header.marker & RKMarkerSweepEnd) {
            // Gather the rays
            n = 0;
            do {
                ray = RKGetRay(engine->rayBuffer, is);
                ray->header.n = is;
                rays[n++] = ray;
                is = RKNextModuloS(is, engine->radarDescription->rayBufferDepth);
            } while (is != j && n < MIN(RKMaximumRaysPerSweep, engine->radarDescription->rayBufferDepth) - 1);
            ray = RKGetRay(engine->rayBuffer, is);
            ray->header.n = is;
            rays[n++] = ray;
            engine->scratchSpaces[engine->scratchSpaceIndex].rayCount = n;
            if (engine->verbose > 1) {
                RKLog("%s Info. RKMarkerSweepEnd   is = %d   j = %d   n = %d\n", engine->name, is, j, n);
            }

            // If the sweepManager is still going, wait for it to finish, launch a new one, wait for engine->rayAnchorsIndex is grabbed through engine->tic
            if (tidSweepManager) {
                pthread_join(tidSweepManager, NULL);
            }
            tic = engine->tic;
            if (pthread_create(&tidSweepManager, NULL, sweepManager, engine)) {
                RKLog("%s Error. Unable to launch a sweep writer.\n", engine->name);
            }
            do {
                usleep(50000);
            } while (tic == engine->tic && engine->state & RKEngineStateActive);

            // If the rayReleaser is still going, wait for it to finish, launch a new one, wait for engine->rayAnchorsIndex is grabbed through engine->tic
            if (tidRayReleaser) {
                pthread_join(tidRayReleaser, NULL);
            }
            tic = engine->tic;
            if (pthread_create(&tidRayReleaser, NULL, rayReleaser, engine)) {
                RKLog("%s Error. Unable to launch a ray releaser.\n", engine->name);
            }
            do {
                usleep(50000);
            } while (tic == engine->tic && engine->state & RKEngineStateActive);

            // Ready for next collection while the sweepManager is busy
            engine->scratchSpaceIndex = RKNextModuloS(engine->scratchSpaceIndex, RKSweepScratchSpaceDepth);
            if (engine->verbose > 1) {
                RKLog("%s Info. RKMarkerSweepEnd   scratchSpaceIndex -> %d.\n", engine->name, engine->scratchSpaceIndex);
            }
            rays = engine->scratchSpaces[engine->scratchSpaceIndex].rays;
            is = j;
        } else if (ray->header.marker & RKMarkerSweepBegin) {
            if (engine->verbose > 1) {
                RKLog("%s Info. RKMarkerSweepBegin   is = %d   j = %d\n", engine->name, is, j);
            }
            if (is != j) {
                // Gather the rays to release
                n = 0;
                do {
                    ray = RKGetRay(engine->rayBuffer, is);
                    ray->header.n = is;
                    rays[n++] = ray;
                    is = RKNextModuloS(is, engine->radarDescription->rayBufferDepth);
                } while (is != j && n < MIN(RKMaximumRaysPerSweep, engine->radarDescription->rayBufferDepth) - 1);
                engine->scratchSpaces[engine->scratchSpaceIndex].rayCount = n;

                // If the rayReleaser is still going, wait for it to finish, launch a new one, wait for engine->rayAnchorsIndex is grabbed through engine->tic
                if (tidRayReleaser) {
                    pthread_join(tidRayReleaser, NULL);
                }
                tic = engine->tic;
                if (pthread_create(&tidRayReleaser, NULL, rayReleaser, engine)) {
                    RKLog("%s Error. Unable to launch a ray releaser.\n", engine->name);
                }
                do {
                    usleep(50000);
                } while (tic == engine->tic && engine->state & RKEngineStateActive);

                // Ready for next collection while the sweepManager is busy
                engine->scratchSpaceIndex = RKNextModuloS(engine->scratchSpaceIndex, RKSweepScratchSpaceDepth);
                if (engine->verbose > 1) {
                    RKLog("%s RKMarkerSweepBegin   scratchSpaceIndex -> %d.\n", engine->name, engine->scratchSpaceIndex);
                }
                rays = engine->scratchSpaces[engine->scratchSpaceIndex].rays;
            }
            is = j;
        }

        // Update k to catch up for the next watch
        j = RKNextModuloS(j, engine->radarDescription->rayBufferDepth);
    }
    if (tidSweepManager) {
        pthread_join(tidSweepManager, NULL);
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
    sprintf(engine->name, "%s<ProductRecorder>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorSweepEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    sprintf(engine->productFileExtension, "nc");
    engine->state = RKEngineStateAllocated;
    engine->memoryUsage = sizeof(RKSweepEngine);
    engine->userProductTimeoutSeconds = 3;
    engine->baseMomentList = RKBaseMomentListProductZVWDPRK;
    engine->productRecorder = &RKProductRecorderNCWriter;
    pthread_mutex_init(&engine->productMutex, NULL);
    return engine;
}

void RKSweepEngineFree(RKSweepEngine *engine) {
    if (engine->state & RKEngineStateActive) {
        RKSweepEngineStop(engine);
    }
    pthread_mutex_destroy(&engine->productMutex);
    free(engine);
}

#pragma mark - Properties

void RKSweepEngineSetVerbose(RKSweepEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKSweepEngineSetInputOutputBuffer(RKSweepEngine *engine, RKRadarDesc *desc, RKFileManager *fileManager,
                                       RKConfig *configBuffer, uint32_t *configIndex,
                                       RKBuffer rayBuffer, uint32_t *rayIndex,
                                       RKProduct *productBuffer, uint32_t *productIndex) {
    engine->radarDescription  = desc;
    engine->fileManager       = fileManager;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->rayBuffer         = rayBuffer;
    engine->rayIndex          = rayIndex;
    engine->productBuffer     = productBuffer;
    engine->productIndex      = productIndex;
    engine->state |= RKEngineStateProperlyWired;
}

void RKSweepEngineSetDoNotWrite(RKSweepEngine *engine, const bool value) {
    engine->doNotWrite = value;
}

void RKSweepEngineSetHandleFilesScript(RKSweepEngine *engine, const char *script, const bool expectTgz) {
    if (RKFilenameExists(script)) {
        strcpy(engine->handleFilesScript, script);
        engine->hasHandleFilesScript = true;
        engine->handleFilesScriptProducesTgz = expectTgz;
    } else {
        RKLog("%s Error. File handler script does not exist.\n", engine->name);
    }
}

void RKSweepEngineSetProductRecorder(RKSweepEngine *engine, int (*routine)(RKProduct *, char *)) {
    engine->productRecorder = routine;
}

#pragma mark - Interactions

int RKSweepEngineStart(RKSweepEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidRayGatherer, NULL, rayGatherer, engine) != 0) {
        RKLog("Error. Failed to start a ray gatherer.\n");
        return RKResultFailedToStartRayGatherer;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKSweepEngineStop(RKSweepEngine *engine) {
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
    if (engine->tidRayGatherer) {
        pthread_join(engine->tidRayGatherer, NULL);
        engine->tidRayGatherer = (pthread_t)0;
    } else {
        RKLog("%s Invalid thread ID.\n", engine->name);
    }
    engine->state ^= RKEngineStateDeactivating;
    RKLog("%s Stopped.\n", engine->name);
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

RKProductId RKSweepEngineRegisterProduct(RKSweepEngine *engine, RKProductDesc desc) {
    int i = 0;
    RKProductId productId = 42;
    while (engine->productBuffer[i].pid != 0 && i < RKMaximumProductCount) {
        productId++;
        i++;
    }
    if (i == RKMaximumProductCount) {
        RKLog("%s Error. Unable to add anymore user products.\n", engine->name);
        return 0;
    }
    pthread_mutex_lock(&engine->productMutex);
    engine->productBuffer[i].i = 0;
    engine->productBuffer[i].pid = productId;
    engine->productBuffer[i].desc = desc;
    engine->productBuffer[i].flag = RKProductStatusActive;
    RKLog("%s Product %s%s%s registered   %s   %s\n", engine->name,
          rkGlobalParameters.showColor ? RKYellowColor : "",
          engine->productBuffer[i].desc.symbol,
          rkGlobalParameters.showColor ? RKNoColor : "",
          RKVariableInString("productId", &engine->productBuffer[i].pid, RKValueTypeProductId),
          RKVariableInString("name", engine->productBuffer[i].desc.name, RKValueTypeString));
    pthread_mutex_unlock(&engine->productMutex);
    return productId;
}

int RKSweepEngineUnregisterProduct(RKSweepEngine *engine, RKProductId productId) {
    int i = 0;
    while (i < RKMaximumProductCount) {
        if (engine->productBuffer[i].pid == productId) {
            break;
        }
        i++;
    }
    if (i == RKMaximumProductCount) {
        RKLog("%s Error. Unable to locate productId = 0x%04x.\n", engine->name, productId);
        return RKResultFailedToFindProductId;
    }
    if (engine->productBuffer[i].flag == RKProductStatusVacant) {
        RKLog("%s Warning. The productId = 0x%04x is vacant.", engine->name, productId);
    }
    pthread_mutex_lock(&engine->productMutex);
    engine->productBuffer[i].flag = RKProductStatusVacant;
    RKLog("%s Product %s%s%s unregistered   %s   %s\n", engine->name,
          rkGlobalParameters.showColor ? RKYellowColor : "",
          engine->productBuffer[i].desc.symbol,
          rkGlobalParameters.showColor ? RKNoColor : "",
          RKVariableInString("productId", &engine->productBuffer[i].pid, RKValueTypeProductId),
          RKVariableInString("name", engine->productBuffer[i].desc.name, RKValueTypeString));
    memset(&engine->productBuffer[i].desc, 0, sizeof(RKProductDesc));
    engine->productBuffer[i].pid = 0;
    pthread_mutex_unlock(&engine->productMutex);
    return RKResultSuccess;
}

RKProduct *RKSweepEngineGetVacantProduct(RKSweepEngine *engine, RKSweep *sweep, RKProductId productId) {
    int i = 0;
    while (i < RKMaximumProductCount) {
        if (engine->productBuffer[i].pid == productId) {
            break;
        }
        i++;
    }
    if (i == RKMaximumProductCount) {
        RKLog("%s Error. Unable to locate productId = %d.\n", engine->name, productId);
        return NULL;
    }
    pthread_mutex_lock(&engine->productMutex);
    engine->productBuffer[i].flag |= RKProductStatusSleep1;
    pthread_mutex_unlock(&engine->productMutex);
    return &engine->productBuffer[i];
}

int RKSweepEngineSetProductComplete(RKSweepEngine *engine, RKSweep *sweep, RKProduct *product) {
    product->i = sweep->header.config.i;
    if (product->flag & RKProductStatusSleep1) {
        pthread_mutex_lock(&engine->productMutex);
        product->flag ^= RKProductStatusSleep1;
        pthread_mutex_unlock(&engine->productMutex);
    } else {
        RKLog("%s That is weird, this buffer has not been requested.\n", engine->name);
    }
    return RKResultSuccess;
}

#pragma mark - Reader

void getGlobalTextAttribute(char *dst, const char *name, const int ncid) {
    size_t n = 0;
    nc_get_att_text(ncid, NC_GLOBAL, name, dst);
    nc_inq_attlen(ncid, NC_GLOBAL, name, &n);
    dst[n] = 0;
}

RKSweep *RKSweepCollect(RKSweepEngine *engine, const uint8_t scratchSpaceIndex) {
    MAKE_FUNCTION_NAME(name)
    int k;
    RKSweep *sweep = NULL;

    if (engine->verbose > 2) {
        RKLog("%s %s   anchorIndex = %u\n", engine->name, name, scratchSpaceIndex);
    }

    uint32_t n = engine->scratchSpaces[scratchSpaceIndex].rayCount;
    if (n < 2) {
        if (engine->verbose > 1) {
            RKLog("%s Empty sweep.   n = %d   anchorIndex = %u\n", engine->name, n, scratchSpaceIndex);
        }
        return NULL;
    }

    RKRay **rays = engine->scratchSpaces[scratchSpaceIndex].rays;
    RKRay *S = rays[0];
    RKRay *T = rays[1];
    RKRay *E = rays[n - 1];
    RKConfig *config = &engine->configBuffer[S->header.configIndex];
    RKBaseMomentList overallMomentList = 0;

    //RKLog(">%s %p %p %p ... %p\n", engine->name, rays[0], rays[1], rays[2], rays[n - 1]);

    // Consolidate some other information and check consistencies
    for (k = 0; k < n; k++) {
        overallMomentList |= rays[k]->header.baseMomentList;
        if (rays[k]->header.gateCount != S->header.gateCount) {
            RKLog("%s Warning. Inconsistent gateCount. ray[%s] has %s vs S has %s\n",
                  engine->name, RKIntegerToCommaStyleString(k), RKIntegerToCommaStyleString(rays[k]->header.gateCount),
                  RKIntegerToCommaStyleString(S->header.gateCount));
        }
        if (rays[k]->header.gateSizeMeters != S->header.gateSizeMeters) {
            RKLog("%s Warning. Inconsistent gateSizeMeters. ray[%s] has %s vs S has %s\n",
                  engine->name, RKIntegerToCommaStyleString(k), RKFloatToCommaStyleString(rays[k]->header.gateSizeMeters),
                  RKFloatToCommaStyleString(S->header.gateSizeMeters));
        }
    }

    if (engine->verbose > 1) {
        RKLog("%s C%02d-%02d-%02d   M%02x-%02x-%02x   products = 0x%x   (%s x %d, %.1f km)\n",
              engine->name,
              S->header.configIndex    , T->header.configIndex    , E->header.configIndex,
              S->header.marker & 0xFF  , T->header.marker & 0xFF  , E->header.marker & 0xFF,
              overallMomentList,
              RKIntegerToCommaStyleString(S->header.gateCount), n, 1.0e-3f * S->header.gateCount * S->header.gateSizeMeters);
    }

    k = 0;
    if (n > 360) {
        if (S->header.marker & RKMarkerSweepBegin) {
            // 361 beams and start at 0, we discard the extra last beam
            n = 360;
            k = 0;
        } else if (T->header.marker & RKMarkerSweepBegin) {
            // 361 beams but start at 1, we discard the extra first beam
            n = 360;
            k = 1;
        }
    }
    S = rays[k];
    T = rays[k + 1];
    E = rays[k + n - 1];

    // Allocate the return object
    sweep = (RKSweep *)malloc(sizeof(RKSweep));
    if (sweep == NULL) {
        RKLog("Error. Unable to allocate memory.\n");
        return NULL;
    }
    memset(sweep, 0, sizeof(RKSweep));

    // Populate the contents
    sweep->header.rayCount = n;
    sweep->header.gateCount = S->header.gateCount;
    sweep->header.gateSizeMeters = S->header.gateSizeMeters;
    sweep->header.startTime = (time_t)S->header.startTime.tv_sec;
    sweep->header.endTime = (time_t)E->header.endTime.tv_sec;
    sweep->header.baseMomentList = overallMomentList;
    sweep->header.isPPI = (S->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI;
    sweep->header.isRHI = (S->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI;
    sweep->header.external = true;
    memcpy(&sweep->header.desc, engine->radarDescription, sizeof(RKRadarDesc));
    memcpy(&sweep->header.config, config, sizeof(RKConfig));
    memcpy(sweep->rays, rays + k, n * sizeof(RKRay *));
    // Make a suggested filename as .../[DATA_PATH]/20170119/PX10k-20170119-012345-E1.0 (no symbol and extension)
    k = sprintf(sweep->header.filename, "%s%s%s/", engine->radarDescription->dataPath, engine->radarDescription->dataPath[0] == '\0' ? "" : "/", RKDataFolderMoment);
    k += strftime(sweep->header.filename + k, 10, "%Y%m%d", gmtime(&sweep->header.startTime));
    k += sprintf(sweep->header.filename + k, "/%s-", engine->radarDescription->filePrefix);
    k += strftime(sweep->header.filename + k, 16, "%Y%m%d-%H%M%S", gmtime(&sweep->header.startTime));
    if (sweep->header.isPPI) {
        k += sprintf(sweep->header.filename + k, "-E%.1f", sweep->header.config.sweepElevation);
    } else if (sweep->header.isRHI) {
        k += sprintf(sweep->header.filename + k, "-A%.1f", sweep->header.config.sweepAzimuth);
    } else {
        k += sprintf(sweep->header.filename + k, "-N%03d", sweep->header.rayCount);
    }
    return sweep;
}

RKSweep *RKSweepRead(const char *inputFile) {
    int j, k, r;
    int ncid, tmpId;
    float *fp, fv;
    int iv;

    MAKE_FUNCTION_NAME(name)

    RKName typeName;
    RKName scanType;
    char filename[RKMaximumPathLength];
    memset(filename, 0, RKMaximumPathLength);

    uint32_t firstPartLength = 0;

    uint32_t productList = 0;

    size_t rayCount = 0;
    size_t gateCount = 0;
    uint32_t capacity = 0;
    RKSweep *sweep = NULL;
    RKRay *ray = NULL;

    // A scratch space for netcdf API
    void *scratch = NULL;

    // Try to go through all the sublings to gather a productLiset
    // For now, we only try Z, V, W, D, P, R, K
    // Filename in conventions of RADAR-20180101-010203-EL10.2-Z.nc

    // Find the last '.'
    char *e = NULL;
    e = strstr(inputFile, ".");
    if (e == NULL) {
        e = (char *)inputFile + strlen(inputFile) - 1;
    }
    while (*(e + 1) >= '0' && *(e + 1) <= '9') {
        e = strstr(e + 1, ".");
    }
    // Find the previous '-'
    char *b = e;
    while (b != inputFile && *b != '-') {
        b--;
    }
    if (b == inputFile) {
        RKLog("%s Unable to find product symbol.\n", name);
        return NULL;
    }

    b++;
    firstPartLength = (uint32_t)(b - inputFile);
    char symbol[8];
    memset(symbol, 0, 8);
    strncpy(symbol, b, MIN(8, e - b));
    // Substitute symbol with the ones I know
    char symbols[][RKNameLength] = {"Z", "V", "W", "D", "P", "R", "K"};
    RKName productNames[] = {
        "Corrected_Intensity",
        "Radial_Velocity",
        "Width",
        "Differential_Reflectivity",
        "PhiDP",
        "RhoHV",
        "KDP"
    };
    uint32_t products[] = {
        RKBaseMomentListProductZ,
        RKBaseMomentListProductV,
        RKBaseMomentListProductW,
        RKBaseMomentListProductD,
        RKBaseMomentListProductP,
        RKBaseMomentListProductR,
        RKBaseMomentListProductK
    };
    uint32_t productIndices[] = {
        RKBaseMomentIndexZ,
        RKBaseMomentIndexV,
        RKBaseMomentIndexW,
        RKBaseMomentIndexD,
        RKBaseMomentIndexP,
        RKBaseMomentIndexR,
        RKBaseMomentIndexK
    };

    // First part: go through all the symbols I know of, get the very first filename
    for (k = 0; k < sizeof(symbols) / RKNameLength; k++) {
        b = symbols[k];
        strncpy(filename, inputFile, firstPartLength);
        snprintf(filename + firstPartLength, RKMaximumPathLength - firstPartLength, "%s%s", b, e);
        // Read in the header from the very first file that exists
        if (RKFilenameExists(filename)) {
            // Read the first file
            if ((r = nc_open(filename, NC_NOWRITE, &ncid)) > 0) {
                RKLog("%s Error opening file %s (%s)\n", name, inputFile, nc_strerror(r));
                return NULL;
            }
            // Dimensions
            if ((r = nc_inq_dimid(ncid, "Azimuth", &tmpId)) != NC_NOERR) {
                r = nc_inq_dimid(ncid, "azimuth", &tmpId);
            }
            if (r != NC_NOERR) {
                if ((r = nc_inq_dimid(ncid, "Beam", &tmpId)) != NC_NOERR) {
                    r = nc_inq_dimid(ncid, "beam", &tmpId);
                }
            }
            if (r == NC_NOERR) {
                nc_inq_dimlen(ncid, tmpId, &rayCount);
            } else {
                nc_close(ncid);
                RKLog("Warning. Early return (rayCount)\n");
                return NULL;
            }
            if ((r = nc_inq_dimid(ncid, "Gate", &tmpId)) != NC_NOERR)
                r = nc_inq_dimid(ncid, "gate", &tmpId);
            if (r == NC_NOERR) {
                nc_inq_dimlen(ncid, tmpId, &gateCount);
            } else {
                RKLog("Warning. Early return (gateCount)\n");
                nc_close(ncid);
                return NULL;
            }

            if (gateCount > RKGateCount) {
                RKLog("Info. gateCount = %d capped to %d\n", gateCount, RKGateCount);
                gateCount = RKGateCount;
            }

            // Derive the RKSIMDAlignSize compliant capacity
            capacity = (uint32_t)ceilf((float)gateCount / RKSIMDAlignSize) * RKSIMDAlignSize;

            RKLog("rayCount = %s   gateCount = %s   capacity = %s\n",
                  RKIntegerToCommaStyleString(rayCount), RKIntegerToCommaStyleString(gateCount), RKIntegerToCommaStyleString(capacity));

            // A scratch space for netcdf API
            scratch = (void *)malloc(rayCount * capacity * sizeof(float));

            // Allocate the return object
            sweep = (RKSweep *)malloc(sizeof(RKSweep));
            if (sweep == NULL) {
                RKLog("Error. Unable to allocate memory.\n");
                return NULL;
            }
            memset(sweep, 0, sizeof(RKSweep));
            RKRayBufferAlloc(&sweep->rayBuffer, (uint32_t)capacity, (uint32_t)rayCount);
            for (j = 0; j < rayCount; j++) {
                sweep->rays[j] = RKGetRay(sweep->rayBuffer, j);
            }
            ray = (RKRay *)sweep->rayBuffer;

            // Global attributes
            getGlobalTextAttribute(typeName, "TypeName", ncid);
            getGlobalTextAttribute(scanType, "ScanType", ncid);
            getGlobalTextAttribute(sweep->header.desc.name, "radarName-value", ncid);
            r = nc_get_att_double(ncid, NC_GLOBAL, "LatitudeDouble", &sweep->header.desc.latitude);
            if (r != NC_NOERR) {
                r = nc_get_att_float(ncid, NC_GLOBAL, "Latitude", &fv);
                if (r == NC_NOERR) {
                    sweep->header.desc.latitude = (double)fv;
                }
            }
            r = nc_get_att(ncid, NC_GLOBAL, "LongitudeDouble", &sweep->header.desc.longitude);
            if (r != NC_NOERR) {
                r = nc_get_att_float(ncid, NC_GLOBAL, "Longitude", &fv);
                if (r == NC_NOERR) {
                    sweep->header.desc.longitude = (double)fv;
                }
            }
            if (!strcmp(scanType, "PPI")) {
                sweep->header.config.sweepElevation = ray->header.sweepElevation;
                sweep->header.config.startMarker |= RKMarkerScanTypePPI;
            } else if (!strcmp(scanType, "RHI")) {
                sweep->header.config.sweepAzimuth = ray->header.sweepAzimuth;
                sweep->header.config.startMarker |= RKMarkerScanTypeRHI;
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Heading", &sweep->header.desc.heading);
            if (r != NC_NOERR) {
                RKLog("No radar heading found.\n");
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Height", &sweep->header.desc.radarHeight);
            if (r != NC_NOERR) {
                RKLog("No radar height found.\n");
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Elevation", &ray->header.sweepElevation);
            if (r != NC_NOERR && sweep->header.config.startMarker & RKMarkerScanTypePPI) {
                RKLog("Warning. No sweep elevation found.\n");
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Azimuth", &ray->header.sweepAzimuth);
            if (r != NC_NOERR && sweep->header.config.startMarker & RKMarkerScanTypeRHI) {
                RKLog("Warning. No sweep azimuth found.\n");
            }
            r = nc_get_att_int(ncid, NC_GLOBAL, "PRF-value", &iv);
            if (r == NC_NOERR) {
                sweep->header.config.prf[0] = iv;
                if (sweep->header.config.prf[0] == 0) {
                    RKLog("Warning. Recorded PRF = 0 Hz.\n");
                }
            } else {
                RKLog("Warning. No PRF information found.\n");
            }
            r = nc_get_att_float(ncid, NC_GLOBAL, "Nyquist_Vel-value", &fv);
            if (r == NC_NOERR) {
                sweep->header.desc.wavelength = 4.0f * fv / (RKFloat)sweep->header.config.prf[0];
                RKLog("Radar wavelength = %.4f m\n", sweep->header.desc.wavelength);
            }

            // Elevation array
            if ((r = nc_inq_varid(ncid, "Elevation", &tmpId)) != NC_NOERR) {
                r = nc_inq_varid(ncid, "elevation", &tmpId);
            }
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                fp = (float *)scratch;
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRay(sweep->rayBuffer, j);
                    ray->header.startElevation = *fp++;
                    ray->header.endElevation = ray->header.startElevation;
                }
            } else {
                RKLog("Warning. No elevation array.\n");
            }

            // Azimuth array
            if ((r = nc_inq_varid(ncid, "Azimuth", &tmpId)) != NC_NOERR) {
                r = nc_inq_varid(ncid, "azimuth", &tmpId);
            }
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                fp = (float *)scratch;
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRay(sweep->rayBuffer, j);
                    ray->header.startAzimuth = *fp++;
                }
            } else {
                RKLog("Warning. No azimuth array.\n");
            }

            // Gatewidth array (this is here for historical reasons)
            if ((r = nc_inq_varid(ncid, "GateWidth", &tmpId)) != NC_NOERR) {
                if ((r = nc_inq_varid(ncid, "Gatewidth", &tmpId)) != NC_NOERR) {
                    r = nc_inq_varid(ncid, "gatewidth", &tmpId);
                }
            }
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                fp = (float *)scratch;
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRay(sweep->rayBuffer, j);
                    ray->header.gateCount = gateCount;
                    ray->header.gateSizeMeters = *fp++;
                }
            } else {
                RKLog("Warning. No gatewidth array.\n");
            }

            // Beamwidth array (this is here for historical reasons)
            if ((r = nc_inq_varid(ncid, "BeamWidth", &tmpId)) != NC_NOERR) {
                if ((r = nc_inq_varid(ncid, "Beamwidth", &tmpId)) != NC_NOERR) {
                    r = nc_inq_varid(ncid, "beamwidth", &tmpId);
                }
            }
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                fp = (float *)scratch;
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRay(sweep->rayBuffer, j);
                    ray->header.endAzimuth = ray->header.startAzimuth + *fp;
                    ray->header.endElevation = ray->header.endElevation + *fp++;
                }
            } else {
                RKLog("Warning. No beamwidth array.\n");
            }

            // Be a good citizen, close the file
            nc_close(ncid);
            break;
        }
    }

    // If none of the files exist, sweep is NULL. There is no point continuing
    if (sweep == NULL) {
        RKLog("%s Inconsistent state.\n", name);
        return NULL;
    }

    // Second pass: go through all the symbols I know of and actually read in the data
    for (k = 0; k < sizeof(symbols) / RKNameLength; k++) {
        b = symbols[k];
        strncpy(filename, inputFile, firstPartLength);
        snprintf(filename + firstPartLength, RKMaximumPathLength - firstPartLength, "%s%s", b, e);
        if (RKFilenameExists(filename)) {
            productList |= products[k];

            // Open the file
            if ((r = nc_open(filename, NC_NOWRITE, &ncid)) > 0) {
                RKLog("%s Error opening file %s (%s)\n", name, inputFile, nc_strerror(r));
                free(scratch);
                return NULL;
            }

            RKLog("%s %s (*)\n", name, filename);

            // Product
            r = nc_inq_varid(ncid, productNames[k], &tmpId);
            if (r == NC_NOERR) {
                nc_get_var_float(ncid, tmpId, scratch);
                for (j = 0; j < rayCount; j++) {
                    ray = RKGetRay(sweep->rayBuffer, j);
                    fp = RKGetFloatDataFromRay(ray, productIndices[k]);
                    memcpy(fp, scratch + j * gateCount * sizeof(float), gateCount * sizeof(float));
                }
                nc_close(ncid);
            } else {
                RKLog("%s not found.\n", productNames[k]);
            }
        } else {
            RKLog("%s not found.\n", filename);
        }
    }

    // We are done with the scratch space at this point, we can free it
    free(scratch);

    // This really should not happen
    if (sweep == NULL) {
        RKLog("%s Inconsistent state towards the end.\n", name);
        return NULL;
    }

    ray = RKGetRay(sweep->rayBuffer, 0);

    sweep->header.rayCount = (uint32_t)rayCount;
    sweep->header.gateCount = (uint32_t)gateCount;
    sweep->header.gateSizeMeters = ray->header.gateSizeMeters;
    sweep->header.baseMomentList = productList;

    for (j = 0; j < rayCount; j++) {
        ray = RKGetRay(sweep->rayBuffer, j);
        ray->header.i += sweep->header.rayCount;
        ray->header.s = RKRayStatusReady;
        ray->header.baseMomentList = productList;
    }

    /*
    RKLog("  -> %s%s%s%s%s%s%s\n",
          productList & RKBaseMomentListProductZ ? "Z" : "",
          productList & RKBaseMomentListProductV ? "V" : "",
          productList & RKBaseMomentListProductW ? "W" : "",
          productList & RKBaseMomentListProductD ? "D" : "",
          productList & RKBaseMomentListProductP ? "P" : "",
          productList & RKBaseMomentListProductR ? "R" : "",
          productList & RKBaseMomentListProductK ? "K" : ""
          );
    */

    return sweep;
}

int RKSweepFree(RKSweep *sweep) {
    if (sweep == NULL) {
        RKLog("No es bueno, amigo!\n");
        return RKResultNullInput;
    }
    if (!sweep->header.external) {
        if (sweep->rayBuffer == NULL) {
            RKLog("Esto es malo!\n");
            return RKResultNullInput;
        }
        RKRayBufferFree(sweep->rayBuffer);
    }
    free(sweep);
    return RKResultSuccess;
}
