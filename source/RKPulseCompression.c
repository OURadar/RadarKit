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
int pulseId(RKPulseCompressionEngine *engine);

// Implementations

int pulseId(RKPulseCompressionEngine *engine) {
    int i;
    pthread_t id = pthread_self();
    for (i = 0; i < engine->coreCount; i++) {
        if (id == engine->tid[i]) {
            return i;
        }
    }
    return -1;
}

#define RK_CORE_PREFIX "                    : [iRadar] \033[3%dmCore %d\033[0m"


void *pulseCompressionCore(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    const int c = pulseId(engine);

    if (c < 0) {
        fprintf(stderr, "Unable to find my thread ID.\n");
        return NULL;
    }

    //const fc = c % engine->filterGroupCount;

    // Filter group id
    //const gid = pulse->header.i % engine->filterGroupCount;

    RKPulseCompressionWorker *me = &engine->workers[c];

    int i, j, k, p;
    struct timeval t0, t1, t2;
    
    sem_t *sem = sem_open(engine->semaphoreName[c], O_RDWR);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve semaphore %d\n", c);
        return (void *)RKResultFailedToRetrieveSemaphore;
    };

    
    // Allocate local resources, use k to keep track of the total allocation
    fftwf_complex *in = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    fftwf_complex *filters[RKMaxMatchedFilterGroupCount][RKMaxMatchedFilterCount];
    k = 2 * RKGateCount * sizeof(fftwf_complex);
    for (i = 0; i < engine->filterGroupCount; i++) {
        for (j = 0; j < engine->filterCounts[i]; j++) {
            filters[i][j] = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
            memset(filters[i][j], 0, RKGateCount * sizeof(fftwf_complex));
            memcpy(filters[i][j], engine->filters[i][j], engine->anchors[i][j].length * sizeof(fftwf_complex));
            k += RKGateCount * sizeof(fftwf_complex);
        }
    }

    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);
    
    // The last pulse of the buffer
    uint32_t i0 = RKBuffer0SlotCount - 1;
    i0 = (i0 / engine->coreCount) * engine->coreCount + c;
    
    double *dutyCycle = &engine->dutyCycle[c];
    *dutyCycle = 0.0;

    int sem_val;
    sem_getvalue(sem, &sem_val);

    pthread_mutex_lock(&engine->coreMutex);
    printf(RK_CORE_PREFIX " started.  planCount = %d  malloc %s  tic = %d  sem_val = %d\n", c + 1, c, me->planCount, RKIntegerToCommaStyleString(k), engine->tic[c], sem_val);
    pthread_mutex_unlock(&engine->coreMutex);

    // Increase the tic once to indicate this processing core is created.
    engine->tic[c]++;

    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]
    //

    uint32_t tic = engine->tic[c];
    int planSize, planIndex;
    bool found = false;

    while (engine->active) {
        if (engine->useSemaphore) {
            printf(RK_CORE_PREFIX " sem_wait()\n", c + 1, c);
            sem_wait(sem);
        } else {
            while (tic == engine->tic[c] && engine->active) {
                usleep(1000);
            }
        }
        if (engine->active != true) {
            break;
        }
        tic = engine->tic[c];

        
        // Something happened
        gettimeofday(&t1, NULL);

        // Start of this cycle
        i0 = RKNextNModuloS(i0, engine->coreCount, engine->size);

        RKInt16Pulse *pulse = &engine->input[i0];

        printf(RK_CORE_PREFIX " i0 = %d  s = %d\n", c + 1, c, i0, pulse->header.s);

        // Do some work with this pulse
        //
        //
        planSize = 1 << (uint32_t)ceilf(log2f((float)pulse->header.gateCount));

        // DFT of the raw data is stored in *in
        // DFT of the filter is stored in *out
        // Their product is stored in *out using in-place multiplication so *out = *out * *in
        // Then, the inverse DFT is performed to get out back to time domain (*in), which is the compressed pulse

        // Process each polarization separately and indepently
        for (p = 0; p < 2; p++) {
            // Convert the samples
            for (k = 0; k < pulse->header.gateCount; k++) {
                in[k][0] = (float)pulse->X[p][k].i;
                in[k][1] = (float)pulse->X[p][k].q;
            }

            // Find the right plan
            k = me->planCount;
            found = false;
            while (k > 0) {
                k--;
                if (planSize == me->planSizes[k]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (me->planCount >= RKPulseCompressionDFTPlanCount) {
                    RKLog("Error. Unable to create another DFT plan.\n");
                    exit(EXIT_FAILURE);
                }
                planIndex = k;
                printf(RK_CORE_PREFIX " creating FFT plan of size %d (gateCount = %d) @ k = %d %d %d\n", c + 1, c, planSize, pulse->header.gateCount, planIndex, engine->filterGroupCount, engine->filterCounts[0]);
                me->planInForward[planIndex] = fftwf_plan_dft_1d(planSize, in, in, FFTW_FORWARD, FFTW_MEASURE);
                me->planOutBackward[planIndex] = fftwf_plan_dft_1d(planSize, out, in, FFTW_BACKWARD, FFTW_MEASURE);
    //            for (i = 0; i < engine->filterGroupCount; i++) {
    //                for (j = 0; j < engine->filterCounts[i]; j++) {
    //                    printf(RK_CORE_PREFIX " new filter plan.  gid = %d  fid = %d\n", c + 1, c, i, j);
    //                    me->planFilterForward[i][j][planIndex] = fftwf_plan_dft_1d(planSize, filters[i][j], out, FFTW_FORWARD, FFTW_MEASURE);
    //                }
    //            }
                me->planSizes[planIndex] = planSize;
                me->planCount++;
            } else {
                planIndex = k;
                planSize = me->planSizes[k];
                printf(RK_CORE_PREFIX " using FFT plan of size %d @ k = %d\n", c + 1, c, planSize, planIndex);
            }
            
    //            fftwf_execute(me->planInForward[planIndex]);
    //            
    //            //fftwf_execute(me->planFilterForward[0][0][planIndex]);
    //
    //            fftwf_execute(me->planOutBackward[planIndex]);
        }

        // Done processing, get the time
        gettimeofday(&t0, NULL);
        printf(RK_CORE_PREFIX " id %d @ buf %d  f = %d  dutyCycle = %.2f %%\n", c + 1, c, pulse->header.i, i0, planSize, 100.0 * *dutyCycle);
        
        

        *dutyCycle = RKTimevalDiff(t0, t1) / RKTimevalDiff(t0, t2);

        t2 = t0;
    }
    
    for (k = 0; k < me->planCount; k++) {
        fftwf_destroy_plan(me->planInForward[k]);
        fftwf_destroy_plan(me->planOutBackward[k]);
        for (i = 0; i < engine->filterGroupCount; i++) {
            for (j = 0; j < engine->filterCounts[i]; j++) {
                fftwf_destroy_plan(me->planFilterForward[i][j][k]);
            }
        }
    }

    fftwf_free(in);
    fftwf_free(out);
    for (i = 0; i < engine->filterGroupCount; i++) {
        for (j = 0; j < engine->filterCounts[i]; j++) {
            fftwf_free(filters[i][j]);
        }
    }

    return NULL;
}

