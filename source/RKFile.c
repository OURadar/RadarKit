//
//  RKFile.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKFile.h>

// Internal Functions

static void RKFileEngineUpdateStatusString(RKFileEngine *);
static void *pulseRecorder(void *);

#pragma mark - Helper Functions

static void RKFileEngineUpdateStatusString(RKFileEngine *engine) {
    int i;
    char *string;
    
    // Status string
    string = engine->statusBuffer[engine->statusBufferIndex];
    
    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';
    
    // Use RKStatusBarWidth characters to draw a bar
    i = *engine->pulseIndex * RKStatusBarWidth / engine->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'F';
    
    // Engine lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKMaximumStringLength - RKStatusBarWidth, " | %s%02.0f%s",
                                    rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                                    99.9f * engine->lag,
                                    rkGlobalParameters.showColor ? RKNoColor : "");
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

#pragma mark - Delegate Workers

static void *pulseRecorder(void *in) {
    RKFileEngine *engine = (RKFileEngine *)in;
    
    int i, j, k, s;
    
    struct timeval t0, t1;
    
    RKPulse *pulse;
    RKConfig *config;
    
    char filename[RKMaximumStringLength];
    
    uint32_t len = 0;
    
    RKFileHeader *fileHeader = (void *)malloc(sizeof(RKFileHeader));
    memset(fileHeader, 0, 4096);
    sprintf(fileHeader->preface, "RadarKit/RawIQ");
    fileHeader->buildNo = 1;
    fileHeader->bytes[4093] = 'E';
    fileHeader->bytes[4094] = 'O';
    fileHeader->bytes[4095] = 'L';
    
    RKLog("%s Started.   mem = %s B   pulseIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);
    
    gettimeofday(&t1, 0); t1.tv_sec -= 1;
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    j = 0;   // config index
    k = 0;   // pulse index
    while (engine->state & RKEngineStateActive) {
        // The pulse
        pulse = RKGetPulse(engine->pulseBuffer, k);
        // Wait until the buffer is advanced
        s = 0;
        while (k == *engine->pulseIndex && engine->state & RKEngineStateActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->pulseIndex, pulse->header.s);
            }
        }
        // Wait until the pulse is completely processed
        while (!(pulse->header.s & RKPulseStatusHasPosition) && engine->state & RKEngineStateActive) {
            usleep(1000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        if (!(engine->state & RKEngineStateActive)) {
            break;
        }
        // Lag of the engine
        engine->lag = fmodf(((float)*engine->pulseIndex + engine->pulseBufferDepth - k) / engine->pulseBufferDepth, 1.0f);
        if (!isfinite(engine->lag)) {
            RKLog("%s Error. %d + %d - %d = %d",
                  engine->name, *engine->pulseIndex, engine->pulseBufferDepth, k, *engine->pulseIndex + engine->pulseBufferDepth - k, engine->lag);
        }
        
        // Consider we are writing at this point
        engine->state |= RKEngineStateWritingFile;
        
        // Assess the configIndex
        if (j != pulse->header.configIndex) {
            j = pulse->header.configIndex;
            config = &engine->configBuffer[pulse->header.configIndex];
            
            // Close the current file
            if (engine->doNotWrite) {
                if (engine->verbose) {
                    RKLog("%s Skipping %s (%s B) ...\n", engine->name, filename, RKIntegerToCommaStyleString(len));
                }
                usleep(250000);
                len = 0;
            } else {
                if (engine->fd != 0) {
                    if (engine->verbose) {
                        RKLog("%s Closing %s (%s B) ...\n", engine->name, filename, RKIntegerToCommaStyleString(len + engine->cacheWriteIndex));
                    }
                    len += RKFileEngineCacheFlush(engine);
                    close(engine->fd);
                }
            }
            
            // New file
            time_t startTime = pulse->header.time.tv_sec;
            i = sprintf(filename, "%s%siq/", engine->radarDescription->dataPath, engine->radarDescription->dataPath[0] == '\0' ? "" : "/");
            i += strftime(filename + i, 16, "%Y%m%d", gmtime(&startTime));
            i += sprintf(filename + i, "/%s-", engine->radarDescription->filePrefix);
            i += strftime(filename + i, 16, "%Y%m%d-%H%M%S", gmtime(&startTime));
            sprintf(filename + i, ".rkr");
            
            if (engine->doNotWrite) {
                if (engine->verbose) {
                    RKLog("%s Skipping %s ...\n", engine->name, filename);
                }
                len = 4096 + sizeof(RKConfig);
            } else {
                //if (engine->verbose) {
                RKLog("%s Creating %s ...\n", engine->name, filename);
                //}
                RKPreparePath(filename);
                
                engine->fd = open(filename, O_CREAT | O_WRONLY, 0000644);
                
                len = RKFileEngineCacheWrite(engine, fileHeader, 4096);
                len += RKFileEngineCacheWrite(engine, config, sizeof(RKConfig));
            }
        }
        
        // Actual cache and write happen here.
        if (engine->doNotWrite) {
            len += sizeof(RKPulseHeader) + 2 * pulse->header.gateCount * sizeof(RKInt16C);
        } else {
            len += RKFileEngineCacheWrite(engine, &pulse->header, sizeof(RKPulseHeader));
            len += RKFileEngineCacheWrite(engine, RKGetInt16CDataFromPulse(pulse, 0), pulse->header.gateCount * sizeof(RKInt16C));
            len += RKFileEngineCacheWrite(engine, RKGetInt16CDataFromPulse(pulse, 1), pulse->header.gateCount * sizeof(RKInt16C));
        }
        
        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            RKFileEngineUpdateStatusString(engine);
        }
        
        // Going to wait mode soon
        engine->state ^= RKEngineStateWritingFile;
        
        // Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->pulseBufferDepth);
    }
    
    free(fileHeader);
    
    return NULL;
}

