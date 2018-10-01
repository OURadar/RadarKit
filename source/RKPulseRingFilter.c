//
//  RKPulseRingFilter.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 11/11/17.
//  Copyright (c) 2015-2018 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulseRingFilter.h>

#pragma mark - Helper Functions

static void RKPulseRingFilterUpdateStatusString(RKPulseRingFilterEngine *engine) {
    int i, c;
    char *string = engine->statusBuffer[engine->statusBufferIndex];
    const bool useCompact = engine->coreCount >= 3;

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';
    
    // Use RKStatusBarWidth characters to draw a bar
    i = *engine->pulseIndex * RKStatusBarWidth / engine->radarDescription->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'R';

    // Engine lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth, " %s%02.0f%s :%s",
                                    rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                                    99.49f * engine->lag,
                                    rkGlobalParameters.showColor ? RKNoColor : "",
                                    useCompact ? " " : "");
    
    RKPulseRingFilterWorker *worker;

    // State: 0 - green, 1 - yellow, 2 - red
    int s1 = -1, s0 = 0;

    // Lag from each core
    for (c = 0; c < engine->coreCount; c++) {
        worker = &engine->workers[c];
        s0 = (worker->lag > RKLagRedThreshold ? 2 : (worker->lag > RKLagOrangeThreshold ? 1 : 0));
        if (s1 != s0 && rkGlobalParameters.showColor) {
            s1 = s0;
            i += snprintf(string + i, RKStatusStringLength - i, "%s",
                          s0 == 2 ? RKBaseRedColor : (s0 == 1 ? RKBaseYellowColor : RKBaseGreenColor));
        }
        if (useCompact) {
            i += snprintf(string + i, RKStatusStringLength - i, "%01.0f", 9.49f * worker->lag);
        } else {
            i += snprintf(string + i, RKStatusStringLength - i, " %02.0f", 99.49f * worker->lag);
        }
    }

    // Put a separator
    i += snprintf(string + i, RKStatusStringLength - i, " ");
    // Duty cycle of each core
    for (c = 0; c < engine->coreCount && i < RKStatusStringLength - RKStatusBarWidth - 5; c++) {
        worker = &engine->workers[c];
        s0 = (worker->dutyCycle > RKDutyCyleRedThreshold ? 2 : (worker->dutyCycle > RKDutyCyleOrangeThreshold ? 1 : 0));
        if (s1 != s0 && rkGlobalParameters.showColor) {
            s1 = s0;
            i += snprintf(string + i, RKStatusStringLength - i, "%s",
                          s0 == 2 ? RKBaseRedColor : (s0 == 1 ? RKBaseYellowColor : RKBaseGreenColor));
        }
        if (useCompact) {
            i += snprintf(string + i, RKStatusStringLength - i, "%01.0f", 9.49f * worker->dutyCycle);
        } else {
            i += snprintf(string + i, RKStatusStringLength - i, " %02.0f", 99.49f * worker->dutyCycle);
        }
    }
    if (rkGlobalParameters.showColor) {
        i += snprintf(string + i, RKStatusStringLength - i, "%s", RKNoColor);
    }

    // Almost full count
    //i += snprintf(string + i, RKStatusStringLength - i, " [%d]", engine->almostFull);
    
    // Concluding string
    if (i > RKStatusStringLength - RKStatusBarWidth - 5) {
        memset(string + i, '#', RKStatusStringLength - i - 1);
    }
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

#pragma mark - Delegate Workers

