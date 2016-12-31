//
//  RKPulseCompression.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulseCompression.h>

// Internal functions

int workerThreadId(RKPulseCompressionEngine *engine);
void RKPulseCompressionShowBuffer(fftwf_complex *in, const int n);
void *pulseCompressionCore(void *in);

#pragma mark -

// Implementations

int workerThreadId(RKPulseCompressionEngine *engine) {
    int i;
    pthread_t id = pthread_self();
    for (i = 0; i < engine->coreCount; i++) {
        if (pthread_equal(id, engine->workers[i].tid) == 0) {
            return i;
        }
    }
    return -1;
}

void RKPulseCompressionShowBuffer(fftwf_complex *in, const int n) {
    for (int k = 0; k < n; k++) {
        printf("    %6.2fd %s %6.2fdi\n", in[k][0], in[k][1] < 0 ? "-" : "+", fabsf(in[k][1]));
    }
}

void *pulseCompressionCore(void *_in) {
    RKPulseCompressionWorker *me = (RKPulseCompressionWorker *)_in;
    RKPulseCompressionEngine *engine = me->parentEngine;

    int bound;
    int i, j, k, p;
    struct timeval t0, t1, t2;

    // Find the thread Id
    i = workerThreadId(engine);
    if (i < 0) {
        i = me->id;
        RKLog("Warning. Unable to find my thread ID. Assume %d\n", me->id);
    } else if (engine->verbose > 1) {
        RKLog("Info. Thread ID %d = %d okay.\n", me->id, i);
    }
    const int c = me->id;

    const int multiplyMethod = 1;

    // Find the semaphore
    sem_t *sem = sem_open(me->semaphoreName, O_RDWR);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve semaphore %d\n", c);
        return (void *)RKResultFailedToRetrieveSemaphore;
    };

    // Initiate a variable to store my name
    char name[20];
    if (rkGlobalParameters.showColor) {
        k = sprintf(name, "\033[3%dm", c % 7 + 1);
    } else {
        k = 0;
    }
    if (engine->coreCount > 9) {
        k += sprintf(name + k, "P%02d", c);
    } else {
        k += sprintf(name + k, "P%d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(name + k, "\033[0m");
    }

#if defined(_GNU_SOURCE)

    // Set my CPU core
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(c, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    pthread_setaffinity_np(me->tid, sizeof(cpu_set_t), &cpuset);

#endif

    RKPulse *pulse = RKGetPulse(engine->buffer, 0);
    const size_t nfft = MIN(RKGateCount, pulse->header.capacity);

    // Allocate local resources, use k to keep track of the total allocation
    // Avoid fftwf_malloc() here so that non-avx-enabled libfftw is compatible
    fftwf_complex *in, *out;
    posix_memalign((void **)&in, RKSIMDAlignSize, nfft * sizeof(fftwf_complex));
    posix_memalign((void **)&out, RKSIMDAlignSize, nfft * sizeof(fftwf_complex));
    if (in == NULL || out == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    size_t mem = 2 * nfft * sizeof(fftwf_complex);
    RKIQZ *zi, *zo;
    posix_memalign((void **)&zi, RKSIMDAlignSize, nfft * sizeof(RKFloat));
    posix_memalign((void **)&zo, RKSIMDAlignSize, nfft * sizeof(RKFloat));
    if (zi == NULL || zo == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    mem += 2 * nfft * sizeof(RKFloat);
    double *busyPeriods, *fullPeriods;
    posix_memalign((void **)&busyPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferSize * sizeof(double));
    posix_memalign((void **)&fullPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferSize * sizeof(double));
    if (busyPeriods == NULL || fullPeriods == NULL) {
        RKLog("Error. Unable to allocate resources for duty cycle calculation\n");
        return (void *)RKResultFailedToAllocateDutyCycleBuffer;
    }
    mem += 2 * RKWorkerDutyCycleBufferSize * sizeof(double);
    memset(busyPeriods, 0, RKWorkerDutyCycleBufferSize * sizeof(double));
    memset(fullPeriods, 0, RKWorkerDutyCycleBufferSize * sizeof(double));
    double allBusyPeriods = 0.0, allFullPeriods = 0.0;

    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);
    
    // The last index of the pulse buffer
    uint32_t i0 = engine->size - engine->coreCount + c;

    // The latest index in the dutyCycle buffer
    int d0 = 0;

    // DFT plan size and plan index in the parent engine
    int planSize = -1, planIndex;

    // Log my initial state
    if (engine->verbose) {
        pthread_mutex_lock(&engine->coreMutex);
        engine->memoryUsage += mem;
        if (engine->verbose) {
            RKLog(">%s started.   i0 = %d   mem = %s B   tic = %d\n", name, i0, RKIntegerToCommaStyleString(mem), me->tic);
        }
        pthread_mutex_unlock(&engine->coreMutex);
    }

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

    while (engine->state == RKPulseCompressionEngineStateActive) {
        if (engine->useSemaphore) {
            #ifdef DEBUG_IQ
            RKLog(">%s sem_wait()\n", coreName);
            #endif
            if (sem_wait(sem)) {
                RKLog("Error. Failed in sem_wait(). errno = %d\n", errno);
            }
        } else {
            while (tic == me->tic && engine->state == RKPulseCompressionEngineStateActive) {
                usleep(1000);
            }
            tic = me->tic;
        }
        if (engine->state != RKPulseCompressionEngineStateActive) {
            break;
        }

        // Something happened
        gettimeofday(&t1, NULL);

        // Start of getting busy
        i0 = RKNextNModuloS(i0, engine->coreCount, engine->size);
        me->lag = fmodf((float)(*engine->index + engine->size - me->pid) / engine->size, 1.0f);

        RKPulse *pulse = RKGetPulse(engine->buffer, i0);

        #ifdef DEBUG_IQ
        RKLog(">%s i0 = %d  stat = %d\n", coreName, i0, input->header.s);
        #endif

        // Filter group id
        const int gid = engine->filterGid[i0];

        // Now we process / skip
        if (gid < 0) {
            pulse->parameters.planSizes[0][0] = 0;
            pulse->parameters.planSizes[1][0] = 0;
            pulse->parameters.filterCounts[0] = 0;
            pulse->parameters.filterCounts[1] = 0;
            pulse->header.s |= RKPulseStatusSkipped | RKPulseStatusProcessed;
        } else {
            // Do some work with this pulse
            // DFT of the raw data is stored in *in
            // DFT of the filter is stored in *out
            // Their product is stored in *out using in-place multiplication: out[i] = out[i] * in[i]
            // Then, the inverse DFT is performed to get out back to time domain, which is the compressed pulse

            // Process each polarization separately and indepently
            for (p = 0; p < 2; p++) {
                // Go through all the filters in this fitler group
                for (j = 0; j < engine->filterCounts[gid]; j++) {
                    // Get the plan index and size from parent engine
                    planIndex = engine->planIndices[i0][j];
                    planSize = engine->planSizes[planIndex];

                    // Copy and convert the samples
                    RKInt16C *X = RKGetInt16DataFromPulse(pulse, p);
                    X += engine->anchors[gid][j].origin;
                    bound = MIN(pulse->header.gateCount, pulse->header.capacity - engine->anchors[gid][j].origin);
                    for (k = 0; k < bound; k++) {
                        in[k][0] = (RKFloat)X->i;
                        in[k][1] = (RKFloat)X++->q;
                    }
                    // Zero pad the input; a filter is always zero-padded in the setter function.
                    memset(in[k], 0, (planSize - k) * sizeof(fftwf_complex));

                    fftwf_execute_dft(engine->planForwardInPlace[planIndex], in, in);

                    //printf("dft(in) =\n"); RKPulseCompressionShowBuffer(in, 8);

                    fftwf_execute_dft(engine->planForwardOutPlace[planIndex], (fftwf_complex *)engine->filters[gid][j], out);

                    //printf("dft(filt) =\n"); RKPulseCompressionShowBuffer(in, 8);

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

                    // Scaling due to a net gain of planSize from forward + backward DFT
                    RKSIMD_iyscl((RKComplex *)out, 1.0f / planSize, planSize);

                    //printf("idft(out) =\n"); RKPulseCompressionShowBuffer(out, 8);

                    RKComplex *Y = RKGetComplexDataFromPulse(pulse, p);
                    RKIQZ Z = RKGetSplitComplexDataFromPulse(pulse, p);
                    Y += engine->anchors[gid][j].origin;
                    Z.i += engine->anchors[gid][j].origin;
                    Z.q += engine->anchors[gid][j].origin;
                    bound = MIN(pulse->header.gateCount - engine->anchors[gid][j].length, engine->anchors[gid][j].maxDataLength);
                    for (i = 0; i < bound; i++) {
                        Y->i = out[i][0];
                        Y++->q = out[i][1];
                        *Z.i++ = out[i][0];
                        *Z.q++ = out[i][1];
                    }

                    // Copy over the parameters used
                    pulse->parameters.planIndices[p][j] = planIndex;
                    pulse->parameters.planSizes[p][j] = planSize;
                } // filterCount
                pulse->parameters.filterCounts[p] = j;
            } // p - polarization
            pulse->header.s |= RKPulseStatusCompressed | RKPulseStatusProcessed;
        }
        // Record down the latest processed pulse index
        me->pid = i0;

        // Done processing, get the time
        gettimeofday(&t0, NULL);

        // Drop the oldest reading, replace it, and add to the calculation
        allBusyPeriods -= busyPeriods[d0];
        allFullPeriods -= fullPeriods[d0];
        busyPeriods[d0] = RKTimevalDiff(t0, t1);
        fullPeriods[d0] = RKTimevalDiff(t0, t2);
        allBusyPeriods += busyPeriods[d0];
        allFullPeriods += fullPeriods[d0];
        d0 = RKNextModuloS(d0, RKWorkerDutyCycleBufferSize);
        me->dutyCycle = allBusyPeriods / allFullPeriods;

        t2 = t0;
    }

    // Clean up
    free(zi);
    free(zo);
    free(in);
    free(out);
    free(busyPeriods);
    free(fullPeriods);

    if (engine->verbose) {
        RKLog(">%s ended.\n", name);
    }
    
    return NULL;
}

void *pulseWatcher(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    int c, i, j, k;

    sem_t *sem[engine->coreCount];

    bool found;
    int gid;
    int planSize;
    int planIndex = 0;
    int skipCounter = 0;
    float lag;

    // The beginning of the buffer is a pulse, it has the capacity info
    RKPulse *pulse = (RKPulse *)engine->buffer;

    // FFTW's memory allocation and plan initialization are not thread safe but others are.
    fftwf_complex *in, *out;
    posix_memalign((void **)&in, RKSIMDAlignSize, pulse->header.capacity * sizeof(fftwf_complex));
    posix_memalign((void **)&out, RKSIMDAlignSize, pulse->header.capacity * sizeof(fftwf_complex));
    engine->memoryUsage += 2 * pulse->header.capacity * sizeof(fftwf_complex);

    // Maximum plan size
    planSize = 1 << (int)ceilf(log2f((float)pulse->header.capacity));
    bool exportWisdom = false;
    const char wisdomFile[] = "fft-wisdom";

    if (RKFilenameExists(wisdomFile)) {
        if (engine->verbose) {
            RKLog(">Loading DFT wisdom ...\n");
        }
        fftwf_import_wisdom_from_filename(wisdomFile);
    } else {
        if (engine->verbose) {
            RKLog(">DFT wisdom file not found.\n");
        }
        exportWisdom = true;
    }

    // Go through the maximum plan size and divide it by two a few times
    for (j = 0; j < 3; j++) {
        if (engine->verbose) {
            RKLog(">Pre-allocate FFTW resources for plan[%d] @ nfft = %s\n", planIndex, RKIntegerToCommaStyleString(planSize));
        }
        engine->planForwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, in, FFTW_FORWARD, FFTW_MEASURE);
        engine->planForwardOutPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, out, FFTW_FORWARD, FFTW_MEASURE);
        engine->planBackwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, out, out, FFTW_BACKWARD, FFTW_MEASURE);
        //fftwf_print_plan(engine->planForwardInPlace[planIndex]);
        engine->planSizes[planIndex++] = planSize;
        engine->planCount++;
        planSize /= 2;
    }

    // Change the state to active so all the processing cores stay in the busy loop
    engine->state = RKPulseCompressionEngineStateActive;
    
    // Spin off N workers to process I/Q pulses
    for (c = 0; c < engine->coreCount; c++) {
        RKPulseCompressionWorker *worker = &engine->workers[c];
        snprintf(worker->semaphoreName, 16, "rk-iq-%03d", c);
        sem[c] = sem_open(worker->semaphoreName, O_CREAT | O_EXCL, 0600, 0);
        if (sem[c] == SEM_FAILED) {
            if (engine->verbose > 1) {
                RKLog("Info. Semaphore %s exists. Try to remove and recreate.\n", worker->semaphoreName);
            }
            if (sem_unlink(worker->semaphoreName)) {
                RKLog("Error. Unable to unlink semaphore %s.\n", worker->semaphoreName);
            }
            // 2nd trial
            sem[c] = sem_open(worker->semaphoreName, O_CREAT | O_EXCL, 0600, 0);
            if (sem[c] == SEM_FAILED) {
                RKLog("Error. Unable to remove then create semaphore %s\n", worker->semaphoreName);
                return (void *)RKResultFailedToInitiateSemaphore;
            } else if (engine->verbose > 1) {
                RKLog("Info. Semaphore %s removed and recreated.\n", worker->semaphoreName);
            }
        }
        worker->id = c;
        worker->parentEngine = engine;
        if (pthread_create(&worker->tid, NULL, pulseCompressionCore, worker) != 0) {
            RKLog("Error. Failed to start a compression core.\n");
            return (void *)RKResultFailedToStartCompressionCore;
        }
    }

    // Wait for the workers to increase the tic count once
    // Using sem_wait here could cause a stolen post within the worker
    // Tested and removed on 9/29/2016
    for (c = 0; c < engine->coreCount; c++) {
        while (engine->workers[c].tic == 0) {
            usleep(1000);
        }
    }

    // Increase the tic once to indicate the watcher is ready
    engine->tic++;

    // Here comes the busy loop
    i = 0;   // anonymous
    j = 0;   // filter index
    k = 0;   // pulse index
    c = 0;   // core index
    if (engine->verbose) {
        RKLog(">pulseWatcher() started.   c = %d   k = %d   engine->index = %d\n", c, k, *engine->index);
    }
    while (engine->state == RKPulseCompressionEngineStateActive) {
        // Wait until the engine index move to the next one for storage
        while (k == *engine->index && engine->state == RKPulseCompressionEngineStateActive) {
            usleep(200);
        }
        pulse = RKGetPulse(engine->buffer, k);
        while (pulse->header.s != RKPulseStatusReady && engine->state == RKPulseCompressionEngineStateActive) {
            usleep(200);
        }
        if (engine->state == RKPulseCompressionEngineStateActive) {
            // Lag of the engine
            engine->lag = fmodf((float)(*engine->index + engine->size - k) / engine->size, 1.0f);

            // Assess the lag of the workers
            lag = engine->workers[0].lag;
            for (i = 1; i < engine->coreCount; i++) {
                lag = MAX(lag, engine->workers[i].lag);
            }
            if (skipCounter == 0 && lag > 0.9f) {
                engine->almostFull++;
                skipCounter = engine->size / 10;
                RKLog("Warning. I/Q Buffer overflow projected by pulseWatcher().\n");
//
//                i = RKPreviousModuloS(*engine->index, engine->size);
//                pulse = RKGetPulse(engine->buffer, i);
//                while (!(engine->pulses[i].header.s & RKPulseStatusProcessed)) {
//                    engine->filterGid[i] = -1;
//                    engine->planIndices[i][0] = 0;
//                    i = RKPreviousModuloS(i, engine->size);
//                    pulse = RKGetPulse(engine->buffer, i);
//                }
//
                i = *engine->index;
                do {
                    i = RKPreviousModuloS(i, engine->size);
                    engine->filterGid[i] = -1;
                    pulse = RKGetPulse(engine->buffer, i);
                } while (!(pulse->header.s & RKPulseStatusProcessed));
            }

            // The pulse
            pulse = RKGetPulse(engine->buffer, k);

            // Skip processing if the buffer is getting full (avoid hitting SEM_VALUE_MAX)
            if (skipCounter > 0) {
                engine->filterGid[k] = -1;
                engine->planIndices[k][0] = 0;
                if (--skipCounter == 0) {
                    RKLog(">Info. pulseWatcher() skipped a chunk.\n");
                }
            } else {
                // Compute the filter group id to use
                engine->filterGid[k] = (gid = pulse->header.i % engine->filterGroupCount);

                // Find the right plan; create it if it does not exist
                for (j = 0; j < engine->filterCounts[gid]; j++) {
                    planSize = 1 << (int)ceilf(log2f((float)MIN(pulse->header.gateCount, engine->anchors[gid][j].maxDataLength)));
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
                        RKLog("A new DFT plan of size %d is needed ...  gid = %d   planCount = %d\n", planSize, gid, engine->planCount);
                        if (engine->planCount >= RKPulseCompressionDFTPlanCount) {
                            RKLog("Error. Unable to create another DFT plan.  engine->planCount = %d\n", engine->planCount);
                            exit(EXIT_FAILURE);
                        }
                        planIndex = engine->planCount;
                        engine->planForwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, in, FFTW_FORWARD, FFTW_MEASURE);
                        engine->planForwardOutPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, out, FFTW_FORWARD, FFTW_MEASURE);
                        engine->planBackwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, out, out, FFTW_BACKWARD, FFTW_MEASURE);
                        engine->planSizes[planIndex] = planSize;
                        engine->planCount++;
                        RKLog("k = %d   j = %d  planIndex = %d\n", k, j, planIndex);
                    }
                    engine->planIndices[k][j] = planIndex;
                }
            }

            // The pulse is considered "inspected" whether it will be skipped / compressed by the desingated worker
            pulse->header.s |= RKPulseStatusInspected;

            // Now we post
            #ifdef DEBUG_IQ
            RKLog("pulseWatcher() posting core-%d for pulse %d gate %d\n", c, k, engine->pulses[k].header.gateCount);
            #endif
            if (engine->useSemaphore) {
                if (sem_post(sem[c])) {
                    RKLog("Error. Failed in sem_post(), errno = %d\n", errno);
                }
            } else {
                engine->workers[c].tic++;
            }
            c = RKNextModuloS(c, engine->coreCount);
        }
        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->size);
    }

    // Wait for workers to return
    for (c = 0; c < engine->coreCount; c++) {
        RKPulseCompressionWorker *worker = &engine->workers[c];
        if (engine->useSemaphore) {
            sem_post(sem[c]);
        }
        pthread_join(worker->tid, NULL);
        sem_unlink(worker->semaphoreName);
    }

    // Export wisdom
    if (exportWisdom) {
        RKLog("Saving DFT wisdom ...\n");
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

//
#pragma mark -

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)malloc(sizeof(RKPulseCompressionEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a pulse compression engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKPulseCompressionEngine));
    engine->state = RKPulseCompressionEngineStateAllocated;
    engine->useSemaphore = true;
    pthread_mutex_init(&engine->coreMutex, NULL);
    return engine;
}