void *pulseWatcher(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    uint32_t k = 0;
    uint32_t c;

    sem_t *sem[engine->coreCount];
    
    if (engine->useSemaphore) {
        for (int i = 0; i < engine->coreCount; i++) {
            sem[i] = sem_open(engine->semaphoreName[i], O_RDWR);
        }
    }
    
    c = 0;
    RKLog("pulseWatcher() started.   c = %d   k = %d\n", c, k);
    while (engine->active) {
        // Wait until the engine index move to the next one for storage or the latest (index k) is ready.
        while (k == *(engine->index) && engine->active) {
            usleep(1000);
        }
        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->size);
        while (engine->input[k].header.s != RKPulseStatusReady && engine->active) {
            usleep(1000);
        }
//        while (engine->active && k == *(engine->index) && engine->input[k].header.s != RKPulseStatusReady) {
//            usleep(1000);
//        }
        if (engine->active) {
            RKLog("pulseWatcher() posting core-%d for pulse %d\n", c, RKPreviousModuloS(k, engine->size));
            if (engine->useSemaphore) {
                sem_post(sem[c]);
            } else {
                engine->tic[c]++;
            }
            c = RKNextModuloS(c, engine->coreCount);
        }
    }

    return NULL;
}

//

RKPulseCompressionEngine *RKPulseCompressionEngineInitWithCoreCount(const unsigned int count) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)malloc(sizeof(RKPulseCompressionEngine));
    memset(engine, 0, sizeof(RKPulseCompressionEngine));
    engine->coreCount = count;
    engine->active = true;
    engine->useSemaphore = true;
    for (int i = 0; i < engine->coreCount; i++) {
        snprintf(engine->semaphoreName[i], 16, "rk-sem-%03d", i);
    }
    engine->workers = (RKPulseCompressionWorker *)malloc(engine->coreCount * sizeof(RKPulseCompressionWorker));
    memset(engine->workers, 0, sizeof(RKPulseCompressionWorker));
    pthread_mutex_init(&engine->coreMutex, NULL);
    return engine;
}

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void) {
    return RKPulseCompressionEngineInitWithCoreCount(5);
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
    free(engine->workers);
    free(engine);
}

void RKPulseCompressionEngineSetInputOutputBuffers(RKPulseCompressionEngine *engine,
                                                   RKInt16Pulse *input,
                                                   RKFloatPulse *output,
                                                   uint32_t *index,
                                                   const uint32_t size) {
    engine->input = input;
    engine->output = output;
    engine->index = index;
    engine->size = size;
}

