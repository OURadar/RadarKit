//
//  RKPulseCompression.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulseCompression.h>

// Internal Functions

static void RKPulseCompressionUpdateStatusString(RKPulseCompressionEngine *);
static void *pulseCompressionCore(void *);
static void *pulseWatcher(void *);

#pragma mark - Helper Functions

static void RKPulseCompressionUpdateStatusString(RKPulseCompressionEngine *engine) {
    int i, c;
    char *string = engine->statusBuffer[engine->statusBufferIndex];
    
    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';
    
    // Use RKStatusBarWidth characters to draw a bar
    i = *engine->pulseIndex * RKStatusBarWidth / engine->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'C';
    
    // Engine lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKMaximumStringLength - RKStatusBarWidth, " | %s%02.0f%s |",
                                    rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                                    99.9f * engine->lag,
                                    rkGlobalParameters.showColor ? RKNoColor : "");
    
    RKPulseCompressionWorker *worker;
    
    // Lag from each core
    for (c = 0; c < engine->coreCount; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKMaximumStringLength - i, " %s%02.0f%s",
                      rkGlobalParameters.showColor ? RKColorLag(worker->lag) : "",
                      99.5f * worker->lag,
                      rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Put a separator
    i += snprintf(string + i, RKMaximumStringLength - i, " |");
    // Duty cycle of each core
    for (c = 0; c < engine->coreCount && i < RKMaximumStringLength - 13; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKMaximumStringLength - i, " %s%02.0f%s",
                      rkGlobalParameters.showColor ? RKColorDutyCycle(worker->dutyCycle) : "",
                      99.5f * worker->dutyCycle,
                      rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Almost full count
    i += snprintf(string + i, RKMaximumStringLength - i, " [%d]", engine->almostFull);
    if (i > RKMaximumStringLength - 13) {
        memset(string + i, '#', RKMaximumStringLength - i - 1);
    }
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

#pragma mark - Delegate Workers

static void *pulseCompressionCore(void *_in) {
    RKPulseCompressionWorker *me = (RKPulseCompressionWorker *)_in;
    RKPulseCompressionEngine *engine = me->parentEngine;

    int bound;
    int i, j, k, p;
    struct timeval t0, t1, t2;

    const int c = me->id;
    const int multiplyMethod = 1;

    uint32_t blindGateCount = 0;

    // Find the semaphore
    sem_t *sem = sem_open(me->semaphoreName, O_RDWR);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve semaphore %d\n", c);
        return (void *)RKResultFailedToRetrieveSemaphore;
    };

    // Initiate a variable to store my name
    char name[RKNameLength];
    if (rkGlobalParameters.showColor) {
        pthread_mutex_lock(&engine->coreMutex);
        k = snprintf(name, RKNameLength - 1, "%s", rkGlobalParameters.showColor ? RKGetColor() : "");
        pthread_mutex_unlock(&engine->coreMutex);
    } else {
        k = 0;
    }
    if (engine->coreCount > 9) {
        k += sprintf(name + k, "P%02d", c);
    } else {
        k += sprintf(name + k, "P%d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(name + k, RKNoColor);
    }

#if defined(_GNU_SOURCE)

    // Set my CPU core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(engine->coreOrigin + c, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    pthread_setaffinity_np(me->tid, sizeof(cpu_set_t), &cpuset);

#endif

    RKPulse *pulse = RKGetPulse(engine->pulseBuffer, 0);
    const size_t nfft = 1 << (int)ceilf(log2f((float)MIN(RKGateCount, pulse->header.capacity)));

    // Allocate local resources, use k to keep track of the total allocation
    // Avoid fftwf_malloc() here so that non-avx-enabled libfftw is compatible
    fftwf_complex *in, *out;
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&in, RKSIMDAlignSize, nfft * sizeof(fftwf_complex)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&out, RKSIMDAlignSize, nfft * sizeof(fftwf_complex)))
    if (in == NULL || out == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    size_t mem = 2 * nfft * sizeof(fftwf_complex);
    RKIQZ *zi, *zo;
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&zi, RKSIMDAlignSize, nfft * sizeof(RKFloat)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&zo, RKSIMDAlignSize, nfft * sizeof(RKFloat)))
    if (zi == NULL || zo == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    mem += 2 * nfft * sizeof(RKFloat);
    double *busyPeriods, *fullPeriods;
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&busyPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&fullPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    if (busyPeriods == NULL || fullPeriods == NULL) {
        RKLog("Error. Unable to allocate resources for duty cycle calculation\n");
        return (void *)RKResultFailedToAllocateDutyCycleBuffer;
    }
    mem += 2 * RKWorkerDutyCycleBufferDepth * sizeof(double);
    memset(busyPeriods, 0, RKWorkerDutyCycleBufferDepth * sizeof(double));
    memset(fullPeriods, 0, RKWorkerDutyCycleBufferDepth * sizeof(double));
    double allBusyPeriods = 0.0, allFullPeriods = 0.0;

    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);
    
    // The last index of the pulse buffer
    uint32_t i0 = engine->pulseBufferDepth - engine->coreCount + c;

    // The latest index in the dutyCycle buffer
    int d0 = 0;

    // DFT plan size and plan index in the parent engine
    int planSize = -1, planIndex;

    // Log my initial state
    pthread_mutex_lock(&engine->coreMutex);
    engine->memoryUsage += mem;

    RKLog(">%s %s Started.   mem = %s B   i0 = %s   nfft = %s\n",
          engine->name, name, RKIntegerToCommaStyleString(mem), RKIntegerToCommaStyleString(i0), RKIntegerToCommaStyleString(nfft));

    pthread_mutex_unlock(&engine->coreMutex);

    // Increase the tic once to indicate this processing core is created.
    me->tic++;

    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]
    //
    uint32_t tic = me->tic;

    while (engine->state & RKEngineStateActive) {
        if (engine->useSemaphore) {
            #ifdef DEBUG_IQ
            RKLog(">%s sem_wait()\n", coreName);
            #endif
            if (sem_wait(sem)) {
                RKLog("%s %s Error. Failed in sem_wait(). errno = %d\n", engine->name, name, errno);
            }
        } else {
            while (tic == me->tic && engine->state & RKEngineStateActive) {
                usleep(1000);
            }
            tic = me->tic;
        }
        if (!(engine->state & RKEngineStateActive)) {
            break;
        }

        // Something happened
        gettimeofday(&t1, NULL);

        // Start of getting busy
        i0 = RKNextNModuloS(i0, engine->coreCount, engine->pulseBufferDepth);

        RKPulse *pulse = RKGetPulse(engine->pulseBuffer, i0);

        #ifdef DEBUG_IQ
        RKLog(">%s i0 = %d  stat = %d\n", coreName, i0, input->header.s);
        #endif

        // Filter group id
        const int gid = engine->filterGid[i0];
//        printf("pulse i = %u   gid = %d\n", (uint32_t)pulse->header.i, gid);

        // Now we process / skip
        if (gid < 0 || gid >= engine->filterGroupCount) {
            pulse->parameters.planSizes[0][0] = 0;
            pulse->parameters.planSizes[1][0] = 0;
            pulse->parameters.filterCounts[0] = 0;
            pulse->parameters.filterCounts[1] = 0;
            pulse->header.s |= RKPulseStatusSkipped | RKPulseStatusProcessed;
            if (engine->verbose > 1) {
            RKLog("%s pulse skipped. header->i = %d   gid = %d\n", engine->name, pulse->header.i, gid);
            }
        } else {
            // Do some work with this pulse
            // DFT of the raw data is stored in *in
            // DFT of the filter is stored in *out
            // Their product is stored in *out using in-place multiplication: out[i] = out[i] * in[i]
            // Then, the inverse DFT is performed to get out back to time domain, which is the compressed pulse

            // Process each polarization separately and indepently
            for (p = 0; p < 2; p++) {
                // Go through all the filters in this filter group
                blindGateCount = 0;
                for (j = 0; j < engine->filterCounts[gid]; j++) {
                    // Get the plan index and size from parent engine
                    planIndex = engine->planIndices[i0][j];
                    planSize = engine->planSizes[planIndex];
                    blindGateCount += engine->filterAnchors[gid][j].length;

                    // Copy and convert the samples
                    RKInt16C *X = RKGetInt16CDataFromPulse(pulse, p);
                    X += engine->filterAnchors[gid][j].origin;
                    bound = MIN(engine->filterAnchors[gid][j].maxDataLength, pulse->header.gateCount - engine->filterAnchors[gid][j].origin);
                    for (k = 0; k < bound; k++) {
                        in[k][0] = (RKFloat)X->i;
                        in[k][1] = (RKFloat)X++->q;
                    }
                    // Zero pad the input; a filter is always zero-padded in the setter function.
                    if (planSize > k) {
                        memset(in[k], 0, (planSize - k) * sizeof(fftwf_complex));
                    }

                    fftwf_execute_dft(engine->planForwardInPlace[planIndex], in, in);

                    //printf("dft(in) =\n"); RKPulseCompressionShowBuffer(in, 8);

                    fftwf_execute_dft(engine->planForwardOutPlace[planIndex], (fftwf_complex *)engine->filters[gid][j], out);

                    //printf("dft(filt[%d][%d]) =\n", gid, j); RKPulseCompressionShowBuffer(out, 8);

                    if (multiplyMethod == 1) {
                        // In-place SIMD multiplication using the interleaved format
                        RKSIMD_iymul((RKComplex *)in, (RKComplex *)out, planSize);
                    } else if (multiplyMethod == 2) {
                        // Deinterleave the RKComplex data into RKIQZ format, multiply using SIMD, then interleave the result back to RKComplex format
                        RKSIMD_Complex2IQZ((RKComplex *)in, zi, planSize);
                        RKSIMD_Complex2IQZ((RKComplex *)out, zo, planSize);
                        RKSIMD_izmul(zi, zo, planSize, true);
                        RKSIMD_IQZ2Complex(zo, (RKComplex *)out, planSize);
                    } else {
                        // Regular multiplication with compiler optimization -Os
                        RKSIMD_iymul_reg((RKComplex *)in, (RKComplex *)out, planSize);
                    }

                    //printf("in * out =\n"); RKPulseCompressionShowBuffer(out, 8);

                    fftwf_execute_dft(engine->planBackwardInPlace[planIndex], out, out);

                    //printf("idft(out) =\n"); RKPulseCompressionShowBuffer(out, 8);

                    // Scaling due to a net gain of planSize from forward + backward DFT, plus the waveform gain
                    RKSIMD_iyscl((RKComplex *)out, 1.0f / planSize, planSize);
                    //RKSIMD_iyscl((RKComplex *)out, 1.0f / planSize / sqrtf(engine->filterAnchors[gid][j].gain), planSize);

                    //printf("idft(out) =\n"); RKPulseCompressionShowBuffer(out, 8);

                    RKComplex *Y = RKGetComplexDataFromPulse(pulse, p);
                    RKIQZ Z = RKGetSplitComplexDataFromPulse(pulse, p);
                    Y += engine->filterAnchors[gid][j].origin;
                    Z.i += engine->filterAnchors[gid][j].origin;
                    Z.q += engine->filterAnchors[gid][j].origin;
                    bound = MIN(pulse->header.gateCount - engine->filterAnchors[gid][j].origin, engine->filterAnchors[gid][j].maxDataLength);
                    for (i = 0; i < bound; i++) {
                        Y->i = out[i][0];
                        Y++->q = out[i][1];
                        *Z.i++ = out[i][0];
                        *Z.q++ = out[i][1];
                    }

                    //Y = RKGetComplexDataFromPulse(pulse, p);
                    //printf("Y real =\n"); RKPulseCompressionShowBuffer((fftwf_complex *)Y, 8);

                    // Copy over the parameters used
                    pulse->parameters.planIndices[p][j] = planIndex;
                    pulse->parameters.planSizes[p][j] = planSize;
                } // filterCount
                pulse->parameters.filterCounts[p] = j;
            } // p - polarization
            pulse->header.gateCount = blindGateCount;
            pulse->header.s |= RKPulseStatusCompressed | RKPulseStatusProcessed;
        }
        // Record down the latest processed pulse index
        me->pid = i0;
        me->lag = fmodf((float)(*engine->pulseIndex + engine->pulseBufferDepth - me->pid) / engine->pulseBufferDepth, 1.0f);

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
        RKLog("%s %s Freeing reources ...\n", engine->name, name);
    }

    free(zi);
    free(zo);
    free(in);
    free(out);
    free(busyPeriods);
    free(fullPeriods);

    RKLog(">%s %s Stopped.\n", engine->name, name);
    
    return NULL;
}