void RKPulseCompressionEngineFree(RKPulseCompressionEngine *engine) {
    if (engine->state == RKPulseCompressionEngineStateActive) {
        RKPulseCompressionEngineStop(engine);
    }
    for (int i = 0; i < engine->filterGroupCount; i++) {
        for (int j = 0; j < engine->filterCounts[i]; j++) {
            if (engine->filters[i][j] != NULL) {
                free(engine->filters[i][j]);
            }
        }
    }
    free(engine->filterGid);
    free(engine->planIndices);
    free(engine);
}

#pragma mark -

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
                                                   void *buffer,
                                                   uint32_t *index,
                                                   const uint32_t size) {
    engine->buffer = buffer;
    engine->index = index;
    engine->size = size;

    if (engine->filterGid != NULL) {
        free(engine->filterGid);
    }
    engine->filterGid = (int *)malloc(size * sizeof(int));
    if (engine->filterGid == NULL) {
        RKLog("Error. Unable to allocate filterGid.\n");
        exit(EXIT_FAILURE);
    }

    if (engine->planIndices != NULL) {
        free(engine->planIndices);
    }
    engine->planIndices = (RKPulseCompressionPlanIndex *)malloc(size * sizeof(RKPulseCompressionPlanIndex));
    engine->memoryUsage += size * sizeof(RKPulseCompressionPlanIndex);
    if (engine->planIndices == NULL) {
        RKLog("Error. Unable to allocate planIndices.\n");
        exit(EXIT_FAILURE);
    }
}

