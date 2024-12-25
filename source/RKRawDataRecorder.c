//
//  RKRawDataRecorder.c
//  RadarKit
//
//  Created by Boonleng Cheong on 3/18/15.
//  Copyright (c) 2015 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKRawDataRecorder.h>

// Internal Functions

static void RKRawDataRecorderUpdateStatusString(RKRawDataRecorder *);
static void *pulseRecorder(void *);

#pragma mark - Helper Functions

static void RKRawDataRecorderUpdateStatusString(RKRawDataRecorder *engine) {
    int i;
    char *string;

    // Status string
    string = engine->statusBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

    // Use RKStatusBarWidth characters to draw a bar
    i = *engine->pulseIndex * RKStatusBarWidth / engine->radarDescription->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'F';

    // Engine lag
    snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth, " %s%02.0f%s",
             rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
             99.49f * engine->lag,
             rkGlobalParameters.showColor ? RKNoColor : "");
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

#pragma mark - Delegate Workers

static void *pulseRecorder(void *in) {
    RKRawDataRecorder *engine = (RKRawDataRecorder *)in;

    int i, j, k, n, s;

    struct timeval t0, t1;

    bool record = engine->record;

    RKPulse *pulse;
    RKConfig *config;
    RKWaveform *waveform;

    size_t len = 0;
    size_t pulseCount = 0;
    uint64_t cacheFlushCount = 0;

    char *filename = engine->filename;

    RKFileHeader *fileHeader = RKFileHeaderInit();
    sprintf(fileHeader->preface, "RadarKit/IQ");
    fileHeader->dataType = RKRawDataTypeAfterMatchedFilter;
    memcpy(&fileHeader->desc, engine->radarDescription, sizeof(RKRadarDesc));
    fileHeader->bytes[sizeof(RKFileHeader) - 3] = 'E';
    fileHeader->bytes[sizeof(RKFileHeader) - 2] = 'O';
    fileHeader->bytes[sizeof(RKFileHeader) - 1] = 'L';

    RKWaveFileGlobalHeader *waveGlobalHeader = (void *)malloc(sizeof(RKWaveFileGlobalHeader));
    memset(waveGlobalHeader, 0, sizeof(RKWaveFileGlobalHeader));

    // Update the engine state
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;

    RKLog("%s Started.   mem = %s B   pulseIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    engine->state |= RKEngineStateActive;

    gettimeofday(&t1, NULL); t1.tv_sec -= 1;

    j = 0;   // config index
    k = 0;   // pulse index
    n = 0;   // pulse sample count
    while (engine->state & RKEngineStateWantActive) {
        // The pulse
        pulse = RKGetPulseFromBuffer(engine->pulseBuffer, k);

        // Wait until the buffer is advanced
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->pulseIndex && engine->state & RKEngineStateWantActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k, *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // Wait until the pulse is completely processed
        while (!(pulse->header.s & RKPulseStatusUsedForMoments) && engine->state & RKEngineStateWantActive) {
            usleep(10000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.01f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateWantActive)) {
            break;
        }

        // Lag of the engine
        engine->lag = fmodf(((float)*engine->pulseIndex + engine->radarDescription->pulseBufferDepth - k) / engine->radarDescription->pulseBufferDepth, 1.0f);
        if (!isfinite(engine->lag)) {
            RKLog("%s Error. %d + %d - %d = %d",
                  engine->name,
                  *engine->pulseIndex, engine->radarDescription->pulseBufferDepth, k,
                  *engine->pulseIndex + engine->radarDescription->pulseBufferDepth - k, engine->lag);
        }

        // Consider we are writing to a file at this point
        engine->state |= RKEngineStateWritingFile;

        // Assess the configIndex, or if we reached the maximum pulse count for a file; or when user just decided to start/stop recording
        if (j != pulse->header.configIndex || n >= engine->maximumRecordDepth || record != engine->record) {
            j = pulse->header.configIndex;
            config = &engine->configBuffer[pulse->header.configIndex];
            waveform = config->waveform;

            // Close the current file
            if (engine->fd) {
                len += RKRawDataRecorderCacheFlush(engine);
                RKRawDataRecorderCloseFile(engine);
                // RKLog("%s len = %s\n", engine->name, RKIntegerToCommaStyleString(len));
                // Notify file manager of a new addition
                if (engine->fileManager) {
                    RKFileManagerAddFile(engine->fileManager, filename, RKFileTypeIQ);
                }
            } else {
                if (strlen(filename)) {
                    RKLog("%s %sSkipped%s %s (%s pulses, %s B)\n",
                          engine->name,
                          rkGlobalParameters.showColor ? RKLightOrangeColor : "",
                          rkGlobalParameters.showColor ? RKNoColor : "",
                          filename,
                          RKIntegerToCommaStyleString(n),
                          RKFloatToCommaStyleString(len));
                }
            }

            // New file as .../[DATA_PATH]/20170119/PX-20170119-012345.123.rk[rc]
            i = sprintf(filename, "%s%s%s/", engine->radarDescription->dataPath, engine->radarDescription->dataPath[0] == '\0' ? "" : "/", RKDataFolderIQ);
            i += sprintf(filename + i, "%s/", RKTimevalToString(pulse->header.time, 800, true));
            i += sprintf(filename + i, "%s-", engine->radarDescription->filePrefix);
            i += sprintf(filename + i, "%s", RKTimevalToString(pulse->header.time, 863, true));
            if (engine->rawDataType == RKRawDataTypeFromTransceiver) {
                sprintf(filename + i, ".rkr");
            } else {
                sprintf(filename + i, ".rkc");
            }
            fileHeader->dataType = engine->rawDataType;

            n = 0;
            engine->filePulseCount = 0;

            if (engine->verbose > 1) {
                RKLog("%s New I/Q %s ... pulse->header.i = %u  .c = %u\n", engine->name, filename, pulse->header.i, pulse->header.configIndex);
            }
            if (engine->record && config->i > 1003) {
                RKPreparePath(filename);
                // 4-KB raw data file header
                memcpy(&fileHeader->config, config, sizeof(RKConfig));
                fileHeader->config.waveform = NULL;
                RKRawDataRecorderNewFile(engine, filename);
                len = RKRawDataRecorderCacheWrite(engine, fileHeader, sizeof(RKFileHeader));
                // 512-B wave header
                strcpy(waveGlobalHeader->name, waveform->name);
                waveGlobalHeader->count = waveform->count;
                waveGlobalHeader->depth = waveform->depth;
                waveGlobalHeader->type = waveform->type;
                waveGlobalHeader->fc = waveform->fc;
                waveGlobalHeader->fs = waveform->fs;
                for (i = 0; i < waveform->count; i++) {
                    waveGlobalHeader->filterCounts[i] = waveform->filterCounts[i];
                }
                len += RKRawDataRecorderCacheWrite(engine, waveGlobalHeader, sizeof(RKWaveFileGlobalHeader));
                for (i = 0; i < waveform->count; i++) {
                    // 32-B wave group header
                    len += RKRawDataRecorderCacheWrite(engine, waveform->filterAnchors[i], waveform->filterCounts[i] * sizeof(RKFilterAnchor));
                    // Waveform samples (flexible size)
                    len += RKRawDataRecorderCacheWrite(engine, waveform->samples[i], waveform->depth * sizeof(RKComplex));
                    len += RKRawDataRecorderCacheWrite(engine, waveform->iSamples[i], waveform->depth * sizeof(RKInt16C));
                }
            } else {
                len = sizeof(RKFileHeader) + sizeof(RKWaveFileGlobalHeader);
                for (i = 0; i < waveform->count; i++) {
                    len += waveform->filterCounts[i] * sizeof(RKFilterAnchor);
                    len += waveform->depth * sizeof(RKComplex);
                    len += waveform->depth * sizeof(RKInt16C);
                }
            }
        }

        // Pulse to write cache
        if (engine->record && engine->fd) {
            if (fileHeader->dataType == RKRawDataTypeFromTransceiver) {
                len += RKRawDataRecorderCacheWrite(engine, &pulse->header, sizeof(RKPulseHeader));
                len += RKRawDataRecorderCacheWrite(engine, RKGetInt16CDataFromPulse(pulse, 0), pulse->header.gateCount * sizeof(RKInt16C));
                len += RKRawDataRecorderCacheWrite(engine, RKGetInt16CDataFromPulse(pulse, 1), pulse->header.gateCount * sizeof(RKInt16C));
            } else {
                len += RKRawDataRecorderCacheWrite(engine, &pulse->header, sizeof(RKPulseHeader));
                len += RKRawDataRecorderCacheWrite(engine, RKGetComplexDataFromPulse(pulse, 0), pulse->header.downSampledGateCount * sizeof(RKComplex));
                len += RKRawDataRecorderCacheWrite(engine, RKGetComplexDataFromPulse(pulse, 1), pulse->header.downSampledGateCount * sizeof(RKComplex));
            }
        } else {
            if (fileHeader->dataType == RKRawDataTypeFromTransceiver) {
                len += sizeof(RKPulseHeader) + 2 * pulse->header.gateCount * sizeof(RKInt16C);
            } else {
                len += sizeof(RKPulseHeader) + 2 * pulse->header.downSampledGateCount * sizeof(RKComplex);
            }
        }
        pulseCount++;

        if (cacheFlushCount != engine->cacheFlushCount) {
            cacheFlushCount = engine->cacheFlushCount;
            i = k;
            while (pulseCount > 0) {
                pulseCount--;
                i = RKPreviousModuloS(i, engine->radarDescription->pulseBufferDepth);
                pulse = RKGetPulseFromBuffer(engine->pulseBuffer, i);
                if (pulse->header.s != RKPulseStatusVacant) {
                    pulse->header.s |= RKPulseStatusRecorded;
                }
            }
        }

        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            RKRawDataRecorderUpdateStatusString(engine);
        }

        // Going to wait mode soon
        engine->state ^= RKEngineStateWritingFile;

        engine->filePulseCount++;
        engine->tic++;

        // Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->radarDescription->pulseBufferDepth);
        record = engine->record;
        n++;
    }

    if (engine->fd) {
        close(engine->fd);
        engine->fd = 0;
        if (engine->fileWriteCount == 0) {
            remove(filename);
        }
    }

    free(fileHeader);
    free(waveGlobalHeader);

    engine->state ^= RKEngineStateActive;
    return NULL;
}