static void *pulseWatcher(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    int c, i, j, k;

    sem_t *sem[engine->coreCount];

    bool found;
    int gid;
    int planSize;
    int planIndex = 0;
    int skipCounter = 0;
    float lag;
    struct timeval t0, t1;

    if (engine->coreCount == 0) {
        RKLog("Error. No processing core?\n");
        return NULL;
    }
    
    // The beginning of the buffer is a pulse, it has the capacity info
    RKPulse *pulse = RKGetPulse(engine->pulseBuffer, 0);
    RKPulse *pulseToSkip;
    
    // Maximum plan size
    planSize = 1 << (int)ceilf(log2f((float)MIN(RKGateCount, pulse->header.capacity)));
    bool exportWisdom = false;
    const char wisdomFile[] = "radarkit-fft-wisdom";

    // FFTW's memory allocation and plan initialization are not thread safe but others are.
    fftwf_complex *in, *out;
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&in, RKSIMDAlignSize, planSize * sizeof(fftwf_complex)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&out, RKSIMDAlignSize, planSize * sizeof(fftwf_complex)))
    engine->memoryUsage += 2 * pulse->header.capacity * sizeof(fftwf_complex);

    if (RKFilenameExists(wisdomFile)) {
        RKLog(">%s Loading DFT wisdom ...\n", engine->name);
        fftwf_import_wisdom_from_filename(wisdomFile);
    } else {
        RKLog(">%s DFT wisdom file not found.\n", engine->name);
        exportWisdom = true;
    }

    // Go through the maximum plan size and divide it by two a few times
    for (j = 0; j < 3; j++) {
        RKLog(">%s Pre-allocate FFTW resources for plan[%d] @ nfft = %s\n", engine->name, planIndex, RKIntegerToCommaStyleString(planSize));
        engine->planForwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, in, FFTW_FORWARD, FFTW_MEASURE);
        engine->planForwardOutPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, out, FFTW_FORWARD, FFTW_MEASURE);
        engine->planBackwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, out, out, FFTW_BACKWARD, FFTW_MEASURE);
        //fftwf_print_plan(engine->planForwardInPlace[planIndex]);
        engine->planSizes[planIndex++] = planSize;
        engine->planCount++;
        planSize /= 2;
    }

    // Change the state to active so all the processing cores stay in the busy loop
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    // Spin off N workers to process I/Q pulses
    memset(sem, 0, engine->coreCount * sizeof(sem_t *));
    for (c = 0; c < engine->coreCount; c++) {
        RKPulseCompressionWorker *worker = &engine->workers[c];
        snprintf(worker->semaphoreName, 16, "rk-iq-%03d", c);
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
        worker->parentEngine = engine;
        if (engine->verbose > 1) {
            RKLog(">%s %s @ %p\n", engine->name, worker->semaphoreName, worker->sem);
        }
        if (pthread_create(&worker->tid, NULL, pulseCompressionCore, worker) != 0) {
            RKLog(">%s Error. Failed to start a compression core.\n", engine->name);
            return (void *)RKResultFailedToStartCompressionCore;
        }
    }

    // Wait for the workers to increase the tic count once
    // Using sem_wait here could cause a stolen post within the worker
    // Tested and removed on 9/29/2016
    engine->state |= RKEngineStateSleep0;
    for (c = 0; c < engine->coreCount; c++) {
        while (engine->workers[c].tic == 0) {
            usleep(1000);
        }
    }
    engine->state ^= RKEngineStateSleep0;

    // Increase the tic once to indicate the engine is ready
    engine->tic++;

    RKLog("%s Started.   mem = %s B   pulseIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);

    gettimeofday(&t1, 0); t1.tv_sec -= 1;

    // Here comes the busy loop
    // i  anonymous
    // j  filter index
    k = 0;   // pulse index
    c = 0;   // core index
    int s = 0;
    while (engine->state & RKEngineStateActive) {
        // The pulse
        pulse = RKGetPulse(engine->pulseBuffer, k);

        // Wait until the engine index move to the next one for storage, which is also the time pulse has data.
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->pulseIndex && engine->state & RKEngineStateActive) {
            usleep(200);
            if (++s % 1000 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.0002f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // Wait until the pulse has position so that this engine won't compete with the tagger to set the status.
        s = 0;
        while (!(pulse->header.s & RKPulseStatusHasIQData) && engine->state & RKEngineStateActive) {
            usleep(200);
            if (++s % 1000 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.0002f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateActive)) {
            break;
        }

        // Lag of the engine
        engine->lag = fmodf(((float)*engine->pulseIndex + engine->pulseBufferDepth - k) / engine->pulseBufferDepth, 1.0f);

        // Assess the lag of the workers
        lag = engine->workers[0].lag;
        for (i = 1; i < engine->coreCount; i++) {
            lag = MAX(lag, engine->workers[i].lag);
        }
        if (skipCounter == 0 && lag > 0.9f) {
            engine->almostFull++;
            skipCounter = engine->pulseBufferDepth / 10;
            RKLog("%s Warning. Projected an I/Q Buffer overflow.\n", engine->name);
            i = *engine->pulseIndex;
            do {
                i = RKPreviousModuloS(i, engine->pulseBufferDepth);
                engine->filterGid[i] = -1;
                pulseToSkip = RKGetPulse(engine->pulseBuffer, i);
            } while (!(pulseToSkip->header.s & RKPulseStatusProcessed));
        }

        // Skip processing if the buffer is getting full (avoid hitting SEM_VALUE_MAX)
        if (skipCounter > 0) {
            engine->filterGid[k] = -1;
            engine->planIndices[k][0] = 0;
            if (--skipCounter == 0) {
                RKLog(">%s Info. Skipped a chunk.\n", engine->name);
            }
        } else {
            // Compute the filter group id to use
            engine->filterGid[k] = (gid = pulse->header.i % engine->filterGroupCount);
            //printf("pulse->header.i = %d   gid = %d\n", (uint32_t)pulse->header.i, gid);

            // Find the right plan; create it if it does not exist
            for (j = 0; j < engine->filterCounts[gid]; j++) {
                planSize = 1 << (int)ceilf(log2f((float)MIN(pulse->header.gateCount, engine->filterAnchors[gid][j].maxDataLength)));
                found = false;
                i = engine->planCount;
                while (i > 0) {
                    i--;
                    if (planSize == engine->planSizes[i]) {
                        planIndex = i;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    RKLog("%s preparing a new FFT plan of size %d ...  gid = %d   planCount = %d\n", engine->name, planSize, gid, engine->planCount);
                    if (engine->planCount >= RKPulseCompressionDFTPlanCount) {
                        RKLog("%s Error. Unable to create another DFT plan.  engine->planCount = %d\n", engine->name, engine->planCount);
                        exit(EXIT_FAILURE);
                    }
                    planIndex = engine->planCount;
                    engine->planForwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, in, FFTW_FORWARD, FFTW_MEASURE);
                    engine->planForwardOutPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, out, FFTW_FORWARD, FFTW_MEASURE);
                    engine->planBackwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, out, out, FFTW_BACKWARD, FFTW_MEASURE);
                    engine->planSizes[planIndex] = planSize;
                    engine->planCount++;
                    RKLog(">%s k = %d   j = %d  planIndex = %d\n", engine->name, k, j, planIndex);
                    exportWisdom = true;
                }
                engine->planIndices[k][j] = planIndex;
            }
        }

        // The pulse is considered "inspected" whether it will be skipped / compressed by the desingated worker
        pulse->header.s |= RKPulseStatusInspected;

        // Now we post
        #ifdef DEBUG_IQ
        RKLog("%s posting core-%d for pulse %d gate %d\n", engine->name, c, k, engine->pulses[k].header.gateCount);
        #endif
        if (engine->useSemaphore) {
            if (sem_post(sem[c])) {
                RKLog("Error. Failed in sem_post(), errno = %d\n", errno);
            }
        } else {
            engine->workers[c].tic++;
        }
        c = RKNextModuloS(c, engine->coreCount);
        
        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            RKPulseCompressionUpdateStatusString(engine);
        }
    
        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->pulseBufferDepth);
    }

    // Wait for workers to return
    for (c = 0; c < engine->coreCount; c++) {
        RKPulseCompressionWorker *worker = &engine->workers[c];
        if (engine->useSemaphore) {
            sem_post(worker->sem);
        }
        pthread_join(worker->tid, NULL);
        sem_unlink(worker->semaphoreName);
    }

    // Export wisdom
    if (exportWisdom) {
        RKLog("%s Saving DFT wisdom ...\n", engine->name);
        fftwf_export_wisdom_to_filename(wisdomFile);
    }

    // Destroy all the DFT plans
    for (k = 0; k < engine->planCount; k++) {
        fftwf_destroy_plan(engine->planForwardInPlace[k]);
        fftwf_destroy_plan(engine->planForwardOutPlace[k]);
        fftwf_destroy_plan(engine->planBackwardInPlace[k]);
    }
    free(in);
    free(out);

    return NULL;
}