void RKPulseCompressionEngineSetCoreCount(RKPulseCompressionEngine *engine, const unsigned int count) {
    if (engine->state == RKPulseCompressionEngineStateActive) {
        RKLog("Error. Core count cannot be changed when the engine is active.\n");
        return;
    }
    engine->coreCount = count;
}

int RKPulseCompressionEngineStart(RKPulseCompressionEngine *engine) {
    engine->state = RKPulseCompressionEngineStateActivating;
    if (engine->filterGroupCount == 0) {
        // Set to default impulse as matched filter
        RKPulseCompressionSetFilterToImpulse(engine);
    }
    if (engine->coreCount == 0) {
        engine->coreCount = 8;
    }
    if (engine->workers != NULL) {
        RKLog("Error. RKPulseCompressionEngine->workers should be NULL here.\n");
    }
    engine->workers = (RKPulseCompressionWorker *)malloc(engine->coreCount * sizeof(RKPulseCompressionWorker));
    engine->memoryUsage += engine->coreCount * sizeof(RKPulseCompressionWorker);
    memset(engine->workers, 0, engine->coreCount * sizeof(RKPulseCompressionWorker));
    if (engine->verbose) {
        RKLog("Starting pulseWatcher() ...\n");
    }
    if (pthread_create(&engine->tidPulseWatcher, NULL, pulseWatcher, engine) != 0) {
        RKLog("Error. Failed to start a pulse watcher.\n");
        return RKResultFailedToStartPulseWatcher;
    }
    while (engine->tic == 0) {
        usleep(1000);
    }

    return RKResultNoError;
}