int RKPulseCompressionEngineStart(RKPulseCompressionEngine *engine) {

    int i;
    sem_t *sem[engine->coreCount];

    if (engine->filterGroupCount == 0) {
        // Set to default impulse as matched filter
        RKPulseCompressionSetFilterToImpulse(engine);
    }

    // Spin off N workers to process I/Q pulses
    for (i = 0; i < engine->coreCount; i++) {
        sem[i] = sem_open(engine->semaphoreName[i], O_CREAT | O_EXCL, 0600, 0);
        if (sem[i] == SEM_FAILED) {
            RKLog("Info. Semaphore %s exists. Try to remove and recreate.\n", engine->semaphoreName[i]);
            if (sem_unlink(engine->semaphoreName[i])) {
                RKLog("Error. Unable to unlink semaphore %s.\n", engine->semaphoreName[i]);
            }
            sem[i] = sem_open(engine->semaphoreName[i], O_CREAT | O_EXCL, 0600, 0);
            if (sem[i] == SEM_FAILED) {
                RKLog("Error. Unable to remove then create semaphore %s\n", engine->semaphoreName[i]);
                return RKResultFailedToInitiateSemaphore;
            } else {
                RKLog("Info. Semaphore %s removed and recreated.\n", engine->semaphoreName[i]);
            }
        }
        if (pthread_create(&engine->tid[i], NULL, pulseCompressionCore, engine) != 0) {
            RKLog("Error. Failed to start a compression core.\n");
            return RKResultFailedToStartCompressionCore;
        }
    }

    // Wait for the workers to increase the tic count once
    // Using sem_wait here could cause a stolen post within the worker
    // Tested and removed on 9/29/2016
    for (i = 0; i < engine->coreCount; i++) {
        while (engine->tic[i] == 0) {
            usleep(1000);
        }
    }

    RKLog("Starting pulse watcher ...\n");
    if (pthread_create(&engine->tidPulseWatcher, NULL, pulseWatcher, engine) != 0) {
        RKLog("Error. Failed to start a pulse watcher.\n");
        return RKResultFailedToStartPulseWatcher;
    }

    return 0;
}

int RKPulseCompressionEngineStop(RKPulseCompressionEngine *engine) {
    int i, k = 0;
    if (engine->active == false) {
        RKLog("Error. Pulse compression engine has stopped before.\n");
        return 1;
    }
    RKLog("RKPulseCompressionEngineStop()\n");
    engine->active = false;
    for (i = 0; i < engine->coreCount; i++) {
        if (engine->useSemaphore) {
            sem_t *sem = sem_open(engine->semaphoreName[i], O_RDWR, 0600, 0);
            sem_post(sem);
        }
        RKLog("Waiting for core %d to end.\n", i);
        k += pthread_join(engine->tid[i], NULL);
        RKLog("Core %d ended.\n", i);
    }
    k += pthread_join(engine->tidPulseWatcher, NULL);
    RKLog("pulseWatcher() ended\n");
    for (i = 0; i < engine->coreCount; i++) {
        sem_unlink(engine->semaphoreName[i]);
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

int RKPulseCompressionSetFilter(RKPulseCompressionEngine *engine, const RKComplex *filter, const int filterLength, const int dataOrigin, const int dataLength, const int group, const int index) {
    if (engine->filterGroupCount >= RKMaxMatchedFilterGroupCount) {
        RKLog("Error. Unable to set anymore filters.\n");
        return RKResultFailedToAddFilter;
    }
    if (engine->filters[group][index] != NULL) {
        free(engine->filters[group][index]);
    }
    //engine->filters[group][index] = (RKComplex *)malloc(length * sizeof(RKComplex));
    if (posix_memalign((void **)&engine->filters[group][index], RKSIMDAlignSize, RKGateCount * sizeof(RKComplex))) {
        RKLog("Error. Unable to allocate filter memory.\n");
        return RKResultFailedToAllocateFilter;
    }
    memset(engine->filters[group][index], 0, RKGateCount * sizeof(RKComplex));
    memcpy(engine->filters[group][index], filter, filterLength * sizeof(RKComplex));
    engine->filterGroupCount = MAX(engine->filterGroupCount, group + 1);
    engine->filterCounts[group] = MAX(engine->filterCounts[group], index + 1);
    engine->anchors[group][index].origin = dataOrigin;
    engine->anchors[group][index].length = dataLength;
    return 0;
}

int RKPulseCompressionSetFilterToImpulse(RKPulseCompressionEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}};
    RKPulseCompressionSetFilter(engine, filter, 1, 0, 1000, 0, 0);
    RKLog("Impulse filter set.  group count = %d / count of group 0 = %d\n", engine->filterGroupCount, engine->filterCounts[0]);
    return 0;
}