#pragma mark - Life Cycle

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)malloc(sizeof(RKPulseCompressionEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a pulse compression engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKPulseCompressionEngine));
    sprintf(engine->name, "%s<PulseCompressor>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPulseCompressionEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->useSemaphore = true;
    engine->memoryUsage = sizeof(RKPulseCompressionEngine);
    pthread_mutex_init(&engine->coreMutex, NULL);
    return engine;
}

void RKPulseCompressionEngineFree(RKPulseCompressionEngine *engine) {
    if (engine->state & RKEngineStateActive) {
        RKPulseCompressionEngineStop(engine);
    }
    for (int i = 0; i < engine->filterGroupCount; i++) {
        for (int j = 0; j < engine->filterCounts[i]; j++) {
            if (engine->filters[i][j] != NULL) {
                free(engine->filters[i][j]);
            }
        }
    }
    pthread_mutex_destroy(&engine->coreMutex);
    free(engine->filterGid);
    free(engine->planIndices);
    free(engine);
}

#pragma mark - Properties

void RKPulseCompressionEngineSetVerbose(RKPulseCompressionEngine *engine, const int verb) {
    engine->verbose = verb;
}

//
// RKPulseCompressionEngineSetInputOutputBuffers
//
// Input:
// engine - the pulse compression engine
// buffer - the raw data buffer
// index - the reference index watch, *index is the latest reading in *pulses
// size - number of slots in *pulses
//
void RKPulseCompressionEngineSetInputOutputBuffers(RKPulseCompressionEngine *engine,
                                                   RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                                   RKBuffer pulseBuffer,   uint32_t *pulseIndex,  const uint32_t pulseBufferDepth) {
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->configBufferDepth = configBufferDepth;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;
    engine->pulseBufferDepth  = pulseBufferDepth;

    if (engine->filterGid != NULL) {
        free(engine->filterGid);
    }
    engine->filterGid = (int *)malloc(pulseBufferDepth * sizeof(int));
    if (engine->filterGid == NULL) {
        RKLog("%s Error. Unable to allocate filterGid.\n", engine->name);
        exit(EXIT_FAILURE);
    }

    if (engine->planIndices != NULL) {
        free(engine->planIndices);
    }
    engine->planIndices = (RKPulseCompressionPlanIndex *)malloc(pulseBufferDepth * sizeof(RKPulseCompressionPlanIndex));
    engine->memoryUsage += pulseBufferDepth * sizeof(RKPulseCompressionPlanIndex);
    if (engine->planIndices == NULL) {
        RKLog("%s Error. Unable to allocate planIndices.\n", engine->name);
        exit(EXIT_FAILURE);
    }
    engine->state |= RKEngineStateProperlyWired;
}

