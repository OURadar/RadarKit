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

    int r;
    struct timespec ts, t0, t1, t2;
    sem_t *sem = sem_open(engine->sem_name[c], O_RDONLY, 0600, 0);

    // Post once to indicate this processing core is created.
    sem_post(sem);

    fftwf_complex *in  = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));

    fftwf_plan planFilterForward[engine->planCount];
    fftwf_plan planDataFoward[engine->planCount];
    fftwf_plan planDataBackward[engine->planCount];

    // Initialize some end-of-loop variables
    clock_gettime(CLOCK_REALTIME, &t0);
    clock_gettime(CLOCK_REALTIME, &t2);

    // The last pulse of the buffer
    uint32_t i0 = RKBuffer0SlotCount - 1;
    i0 = (i0 / engine->coreCount) * engine->coreCount + c;

    RKLog("Core %d starts @ %d\n", c, i0);

    double *dutyCycle = &engine->dutyCycle[c];
    *dutyCycle = 0.1;

    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]

    while (engine->active) {
        // Semaphore wait with timeout
        ts.tv_sec = t1.tv_sec + 1;
        ts.tv_nsec = t1.tv_nsec;
        r = sem_timedwait(sem, &ts);

        if (r == 0) {
            // Start of this cycle
            clock_gettime(CLOCK_REALTIME, &t1);

            i0 = RKNextNBuffer0Slot(i0, engine->coreCount);

            // Done processing, get the time
            clock_gettime(CLOCK_REALTIME, &t0);
            printf("                    : [iRadar] Core %d got a pulse @ %d  dutyCycle = %.2f %%\n", c, i0, 100.0 * *dutyCycle);
        } else if (errno == ETIMEDOUT) {
            t1 = t0;
        } else {
            RKLog("Error. Failed in sem_timedwait(). errno = %d\n", errno);
            exit(EXIT_FAILURE);
        }

        *dutyCycle = 0.8 * *dutyCycle + 0.2 * (RKTimespecDiff(t0, t1) / RKTimespecDiff(t0, t2));

        t2 = t0;
    }

    fftwf_free(in);
    fftwf_free(out);

    return NULL;
}

void *pulseWatcher(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    uint32_t k = engine->index;
    uint32_t m;

    sem_t *sem[engine->coreCount];
    for (int i = 0; i < engine->coreCount; i++) {
        sem[i] = sem_open(engine->sem_name[i], O_RDONLY, 0600, 0);
    }

    RKLog("Pulse watcher started.\n");
    while (engine->active) {
        // Wait until the engine index move to the next one for storage,
        // which also means k is ready
        while (k == engine->index && engine->active) {
            usleep(1000);
        }
        if (engine->active) {
            m = k % engine->coreCount;
            RKLog("pulseWatcher posting sem[%d] for %d\n", m, k);
            if (sem_post(sem[m]) == -1) {
                RKLog("Error. Unable to perform sem_post().\n");
            }
        }
        // Update k for the next watch
        k = engine->index;
    }

    RKLog("Pulse watcher ended\n");

    return NULL;
}

//

RKPulseCompressionEngine *RKPulseCompressionEngineInitWithCoreCount(const unsigned int count) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)malloc(sizeof(RKPulseCompressionEngine));
    memset(engine, 0, sizeof(RKPulseCompressionEngine));
    engine->coreCount = count;
    engine->active = true;
    return engine;
}

RKPulseCompressionEngine *RKPulseCompressionEngineInit(void) {
    return RKPulseCompressionEngineInitWithCoreCount(4);
}


void RKPulseCompressionEngineFree(RKPulseCompressionEngine *engine) {
    int i;
    for (i = 0; i < engine->coreCount; i++) {
        pthread_join(engine->tid[i], NULL);
        sem_unlink(engine->sem_name[i]);
    }
    free(engine);
}

void RKPulseCompressionEngineSetInputOutputBuffers(RKPulseCompressionEngine *engine, RKInt16Pulse *input, RKFloatPulse *output, const uint32_t size) {
    engine->input = input;
    engine->output = output;
    engine->bufferSize = size;
}

int RKPulseCompressionEngineStart(RKPulseCompressionEngine *engine) {

    int i, r;
    sem_t *sem;

    // Spin off N workers to process I/Q pulses
    for (i = 0; i < engine->coreCount; i++) {
        snprintf(engine->sem_name[i], 16, "rk-%02d", i);
        //engine->sem[i] = sem_open(engine->sem_name[i], O_CREAT, 0600, 0);
        sem = sem_open(engine->sem_name[i], O_CREAT, 0600, 0);
        if (sem == SEM_FAILED) {
            RKLog("Error. Unable to initialize a semaphore.\n");
            return RKResultFailedToInitiateSemaphore;
        }
        if ((r = pthread_create(&engine->tid[i], NULL, pulseCompressionCore, engine)) != 0) {
            RKLog("Error. Failed to start a compression core.\n");
            return RKResultFailedToStartCompressionCore;
        }
        // Wait for each worker to post once
        sem_wait(sem);
    }

    RKLog("Starting pulse watcher ...\n");
    if ((r = pthread_create(&engine->tidPulseWatcher, NULL, pulseWatcher, engine)) != 0) {
        RKLog("Error. Failed to start a pulse watcher.\n");
        return RKResultFailedToStartPulseWatcher;
    }

    return 0;
}
