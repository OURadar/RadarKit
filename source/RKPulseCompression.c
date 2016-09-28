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


#if !defined(__APPLE__)
void *pulseCompressionCoreTS(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;
    const int c = pulseId(engine);
    
    if (c < 0) {
        fprintf(stderr, "Unable to find my thread ID.\n");
        return NULL;
    }
    
    RKLog("CoreTS %d started.\n", c);
    
    struct timespec t0, t1, t2, ts;
    
    sem_t *sem = sem_open(engine->sem_name[c], O_RDWR, 0600, 0);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve the semaphore.\n");
        return (void *)RKResultFailedToRetrieveSemaphore;
    }

    // Allocate local resources
    fftwf_complex *in  = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    
    fftwf_plan planFilterForward[engine->planCount];
    fftwf_plan planDataFoward[engine->planCount];
    fftwf_plan planDataBackward[engine->planCount];
    
    // Initialize some end-of-loop variables
    //clock_gettime(CLOCK_REALTIME, &t0);
    //clock_gettime(CLOCK_REALTIME, &t2);
    RKUTCTime(&t0);
    RKUTCTime(&t2);
    
    // The last pulse of the buffer
    uint32_t i0 = RKBuffer0SlotCount - 1;
    i0 = (i0 / engine->coreCount) * engine->coreCount + c;
    
    double *dutyCycle = &engine->dutyCycle[c];
    *dutyCycle = 0.0;
    
    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]
    
    uint32_t tic = engine->tic[c];
    int r;
    
    while (engine->active) {
        // Semaphore wait with timeout
        //clock_gettime(CLOCK_REALTIME, &ts);
        RKUTCTime(&ts);
        ts.tv_nsec += 100000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec += 1;
        }
        r = sem_timedwait(sem, &ts);
        
        // Something happened
        RKUTCTime(&t1);
        
        
        if (r == 0) {
            // Start of this cycle
            i0 = RKNextNBuffer0Slot(i0, engine->coreCount);
            
            // Do some work
            //
            //
            
            // Done processing, get the time
            //clock_gettime(CLOCK_REALTIME, &t0);
            RKUTCTime(&t0);
            printf("                    : [iRadar] Core %d got a pulse @ %d  dutyCycle = %.2f %%\n", c, i0, 100.0 * *dutyCycle);
        } else if (errno == ETIMEDOUT) {
            //printf("                    : [iRadar] Nothing ... %ld.%ld\n", t0.tv_sec, t0.tv_nsec);
            t0 = t1;
        } else {
            RKLog("Error. Failed in sem_timedwait(). errno = %d\n", errno);
            exit(EXIT_FAILURE);
        }
        
        //*dutyCycle = 0.8 * *dutyCycle + 0.2 * (RKTimespecDiff(t0, t1) / RKTimespecDiff(t0, t2));
        *dutyCycle = (RKTimespecDiff(t0, t1) / RKTimespecDiff(t0, t2));
        
        t2 = t0;
    }
    
    fftwf_free(in);
    fftwf_free(out);
    
    RKLog("Core %d ended.\n", c);
    
    return NULL;
}
#endif