void RKPulseCompressionEngineSetCoreCount(RKPulseCompressionEngine *engine, const uint8_t count) {
    if (engine->state & RKEngineStateActive) {
        RKLog("%s Error. Core count cannot change when the engine is active.\n", engine->name);
        return;
    }
    engine->coreCount = count;
}

void RKPulseCompressionEngineSetCoreOrigin(RKPulseCompressionEngine *engine, const uint8_t origin) {
    if (engine->state & RKEngineStateActive) {
        RKLog("%s Error. Core origin cannot change when the engine is active.\n", engine->name);
        return;
    }
    engine->coreOrigin = origin;
}

int RKPulseCompressionResetFilters(RKPulseCompressionEngine *engine) {
    // If engine->filterGroupCount is set to 0, gid may be undefined segmentation fault
    engine->filterGroupCount = 1;
    engine->filterCounts[0] = 1;
    for (int k = 1; k < RKMaxFilterCount; k++) {
        engine->filterCounts[k] = 0;
    }
    return RKResultNoError;
}

int RKPulseCompressionSetFilterCountOfGroup(RKPulseCompressionEngine *engine, const int group, const int count) {
    engine->filterCounts[group] = count;
    return RKResultNoError;
}

int RKPulseCompressionSetFilterGroupCount(RKPulseCompressionEngine *engine, const int groupCount) {
    engine->filterGroupCount = groupCount;
    return RKResultNoError;
}

