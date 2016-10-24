//
//  RKPulseCompression.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulseCompression.h>

// Internal functions

void *pulseCompressionCore(void *in);
int workerThreadId(RKPulseCompressionEngine *engine);

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


void *pulseCompressionCore(void *_in) {
    RKPulseCompressionWorker *me = (RKPulseCompressionWorker *)_in;
    RKPulseCompressionEngine *engine = me->parentEngine;

    int i, j, k, p;
    struct timeval t0, t1, t2;

    i = workerThreadId(engine);
    if (i < 0) {
        i = me->id;
        RKLog("Warning. Unable to find my thread ID. Assume %d\n", me->id);
    } else if (engine->verbose > 1) {
        RKLog("Info. Thread ID %d = %d okay.\n", me->id, i);
    }
    const int c = me->id;
    const int gid = 0;

    // Find the semaphore
    sem_t *sem = sem_open(me->semaphoreName, O_RDWR);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve semaphore %d\n", c);
        return (void *)RKResultFailedToRetrieveSemaphore;
    };

    // Allocate local resources, use k to keep track of the total allocation
    // Avoid fftwf_malloc() here so that if a non-avx-enabled libfftw is compatible
    fftwf_complex *in, *out;
    posix_memalign((void **)&in, RKSIMDAlignSize, RKGateCount * sizeof(fftwf_complex));
    posix_memalign((void **)&out, RKSIMDAlignSize, RKGateCount * sizeof(fftwf_complex));
    if (in == NULL || out == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    k = 2 * RKGateCount * sizeof(fftwf_complex);
    RKIQZ *zi, *zo;
    posix_memalign((void **)&zi, RKSIMDAlignSize, sizeof(RKIQZ));
    posix_memalign((void **)&zo, RKSIMDAlignSize, sizeof(RKIQZ));
    if (zi == NULL || zo == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    k += 2 * sizeof(RKIQZ);
    double *busyPeriods, *fullPeriods;
    posix_memalign((void **)&busyPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferSize * sizeof(double));
    posix_memalign((void **)&fullPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferSize * sizeof(double));
    if (busyPeriods == NULL || fullPeriods == NULL) {
        RKLog("Error. Unable to allocate resources for duty cycle calculation\n");
        return (void *)RKResultFailedToAllocateDutyCycleBuffer;
    }
    k += 2 * RKWorkerDutyCycleBufferSize * sizeof(double);
    memset(busyPeriods, 0, RKWorkerDutyCycleBufferSize * sizeof(double));
    memset(fullPeriods, 0, RKWorkerDutyCycleBufferSize * sizeof(double));
    double allBusyPeriods = 0.0, allFullPeriods = 0.0;

    // Initiate a variable to store my name
    char name[20];
    if (rkGlobalParameters.showColor) {
        i = sprintf(name, "\033[3%dm", c % 8 + 1);
    }
    if (engine->coreCount > 9) {
        i += sprintf(name + i, "Core %02d", c);
    } else {
        i += sprintf(name + i, "Core %d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(name + i, "\033[0m");
    }

    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);
    
    // The last index of the pulse buffer
    uint32_t i0 = RKBuffer0SlotCount - engine->coreCount + c;

    // The latest index in the dutyCycle buffer
    int d0 = 0;

    // DFT plan size and plan index in the parent engine
    int planSize = -1, planIndex;

    // Log my initial state
    if (engine->verbose) {
        pthread_mutex_lock(&engine->coreMutex);
        RKLog(">%s started.  i0 = %d   mem = %s  tic = %d\n", name, i0, RKIntegerToCommaStyleString(k), me->tic);
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

    while (engine->active) {
        if (engine->useSemaphore) {
            #ifdef DEBUG_IQ
            RKLog(">%s sem_wait()\n", coreName);
            #endif
            sem_wait(sem);
        } else {
            while (tic == me->tic && engine->active) {
                usleep(1000);
            }
            tic = me->tic;
        }
        if (engine->active != true) {
            break;
        }

        // Something happened
        gettimeofday(&t1, NULL);

        // Start of getting busy
        i0 = RKNextNModuloS(i0, engine->coreCount, engine->size);

        RKPulse *pulse = &engine->pulses[i0];

        #ifdef DEBUG_IQ
        RKLog(">%s i0 = %d  stat = %d\n", coreName, i0, input->header.s);
        #endif

        // Filter group id
        //const int gid = pulse->header.i % engine->filterGroupCount;

        // Do some work with this pulse
        // DFT of the raw data is stored in *in
        // DFT of the filter is stored in *out
        // Their product is stored in *out using in-place multiplication so *out = *out * *in
        // Then, the inverse DFT is performed to get out back to time domain (*in), which is the compressed pulse

        // Process each polarization separately and indepently
        for (p = 0; p < 2; p++) {
            // Go through all the filters in this fitler group
            for (j = 0; j < engine->filterCounts[gid]; j++) {
                // Get the plan index and size from parent engine
                planIndex = engine->planIndices[i0][j];
                planSize = engine->planSizes[planIndex];

                // Copy and convert the samples
                for (k = 0, i = engine->anchors[gid][j].origin;
                     k < planSize && i < pulse->header.gateCount;
                     k++, i++) {
                    in[k][0] = (RKFloat)pulse->X[p][i].i;
                    in[k][1] = (RKFloat)pulse->X[p][i].q;
                }
                // Zero pad the input; a filter is always zero-padded in the setter function.
                memset(&in[k][0], 0, (planSize - k) * sizeof(fftwf_complex));

                fftwf_execute_dft(engine->planForwardInPlace[planIndex], in, in);

                fftwf_execute_dft(engine->planForwardOutPlace[planIndex], (fftwf_complex *)engine->filters[gid][j], out);

                RKSIMD_iymul((RKComplex *)in, (RKComplex *)out, planSize);

                //RKSIMD_iymul_reg((RKComplex *)in, (RKComplex *)out, planSize);

//                // Deinterleave the RKComplex data into RKIQZ format, multiply using SIMD, then interleave the result back to RKComplex format
//                RKSIMD_Complex2IQZ((RKComplex *)in, zi, planSize);
//                RKSIMD_Complex2IQZ((RKComplex *)out, zo, planSize);
//                RKSIMD_izmul(zi, zo, planSize, true);
//                RKSIMD_IQZ2Complex(zo, (RKComplex *)out, planSize);

                fftwf_execute_dft(engine->planBackwardInPlace[planIndex], out, out);

                RKSIMD_iyscl((RKComplex *)in, 1.0f / planSize, planSize);

                for (k = 0, i = engine->anchors[gid][j].length - 1;
                     k < MIN(pulse->header.gateCount - engine->anchors[gid][j].length, engine->anchors[gid][j].maxDataLength);
                     k++, i++) {
                    pulse->Y[p][k].i = out[i][0];
                    pulse->Y[p][k].q = out[i][1];
                }
            } // filterCount
        } // p - polarization

        pulse->header.s |= RKPulseStatusCompressed;
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
    free(in);
    free(out);
    free(zi);
    free(zo);
    free(busyPeriods);
    free(fullPeriods);

    RKLog(">%s ended.\n", name);
    
    return NULL;
}

void *pulseWatcher(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    int i, j, k = 0;
    uint32_t c;

    sem_t *sem[engine->coreCount];

    bool found;
    int planSize, planIndex = 0;

    // FFTW's memory allocation and plan initialization are not thread safe but others are.
    fftwf_complex *in, *out;
    posix_memalign((void **)&in, RKSIMDAlignSize, RKGateCount * sizeof(fftwf_complex));
    posix_memalign((void **)&out, RKSIMDAlignSize, RKGateCount * sizeof(fftwf_complex));

    planSize = 1 << (int)ceilf(log2f((float)RKGateCount));

    bool exportWisdom = false;
    const char *wisdomFile = "fft-wisdom";

    if (RKFilenameExists(wisdomFile)) {
        fftwf_import_wisdom_from_filename(wisdomFile);
    } else {
        exportWisdom = true;
    }

    for (j = 0; j < 3; j++) {
        RKLog("Pre-allocate FFTW resources for plan size %s (%d)\n", RKIntegerToCommaStyleString(planSize), planIndex);
        engine->planForwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, in, FFTW_FORWARD, FFTW_MEASURE);
        engine->planForwardOutPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, out, FFTW_FORWARD, FFTW_MEASURE);
        engine->planBackwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, out, out, FFTW_BACKWARD, FFTW_MEASURE);
        engine->planSizes[planIndex++] = planSize;
        engine->planCount++;
        planSize /= 2;
    }

    // Spin off N workers to process I/Q pulses
    for (i = 0; i < engine->coreCount; i++) {
        RKPulseCompressionWorker *worker = &engine->workers[i];
        sem[i] = sem_open(worker->semaphoreName, O_CREAT | O_EXCL, 0600, 0);
        if (sem[i] == SEM_FAILED) {
            if (engine->verbose > 1) {
                RKLog("Info. Semaphore %s exists. Try to remove and recreate.\n", worker->semaphoreName);
            }
            if (sem_unlink(worker->semaphoreName)) {
                RKLog("Error. Unable to unlink semaphore %s.\n", worker->semaphoreName);
            }
            sem[i] = sem_open(worker->semaphoreName, O_CREAT | O_EXCL, 0600, 0);
            if (sem[i] == SEM_FAILED) {
                RKLog("Error. Unable to remove then create semaphore %s\n", worker->semaphoreName);
                return (void *)RKResultFailedToInitiateSemaphore;
            } else if (engine->verbose > 1) {
                RKLog("Info. Semaphore %s removed and recreated.\n", worker->semaphoreName);
            }
        }
        engine->workers[i].id = i;
        engine->workers[i].parentEngine = engine;
        if (pthread_create(&engine->workers[i].tid, NULL, pulseCompressionCore, &engine->workers[i]) != 0) {
            RKLog("Error. Failed to start a compression core.\n");
            return (void *)RKResultFailedToStartCompressionCore;
        }
    }

    // Wait for the workers to increase the tic count once
    // Using sem_wait here could cause a stolen post within the worker
    // Tested and removed on 9/29/2016
    for (i = 0; i < engine->coreCount; i++) {
        while (engine->workers[i].tic == 0) {
            usleep(1000);
        }
    }

    // Increase the tic once to indicate the watcher is ready
    engine->tic++;

    // Here comes the busy loop
    c = 0;
    RKLog("pulseWatcher() started.   c = %d   k = %d   engine->index = %d\n", c, k, *engine->index);
    while (engine->active) {
        // Wait until the engine index move to the next one for storage
        while (k == *engine->index && engine->active) {
            usleep(200);
        }
        while (engine->pulses[k].header.s != RKPulseStatusReady && engine->active) {
            usleep(200);
        }
        if (engine->active) {
            RKPulse *pulse = &engine->pulses[k];
            int gid = pulse->header.i % engine->filterGroupCount;
            // Find the right plan
            for (j = 0; j < engine->filterCounts[gid]; j++) {
                planSize = 1 << (int)ceilf(log2f((float)MIN(pulse->header.gateCount, engine->anchors[gid][j].maxDataLength)));
                i = engine->planCount;
                found = false;
                while (i > 0) {
                    i--;
                    if (planSize == engine->planSizes[i]) {
                        planIndex = i;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    RKLog("A new DFT plan of size %d is needed ...\n", planSize);
                    if (engine->planCount >= RKPulseCompressionDFTPlanCount) {
                        RKLog("Error. Unable to create another DFT plan.  engine->planCount = %d\n", engine->planCount);
                        exit(EXIT_FAILURE);
                    }
                    planIndex = engine->planCount - 1;
                    engine->planForwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, in, FFTW_FORWARD, FFTW_MEASURE);
                    engine->planForwardOutPlace[planIndex] = fftwf_plan_dft_1d(planSize, in, out, FFTW_FORWARD, FFTW_MEASURE);
                    engine->planBackwardInPlace[planIndex] = fftwf_plan_dft_1d(planSize, out, out, FFTW_BACKWARD, FFTW_MEASURE);
                    engine->planCount++;
                }
                engine->planIndices[k][j] = planIndex;
            }

            #ifdef DEBUG_IQ
            RKLog("pulseWatcher() posting core-%d for pulse %d gate %d\n", c, k, engine->pulses[k].header.gateCount);
            #endif

            if (engine->useSemaphore) {
                sem_post(sem[c]);
            } else {
                engine->workers[c].tic++;
            }
            c = RKNextModuloS(c, engine->coreCount);
        }
        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->size);
    }

    // Wait for workers to return
    for (i = 0; i < engine->coreCount; i++) {
        RKPulseCompressionWorker *worker = &engine->workers[i];
        if (engine->useSemaphore) {
            sem_post(sem[i]);
        }
        pthread_join(worker->tid, NULL);
        sem_unlink(worker->semaphoreName);
    }

    // Export wisdom
    if (exportWisdom) {
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

RKPulseCompressionEngine *RKPulseCompressionEngineInitWithCoreCount(const unsigned int count) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)malloc(sizeof(RKPulseCompressionEngine));
    memset(engine, 0, sizeof(RKPulseCompressionEngine));
    engine->coreCount = count;
    engine->active = true;
    engine->verbose = 1;
    engine->useSemaphore = true;
    engine->workers = (RKPulseCompressionWorker *)malloc(engine->coreCount * sizeof(RKPulseCompressionWorker));
    memset(engine->workers, 0, sizeof(RKPulseCompressionWorker));
    for (int i = 0; i < engine->coreCount; i++) {
        snprintf(engine->workers[i].semaphoreName, 16, "rk-sem-%03d", i);
    }
    pthread_mutex_init(&engine->coreMutex, NULL);
    return engine;
}

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void) {
    return RKPulseCompressionEngineInitWithCoreCount(12);
}

