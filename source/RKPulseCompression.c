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

    const int k = pulseId(engine);

    if (k < 0) {
        fprintf(stderr, "Unable to find my thread ID.\n");
        return NULL;
    }

    //printf("I am core %d\n", k);

    fftwf_complex *in = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));

    fftwf_plan planFilterForward[engine->planCount];
    fftwf_plan planDataFoward[engine->planCount];
    fftwf_plan planDataBackward[engine->planCount];

    struct timespec ts;

    while (engine->active) {

#if defined(__APPLE__)
        usleep(1000);
#else
        // Only process pulses that
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        sem_timedwait(&engine->sem[k], &ts);
#endif
    }

    fftw_free(in);
    fftw_free(out);

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
        sem_destroy(&engine->sem[i]);
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
        printf("Starting core %d ...\n", i);
        if ((r = pthread_create(&engine->tid[i], NULL, pulseCompressionCore, engine)) != 0) {
            RKLog("Error. Failed to start a compression core.\n");
            return RKResultFailedToStartCompressionCore;
        }
        if ((r = sem_init(&engine->sem[i], 0, 0)) != 0) {
            RKLog("Error. Unable to initialize a semaphore.\n");
            return RKResultFailedToInitiateSemaphore;
        }
    }

    return 0;
}