int RKPulseCompressionSetFilter(RKPulseCompressionEngine *engine, const RKComplex *filter, const RKFilterAnchor anchor, const int group, const int index) {
    if (group >= RKMaxFilterGroups) {
        RKLog("Error. Group %d is invalid.\n", group);
        return RKResultFailedToSetFilter;
    }
    if (engine->pulseBuffer == NULL) {
        RKLog("Warning. Pulse buffer has not been set.\n");
        return RKResultNoPulseBuffer;
    }
    if (engine->filters[group][index] != NULL) {
        free(engine->filters[group][index]);
    }
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    size_t nfft = 1 << (int)ceilf(log2f((float)MIN(pulse->header.capacity, anchor.maxDataLength)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&engine->filters[group][index], RKSIMDAlignSize, nfft * sizeof(RKComplex)))
    memset(engine->filters[group][index], 0, nfft * sizeof(RKComplex));
    memcpy(engine->filters[group][index], filter, anchor.length * sizeof(RKComplex));
    memcpy(&engine->filterAnchors[group][index], &anchor, sizeof(RKFilterAnchor));
    engine->filterAnchors[group][index].length = (uint32_t)MIN(nfft, anchor.length);
    engine->filterGroupCount = MAX(engine->filterGroupCount, group + 1);
    engine->filterCounts[group] = MAX(engine->filterCounts[group], index + 1);
    return RKResultNoError;
}