static void *ringFilterCore(void *_in) {
    RKPulseRingFilterWorker *me = (RKPulseRingFilterWorker *)_in;
    RKPulseRingFilterEngine *engine = me->parent;

    int i, j, k, p;
    struct timeval t0, t1, t2;

    const int c = me->id;
    const int ci = engine->radarDescription->initFlags & RKInitFlagManuallyAssignCPU ? engine->coreOrigin + c : -1;

    // Find the semaphore
    sem_t *sem = sem_open(me->semaphoreName, O_RDWR);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve semaphore %d\n", c);
        return (void *)RKResultFailedToRetrieveSemaphore;
    };

    // Initiate my name
    if (rkGlobalParameters.showColor) {
        pthread_mutex_lock(&engine->mutex);
        k = snprintf(me->name, RKShortNameLength - 1, "%s", rkGlobalParameters.showColor ? RKGetColor() : "");
        pthread_mutex_unlock(&engine->mutex);
    } else {
        k = 0;
    }
    if (engine->coreCount > 9) {
        k += sprintf(me->name + k, "C%02d", c);
    } else {
        k += sprintf(me->name + k, "C%d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(me->name + k, RKNoColor);
    }

#if defined(_GNU_SOURCE)
    
    if (engine->radarDescription->initFlags & RKInitFlagManuallyAssignCPU) {
        // Set my CPU core
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(ci, &cpuset);
        sched_setaffinity(0, sizeof(cpuset), &cpuset);
        pthread_setaffinity_np(me->tid, sizeof(cpu_set_t), &cpuset);
    }
    
#endif

    RKPulse *pulse;
    size_t mem = 0;
    
    if ((me->processOrigin * sizeof(RKFloat)) % RKSIMDAlignSize > 0) {
        RKLog("%s %s Error. Each filter origin must align to the SIMD requirements.\n", engine->name, me->name);
        return NULL;
    }
    // Allocate local resources, use k to keep track of the total allocation
    // Each block is depth x pols (2) x gates (me->processLength)
    RKIQZ xx;
    RKIQZ yy;
    const int depth = RKMaximumIIRFilterTaps;
    size_t filterSize = depth * engine->radarDescription->pulseCapacity / engine->coreCount * sizeof(RKFloat);
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&xx.i, RKSIMDAlignSize, 2 * filterSize));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&xx.q, RKSIMDAlignSize, 2 * filterSize));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&yy.i, RKSIMDAlignSize, 2 * filterSize));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&yy.q, RKSIMDAlignSize, 2 * filterSize));
    memset(xx.i, 0, 2 * filterSize);
    memset(xx.q, 0, 2 * filterSize);
    memset(yy.i, 0, 2 * filterSize);
    memset(yy.q, 0, 2 * filterSize);
    mem += 8 * filterSize;

    double *busyPeriods, *fullPeriods;
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&busyPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&fullPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    if (busyPeriods == NULL || fullPeriods == NULL) {
        RKLog("Error. Unable to allocate resources for duty cycle calculation\n");
        exit(EXIT_FAILURE);
    }
    mem += 2 * RKWorkerDutyCycleBufferDepth * sizeof(double);
    memset(busyPeriods, 0, RKWorkerDutyCycleBufferDepth * sizeof(double));
    memset(fullPeriods, 0, RKWorkerDutyCycleBufferDepth * sizeof(double));
    double allBusyPeriods = 0.0, allFullPeriods = 0.0;

    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);

    // The last index of the pulse buffer
    uint32_t i0 = engine->radarDescription->pulseBufferDepth - 1;

    // The latest index in the dutyCycle buffer
    int d0 = 0;

    // Log my initial state
    pthread_mutex_lock(&engine->mutex);
    engine->memoryUsage += mem;
    
    RKLog(">%s %s Started.   mem = %s B   i0 = %s   filter @ (%s, %s, %s)   ci = %d\n",
          engine->name, me->name,
          RKUIntegerToCommaStyleString(mem),
          RKIntegerToCommaStyleString(i0),
          RKIntegerToCommaStyleString(me->processOrigin),
          RKIntegerToCommaStyleString(me->processLength),
          RKIntegerToCommaStyleString(me->outputLength),
          ci);

    pthread_mutex_unlock(&engine->mutex);

    // Increase the tic once to indicate this processing core is created.
    me->tic++;
    
    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]
    //
    uint64_t tic = me->tic;

    RKIQZ Z, xi, yi, yk;
    int kOffset, iOffset;
    
    k = 0;    // pulse index
    while (engine->state & RKEngineStateWantActive) {
        if (engine->useSemaphore) {
            #ifdef DEBUG_IQ
            RKLog(">%s sem_wait()\n", coreName);
            #endif
            if (sem_wait(sem)) {
                RKLog("%s %s Error. Failed in sem_wait(). errno = %d\n", engine->name, me->name, errno);
            }
        } else {
            while (tic == me->tic && engine->state & RKEngineStateWantActive) {
                usleep(1000);
            }
            tic = me->tic;
        }
        if (!(engine->state & RKEngineStateWantActive)) {
            break;
        }
        
        // Something happened
        gettimeofday(&t1, NULL);
        
        // Start of getting busy
        i0 = RKNextModuloS(i0, engine->radarDescription->pulseBufferDepth);

        pulse = RKGetPulseFromBuffer(engine->pulseBuffer, i0);
		if (!(pulse->header.s & RKPulseStatusRingInspected)) {
			fprintf(stderr, "This should not happen.   i0 = %d\n", i0);
		}

        // Now we do the work
        // Should only focus on the tasked range bins
        //
        if (engine->workerTaskDone[i0 * engine->coreCount + c] == true) {
            fprintf(stderr, "Already done?   i0 = %d\n", i0);
            j = RKPreviousModuloS(i0, engine->radarDescription->pulseBufferDepth);
            fprintf(stderr, "j = %d  --> %d\n", j, engine->workerTaskDone[i0 * engine->coreCount + c]);
            j = RKNextModuloS(j, engine->radarDescription->pulseBufferDepth);
            fprintf(stderr, "j = %d  --> %d\n", j, engine->workerTaskDone[i0 * engine->coreCount + c]);
            j = RKNextModuloS(j, engine->radarDescription->pulseBufferDepth);
            fprintf(stderr, "j = %d  --> %d\n", j, engine->workerTaskDone[i0 * engine->coreCount + c]);
        }
        if (engine->useFilter) {
            // Now we perform the difference equation on each polarization
            // y[n] = B[0] * x[n] + B[1] * x[n - 1] + ...
            //
            for (p = 0; p < 2; p++) {
                // Store x[n] at index k
                kOffset = (k * 2 + p) * me->processLength;
                Z = RKGetSplitComplexDataFromPulse(pulse, p);
                memcpy(xx.i + kOffset, Z.i + me->processOrigin, me->processLength * sizeof(RKFloat));
                memcpy(xx.q + kOffset, Z.q + me->processOrigin, me->processLength * sizeof(RKFloat));
                
#if defined(DEBUG_IIR)
                
                pthread_mutex_lock(&engine->mutex);
                RKLog(">%s %s %s   %s   %s   %s   %s\n", engine->name, name,
                      RKVariableInString("p", &p, RKValueTypeInt),
                      RKVariableInString("k", &k, RKValueTypeInt),
                      RKVariableInString("bLength", &engine->filter.bLength, RKValueTypeUInt32),
                      RKVariableInString("aLength", &engine->filter.aLength, RKValueTypeUInt32),
                      RKVariableInString("outpuLength", &me->outputLength, RKValueTypeUInt32));
                
#endif
                
                // Store y[n] at local buffer y at offset k
                yk.i = yy.i + kOffset;
                yk.q = yy.q + kOffset;
                memset(yk.i, 0, me->processLength * sizeof(RKFloat));
                memset(yk.q, 0, me->processLength * sizeof(RKFloat));
                
                // B's
                i = k;
                for (j = 0; j < engine->filter.bLength; j++) {
                    iOffset = (i * 2 + p) * me->processLength;
                    xi.i = xx.i + iOffset;
                    xi.q = xx.q + iOffset;
                    RKSIMD_csz(engine->filter.B[j].i, &xi, &yk, me->processLength);

#if defined(DEBUG_IIR)
                    
                    RKLog(">%s %s B portion   %s   %s   %s   %s\n", engine->name, name,
                          RKVariableInString("k", &k, RKValueTypeInt),
                          RKVariableInString("j", &j, RKValueTypeInt),
                          RKVariableInString("i", &i, RKValueTypeInt),
                          RKVariableInString("iOffset", &iOffset, RKValueTypeInt));
                    RKShowArray(xi.i, "xi.i", 8, 1);
                    RKShowArray(xi.q, "xi.q", 8, 1);
                    RKLog(">%s\n", RKVariableInString("b", &engine->filter.B[j].i, RKValueTypeFloat));
                    RKShowArray(yk.i, "yk.i", 8, 1);
                    RKShowArray(yk.q, "yk.q", 8, 1);
                    
#endif
                    
                    i = RKPreviousModuloS(i, depth);
                }

                // A's
                i = RKPreviousModuloS(k, depth);
                for (j = 1; j < engine->filter.aLength; j++) {
                    iOffset = (i * 2 + p) * me->processLength;
                    yi.i = yy.i + iOffset;
                    yi.q = yy.q + iOffset;
                    RKSIMD_csz(-engine->filter.A[j].i, &yi, &yk, me->processLength);
                    
#if defined(DEBUG_IIR)
                    
                    RKLog(">%s %s A portion   %s   %s   %s\n", engine->name, name,
                          RKVariableInString("j", &j, RKValueTypeInt),
                          RKVariableInString("i", &i, RKValueTypeInt),
                          RKVariableInString("iOffset", &iOffset, RKValueTypeInt));
                    RKShowArray(yi.i, "yi.i", 8, 1);
                    RKShowArray(yi.q, "yi.q", 8, 1);
                    RKLog(">%s\n", RKVariableInString("a", &engine->filter.A[j].i, RKValueTypeFloat));
                    RKShowArray(yk.i, "yk.i", 8, 1);
                    RKShowArray(yk.q, "yk.q", 8, 1);
                    
#endif
                    
                    i = RKPreviousModuloS(i, depth);
                }
                
                // Override pulse data with y[k] up to gateCount only
                memcpy(Z.i + me->processOrigin, yk.i, me->outputLength * sizeof(RKFloat));
                memcpy(Z.q + me->processOrigin, yk.q, me->outputLength * sizeof(RKFloat));
                
#if defined(DEBUG_IIR)
                
                RKLog("%s %s Output copied with path @ (%s, %s)", engine->name, name,
                      RKIntegerToCommaStyleString(me->processOrigin),
                      RKIntegerToCommaStyleString(me->outputLength));
                pthread_mutex_unlock(&engine->mutex);
                
#endif
                
            } // for (p = 0; ...
            // Move to the next index of local buffer
            k = RKNextModuloS(k, depth);
        } // if (engine->useFilter) ...
        
        // The task for this core is now done at this point
        engine->workerTaskDone[i0 * engine->coreCount + c] = true;

        #ifdef DEBUG_IQ
        RKLog(">%s i0 = %d  stat = %d\n", coreName, i0, input->header.s);
        #endif

        // Record down the latest processed pulse index
        me->pid = i0;
        me->lag = fmodf((float)(*engine->pulseIndex + engine->radarDescription->pulseBufferDepth - me->pid) / engine->radarDescription->pulseBufferDepth, 1.0f);

        // Done processing, get the time
        gettimeofday(&t0, NULL);
        
        // Drop the oldest reading, replace it, and add to the calculation
        allBusyPeriods -= busyPeriods[d0];
        allFullPeriods -= fullPeriods[d0];
        busyPeriods[d0] = RKTimevalDiff(t0, t1);
        fullPeriods[d0] = RKTimevalDiff(t0, t2);
        allBusyPeriods += busyPeriods[d0];
        allFullPeriods += fullPeriods[d0];
        d0 = RKNextModuloS(d0, RKWorkerDutyCycleBufferDepth);
        me->dutyCycle = allBusyPeriods / allFullPeriods;

        t2 = t0;
    }

    // Clean up
    if (engine->verbose > 1) {
        RKLog("%s %s Freeing reources ...\n", engine->name, me->name);
    }
    
    free(xx.i);
    free(xx.q);
    free(yy.i);
    free(yy.q);
    free(busyPeriods);
    free(fullPeriods);

    RKLog(">%s %s Stopped.\n", engine->name, me->name);

    return NULL;
}

