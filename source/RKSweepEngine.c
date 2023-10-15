//
//  RKSweep.c
//  RadarKit
//
//  Created by Boonleng Cheong on 1/15/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKSweepEngine.h>

// Internal Functions

static void RKSweepEngineUpdateStatusString(RKSweepEngine *);

#pragma mark - Helper Functions

static void RKSweepEngineUpdateStatusString(RKSweepEngine *engine) {
    int i;
    char *string;

    // Status string
    string = engine->statusBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

    // Use RKStatusBarWidth characters to draw a bar
    i = *engine->rayIndex * RKStatusBarWidth / engine->radarDescription->rayBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'S';

    // Engine lag
    snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth, " %s%02.0f%s",
             rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
             99.49f * engine->lag,
             rkGlobalParameters.showColor ? RKNoColor : "");
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

static void *rayReleaser(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;

    int i, s;
    RKRay *ray;

    // Grab the anchor reference as soon as possible
    const uint8_t scratchSpaceIndex = engine->scratchSpaceIndex;

    // Notify the thread creator that I have grabbed the parameter
    engine->tic++;

    // Wait for a moment
    s = 0;
    do {
        usleep(10000);
    } while (++s < 10 && engine->state & RKEngineStateWantActive);

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

    int i, j, p;

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
        pthread_mutex_lock(&engine->productMutex);
        engine->business--;
        pthread_mutex_unlock(&engine->productMutex);
        return NULL;
    }
    if (engine->verbose) {
        RKRay *S = sweep->rays[0];
        RKRay *E = sweep->rays[sweep->header.rayCount - 1];
        RKLog("%s C%02d concluded   E%.2f/%.2f-%.2f   A%.2f-%.2f   M%02x-%02x   (%s x %s%d%s, %.1f km)\n",
              engine->name,
              S->header.configIndex,
              sweep->header.config.sweepElevation,
              S->header.startElevation , E->header.endElevation,
              S->header.startAzimuth   , E->header.endAzimuth,
              S->header.marker & 0xFF  , E->header.marker & 0xFF,
              RKIntegerToCommaStyleString(sweep->header.gateCount),
              rkGlobalParameters.showColor && sweep->header.rayCount != 360 ? RKGetColorOfIndex(1) : "",
              sweep->header.rayCount,
              rkGlobalParameters.showColor ? RKNoColor : "",
              1.0e-3f * S->header.gateCount * S->header.gateSizeMeters);
    }

    // Increase the sweep identifier
    pthread_mutex_lock(&engine->productMutex);
    sweep->header.i = engine->sweepIndex++;
    pthread_mutex_unlock(&engine->productMutex);

    // Mark the rays being used by user algorithms
    for (j = 0; j < sweep->header.rayCount; j++) {
        sweep->rays[j]->header.s |= RKRayStatusOverviewed;
    }

    // Localize the scratch space storage
    char *filename = engine->scratchSpaces[scratchSpaceIndex].filename;
    char *filelist = engine->scratchSpaces[scratchSpaceIndex].filelist;
    char *summary = engine->scratchSpaces[scratchSpaceIndex].summary;

    int productCount = __builtin_popcount(sweep->header.baseProductList & engine->baseProductList);

    // Base products
    size_t productMemoryUsage = 0;
    for (p = 0; p < productCount; p++) {
        if (engine->baseMomentProductIds[p]) {
            RKProduct *product = RKSweepEngineGetVacantProduct(engine, sweep, engine->baseMomentProductIds[p]);
            if (product == NULL) {
                RKLog("Error. Unable to get a product slot   p = %d   pid = %d.\n", p, engine->baseMomentProductIds[p]);
                continue;
            }
            RKProductInitFromSweep(product, sweep);
            RKSweepEngineSetProductComplete(engine, sweep, product);
            productMemoryUsage += product->totalBufferSize;
        }
    }
    engine->memoryUsage = sizeof(RKSweepEngine) + productMemoryUsage;

    if (engine->productBuffer == NULL) {
        RKLog("%s Unexpected NULL memory.\n", engine->name);
        pthread_mutex_lock(&engine->productMutex);
        engine->business--;
        pthread_mutex_unlock(&engine->productMutex);
        return NULL;
    }

    // Product registration now takes on different behavior, the following block is no longer relevant
    // Commented out for now until it is removed

    // Other clients may report products at the same time here, so we wait
    // int s = 0;
    // bool allReported = true;
    // for (i = 0; i < engine->productBufferDepth; i++) {
    //     RKLog("%s flag[%d] = %x\n", engine->name, i, engine->productBuffer[i].flag);
    //     if (engine->productBuffer[i].flag == RKProductStatusVacant) {
    //         continue;
    //     }
    //     pthread_mutex_lock(&engine->productMutex);
    //     engine->productBuffer[i].flag |= RKProductStatusSleep0;
    //     pthread_mutex_unlock(&engine->productMutex);
    //     while (engine->productBuffer[i].i != sweep->header.config.i &&
    //            engine->productTimeoutSeconds * 100 > s &&
    //            engine->state & RKEngineStateWantActive) {
    //         usleep(10000);
    //         if (++s % 100 == 0 && engine->verbose > 1) {
    //             RKLog("%s sleep 0/%.1f s\n", engine->name, (float)s * 0.01f);
    //         };
    //     }
    //     if (engine->verbose > 2) {
    //         RKLog("%s %s @ i = %zu ==? %zu\n", engine->name,
    //               RKVariableInString("productId", &engine->productBuffer[i].pid, RKValueTypeProductId), engine->productBuffer[i].i, sweep->header.config.i);
    //     }
    //     pthread_mutex_lock(&engine->productMutex);
    //     engine->productBuffer[i].flag ^= RKProductStatusSleep0;
    //     pthread_mutex_unlock(&engine->productMutex);
    //     if (engine->productBuffer[i].i != sweep->header.config.i) {
    //         allReported = false;
    //     }
    // }
    if (!(engine->state & RKEngineStateWantActive)) {
        pthread_mutex_lock(&engine->productMutex);
        engine->business--;
        pthread_mutex_unlock(&engine->productMutex);
    }

    // j = 0;
    // summary[0] = '\0';

    // for (i = 0; i < engine->productBufferDepth; i++) {
    //     if (engine->productBuffer[i].desc.baseProductList & sweep->header.baseProductList) {
    //         j += snprintf(summary + j, RKMaximumCommandLength - j,
    //                       rkGlobalParameters.showColor ? " " RKLimeColor "%d" RKNoColor "/%1x" : " %d/%1x",
    //                       engine->productBuffer[i].pid, engine->productBuffer[i].flag & 0x07);
    //     }
    // }
    // RKLog("%s Concluding sweep.   %s   %s\n", engine->name, RKVariableInString("allReported", &allReported, RKValueTypeBool), summary);

    // Mark the state
    engine->state |= RKEngineStateWritingFile;

    // Initiate a system command with handleFileScript
    if (engine->hasFileHandlingScript) {
        strncpy(filelist, engine->fileHandlingScript, RKMaximumPathLength);
    }

    int summarySize = 0;
    bool filenameTooLong;

    // Product recording
    for (p = 0; p < engine->productBufferDepth; p++) {
        if (!(engine->productBuffer[p].desc.baseProductList & sweep->header.baseProductList) || engine->productBuffer[p].flag == RKProductStatusVacant) {
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
        //strncpy(filename, product->header.suggestedFilename, RKMaximumPathLength - 80);
        snprintf(filename, RKMaximumPathLength - 80, "%s", product->header.suggestedFilename) < 0 ? abort() : (void)0;
        filenameTooLong = strlen(sweep->header.filename) > 48;

        // Keep concatenating the filename into filelist
        if (engine->hasFileHandlingScript) {
            sprintf(filelist + strlen(filelist), " %s", filename);
        }

        // Convert data in radians to degrees if necessary
         if (engine->convertToDegrees && !strcasecmp(product->desc.unit, "radians")) {
             const RKFloat radiansToDegrees = 180.0f / M_PI;
             RKLog("%s Converting '%s' to degrees ...", engine->name, product->desc.name);
             RKFloat *x =  product->data;
             for (j = 0; j < product->header.gateCount * product->header.rayCount; j++) {
                 *x = *x * radiansToDegrees;
                 x++;
             }
             sprintf(product->desc.unit, "Degrees");
         }

        // Call a product writer only if the engine is set to record and the is a valid product recorder
        if (!(engine->productBuffer[p].desc.baseProductList & sweep->header.baseProductList)) {
            if (engine->verbose > 1){
                RKLog("%s Skipping %s ...\n", engine->name, filename);
            }
        } else if (engine->record && engine->productRecorder) {
            if (engine->verbose > 1) {
                RKLog("%s Creating %s ...\n", engine->name, filename);
            }
            RKPreparePath(filename);
            i = engine->productRecorder(product, filename);
            if (i != RKResultSuccess) {
                RKLog("%s Error creating %s\n", engine->name, filename);
            }
            // Notify file manager of a new addition if the file handling script does not remove them
            if (engine->fileManager && !(engine->fileHandlingScriptProperties & RKScriptPropertyRemoveNCFiles)) {
                RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeMoment);
            }
        } else if (engine->verbose > 1) {
            RKLog("%s Skipping %s ...\n", engine->name, filename);
        }

        // Make a summary for logging
        if (p == 0) {
            summarySize = sprintf(summary, "%s%s%s %s%s-%s%s%s.%s",
                                  rkGlobalParameters.showColor ? (engine->record ? RKGreenColor : RKOrangeColor) : "",
                                  engine->record ? "Recorded": "Skipped",
                                  rkGlobalParameters.showColor ? RKNoColor : "",
                                  filenameTooLong ? "..." : "",
                                  filenameTooLong ? RKLastTwoPartsOfPath(sweep->header.filename) : sweep->header.filename,
                                  rkGlobalParameters.showColor ? RKYellowColor : "",
                                  product->desc.symbol,
                                  rkGlobalParameters.showColor ? RKNoColor : "",
                                engine->productFileExtension);
        } else {
            summarySize += sprintf(summary + summarySize, rkGlobalParameters.showColor ? ", " RKYellowColor "%s" RKNoColor : ", %s", product->desc.symbol);
        }
    }

    // Unmark the state
    engine->state ^= RKEngineStateWritingFile;

    // We are done with the sweep
    RKSweepFree(sweep);

    // Show a summary of all the files created
    if (summarySize) {
        RKLog("%s %s\n", engine->name, summary);
    }

    if (engine->record && engine->hasFileHandlingScript) {
        j = system(filelist);
        if (j) {
            RKLog("Error. CMD: %s", filelist);
            RKLog("Error. Failed using system() -> %d   errno = %d\n", j, errno);
        }
        // Potential filenames that may be generated by the custom command. Need to notify file manager about them.
        RKReplaceFileExtension(filename, strrchr(filename, '-'), ".__");
        if (engine->fileHandlingScriptProperties & RKScriptPropertyProduceArchive) {
            if (engine->fileHandlingScriptProperties & RKScriptPropertyProduceTarXz) {
                RKReplaceFileExtension(filename, "__", "tar.xz");
            } else if (engine->fileHandlingScriptProperties & RKScriptPropertyProduceTgz) {
                RKReplaceFileExtension(filename, "__", "tgz");
            } else if (engine->fileHandlingScriptProperties & RKScriptPropertyProduceTxz) {
                RKReplaceFileExtension(filename, "__", "txz");
            } else {
                RKReplaceFileExtension(filename, "__", "zip");
            }
            RKLog("%s %s", engine->name, filename);
            if (engine->fileManager && RKFilenameExists(filename)) {
                RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeMoment);
            }
        }
    }

    pthread_mutex_lock(&engine->productMutex);
    engine->business--;
    pthread_mutex_unlock(&engine->productMutex);

    return NULL;
}