int RKPulseCompressionSetFilterToImpulse(RKPulseCompressionEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}};
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    if (pulse == NULL) {
        RKLog("%s Error. RKPulseCompressionSetFilterToImpulse() should be called after pulse buffer is set\n", engine->name);
        return RKResultNoPulseBuffer;
    }
    RKPulseCompressionResetFilters(engine);
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    anchor.length = sizeof(filter) / sizeof(RKComplex);
    anchor.maxDataLength = pulse->header.capacity;
    anchor.subCarrierFrequency = 0.0f;
    return RKPulseCompressionSetFilter(engine, filter, anchor, 0, 0);
}

int RKPulseCompressionSetFilterTo121(RKPulseCompressionEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}, {2.0f, 0.0f}, {1.0f, 0.0f}};
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    if (pulse == NULL) {
        RKLog("%s Error. RKPulseCompressionSetFilterTo*() should be called after pulse buffer is set\n", engine->name);
        return RKResultNoPulseBuffer;
    }
    RKPulseCompressionResetFilters(engine);
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    anchor.length = sizeof(filter) / sizeof(RKComplex);
    anchor.maxDataLength = pulse->header.capacity;
    anchor.subCarrierFrequency = 0.0f;
    return RKPulseCompressionSetFilter(engine, filter, anchor, 0, 0);
}