void RKPulseCompressionEngineFree(RKPulseCompressionEngine *engine) {
    if (engine->active) {
        RKPulseCompressionEngineStop(engine);
    }
    for (int i = 0; i < engine->filterGroupCount; i++) {
        for (int j = 0; j < engine->filterCounts[i]; j++) {
            if (engine->filters[i][j] != NULL) {
                free(engine->filters[i][j]);
            }
        }
    }
    free(engine->planIndices);
    free(engine->workers);
    free(engine);
}

void RKPulseCompressionEngineSetInputOutputBuffers(RKPulseCompressionEngine *engine,
                                                   RKPulse *pulses,
                                                   uint32_t *index,
                                                   const uint32_t size) {
    engine->pulses = pulses;
    engine->index = index;
    engine->size = size;
    if (engine->planIndices != NULL) {
        free(engine->planIndices);
    }
    engine->planIndices = (RKPulseCompressionPlanIndex *)malloc(size * sizeof(RKPulseCompressionPlanIndex));
    if (engine->planIndices == NULL) {
        RKLog("Error. Unable to allocate planIndices.\n");
        return;
    }
}

int RKPulseCompressionEngineStart(RKPulseCompressionEngine *engine) {
    if (engine->filterGroupCount == 0) {
        // Set to default impulse as matched filter
        RKPulseCompressionSetFilterToImpulse(engine);
    }

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

    return 0;
}