int RKPulseCompressionEngineStop(RKPulseCompressionEngine *engine) {
    if (engine->state != RKPulseCompressionEngineStateActive) {
        if (engine->verbose > 1) {
            RKLog("Info. Pulse compression engine is being or has been deactivated.\n");
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    engine->state = RKPulseCompressionEngineStateDeactivating;
    pthread_join(engine->tidPulseWatcher, NULL);
    if (engine->verbose) {
        RKLog("pulseWatcher() ended\n");
    }
    free(engine->workers);
    engine->workers = NULL;
    engine->state = RKPulseCompressionEngineStateNull;
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

int RKPulseCompressionSetFilter(RKPulseCompressionEngine *engine, const RKComplex *filter, const int filterLength, const int origin, const int maxDataLength, const int group, const int index) {
    if (engine->filterGroupCount >= RKMaxMatchedFilterGroupCount) {
        RKLog("Error. Unable to set anymore filters.\n");
        return RKResultFailedToAddFilter;
    }
    if (engine->filters[group][index] != NULL) {
        free(engine->filters[group][index]);
    }
    if (posix_memalign((void **)&engine->filters[group][index], RKSIMDAlignSize, maxDataLength * sizeof(RKComplex))) {
        RKLog("Error. Unable to allocate filter memory.\n");
        return RKResultFailedToAllocateFilter;
    }
    memset(engine->filters[group][index], 0, maxDataLength * sizeof(RKComplex));
    memcpy(engine->filters[group][index], filter, filterLength * sizeof(RKComplex));
    engine->filterGroupCount = MAX(engine->filterGroupCount, group + 1);
    engine->filterCounts[group] = MAX(engine->filterCounts[group], index + 1);
    engine->anchors[group][index].origin = origin;
    engine->anchors[group][index].length = filterLength;
    engine->anchors[group][index].maxDataLength = maxDataLength;
    if (engine->verbose) {
        RKLog("Matched filter set.  group count = %d\n", engine->filterGroupCount);
        for (int i = 0; i < engine->filterGroupCount; i++) {
            RKLog(">Filter count of group[%d] = %d\n", i, engine->filterCounts[i]);
            for (int j = 0; j < engine->filterCounts[i]; j++) {
                RKLog(">   Filter[%d] @ length = %d  origin = %d  maximum data length = %s\n", j, engine->anchors[i][j].length, engine->anchors[i][j].origin, RKIntegerToCommaStyleString(engine->anchors[i][j].maxDataLength));
            }
        }
    }
    return RKResultNoError;
}

int RKPulseCompressionSetFilterToImpulse(RKPulseCompressionEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}};
    RKPulse *pulse = (RKPulse *)engine->buffer;
    return RKPulseCompressionSetFilter(engine, filter, sizeof(filter) / sizeof(RKComplex), 0, pulse->header.capacity, 0, 0);
}

int RKPulseCompressionSetFilterTo121(RKPulseCompressionEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}, {2.0f, 0.0f}, {1.0f, 0.0f}};
    RKPulse *pulse = (RKPulse *)engine->buffer;
    return RKPulseCompressionSetFilter(engine, filter, sizeof(filter) / sizeof(RKComplex), 0, pulse->header.capacity, 0, 0);
}