int RKPulseCompressionSetFilterTo11(RKPulseCompressionEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}, {1.0f, 0.0f}};
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    if (pulse == NULL) {
        RKLog("%s Error. RKPulseCompressionSetFilterTo*() should be called after pulse buffer is set\n", engine->name);
        return RKResultNoPulseBuffer;
    }
    RKPulseCompressionResetFilters(engine);
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    anchor.length = sizeof(filter) / sizeof(RKComplex);
    anchor.maxDataLength = pulse->header.capacity;
    anchor.subCarrierFrequency = 0.0f;
    return RKPulseCompressionSetFilter(engine, filter, anchor, 0, 0);
}

#pragma mark - Interactions

int RKPulseCompressionEngineStart(RKPulseCompressionEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    if (engine->filterGroupCount == 0) {
        // Set to default impulse as matched filter
        RKPulseCompressionSetFilterToImpulse(engine);
    }
    if (engine->coreCount == 0) {
        engine->coreCount = 8;
    }
    if (engine->coreOrigin == 0) {
        engine->coreOrigin = 1;
    }
    if (engine->workers != NULL) {
        RKLog("%s Error. workers should be NULL here.\n", engine->name);
    }
    engine->workers = (RKPulseCompressionWorker *)malloc(engine->coreCount * sizeof(RKPulseCompressionWorker));
    engine->memoryUsage += engine->coreCount * sizeof(RKPulseCompressionWorker);
    memset(engine->workers, 0, engine->coreCount * sizeof(RKPulseCompressionWorker));
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidPulseWatcher, NULL, pulseWatcher, engine) != 0) {
        RKLog("%s Error. Failed to start.\n", engine->name);
        return RKResultFailedToStartPulseWatcher;
    }
    while (engine->tic == 0) {
        usleep(1000);
    }

    return RKResultNoError;
}