int RKPulseCompressionEngineStop(RKPulseCompressionEngine *engine) {
    int k;
    if (engine->active == false) {
        if (engine->verbose > 1) {
            RKLog("Info. Pulse compression engine has been called to stop before.\n");
        }
        return 1;
    }
    if (engine->verbose) {
        RKLog("RKPulseCompressionEngineStop()\n");
    }
    engine->active = false;
    k = pthread_join(engine->tidPulseWatcher, NULL);
    if (engine->verbose) {
        RKLog("pulseWatcher() ended\n");
    }
    return k;
}

int RKPulseCompressionSetFilterCountOfGroup(RKPulseCompressionEngine *engine, const int group, const int count) {
    engine->filterCounts[group] = count;
    return 0;
}

int RKPulseCompressionSetFilterGroupCount(RKPulseCompressionEngine *engine, const int groupCount) {
    engine->filterGroupCount = groupCount;
    return 0;
}

int RKPulseCompressionSetFilter(RKPulseCompressionEngine *engine, const RKComplex *filter, const int filterLength, const int origin, const int maxDataLength, const int group, const int index) {
    if (engine->filterGroupCount >= RKMaxMatchedFilterGroupCount) {
        RKLog("Error. Unable to set anymore filters.\n");
        return RKResultFailedToAddFilter;
    }
    if (engine->filters[group][index] != NULL) {
        free(engine->filters[group][index]);
    }
    if (posix_memalign((void **)&engine->filters[group][index], RKSIMDAlignSize, RKGateCount * sizeof(RKComplex))) {
        RKLog("Error. Unable to allocate filter memory.\n");
        return RKResultFailedToAllocateFilter;
    }
    memset(engine->filters[group][index], 0, RKGateCount * sizeof(RKComplex));
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
    return 0;
}