#pragma mark - Life Cycle

RKRawDataRecorder *RKRawDataRecorderInit(void) {
    RKRawDataRecorder *engine = (RKRawDataRecorder *)malloc(sizeof(RKRawDataRecorder));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate RKRawDataRecorder.\r");
        exit(EXIT_FAILURE);
    }
    memset(engine, 0, sizeof(RKRawDataRecorder));
    sprintf(engine->name, "%s<RawDataRecorder>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorDataRecorder) : "", rkGlobalParameters.showColor ? RKNoColor : "");
    RKRawDataRecorderSetCacheSize(engine, RKRawDataRecorderDefaultCacheSize);
    engine->state = RKEngineStateAllocated;
    engine->rawDataType = RKRawDataTypeAfterMatchedFilter;
    engine->maximumRecordDepth = RKRawDataRecorderDefaultMaximumRecorderDepth;
    engine->memoryUsage = sizeof(RKRawDataRecorder) + engine->cacheSize;
    return engine;
}

void RKRawDataRecorderFree(RKRawDataRecorder *engine) {
    if (engine->state & RKEngineStateWantActive) {
        RKRawDataRecorderStop(engine);
    }
    free(engine->cache);
    free(engine);
}

#pragma mark - Properties

void RKRawDataRecorderSetVerbose(RKRawDataRecorder *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKRawDataRecorderSetEssentials(RKRawDataRecorder *engine, RKRadarDesc *desc, RKFileManager _Nullable *fileManager,
                                    RKConfig *configBuffer, uint32_t *configIndex,
                                    RKBuffer pulseBuffer,   uint32_t *pulseIndex) {
    engine->radarDescription  = desc;
    engine->fileManager       = fileManager;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;
    engine->state |= RKEngineStateProperlyWired;
}

void RKRawDataRecorderSetRecord(RKRawDataRecorder *engine, const bool value) {
    engine->record = value;
}

void RKRawDataRecorderSetRawDataType(RKRawDataRecorder *engine, const RKRawDataType type) {
    engine->rawDataType = type;
}

void RKRawDataRecorderSetMaximumRecordDepth(RKRawDataRecorder *engine, const uint32_t depth) {
    engine->maximumRecordDepth = depth;
}

void RKRawDataRecorderSetCacheSize(RKRawDataRecorder *engine, uint32_t size) {
    if (engine->cacheSize == size) {
        return;
    }
    if (engine->cache != NULL) {
        free(engine->cache);
        engine->memoryUsage -= engine->cacheSize;
    }
    engine->cacheSize = size;
    if (posix_memalign((void **)&engine->cache, RKMemoryAlignSize, engine->cacheSize)) {
        RKLog("%s Error. Unable to allocate cache.", engine->name);
        exit(EXIT_FAILURE);
    }
    engine->memoryUsage += engine->cacheSize;
}

#pragma mark - Interactions

int RKRawDataRecorderStart(RKRawDataRecorder *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidPulseRecorder, NULL, pulseRecorder, engine) != 0) {
        RKLog("%s Error. Failed to start pulse recorder.\n", engine->name);
        return RKResultFailedToStartPulseRecorder;
    }
    while (!(engine->state & RKEngineStateWantActive)) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKRawDataRecorderStop(RKRawDataRecorder *engine) {
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
    if (engine->tidPulseRecorder) {
        pthread_join(engine->tidPulseRecorder, NULL);
        engine->tidPulseRecorder = (pthread_t)0;
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

char *RKRawDataRecorderStatusString(RKRawDataRecorder *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}

int RKRawDataRecorderNewFile(RKRawDataRecorder *engine, const char *filename) {
    if (engine->fd) {
        close(engine->fd);
        engine->fd = 0;
    }
    if (strlen(filename) >= RKMaximumPathLength - 1) {
        RKLog("%s Error. Filename is too long.\n", engine->name);
        return RKResultFailedToOpenFile;
    }
    strncpy(engine->filename, filename, sizeof(engine->filename) - 1);
    engine->fd = open(engine->filename, O_CREAT | O_WRONLY, 0000644);
    if (engine->fd < 0) {
        RKLog("%s Error. Failed to open file %s\n", engine->name, engine->filename);
        return RKResultFailedToOpenFile;
    }
    engine->fileWriteSize = 0;
    engine->filePulseCount = 0;
    engine->fileWriteCount = 0;
    engine->cacheWriteIndex = 0;
    return RKResultSuccess;
}

int RKRawDataRecorderCloseFileVerbose(RKRawDataRecorder *engine, int verbose) {
    if (engine->fd) {
        close(engine->fd);
        engine->fd = 0;
    }
    if (verbose) {
        bool filenameTooLong = strlen(engine->filename) > 44;
        RKLog("%s %sRecorded %s%s%s%s (%s pulses, %s B) w%d\n",
                engine->name,
                rkGlobalParameters.showColor ? RKGreenColor : "",
                rkGlobalParameters.showColor ? RKMonokaiYellow : "",
                filenameTooLong ? "..." : "",
                filenameTooLong ? RKLastTwoPartsOfPath(engine->filename) : engine->filename,
                rkGlobalParameters.showColor ? RKNoColor : "",
                RKIntegerToCommaStyleString(engine->filePulseCount),
                RKIntegerToCommaStyleString(engine->fileWriteSize),
                engine->fileWriteCount);
    }
    if (engine->fileWriteCount == 0) {
        remove(engine->filename);
    }
    return RKResultSuccess;
}

int RKRawDataRecorderCloseFileQuiet(RKRawDataRecorder *engine) {
    return RKRawDataRecorderCloseFileVerbose(engine, 0);
}

int RKRawDataRecorderCloseFile(RKRawDataRecorder *engine) {
    return RKRawDataRecorderCloseFileVerbose(engine, 1);
}

int RKRawDataRecorderIncreasePulseCount(RKRawDataRecorder *engine) {
    engine->filePulseCount++;
    return RKResultSuccess;
}

size_t RKRawDataRecorderCacheWrite(RKRawDataRecorder *engine, const void *payload, const size_t size) {
    if (size == 0) {
        return 0;
    }
    size_t remainingSize = size;
    size_t lastChunkSize = 0;
    ssize_t writtenSize = 0;
    ssize_t returnSize = 0;
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
        returnSize = write(engine->fd, engine->cache, engine->cacheSize);
        if (returnSize < 0) {
            RKLog("%s Error in write().   writtenSize = %s / %s   errno = %s\n", engine->name,
                RKIntegerToCommaStyleString((long long)writtenSize),
                RKIntegerToCommaStyleString((long long)engine->cacheSize),
                strerror(errno));
            exit(EXIT_FAILURE);
        }
        writtenSize = returnSize;
        engine->fileWriteCount++;
        engine->cacheFlushCount++;
        engine->cacheWriteIndex = 0;
        if (remainingSize >= engine->cacheSize) {
            writtenSize += write(engine->fd, (char *)(payload + lastChunkSize), remainingSize);
            engine->fileWriteCount++;
            return writtenSize;
        }
    }
    memcpy(engine->cache + engine->cacheWriteIndex, payload + lastChunkSize, remainingSize);
    engine->cacheWriteIndex += remainingSize;
    engine->fileWriteSize += writtenSize;
    return writtenSize;
}

size_t RKRawDataRecorderCacheFlush(RKRawDataRecorder *engine) {
    if (engine->cacheWriteIndex == 0) {
        return 0;
    }
    if (engine->fd <= 0) {
        RKLog("%s Error. File descriptor is not open (%d).\n", engine->name, engine->fd);
        return 0;
    }
    ssize_t writtenSize = write(engine->fd, engine->cache, engine->cacheWriteIndex);
    if (writtenSize < 0) {
        RKLog("%s Error in write().   writtenSize = %s / %s   errno = %s\n", engine->name,
              RKIntegerToCommaStyleString((long long)writtenSize),
              RKIntegerToCommaStyleString((long long)engine->cacheWriteIndex),
              strerror(errno));
        exit(EXIT_FAILURE);
    }
    engine->fileWriteSize += writtenSize;
    engine->cacheWriteIndex = 0;
    engine->fileWriteCount++;
    return writtenSize;
}
