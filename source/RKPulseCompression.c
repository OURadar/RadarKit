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


void *pulseCompressionCore(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    const int c = pulseId(engine);

    if (c < 0) {
        fprintf(stderr, "Unable to find my thread ID.\n");
        return NULL;
    }

    RKPulseCompressionWorker *me = &engine->workers[c];

    int j, k, p;
    struct timeval t0, t1, t2;
    
    sem_t *sem = sem_open(engine->semaphoreName[c], O_RDWR);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve semaphore %d\n", c);
        return (void *)RKResultFailedToRetrieveSemaphore;
    };

    
    // Allocate local resources
    fftwf_complex *in, *out;
    
    in = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    out = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    k = RKGateCount * sizeof(fftwf_complex) * 2;
    
    fftwf_plan planInForward[RKPulseCompressionDFTPlanCount];
    fftwf_plan planOutBackward[RKPulseCompressionDFTPlanCount];
    
    RKLog("Core %d started.  planCount = %d  malloc %s\n", c, me->planCount, RKIntegerToCommaStyleString(k));

    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);
    
    // The last pulse of the buffer
    uint32_t i0 = RKBuffer0SlotCount - 1;
    i0 = (i0 / engine->coreCount) * engine->coreCount + c;
    
    double *dutyCycle = &engine->dutyCycle[c];
    *dutyCycle = 0.0;

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

        // Do some work with this pulse
        //
        //
        planSize = 1 << (uint32_t)ceilf(log2f((float)pulse->header.gateCount));

        // DFT of the raw data is stored in *in
        // DFT of the filter is stored in *out
        // Their product is stored in *out using in-place multiplication so *out = *out * *in
        // Then, the inverse DFT is performed to get out back to time domain (*in), which is the compressed pulse

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
            RKLog("Creating FFT plan of size %d @ k = %d\n", planSize, planIndex);
            planInForward[k] = fftwf_plan_dft_1d(planSize, in, out, FFTW_FORWARD, FFTW_MEASURE);
            planOutBackward[k] = fftwf_plan_dft_1d(planSize, out, in, FFTW_BACKWARD, FFTW_MEASURE);
            me->planSizes[k] = planSize;
            me->planCount++;
        } else {
            planIndex = k;
            planSize = me->planSizes[k];
            RKLog("Using FFT plan of size %d @ k = %d\n", planSize, planIndex);
        }
        
        // Process each polarization separately and indepently
        for (p = 0; p < 2; p++) {
            // Convert the samples
            for (k = 0; k < pulse->header.gateCount; k++) {
                in[k][0] = (float)pulse->X[p][k].i;
                in[k][1] = (float)pulse->X[p][k].q;
            }
            fftwf_execute(planInForward[planIndex]);
            
            
            fftwf_execute(planOutBackward[planIndex]);
        }

        // Done processing, get the time
        gettimeofday(&t0, NULL);
        printf("                    : [iRadar] Core %d pulse %d @ %d  f = %d  dutyCycle = %.2f %%\n", c, pulse->header.i, i0, planSize, 100.0 * *dutyCycle);
        
        

        *dutyCycle = RKTimevalDiff(t0, t1) / RKTimevalDiff(t0, t2);

        t2 = t0;
    }
    
    for (k = 0; k < me->planCount; k++) {
        fftwf_destroy_plan(planInForward[k]);
        fftwf_destroy_plan(planOutBackward[k]);
    }

    fftwf_free(in);
    fftwf_free(out);

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
    RKLog("Pulse watcher started.   c = %d   k = %d\n", c, k);
    while (engine->active) {
        // Wait until the engine index move to the next one for storage, which also means k is ready
        while (k == *(engine->index) && engine->active) {
            usleep(1000);
        }
        if (engine->active) {
            RKLog("pulseWatcher posting core-%d for pulse %d\n", c, k);
            if (engine->useSemaphore) {
                sem_post(sem[c]);
            } else {
                engine->tic[c]++;
            }
            c = RKNextModuloS(c, engine->coreCount);
            
        }
        // Update k for the next watch
        k = RKNextModuloS(k, engine->size);
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
    return engine;
}

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void) {
    return RKPulseCompressionEngineInitWithCoreCount(5);
}

void RKPulseCompressionEngineFree(RKPulseCompressionEngine *engine) {
    if (engine->active) {
        RKPulseCompressionEngineStop(engine);
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

    // Spin off N workers to process I/Q pulses
    for (i = 0; i < engine->coreCount; i++) {
        sem[i] = sem_open(engine->semaphoreName[i], O_CREAT, 0600, 0);
        if (sem[i] == SEM_FAILED) {
            RKLog("Error. Unable to create semaphore %s\n", engine->semaphoreName[i]);
            return RKResultFailedToInitiateSemaphore;
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
        return 1;
    }
    engine->active = false;
    for (i = 0; i < engine->coreCount; i++) {
        if (engine->useSemaphore) {
            sem_t *sem = sem_open(engine->semaphoreName[i], O_RDWR, 0600, 0);
            sem_post(sem);
        }
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