#pragma mark - Delegate Workers

static void *rayGatherer(void *in) {
    RKSweepEngine *engine = (RKSweepEngine *)in;

    int j, n, p, s;

    struct timeval t0, t1;

    uint32_t is = 0;   // Start index
    uint64_t tic = 0;  // Local copy of engine tic

    pthread_t tidSweepManager = (pthread_t)0;
    pthread_t tidRayReleaser = (pthread_t)0;

    RKRay *ray;
    RKRay **rays = engine->scratchSpaces[engine->scratchSpaceIndex].rays;

    // Update the engine state
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;

    RKBaseProductList momentList = engine->baseProductList;
    const int productCount = __builtin_popcount(momentList);
    for (p = 0; p < productCount; p++) {
        RKProductDesc productDescription = RKGetNextProductDescription(&momentList);
        engine->baseMomentProductIds[p] = RKSweepEngineDescribeProduct(engine, productDescription);
    }

    if (engine->hasFileHandlingScript) {
        RKLog(">%s Handle files using '%s%s%s'%s%s\n", engine->name,
              rkGlobalParameters.showColor ? "\033[3;4m" : "",
              engine->fileHandlingScript,
              rkGlobalParameters.showColor ? "\033[23;24m" : "",
              engine->fileHandlingScriptProperties & RKScriptPropertyProduceArchive ? " --> " : "",
              engine->fileHandlingScriptProperties & RKScriptPropertyProduceArchive ?
              (engine->fileHandlingScriptProperties & RKScriptPropertyProduceTarXz ? ".tar.xz" :
               (engine->fileHandlingScriptProperties & RKScriptPropertyProduceTxz ? ".txz" :
                (engine->fileHandlingScriptProperties & RKScriptPropertyProduceTgz ? ".tgz" : ".zip"))) : "");
    }
    RKLog("%s Started.   mem = %s B   productCount = %d   rayIndex = %d\n",
        engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), productCount, *engine->rayIndex);

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    // Update the engine state
    engine->state |= RKEngineStateActive;

    gettimeofday(&t1, NULL); t1.tv_sec -= 1;

    j = 0;   // ray index
    while (engine->state & RKEngineStateWantActive) {
        // The ray
        ray = RKGetRayFromBuffer(engine->rayBuffer, j);

        // Wait until the buffer is advanced
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (j == *engine->rayIndex && engine->state & RKEngineStateWantActive) {
            if (engine->state & RKEngineStateReserved) {
                engine->state ^= RKEngineStateReserved;
                if (engine->verbose) {
                    RKLog("%s Flushing ...\n", engine->name);
                }
                j = RKPreviousModuloS(j, engine->radarDescription->rayBufferDepth);
                ray = RKGetRayFromBuffer(engine->rayBuffer, j);
                ray->header.marker |= RKMarkerSweepEnd;
                break;
            }
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
        while (!(ray->header.s & RKRayStatusReady) && engine->state & RKEngineStateWantActive) {
            //RKLog("%s I can happen.   j = %d   is = %d\n", engine->name, j, is);
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   rayIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, j, *engine->rayIndex, ray->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateWantActive)) {
            break;
        }

        // Lag of the engine
        engine->lag = fmodf(((float)*engine->rayIndex + engine->radarDescription->rayBufferDepth - j) / engine->radarDescription->rayBufferDepth, 1.0f);

        // A sweep is complete
        if (ray->header.marker & RKMarkerSweepEnd) {
            // Gather the rays
            n = 0;
            do {
                ray = RKGetRayFromBuffer(engine->rayBuffer, is);
                ray->header.n = is;
                rays[n++] = ray;
                is = RKNextModuloS(is, engine->radarDescription->rayBufferDepth);
            } while (is != j && n < MIN(RKMaximumRaysPerSweep, engine->radarDescription->rayBufferDepth) - 1);
            ray = RKGetRayFromBuffer(engine->rayBuffer, is);
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
            } while (tic == engine->tic && engine->state & RKEngineStateWantActive);

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
            } while (tic == engine->tic && engine->state & RKEngineStateWantActive);

            // Ready for next collection while the sweepManager is busy
            engine->scratchSpaceIndex = RKNextModuloS(engine->scratchSpaceIndex, RKSweepScratchSpaceDepth);
            if (engine->verbose > 1) {
                RKLog("%s Info. RKMarkerSweepEnd   scratchSpaceIndex -> %d.\n", engine->name, engine->scratchSpaceIndex);
            }
            rays = engine->scratchSpaces[engine->scratchSpaceIndex].rays;
            is = j;
        } else if (ray->header.marker & RKMarkerSweepBegin) {
            pthread_mutex_lock(&engine->productMutex);
            engine->business++;
            pthread_mutex_unlock(&engine->productMutex);
            if (engine->verbose > 1) {
                RKLog("%s Info. RKMarkerSweepBegin   is = %d   j = %d\n", engine->name, is, j);
            }
            if (is != j) {
                // Gather the rays to release
                n = 0;
                do {
                    ray = RKGetRayFromBuffer(engine->rayBuffer, is);
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
                } while (tic == engine->tic && engine->state & RKEngineStateWantActive);

                // Ready for next collection while the sweepManager is busy
                engine->scratchSpaceIndex = RKNextModuloS(engine->scratchSpaceIndex, RKSweepScratchSpaceDepth);
                if (engine->verbose > 1) {
                    RKLog("%s RKMarkerSweepBegin   scratchSpaceIndex -> %d.\n", engine->name, engine->scratchSpaceIndex);
                }
                rays = engine->scratchSpaces[engine->scratchSpaceIndex].rays;
            }
            is = j;
        }

        // Record down the ray that is just processed
        engine->processedRayIndex = j;

        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            RKSweepEngineUpdateStatusString(engine);
        }

        engine->tic++;

        // Update k to catch up for the next watch
        j = RKNextModuloS(j, engine->radarDescription->rayBufferDepth);
    }
    if (tidSweepManager) {
        pthread_join(tidSweepManager, NULL);
        tidSweepManager = (pthread_t)0;
    }
    for (p = 0; p < productCount; p++) {
        RKSweepEngineUndescribeProduct(engine, engine->baseMomentProductIds[p]);
    }
    engine->state ^= RKEngineStateActive;
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
    snprintf(engine->productFileExtension, RKMaximumFileExtensionLength, "nc");
    engine->state = RKEngineStateAllocated;
    engine->productTimeoutSeconds = 5;
    engine->baseProductList = RKBaseProductListFloatZVWDPRKLRXPX;
    engine->productRecorder = &RKProductFileWriterNC;
    engine->productBufferDepth = 20;
    size_t bytes = RKProductBufferAlloc(&engine->productBuffer, engine->productBufferDepth, RKMaximumRaysPerSweep, 2000);
    if (engine->productBuffer == NULL) {
        RKLog("Error. Unable to allocate a product buffer for sweep engine.\n");
        return NULL;
    }
    pthread_mutex_init(&engine->productMutex, NULL);
    engine->memoryUsage = sizeof(RKSweepEngine) + bytes;
    return engine;
}