int RKPulseCompressionSetFilterToImpulse(RKPulseCompressionEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}};
    RKPulseCompressionSetFilter(engine, filter, 1, 0, RKGateCount, 0, 0);
    return 0;
}

void RKPulseCompressionEngineLogStatus(RKPulseCompressionEngine *engine) {
    int i, k;
    char string[RKMaximumStringLength];
    float lag;
    i = *engine->index * 10 / engine->size;
    RKPulseCompressionWorker *worker;

    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';
    memset(string, '|', i);
    memset(string + i, '.', 10 - i);
    i = 10;
    i += snprintf(string + i, RKMaximumStringLength, " :");
    for (k = 0; k < engine->coreCount; k++) {
        lag = fmodf((float)(*engine->index - engine->workers[k].pid + engine->size) / engine->size, 1.0f);
        if (rkGlobalParameters.showColor) {
            i += snprintf(string + i, RKMaximumStringLength - i, " \033[3%dm%4.2f\033[0m",
                          lag > 0.7 ? 1 : (lag > 0.5 ? 3 : 2),
                          lag);
        } else {
            i += snprintf(string + i, RKMaximumStringLength - i, " %4.2f", lag);
        }
    }
    i += snprintf(string + i, RKMaximumStringLength - i, " |");
    for (k = 0; k < engine->coreCount && i < RKMaximumStringLength - 13; k++) {
        worker = &engine->workers[k];
        if (rkGlobalParameters.showColor) {
            i += snprintf(string + i, RKMaximumStringLength - i, " \033[3%dm%4.2f\033[0m",
                          worker->dutyCycle > 0.9 ? 1 : (worker->dutyCycle > 0.75 ? 3 : 2),
                          worker->dutyCycle);
        } else {
            i += snprintf(string + i, RKMaximumStringLength - i, "  %4.2f", worker->dutyCycle);
        }
    }
    if (i > RKMaximumStringLength - 13) {
        memset(string + i, '#', RKMaximumStringLength - i - 1);
    }
    RKLog("%s", string);
}