void *pulseCompressionCore(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    const int c = pulseId(engine);

    if (c < 0) {
        fprintf(stderr, "Unable to find my thread ID.\n");
        return NULL;
    }

    RKLog("Core %d started.\n", c);

    struct timeval t0, t1, t2;

    // Increase the tic once to indicate this processing core is created.
    engine->tic[c]++;

    // Allocate local resources
    fftwf_complex *in  = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));
    fftwf_complex *out = (fftwf_complex *)fftwf_malloc(RKGateCount * sizeof(fftwf_complex));

    fftwf_plan planFilterForward[engine->planCount];
    fftwf_plan planDataFoward[engine->planCount];
    fftwf_plan planDataBackward[engine->planCount];

    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);

    // The last pulse of the buffer
    uint32_t i0 = RKBuffer0SlotCount - 1;
    i0 = (i0 / engine->coreCount) * engine->coreCount + c;

    double *dutyCycle = &engine->dutyCycle[c];
    *dutyCycle = 0.0;

    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]

    uint32_t tic = engine->tic[c];

    while (engine->active) {
        while (tic == engine->tic[c] && engine->active) {
            usleep(1000);
        }
        if (engine->active != true) {
            break;
        }
        tic = engine->tic[c];

        
        // Something happened
        gettimeofday(&t1, NULL);

        // Start of this cycle
        //i0 = RKNextNBuffer0Slot(i0, engine->coreCount);
        i0 = RKNextNModuloS(i0, engine->coreCount, engine->size);

        // Do some work
        //
        //


        // Done processing, get the time
        gettimeofday(&t0, NULL);
        RKInt16Pulse *pulse = &engine->input[i0];
        printf("                    : [iRadar] Core %d pulse %d @ %d  dutyCycle = %.2f %%\n", c, pulse->header.i, i0, 100.0 * *dutyCycle);

        //*dutyCycle = 0.8 * *dutyCycle + 0.2 * (RKTimespecDiff(t0, t1) / RKTimespecDiff(t0, t2));
        //*dutyCycle = (RKTimespecDiff(t0, t1) / RKTimespecDiff(t0, t2));
        *dutyCycle = RKTimevalDiff(t0, t1) / RKTimevalDiff(t0, t2);

        t2 = t0;
    }

    fftwf_free(in);
    fftwf_free(out);

    RKLog("Core %d ended.\n", c);

    return NULL;
}

void *pulseWatcher(void *_in) {
    RKPulseCompressionEngine *engine = (RKPulseCompressionEngine *)_in;

    uint32_t k = 0;
    uint32_t c;

//    sem_t *sem[engine->coreCount];
//    
//    for (m = 0; m < engine->coreCount; m++) {
//        sem[m] = sem_open(engine->sem_name[m], O_RDWR, 0600, 0);
//    }
    
    c = 0;
    RKLog("Pulse watcher started.   c = %d   k = %d\n", c, k);
    while (engine->active) {
        // Wait until the engine index move to the next one for storage, which also means k is ready
        while (k == *(engine->index) && engine->active) {
            usleep(1000);
        }
        if (engine->active) {
            // m = k % engine->coreCount;
            RKLog("pulseWatcher posting core-%d for pulse %d\n", c, k);
            engine->tic[c]++;

            c = c == engine->coreCount - 1 ? 0 : c + 1;
            
//            sem_post(sem[m]);
        }
        // Update k for the next watch
        k = k == engine->size - 1 ? 0 : k + 1;
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

    // Spin off N workers to process I/Q pulses
    for (i = 0; i < engine->coreCount; i++) {
        engine->tic[i] = 0;
        if (pthread_create(&engine->tid[i], NULL, pulseCompressionCore, engine) != 0) {
            RKLog("Error. Failed to start a compression core.\n");
            return RKResultFailedToStartCompressionCore;
        }
    }

#if defined(__APPLE__)
    // Wait for the workers to increase the tic count once
    for (i = 0; i < engine->coreCount; i++) {
        while (engine->tic[i] == 0) {
            usleep(1000);
        }
    }
#else
    int r;

    struct timespec ts;
    
    for (i = 0; i < engine->coreCount; i++) {
        RKUTCTime(&ts);
        sem_t *sem = sem_open(engine->sem_name[i], O_RDWR, 0600, 0);
        ts.tv_nsec += 100000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_nsec -= 1000000000;
            ts.tv_sec += 1;
        }
        r = sem_timedwait(sem, &ts);
        
        if (r == 0) {
            printf("                    : [iRadar] Core %d posted.\n", i);
        } else if (errno == ETIMEDOUT) {
            printf("                    : [iRadar] Nothing ... \n");
        } else {
            RKLog("Error. Failed in sem_timedwait(). errno = %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }
#endif

    RKLog("Starting pulse watcher ...\n");
    if (pthread_create(&engine->tidPulseWatcher, NULL, pulseWatcher, engine) != 0) {
        RKLog("Error. Failed to start a pulse watcher.\n");
        return RKResultFailedToStartPulseWatcher;
    }

    return 0;
}

int RKPulseCompressionEngineStop(RKPulseCompressionEngine *engine) {
    int i, k = 0;
    engine->active = false;
    for (i = 0; i < engine->coreCount; i++) {
        k += pthread_join(engine->tid[i], NULL);
    }
    return k;
}