#pragma mark - Life Cycle

RKFileEngine *RKFileEngineInit(void) {
    RKFileEngine *engine = (RKFileEngine *)malloc(sizeof(RKFileEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate RKFileEngine.\r");
        exit(EXIT_FAILURE);
    }
    memset(engine, 0, sizeof(RKFileEngine));
    sprintf(engine->name, "%s<RawDataRecorder>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(8) : "", rkGlobalParameters.showColor ? RKNoColor : "");
    RKFileEngineSetCacheSize(engine, 32 * 1024 * 1024);
    engine->state = RKEngineStateAllocated;
    engine->memoryUsage = sizeof(RKFileEngine) + engine->cacheSize;
    return engine;
}

void RKFileEngineFree(RKFileEngine *engine) {
    if (engine->state & RKEngineStateActive) {
        RKFileEngineStop(engine);
    }
    free(engine->cache);
    free(engine);
}

#pragma mark - Properties

void RKFileEngineSetVerbose(RKFileEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKFileEngineSetInputOutputBuffers(RKFileEngine *engine, RKRadarDesc *desc,
                                       RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                       RKBuffer pulseBuffer,   uint32_t *pulseIndex,  const uint32_t pulseBufferDepth) {
    engine->radarDescription  = desc;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->configBufferDepth = configBufferDepth;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;
    engine->pulseBufferDepth  = pulseBufferDepth;
    engine->state |= RKEngineStateProperlyWired;
}

void RKFileEngineSetDoNotWrite(RKFileEngine *engine, const bool value) {
    engine->doNotWrite = value;
}

void RKFileEngineSetCacheSize(RKFileEngine *engine, uint32_t size) {
    if (engine->cacheSize == size) {
        return;
    }
    if (engine->cache != NULL) {
        free(engine->cache);
        engine->memoryUsage -= engine->cacheSize;
    }
    engine->cacheSize = size;
    if (posix_memalign((void **)&engine->cache, RKSIMDAlignSize, engine->cacheSize)) {
        RKLog("%s Error. Unable to allocate cache.", engine->name);
        exit(EXIT_FAILURE);
    }
    engine->memoryUsage += engine->cacheSize;
}

#pragma mark - Interactions

int RKFileEngineStart(RKFileEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidPulseRecorder, NULL, pulseRecorder, engine) != 0) {
        RKLog("%s Error. Failed to start pulse recorder.\n", engine->name);
        return RKResultFailedToStartPulseRecorder;
    }
    while (!(engine->state & RKEngineStateActive)) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKFileEngineStop(RKFileEngine *engine) {
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
    pthread_join(engine->tidPulseRecorder, NULL);
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    engine->state = RKEngineStateAllocated;
    return RKResultSuccess;
}

uint32_t RKFileEngineCacheWrite(RKFileEngine *engine, const void *payload, const uint32_t size) {
    if (size == 0) {
        return 0;
    }
    uint32_t remainingSize = size;
    uint32_t lastChunkSize = 0;
    uint32_t writtenSize = 0;
    //
    // Method:
    //
    // If the remainder of cache is less than then payload size, copy the whatever that fits, called it lastChunkSize
    // Then, the last part of the payload (starting lastChunkSize) should go into the cache. Otherwise, just
    // write out the remainig payload entirely, leaving the cache empty.
    //
    if (engine->cacheWriteIndex + remainingSize >= engine->cacheSize) {
        lastChunkSize = engine->cacheSize - engine->cacheWriteIndex;
        memcpy(engine->cache + engine->cacheWriteIndex, payload, lastChunkSize);
        remainingSize = size - lastChunkSize;
        writtenSize = (uint32_t)write(engine->fd, engine->cache, engine->cacheSize);
        if (writtenSize != engine->cacheSize) {
            RKLog("%s Error in write().   writtenSize = %s\n", RKIntegerToCommaStyleString((long)writtenSize));
        }
        engine->cacheWriteIndex = 0;
        if (remainingSize >= engine->cacheSize) {
            writtenSize += (uint32_t)write(engine->fd, (char *)(payload + lastChunkSize), remainingSize);
            return writtenSize;
        }
    }
    memcpy(engine->cache + engine->cacheWriteIndex, payload + lastChunkSize, remainingSize);
    engine->cacheWriteIndex += remainingSize;
    return writtenSize;
}

uint32_t RKFileEngineCacheFlush(RKFileEngine *engine) {
    if (engine->cacheWriteIndex == 0) {
        return 0;
    }
    uint32_t writtenSize = (uint32_t)write(engine->fd, engine->cache, engine->cacheWriteIndex);
    engine->cacheWriteIndex = 0;
    return writtenSize;
}

char *RKFileEngineStatusString(RKFileEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