void RKSweepEngineFree(RKSweepEngine *engine) {
    if (engine->state & RKEngineStateWantActive) {
        RKSweepEngineStop(engine);
    }
    pthread_mutex_destroy(&engine->productMutex);
    RKProductBufferFree(engine->productBuffer, engine->productBufferDepth);
    free(engine);
}

#pragma mark - Properties

void RKSweepEngineSetVerbose(RKSweepEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKSweepEngineSetEssentials(RKSweepEngine *engine, RKRadarDesc *desc, RKFileManager _Nullable *fileManager,
                                RKConfig *configBuffer, uint32_t *configIndex,
                                RKBuffer rayBuffer, uint32_t *rayIndex) {
    engine->radarDescription  = desc;
    engine->fileManager       = fileManager;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->rayBuffer         = rayBuffer;
    engine->rayIndex          = rayIndex;
    engine->state |= RKEngineStateProperlyWired;
}

void RKSweepEngineSetRecord(RKSweepEngine *engine, const bool value) {
    engine->record = value;
}

void RKSweepEngineSetFilesHandlingScript(RKSweepEngine *engine, const char *script, const RKScriptProperty flag) {
    if (RKFilenameExists(script)) {
        engine->hasFileHandlingScript = true;
        engine->fileHandlingScriptProperties = flag;
        strcpy(engine->fileHandlingScript, script);
    } else {
        RKLog("%s Error. File handler script does not exist.\n", engine->name);
    }
}

void RKSweepEngineSetProductRecorder(RKSweepEngine *engine, int (*routine)(RKProduct *, const char *)) {
    engine->productRecorder = routine;
}

void RKSweepEngineFlush(RKSweepEngine *engine) {
    int k;
    uint32_t waitIndex = *engine->rayIndex;

    // Make sure rayIndex no longer changes
    k = 0;
    do {
        usleep(1000);
    } while (k++ < 50 && *engine->rayIndex == waitIndex);
    if (engine->verbose > 1) {
        RKLog("%s Engine->state = %s   tic = %zu\n", engine->name,
            engine->state & RKEngineStateSleep1 ? "sleep 1" : (
            engine->state & RKEngineStateSleep2 ? "sleep 2" : "-"),
            engine->tic);
    }
    // By now we should be in sleep 1. Otherwise, we wait until it gets there
    if (!(engine->state & RKEngineStateSleep1)) {
        if (engine->verbose > 1) {
            RKLog(">%s Wait until sleep 1   rayIndex = %u / %u   tic = %zu\n", engine->name, *engine->rayIndex, waitIndex, engine->tic);
        }
        k = 0;
        do {
            usleep(10000);
            if (waitIndex != *engine->rayIndex) {
                waitIndex = *engine->rayIndex;
                k = 0;
                RKLog(">%s wait reset\n", engine->name);
            }
        } while (k++ < 100 && !(engine->state & RKEngineStateSleep1));
    }
    if (engine->verbose > 1) {
        RKLog("%s Engine->state = %s   rayIndex = %u / %u   tic = %zu", engine->name,
            engine->state & RKEngineStateSleep1 ? "sleep 1" : (
            engine->state & RKEngineStateSleep2 ? "sleep 2" : "-"),
            *engine->rayIndex, waitIndex,
            engine->tic);
    }
    // sweepManager and rayReleaser each increments by 1
    uint64_t tic = engine->tic + 2;
    engine->state |= RKEngineStateReserved;
    k = 0;
    do {
        usleep(10000);
    } while (k++ < 200 && (engine->tic < tic || engine->state & RKEngineStateWritingFile));
    if (engine->verbose > 1) {
        RKLog("%s Flushed.   tic = %zu / %zu   k = %d\n", engine->name, engine->tic, tic, k);
    }
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
    if (!(engine->state & RKEngineStateWantActive)) {
        RKLog("%s Not active.\n", engine->name);
        return RKResultEngineDeactivatedMultipleTimes;
    }
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateWantActive;
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

char *RKSweepEngineStatusString(RKSweepEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}

char *RKSweepEngineLatestSummary(RKSweepEngine *engine) {
    return engine->scratchSpaces[engine->scratchSpaceIndex].summary;
}

RKProductId RKSweepEngineDescribeProduct(RKSweepEngine *engine, RKProductDesc desc) {
    int i = 0;
    RKProductId productId = 42;
    while (engine->productBuffer[i].flag & RKProductStatusActive && i < RKMaximumProductCount) {
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
    RKLog(">%s Product %s%s%s described   %s   %s\n", engine->name,
          rkGlobalParameters.showColor ? RKYellowColor : "",
          engine->productBuffer[i].desc.symbol,
          rkGlobalParameters.showColor ? RKNoColor : "",
          RKVariableInString("productId", &engine->productBuffer[i].pid, RKValueTypeProductId),
          RKVariableInString("name", engine->productBuffer[i].desc.name, RKValueTypeString));
    pthread_mutex_unlock(&engine->productMutex);
    return productId;
}

int RKSweepEngineUndescribeProduct(RKSweepEngine *engine, RKProductId productId) {
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
    RKLog(">%s Product %s%s%s undescribed\n", engine->name,
          rkGlobalParameters.showColor ? RKYellowColor : "",
          engine->productBuffer[i].desc.symbol,
          rkGlobalParameters.showColor ? RKNoColor : "");
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
    // Store a copy for the record
    engine->radarDescription->productBufferDepth = engine->productBufferDepth;
    engine->radarDescription->productBufferSize = engine->productBufferDepth * product->totalBufferSize;
    return RKResultSuccess;
}

void RKSweepEngineWaitWhileBusy(RKSweepEngine *engine) {
    int s = 0;
    do {
        usleep(1000);
    } while (engine->business > 0 && s++ < 2000);
    if (engine->business) {
        RKLog("%s Warning. Waited for %.2f s but still busy.\n", engine->name, s * 0.001f);
    }
}

#pragma mark - RKSweep

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
    RKConfig *config = &engine->configBuffer[T->header.configIndex];
    RKMomentList overallMomentList = 0;
    RKBaseProductList overallBaseProductList = 0;

    if (engine->verbose > 2) {
        RKLog(">%s n = %d   %p %p %p ... %p\n", engine->name, n, rays[0], rays[1], rays[2], rays[n - 1]);
    }

    // Consolidate some other information and check consistencies
    uint8_t gateCountWarningCount = 0;
    uint8_t gateSizeWarningCount = 0;
    for (k = 0; k < n; k++) {
        overallMomentList |= rays[k]->header.baseMomentList;
        overallBaseProductList |= rays[k]->header.baseProductList;
        if (rays[k]->header.gateCount != S->header.gateCount) {
            if (++gateCountWarningCount < 5) {
                RKLog("%s Warning. Inconsistent gateCount. ray[%s] has %s vs S has %s\n",
                      engine->name, RKIntegerToCommaStyleString(k), RKIntegerToCommaStyleString(rays[k]->header.gateCount),
                      RKIntegerToCommaStyleString(S->header.gateCount));
            } else if (gateCountWarningCount == 5) {
                RKLog("%s Warning. Inconsistent gateCount more than 5 rays / sweep.\n", engine->name);
            }
        }
        if (rays[k]->header.gateSizeMeters != S->header.gateSizeMeters) {
            if (++gateSizeWarningCount < 5) {
                RKLog("%s Warning. Inconsistent gateSizeMeters. ray[%s] has %s vs S has %s\n",
                      engine->name, RKIntegerToCommaStyleString(k), RKFloatToCommaStyleString(rays[k]->header.gateSizeMeters),
                      RKFloatToCommaStyleString(S->header.gateSizeMeters));
            } else if (gateSizeWarningCount == 5) {
                RKLog("%s Warning. Inconsistent gateSize more than 5 rays / sweep.\n", engine->name);
            }
        }
    }

    if (engine->verbose > 1) {
        RKLog("%s C%02d-%02d-%02d   M%02x-%02x-%02x   moments = 0x%x   products = 0x%x   (%s x %d, %.1f km)\n",
              engine->name,
              S->header.configIndex    , T->header.configIndex    , E->header.configIndex,
              S->header.marker & 0xFF  , T->header.marker & 0xFF  , E->header.marker & 0xFF,
              overallMomentList, overallBaseProductList,
              RKIntegerToCommaStyleString(S->header.gateCount), n, 1.0e-3f * S->header.gateCount * S->header.gateSizeMeters);
    }

    k = 0;
    if (n > 360 && n < 380) {
        //RKLog("%s n = %d  %d  %d\n", engine->name, n, S->header.marker & RKMarkerSweepBegin, T->header.marker & RKMarkerSweepBegin);
        if (T->header.marker & RKMarkerSweepBegin) {
            // 361 beams but start at 1, we discard the extra first beam
            n = 360;
            k = 1;
        } else if (n > 361) {
            // From park to a first sweep, we may end up with more than 361 rays
            if (E->header.marker & RKMarkerSweepEnd) {
                // The end sweep is also a start of the next sweep. In this case, we have >361 rays but only need 360
                k = n - 361;
                n = 360;
                if (engine->verbose > 1) {
                    RKLog("%s SweepCollect() n = %d --> %d  k = %d\n", engine->name, k + 361, n, k);
                }
            } else {
                // Otherwise, we have exactly 361 rays. Again, only need to latest 360 rays
                k = n - 360;
                n = 360;
                if (engine->verbose > 1) {
                    RKLog("%s SweepCollect() n = %d --> %d  k = %d\n", engine->name, k + 360, n, k);
                }
            }
        } else if (S->header.marker & RKMarkerSweepBegin) {
            // First beam is both start and end
            n = 360;
        }
    }
    S = rays[k];
    T = rays[k + 1];
    E = rays[k + n - 1];
    if (S->header.configIndex != T->header.configIndex) {
        RKLog("%s Info. Inconsistent configIndex S -> %d vs T -> %d\n", engine->name, S->header.configIndex, T->header.configIndex);
    }

    // Allocate the return object
    sweep = (RKSweep *)malloc(sizeof(RKSweep));
    if (sweep == NULL) {
        RKLog("%s Error. Unable to allocate memory.\n", engine->name);
        return NULL;
    }
    memset(sweep, 0, sizeof(RKSweep));

    // Populate the contents
    sweep->header.rayCount = n;
    sweep->header.gateCount = S->header.gateCount;
    sweep->header.gateSizeMeters = S->header.gateSizeMeters;
    sweep->header.startTime = (time_t)S->header.startTime.tv_sec;
    sweep->header.endTime = (time_t)E->header.endTime.tv_sec;
    sweep->header.baseProductList = overallBaseProductList;
    sweep->header.isPPI = (config->startMarker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI;
    sweep->header.isRHI = (config->startMarker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI;
    if (!sweep->header.isPPI && !sweep->header.isRHI) {
        config = &engine->configBuffer[T->header.configIndex];
        sweep->header.isPPI = (config->startMarker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI;
        sweep->header.isRHI = (config->startMarker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI;
        RKLog("%s Using %s   %s\n", engine->name,
              RKVariableInString("configId", &config->i, RKValueTypeIdentifier),
              RKMarkerScanTypeString(config->startMarker));
    }
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
    if (k > RKMaximumFolderPathLength + RKMaximumPrefixLength + 25 + RKMaximumFileExtensionLength) {
        RKLog("%s Error. Suggested filename %s is longer than expected.\n", engine->name, sweep->header.filename);
    }
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
