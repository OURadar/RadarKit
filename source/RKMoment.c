//
//  RKMoment.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/20/15.
//
//

#include <RadarKit/RKMoment.h>

void *momentCore(void *in);

#pragma mark -

void *momentCore(void *in) {
    return NULL;
}

void *pulseWatcher(void *_in) {
    RKMomentEngine *engine = (RKMomentEngine *)_in;

    int i, j, k;
    uint32_t c;

    sem_t *sem[engine->coreCount];

//    int skipCounter = 0;

    // Change the state to active so all the processing cores stay in the busy loop
    engine->state = RKMomentEngineStateActive;

    // Spin off N workers to process I/Q pulses
    for (i = 0; i < engine->coreCount; i++) {
        RKMomentWorker *worker = &engine->workers[i];
        snprintf(worker->semaphoreName, 16, "rk-mm-%03d", i);
        sem[i] = sem_open(worker->semaphoreName, O_CREAT | O_EXCL, 0600, 0);
        if (sem[i] == SEM_FAILED) {
            if (engine->verbose > 1) {
                RKLog("Info. Semaphore %s exists. Try to remove and recreate.\n", worker->semaphoreName);
            }
            if (sem_unlink(worker->semaphoreName)) {
                RKLog("Error. Unable to unlink semaphore %s.\n", worker->semaphoreName);
            }
            // 2nd trial
            sem[i] = sem_open(worker->semaphoreName, O_CREAT | O_EXCL, 0600, 0);
            if (sem[i] == SEM_FAILED) {
                RKLog("Error. Unable to remove then create semaphore %s\n", worker->semaphoreName);
                return (void *)RKResultFailedToInitiateSemaphore;
            } else if (engine->verbose > 1) {
                RKLog("Info. Semaphore %s removed and recreated.\n", worker->semaphoreName);
            }
        }
        worker->id = i;
        worker->parentEngine = engine;
        if (pthread_create(&worker->tid, NULL, momentCore, worker) != 0) {
            RKLog("Error. Failed to start a compression core.\n");
            return (void *)RKResultFailedToStartCompressionCore;
        }
    }

    // Wait for the workers to increase the tic count once
    // Using sem_wait here could cause a stolen post within the worker
    // See RKPulseCompression.c
    for (i = 0; i < engine->coreCount; i++) {
        while (engine->workers[i].tic == 0) {
            usleep(1000);
        }
    }

    // Increase the tic once to indicate the watcher is ready
    engine->tic++;

    // Here comes the busy loop
    k = 0;
    c = 0;
    RKLog("pulseWatcher() started.   c = %d   k = %d   engine->index = %d\n", c, k, *engine->pulseIndex);
    while (engine->state == RKMomentEngineStateActive) {
        // Wait until the engine index move to the next one for storage
        while (k == *engine->pulseIndex && engine->state == RKMomentEngineStateActive) {
            usleep(200);
        }
        while (engine->pulses[k].header.s != RKPulseStatusCompressed && engine->state == RKMomentEngineStateActive) {
            usleep(200);
        }
        if (engine->state == RKPulseStatusCompressed) {
            // RKPulse *pulse = &engine->pulses[k];

            // Gather the start and end pulses
            // Get the vacant ray
            // Post a worker to process for a ray


        }
        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->pulseBufferSize);
    }

    // Wait for workers to return
    for (i = 0; i < engine->coreCount; i++) {
        RKMomentWorker *worker = &engine->workers[i];
        if (engine->useSemaphore) {
            sem_post(sem[i]);
        }
        pthread_join(worker->tid, NULL);
        sem_unlink(worker->semaphoreName);
    }

    return NULL;
}

#pragma mark -

RKMomentEngine *RKMomentEngineInit(void) {
    RKMomentEngine *engine = (RKMomentEngine *)malloc(sizeof(RKMomentEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a momment engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKMomentEngine));
    engine->state = RKMomentEngineStateAllocated;
    engine->verbose = 1;
    engine->useSemaphore = true;
    pthread_mutex_init(&engine->coreMutex, NULL);
    return engine;
}

void RKMomentEngineFree(RKMomentEngine *engine) {
    if (engine->state == RKMomentEngineStateActive) {
        RKMomentEngineStop(engine);
    }
    free(engine);
}

void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *engine,
                                         RKPulse *pulses,
                                         uint32_t *pulseIndex,
                                         const uint32_t pulseBufferSize,
                                         RKFloatRay *rays,
                                         uint32_t *rayIndex,
                                         const uint32_t rayBufferSize) {
    engine->pulses = pulses;
    engine->pulseIndex = pulseIndex;
    engine->pulseBufferSize = pulseBufferSize;
    engine->rawRays = rays;
    engine->rayIndex = rayIndex;
    engine->rayBufferSize = rayBufferSize;
    engine->encodedRays = NULL;
}


void RKMomentEngineSetCoreCount(RKMomentEngine *engine, const unsigned int count) {
    if (engine->state == RKMomentEngineStateActive) {
        RKLog("Error. Core count cannot be changed when the engine is active.\n");
        return;
    }
    engine->coreCount = count;
}

int RKMomentEngineStart(RKMomentEngine *engine) {
    engine->state = RKMomentEngineStateActivating;
    if (engine->coreCount == 0) {
        engine->coreCount = 2;
    }
    if (engine->workers != NULL) {
        RKLog("Error. RKMomentEngine->workers should be NULL here.\n");
    }
    engine->workers = (RKMomentWorker *)malloc(engine->coreCount * sizeof(RKMomentWorker));
    memset(engine->workers, 0, engine->coreCount * sizeof(RKMomentWorker));
    if (engine->verbose) {
        RKLog("Starting RKMoment pulseWatcher() ...\n");
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

int RKMomentEngineStop(RKMomentEngine *engine) {
    if (engine->state != RKMomentEngineStateActive) {
        if (engine->verbose > 1) {
            RKLog("Info. Pulse compression engine is being or has been deactivated.\n");
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    engine->state = RKMomentEngineStateDeactivating;
    pthread_join(engine->tidPulseWatcher, NULL);
    if (engine->verbose) {
        RKLog("pulseWatcher() ended\n");
    }
    free(engine->workers);
    engine->workers = NULL;
    engine->state = RKMomentEngineStateNull;
    return RKResultNoError;
}