int RKPulseCompressionEngineStop(RKPulseCompressionEngine *engine) {
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
    pthread_join(engine->tidPulseWatcher, NULL);
    free(engine->workers);
    engine->workers = NULL;
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->state);
    }
    return RKResultNoError;
}

char *RKPulseCompressionEngineStatusString(RKPulseCompressionEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}

void RKPulseCompressionFilterSummary(RKPulseCompressionEngine *engine) {
    RKLog("%s I/Q filter set.  group count = %d\n", engine->name, engine->filterGroupCount);
    for (int i = 0; i < engine->filterGroupCount; i += 2) {
        for (int j = 0; j < engine->filterCounts[i]; j++) {
            RKLog(">%s  - Filter[%2d][%d] @ {%s, %s, %s / %02d, %+.3f, %s}\n",
                  engine->name, i, j,
                  RKIntegerToCommaStyleString(engine->filterAnchors[i][j].origin),
                  RKIntegerToCommaStyleString(engine->filterAnchors[i][j].length),
                  RKIntegerToCommaStyleString(engine->filterAnchors[i][j].maxDataLength),
                  engine->filterAnchors[i][j].name,
                  engine->filterAnchors[i][j].subCarrierFrequency,
                  RKFloatToCommaStyleString(engine->filterAnchors[i][j].gain));
        }
    }
}

void RKPulseCompressionShowBuffer(fftwf_complex *in, const int n) {
    for (int k = 0; k < n; k++) {
        printf("    %6.2f %s %6.2fi\n", in[k][0], in[k][1] < 0 ? "-" : "+", fabsf(in[k][1]));
    }
}
