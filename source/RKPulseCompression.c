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
    struct timespec ts;
    //sem_t *sem = engine->sem[c];
    sem_t *sem = sem_open(engine->sem_name[c], O_CREAT, 0600, 0);

    // Post once to indicate this processing core is created.
//    sem_post(sem);

    fftwf_complex *in  = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));

    fftwf_plan planFilterForward[engine->planCount];
    fftwf_plan planDataFoward[engine->planCount];
    fftwf_plan planDataBackward[engine->planCount];


    // The latest processed index
    uint32_t i0 = RKPreviousBuffer0Slot(engine->index);
    i0 = (i0 / engine->coreCount) * engine->coreCount + c;

    RKLog("Core %d starts @ %d\n", c, i0);


    while (engine->active) {

        // Only process pulses that
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        r = sem_timedwait(sem, &ts);

        if (r == 0) {
            i0 = RKNextNBuffer0Slot(i0, engine->coreCount);
            printf("                    : [iRadar] Core %d got a pulse @ %d\n", c, i0);
        } else if (errno != ETIMEDOUT) {
            RKLog("Error. Failed in sem_timedwait(). errno = %d\n", errno);
        }
    }

    fftwf_free(in);
    fftwf_free(out);

    return NULL;
}

void *pulseWatcher(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    uint32_t k = engine->index;
    uint32_t m;

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
            if (sem_post(engine->sem[m]) == -1) {
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
//        sem_close(&engine->sem[i]);
        pthread_join(engine->tid[i], NULL);
        sem_destroy(engine->sem[i]);
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

    // Spin off N workers to process I/Q pulses
    for (i = 0; i < engine->coreCount; i++) {
        snprintf(engine->sem_name[i], 16, "rk-%02d", i);
//        if ((r = sem_init(&engine->sem[i], 0, 0)) != 0) {
//            RKLog("Error. Unable to initialize a semaphore.\n");
//            return RKResultFailedToInitiateSemaphore;
//        }
        engine->sem[i] = sem_open(engine->sem_name[i], O_CREAT, 0600, 0);
        if (engine->sem[i] == SEM_FAILED) {
            RKLog("Error. Unable to initialize a semaphore.\n");
            return RKResultFailedToInitiateSemaphore;
        }
        RKLog("Starting core %d ...\n", i);
        if ((r = pthread_create(&engine->tid[i], NULL, pulseCompressionCore, engine)) != 0) {
            RKLog("Error. Failed to start a compression core.\n");
            return RKResultFailedToStartCompressionCore;
        }
    }
    // Wait for each worker to post once
//    for (i = 0; i < engine->coreCount; i++) {
//        sem_wait(engine->sem[i]);
//    }
    RKLog("Starting pulse watcher ...\n");
    if ((r = pthread_create(&engine->tidPulseWatcher, NULL, pulseWatcher, engine)) != 0) {
        RKLog("Error. Failed to start a pulse watcher.\n");
        return RKResultFailedToStartPulseWatcher;
    }

    return 0;
}