char *RKPulseCompressionEngineStatusString(RKPulseCompressionEngine *engine) {
    int i, c;
    static char string[RKMaximumStringLength];

    // Full / compact string: Some spaces
    bool full = true;
    char spacer[2] = "";
    if (full) {
        sprintf(spacer, " ");
    }

    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';

    // Use b characters to draw a bar
    const int b = 10;
    i = *engine->index * (b + 1) / engine->size;
    memset(string, '#', i);
    memset(string + i, '.', b - i);
    i = b + sprintf(string + b, "%s|", spacer);

    // Engine lag
    i += snprintf(string + i, RKMaximumStringLength - i, "%s%s%02.0f%s%s|",
                  spacer,
                  rkGlobalParameters.showColor ? (engine->lag > 0.7 ? "\033[31m" : (engine->lag > 0.5 ? "\033[33m" : "\033[32m")) : "",
                  99.0f * engine->lag,
                  rkGlobalParameters.showColor ? "\033[0m" : "",
                  spacer);

    RKPulseCompressionWorker *worker;

    // Lag from each core
    for (c = 0; c < engine->coreCount; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKMaximumStringLength - i, "%s%s%02.0f%s",
                      spacer,
                      rkGlobalParameters.showColor ? (worker->lag > 0.7 ? "\033[31m" : (worker->lag > 0.5 ? "\033[33m" : "\033[32m")) : "",
                      99.0f * worker->lag,
                      rkGlobalParameters.showColor ? "\033[0m" : "");
    }
    i += snprintf(string + i, RKMaximumStringLength - i, "%s|", full ? " " : "");
    // Duty cycle of each core
    for (c = 0; c < engine->coreCount && i < RKMaximumStringLength - 13; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKMaximumStringLength - i, "%s%s%2.0f%s",
                      spacer,
                      rkGlobalParameters.showColor ? (worker->dutyCycle > 0.99 ? "\033[31m" : (worker->dutyCycle > 0.95 ? "\033[33m" : "\033[32m")) : "",
                      99.0f * worker->dutyCycle,
                      rkGlobalParameters.showColor ? "\033[0m" : "");
    }
    // Almost Full flag
    i += snprintf(string + i, RKMaximumStringLength - i, " [%d]", engine->almostFull);
    if (i > RKMaximumStringLength - 13) {
        memset(string + i, '#', RKMaximumStringLength - i - 1);
    }
    return string;
}

void RKPulseCompressionEngineLogStatus(RKPulseCompressionEngine *engine) {
    RKLog(RKPulseCompressionEngineStatusString(engine));
}