static void *pulseRingWatcher(void *_in) {
    RKPulseRingFilterEngine *engine = (RKPulseRingFilterEngine *)_in;
    
    int c, i, j, k, s;
	struct timeval t0, t1;
	float lag;

	sem_t *sem[engine->coreCount];
    
    unsigned int skipCounter = 0;

    bool allDone;
    bool *workerTaskDone;
    
    if (engine->coreCount == 0) {
        RKLog("Error. No processing core?\n");
        return NULL;
    }
    
    RKConfig *config = &engine->configBuffer[RKPreviousModuloS(*engine->configIndex, engine->radarDescription->configBufferDepth)];
    uint32_t gateCount = MIN(engine->radarDescription->pulseCapacity, config->pulseRingFilterGateCount);
    
    RKPulse *pulse;
    RKPulse *pulseToSkip;

	// Filter status of each worker: the beginning of the buffer is a pulse, it has the capacity info
    engine->workerTaskDone = (bool *)malloc(engine->radarDescription->pulseBufferDepth * engine->coreCount * sizeof(bool));
    memset(engine->workerTaskDone, 0, engine->radarDescription->pulseBufferDepth * engine->coreCount * sizeof(bool));
    
	// Update the engine state
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;

    // Show filter summary
    RKPulseRingFilterEngineShowFilterSummary(engine);
    
    // Spin off N workers to process I/Q pulses
    memset(sem, 0, engine->coreCount * sizeof(sem_t *));
    uint32_t paddedGateCount = ((int)ceilf((float)gateCount * sizeof(RKFloat) / engine->coreCount / RKSIMDAlignSize) * engine->coreCount * RKSIMDAlignSize / sizeof(RKFloat));
    uint32_t length = paddedGateCount / engine->coreCount;
    uint32_t origin = 0;
    if (engine->verbose > 2) {
        RKLog("%s Initial paddedGateCount = %s   length = %s\n", engine->name, RKUIntegerToCommaStyleString(paddedGateCount), RKUIntegerToCommaStyleString(length));
    }
    for (c = 0; c < engine->coreCount; c++) {
        RKPulseRingFilterWorker *worker = &engine->workers[c];
        snprintf(worker->semaphoreName, 32, "rk-cf-%03d", c);
        sem[c] = sem_open(worker->semaphoreName, O_CREAT | O_EXCL, 0600, 0);
        if (sem[c] == SEM_FAILED) {
            if (engine->verbose > 1) {
                RKLog(">%s Info. Semaphore %s exists. Try to remove and recreate.\n", engine->name, worker->semaphoreName);
            }
            if (sem_unlink(worker->semaphoreName)) {
                RKLog(">%s Error. Unable to unlink semaphore %s.\n", engine->name, worker->semaphoreName);
            }
            // 2nd trial
            sem[c] = sem_open(worker->semaphoreName, O_CREAT | O_EXCL, 0600, 0);
            if (sem[c] == SEM_FAILED) {
                RKLog(">%s Error. Unable to remove then create semaphore %s\n", engine->name, worker->semaphoreName);
                return (void *)RKResultFailedToInitiateSemaphore;
            } else if (engine->verbose > 1) {
                RKLog(">%s Info. Semaphore %s removed and recreated.\n", engine->name, worker->semaphoreName);
            }
        }
        worker->id = c;
        worker->sem = sem[c];
        worker->parent = engine;
        worker->processOrigin = origin;
        worker->processLength = length;
        worker->outputLength = MIN(gateCount - origin, length);
        origin += length;
        if (engine->verbose > 1) {
            RKLog(">%s %s @ %p\n", engine->name, worker->semaphoreName, worker->sem);
        }
        if (pthread_create(&worker->tid, NULL, ringFilterCore, worker) != 0) {
            RKLog(">%s Error. Failed to start a ring core.\n", engine->name);
            return (void *)RKResultFailedToStartRingCore;
        }
    }

    // Wait for the workers to increase the tic count once
    engine->state |= RKEngineStateSleep0;
    for (c = 0; c < engine->coreCount; c++) {
        while (engine->workers[c].tic == 0) {
            usleep(1000);
        }
    }
    engine->state ^= RKEngineStateSleep0;
    engine->state |= RKEngineStateActive;

    RKLog("%s Started.   mem = %s B   pulseIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);

	// Increase the tic once to indicate the engine is ready
	engine->tic = 1;

    gettimeofday(&t1, NULL); t1.tv_sec -= 1;

    // Here comes the busy loop
    // i  anonymous
	// c  core index
    j = 0;   // filtered pulse index
    k = 0;   // pulse index
    while (engine->state & RKEngineStateWantActive) {
        // The pulse
        pulse = RKGetPulseFromBuffer(engine->pulseBuffer, k);

        // Wait until the engine index move to the next one for storage, which is also the time pulse has data.
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->pulseIndex && engine->state & RKEngineStateWantActive) {
            usleep(200);
            if (++s % 1000 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.0002f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // Wait until the pulse has has been processed (compressed or skipped) so that this engine won't compete with the pulse compression engine to set the status.
        s = 0;
        while (!(pulse->header.s & RKPulseStatusProcessed) && engine->state & RKEngineStateWantActive) {
            usleep(200);
            if (++s % 1000 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.0002f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep2;
        
        if (!(engine->state & RKEngineStateWantActive)) {
            break;
        }

        // Lag of the engine
        engine->lag = fmodf(((float)*engine->pulseIndex + engine->radarDescription->pulseBufferDepth - k) / engine->radarDescription->pulseBufferDepth, 1.0f);

        // Assess the lag of the workers
        lag = engine->workers[0].lag;
        for (i = 1; i < engine->coreCount; i++) {
            lag = MAX(lag, engine->workers[i].lag);
        }
        if (skipCounter == 0 && lag > 0.9f) {
            engine->almostFull++;
            skipCounter = engine->radarDescription->pulseBufferDepth / 10;
            RKLog("%s Warning. Projected an I/Q Buffer overflow.\n", engine->name);
            i = *engine->pulseIndex;
            do {
                i = RKPreviousModuloS(i, engine->radarDescription->pulseBufferDepth);
                // Have some way to skip processing
                pulseToSkip = RKGetPulseFromBuffer(engine->pulseBuffer, i);
            } while (!(pulseToSkip->header.s & RKPulseStatusRingFiltered));
        } else if (skipCounter > 0) {
            // Skip processing if the buffer is getting full (avoid hitting SEM_VALUE_MAX)
            // Have some way to record skipping
            if (--skipCounter == 0) {
                RKLog(">%s Info. Skipped a chunk.\n", engine->name);
                for (i = 0; i < engine->coreCount; i++) {
                    engine->workers[i].lag = 0.0f;
                }
            }
        }
        
        // The config to get PulseRingFilterGateCount
        config = &engine->configBuffer[RKPreviousModuloS(*engine->configIndex, engine->radarDescription->configBufferDepth)];

        // Update processing region if necessary
        if (gateCount != MIN(pulse->header.downSampledGateCount, config->pulseRingFilterGateCount) && pulse->header.s & RKPulseStatusProcessed) {
            gateCount = MIN(pulse->header.downSampledGateCount, config->pulseRingFilterGateCount);
            paddedGateCount = ((int)ceilf((float)gateCount * sizeof(RKFloat) / engine->coreCount / RKSIMDAlignSize) * engine->coreCount * RKSIMDAlignSize / sizeof(RKFloat));
            RKLog("%s %s   %s", engine->name,
                  RKVariableInString("gateCount", &gateCount, RKValueTypeUInt32),
                  RKVariableInString("paddedGateCount", &paddedGateCount, RKValueTypeUInt32));
            length = engine->coreCount < 2 ? paddedGateCount : paddedGateCount / engine->coreCount;
            origin = 0;
            for (c = 0; c < engine->coreCount; c++) {
                RKPulseRingFilterWorker *worker = &engine->workers[c];
                worker->processOrigin = origin;
                worker->processLength = length;
                worker->outputLength = MIN(gateCount - origin, length);
                origin += length;
                if (engine->verbose) {
                    RKLog("%s C%d %s    %s    %s  %s\n", engine->name, c,
                          RKVariableInString("gateCount", &gateCount, RKValueTypeUInt32),
                          RKVariableInString("origin", &worker->processOrigin, RKValueTypeUInt32),
                          RKVariableInString("length", &worker->processLength, RKValueTypeUInt32),
                          RKVariableInString("outpuLength", &worker->processLength, RKValueTypeUInt32));
                }
            }
        }
        
        // The pulse is considered "inspected" whether it will be skipped / filtered by the designated worker
        pulse->header.s |= RKPulseStatusRingInspected;
        
		#ifdef SHOW_RING_FILTER_DOUBLE_BUFFERING
		for (c = 0; c < engine->coreCount; c++) {
			*workerTaskDone++ = false;
		}
		for (c = 0; c < engine->coreCount; c++) {
			printf("c=%d:", c);
			for (i = 0; i < engine->radarDescription->pulseBufferDepth; i++) {
				printf(" %d", engine->workerTaskDone[i * engine->coreCount + c]);
			}
			printf("\n");
		}
		printf("===\n");
        RKLog("%s k = %d   pulseIndex = %u / %zu\n", engine->name, k, *engine->pulseIndex, pulse->header.i);
		#endif

		// Now we set this pulse to be "not done" and post
		workerTaskDone = engine->workerTaskDone + k * engine->coreCount;
		for (c = 0; c < engine->coreCount; c++) {
			*workerTaskDone++ = false;
			if (engine->useSemaphore) {
				if (sem_post(sem[c])) {
					RKLog("%s Error. Failed in sem_post(), errno = %d\n", engine->name, errno);
				}
			} else {
				engine->workers[c].tic++;
			}
		}

		// Now we check on and catch up with the pulses that are done
        allDone = true;
        while (j != k && allDone) {
            // Decide whether the pulse has been processed by FIR/IIR filter
            workerTaskDone = engine->workerTaskDone + j * engine->coreCount;
            for (c = 0; i < engine->coreCount; c++) {
                allDone &= *workerTaskDone++;
            }
            if (allDone) {
                pulse = RKGetPulseFromBuffer(engine->pulseBuffer, j);
                pulse->header.s |= RKPulseStatusRingFiltered | RKPulseStatusRingProcessed;
                j = RKNextModuloS(j, engine->radarDescription->pulseBufferDepth);
            }
        }
        
        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            RKPulseRingFilterUpdateStatusString(engine);
            if (engine->verbose > 2) {
                RKLog("%s %s\n", engine->name, RKVariableInString("useFilter", &engine->useFilter, RKValueTypeBool));
                for (c = 0; c < engine->coreCount; c++) {
                    RKLog("%s %d %s   %s   %s\n", engine->name, c,
                          RKVariableInString("origin", &engine->workers[c].processOrigin, RKValueTypeUInt32),
                          RKVariableInString("length", &engine->workers[c].processLength, RKValueTypeUInt32),
                          RKVariableInString("output", &engine->workers[c].outputLength, RKValueTypeUInt32));
                }
            }
        }

		engine->tic++;

        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->radarDescription->pulseBufferDepth);
    }

    // Wait for workers to return
    for (c = 0; c < engine->coreCount; c++) {
        RKPulseRingFilterWorker *worker = &engine->workers[c];
        if (engine->useSemaphore) {
            sem_post(worker->sem);
        }
        pthread_join(worker->tid, NULL);
        sem_unlink(worker->semaphoreName);
    }
    
    // Clean up
    free(engine->workerTaskDone);
    
    engine->state ^= RKEngineStateActive;
    return NULL;
}

#pragma mark - Life Cycle

RKPulseRingFilterEngine *RKPulseRingFilterEngineInit(void) {
    RKPulseRingFilterEngine *engine = (RKPulseRingFilterEngine *)malloc(sizeof(RKPulseRingFilterEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a pulse ring filter engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKPulseRingFilterEngine));
    sprintf(engine->name, "%s<PulseRingFilter>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPulseRingFilterEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->useSemaphore = true;
    engine->memoryUsage = sizeof(RKPulseRingFilterEngine);
    pthread_mutex_init(&engine->mutex, NULL);
    return engine;
}

void RKPulseRingFilterEngineFree(RKPulseRingFilterEngine *engine) {
    if (engine->state & RKEngineStateWantActive) {
        RKPulseRingFilterEngineStop(engine);
    }
    pthread_mutex_destroy(&engine->mutex);
    free(engine);
}

#pragma mark - Properties

void RKPulseRingFilterEngineSetVerbose(RKPulseRingFilterEngine *engine, const int verb) {
    engine->verbose = verb;
}

void RKPulseRingFilterEngineSetInputOutputBuffers(RKPulseRingFilterEngine *engine, const RKRadarDesc *desc,
                                                  RKConfig *configBuffer, uint32_t *configIndex,
                                                  RKBuffer pulseBuffer,   uint32_t *pulseIndex) {
    engine->radarDescription  = (RKRadarDesc *)desc;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;

    engine->state |= RKEngineStateProperlyWired;
}

void RKPulseRingFilterEngineSetCoreCount(RKPulseRingFilterEngine *engine, const uint8_t count) {
    if (engine->state & RKEngineStateWantActive) {
        RKLog("%s Error. Core count cannot change when the engine is active.\n", engine->name);
        return;
    }
    engine->coreCount = count;
}

void RKPulseRingFilterEngineSetCoreOrigin(RKPulseRingFilterEngine *engine, const uint8_t origin) {
    if (engine->state & RKEngineStateWantActive) {
        RKLog("%s Error. Core origin cannot change when the engine is active.\n", engine->name);
        return;
    }
    engine->coreOrigin = origin;
}

void RKPulseRingFilterEngineEnableFilter(RKPulseRingFilterEngine *engine) {
    engine->useFilter = true;
    if (engine->state & RKEngineStateActive) {
        RKLog("%s %s   %s\n", engine->name,
              RKVariableInString("filter", engine->filter.name, RKValueTypeString),
              RKVariableInString("useFilter", &engine->useFilter, RKValueTypeBool));
    }
}

void RKPulseRingFilterEngineDisableFilter(RKPulseRingFilterEngine *engine) {
    engine->useFilter = false;
    if (engine->state & RKEngineStateActive) {
        RKLog("%s %s\n", engine->name, RKVariableInString("useFilter", &engine->useFilter, RKValueTypeBool));
    }
}

int RKPulseRingFilterEngineSetFilter(RKPulseRingFilterEngine *engine, RKIIRFilter *filter) {
    memcpy(&engine->filter, filter, sizeof(RKIIRFilter));
    engine->filterId++;
    return RKResultSuccess;
}

#pragma mark - Interactions

int RKPulseRingFilterEngineStart(RKPulseRingFilterEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    if (engine->coreCount == 0) {
        engine->coreCount = 2;
    }
    if (engine->coreOrigin == 0) {
        engine->coreOrigin = 4;
    }
    if (engine->workers != NULL) {
        RKLog("%s Error. workers should be NULL here.\n", engine->name);
    }
    engine->workers = (RKPulseRingFilterWorker *)malloc(engine->coreCount * sizeof(RKPulseRingFilterWorker));
    engine->memoryUsage += engine->coreCount * sizeof(RKPulseRingFilterWorker);
    memset(engine->workers, 0, engine->coreCount * sizeof(RKPulseRingFilterWorker));
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidPulseWatcher, NULL, pulseRingWatcher, engine) != 0) {
        RKLog("%s Error. Failed to start.\n", engine->name);
        return RKResultFailedToStartRingPulseWatcher;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKPulseRingFilterEngineStop(RKPulseRingFilterEngine *engine) {
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
	if (engine->tidPulseWatcher) {
		pthread_join(engine->tidPulseWatcher, NULL);
		engine->tidPulseWatcher = (pthread_t)0;
		free(engine->workers);
		engine->workers = NULL;
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

char *RKPulseRingFilterEngineStatusString(RKPulseRingFilterEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}

void RKPulseRingFilterEngineShowFilterSummary(RKPulseRingFilterEngine *engine) {
    int i, k;
    char *string = (char *)malloc(1024);
    i = sprintf(string, "b = [");
    for (k = 0; k < engine->filter.bLength; k++) {
        i += sprintf(string + i, "%s%.4f", k > 0 ? ", " : "", engine->filter.B[k].i);
    }
    sprintf(string + i, "]");
    RKLog(">%s %s", engine->name, string);
    i = sprintf(string, "a = [");
    for (k = 0; k < engine->filter.aLength; k++) {
        i += sprintf(string + i, "%s%.4f", k > 0 ? ", " : "", engine->filter.A[k].i);
    }
    sprintf(string + i, "]");
    RKLog(">%s %s", engine->name, string);
    free(string);
}

#pragma mark - Interactions

