//
//  RKMoment.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/20/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKMoment.h>

// Internal functions

void *momentCore(void *);
void *pulseGatherer(void *);
int makeRayFromScratch(RKScratch *, RKRay *, const int gateCount, const int stride);

// Implementations

#pragma mark -
#pragma mark Helper Functions

void RKMomentUpdateStatusString(RKMomentEngine *engine) {
    int i, c;
    char *string = engine->statusBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKMaximumStringLength - 1] = '\0';
    string[RKMaximumStringLength - 2] = '#';

    // Use b characters to draw a bar
    i = engine->processedPulseIndex * (RKStatusBarWidth + 1) / engine->pulseBufferDepth;
    memset(string, 'M', i);
    memset(string + i, '.', RKStatusBarWidth - i);

    // Engine lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKMaximumStringLength - RKStatusBarWidth, " | %s%02.0f%s |",
                                    rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                                    99.9f * engine->lag,
                                    rkGlobalParameters.showColor ? RKNoColor : "");

    RKMomentWorker *worker;

    // Lag from each core
    for (c = 0; c < engine->coreCount; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKMaximumStringLength - i, " %s%02.0f%s",
                      rkGlobalParameters.showColor ? RKColorLag(worker->lag) : "",
                      99.9f * worker->lag,
                      rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Put a separator
    i += snprintf(string + i, RKMaximumStringLength - i, " |");
    // Duty cycle of each core
    for (c = 0; c < engine->coreCount && i < RKMaximumStringLength - 13; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKMaximumStringLength - i, " %s%02.0f%s",
                      rkGlobalParameters.showColor ? RKColorDutyCycle(worker->dutyCycle) : "",
                      99.9f * worker->dutyCycle,
                      rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Almost full count
    i += snprintf(string + i, RKMaximumStringLength - i, " [%d]", engine->almostFull);
    if (i > RKMaximumStringLength - 13) {
        memset(string + i, '#', RKMaximumStringLength - i - 1);
    }
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

int makeRayFromScratch(RKScratch *space, RKRay *ray, const int gateCount, const int stride) {
    int i, k;
    // Grab the data from scratch space and perform down-sampling, if stride > 1
    float *Zi = space->Z[0],  *Zo = RKGetFloatDataFromRay(ray, RKProductIndexZ);
    float *Vi = space->V[0],  *Vo = RKGetFloatDataFromRay(ray, RKProductIndexV);
    float *Wi = space->W[0],  *Wo = RKGetFloatDataFromRay(ray, RKProductIndexW);
    float *Di = space->ZDR,   *Do = RKGetFloatDataFromRay(ray, RKProductIndexD);
    float *Pi = space->PhiDP, *Po = RKGetFloatDataFromRay(ray, RKProductIndexP);
    float *Ri = space->RhoHV, *Ro = RKGetFloatDataFromRay(ray, RKProductIndexR);
    for (i = 0, k = 0; k < gateCount; i++, k += stride) {
        *Zo++ = *Zi;  Zi += stride;
        *Vo++ = *Vi;  Vi += stride;
        *Wo++ = *Wi;  Wi += stride;
        *Do++ = *Di;  Di += stride;
        *Po++ = *Pi;  Pi += stride;
        *Ro++ = *Ri;  Ri += stride;
    }
    // Record down the down-sampled gate count
    ray->header.gateCount = i;
    if (i != (gateCount + stride - 1) / stride) {
        RKLog("Equation (314) does not work.  gateCount = %d  stride = %d  i = %d vs %d\n",
              gateCount, stride, i, (gateCount + stride - 1) / stride);
    }
    // Convert float to color representation (0.0 - 255.0) using M * (value) + A; RhoHV is special
    RKFloat lhma[4];
    int K = (ray->header.gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKZLHMAC   RKVec zl = _rk_mm_set1_pf(lhma[0]);  RKVec zh = _rk_mm_set1_pf(lhma[1]);  RKVec zm = _rk_mm_set1_pf(lhma[2]);  RKVec za = _rk_mm_set1_pf(lhma[3]);
    RKV2LHMAC  RKVec vl = _rk_mm_set1_pf(lhma[0]);  RKVec vh = _rk_mm_set1_pf(lhma[1]);  RKVec vm = _rk_mm_set1_pf(lhma[2]);  RKVec va = _rk_mm_set1_pf(lhma[3]);
    RKWLHMAC   RKVec wl = _rk_mm_set1_pf(lhma[0]);  RKVec wh = _rk_mm_set1_pf(lhma[1]);  RKVec wm = _rk_mm_set1_pf(lhma[2]);  RKVec wa = _rk_mm_set1_pf(lhma[3]);
    RKDLHMAC   RKVec dl = _rk_mm_set1_pf(lhma[0]);  RKVec dh = _rk_mm_set1_pf(lhma[1]);  RKVec dm = _rk_mm_set1_pf(lhma[2]);  RKVec da = _rk_mm_set1_pf(lhma[3]);
    RKPLHMAC   RKVec pl = _rk_mm_set1_pf(lhma[0]);  RKVec ph = _rk_mm_set1_pf(lhma[1]);  RKVec pm = _rk_mm_set1_pf(lhma[2]);  RKVec pa = _rk_mm_set1_pf(lhma[3]);
    RKRLHMAC   RKVec rl = _rk_mm_set1_pf(lhma[0]);  RKVec rh = _rk_mm_set1_pf(lhma[1]);
    RKVec *Zi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKProductIndexZ);  RKVec *Zo_pf = (RKVec *)space->Z[0];
    RKVec *Vi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKProductIndexV);  RKVec *Vo_pf = (RKVec *)space->V[0];
    RKVec *Wi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKProductIndexW);  RKVec *Wo_pf = (RKVec *)space->W[0];
    RKVec *Di_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKProductIndexD);  RKVec *Do_pf = (RKVec *)space->ZDR;
    RKVec *Pi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKProductIndexP);  RKVec *Po_pf = (RKVec *)space->PhiDP;
    RKVec *Ri_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKProductIndexP);  RKVec *Ro_pf = (RKVec *)space->RhoHV;
    for (k = 0; k < K; k++) {
        *Zo_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Zi_pf++, zl), zh), zm), za);
        *Vo_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Vi_pf++, vl), vh), vm), va);
        *Wo_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Wi_pf++, wl), wh), wm), wa);
        *Do_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Di_pf++, dl), dh), dm), da);
        *Po_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Pi_pf++, pl), ph), pm), pa);
        *Ro_pf++ = _rk_mm_min_pf(_rk_mm_max_pf(*Ri_pf++, rl), rh);
    }
    // Convert to uint8 type
    Zi = space->Z[0];  uint8_t *zu = RKGetUInt8DataFromRay(ray, RKProductIndexZ);
    Vi = space->V[0];  uint8_t *vu = RKGetUInt8DataFromRay(ray, RKProductIndexV);
    Wi = space->W[0];  uint8_t *wu = RKGetUInt8DataFromRay(ray, RKProductIndexW);
    Di = space->ZDR;   uint8_t *du = RKGetUInt8DataFromRay(ray, RKProductIndexD);
    Pi = space->PhiDP; uint8_t *pu = RKGetUInt8DataFromRay(ray, RKProductIndexP);
    Ri = space->RhoHV; uint8_t *ru = RKGetUInt8DataFromRay(ray, RKProductIndexR);
    for (k = 0; k < ray->header.gateCount; k++) {
        *zu++ = (uint8_t)*Zi++;
        *vu++ = (uint8_t)*Vi++;
        *wu++ = (uint8_t)*Wi++;
        *du++ = (uint8_t)*Di++;
        *pu++ = (uint8_t)*Pi++;
        *ru++ = (uint8_t)RKRho2Uint8(*Ri++);
    }
    ray->header.productList = RKProductListProductZVWDPR;
    return i;
}

#pragma mark -
#pragma mark Delegate Workers

void *momentCore(void *in) {
    RKMomentWorker *me = (RKMomentWorker *)in;
    RKMomentEngine *engine = me->parentEngine;

    int i, k;
    struct timeval t0, t1, t2;

    // My ID that is suppose to be constant
    const int c = me->id;
    
    // A tag for header identification, will increase by engine->coreCount later
    uint32_t tag = c;

    // Grab the semaphore
    sem_t *sem = me->sem;

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
        k += sprintf(name + k, "M%02d", c);
    } else {
        k += sprintf(name + k, "M%d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(name + k, RKNoColor);
    }

    RKRay *ray;
    RKPulse *pulse;
    RKScratch *space;

    // Allocate local resources and keep track of the total allocation
    ray = RKGetRay(engine->rayBuffer, 0);
    pulse = RKGetPulse(engine->pulseBuffer, 0);
    size_t mem = RKScratchAlloc(&space, pulse->header.capacity, engine->processorLagCount, engine->verbose > 3);
    if (space == NULL) {
        RKLog("Error. Unable to allocate resources for duty cycle calculation\n");
        return (void *)RKResultFailedToAllocateScratchSpace;
    }
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

    // Output index for current ray
    uint32_t io = engine->rayBufferDepth - engine->coreCount + c;
    
    // Update index of the status for current ray
    uint32_t iu = RKBufferSSlotCount - engine->coreCount + c;

    // Start and end indices of the input pulses
    uint32_t is;
    uint32_t ie;

    // Gate size in meters for range correction
    RKFloat gateSizeMeters = 30.0f;

    // The latest index in the dutyCycle buffer
    int d0 = 0;

    // Log my initial state
    pthread_mutex_lock(&engine->coreMutex);
    engine->memoryUsage += mem;
    RKLog(">%s %s started.   i0 = %d   mem = %s B   tic = %d   %s @ %p\n", engine->name, name, io, RKIntegerToCommaStyleString(mem), me->tic, me->semaphoreName, sem);
    pthread_mutex_unlock(&engine->coreMutex);

    // Increase the tic once to indicate this processing core is created.
    me->tic++;

    //
    // Same as in RKPulseCompression.c
    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]
    //
    uint32_t tic = me->tic;

    RKModuloPath path;
    RKPulse *S, *E, *pulses[RKMaxPulsesPerRay];
    uint32_t marker = RKMarkerNull;
    float deltaAzimuth, deltaElevation;
    char *string;
    int stride = pulse->header.capacity / ray->header.capacity;

    char sweepBeginMarker[RKNameLength] = "S", sweepEndMarker[RKNameLength] = "E";
    if (rkGlobalParameters.showColor) {
        sprintf(sweepBeginMarker, "%sS%s", RKGetColorOfIndex(3), RKNoColor);
        sprintf(sweepEndMarker, "%sE%s", RKGetColorOfIndex(2), RKNoColor);
    }

    while (engine->state == RKMomentEngineStateActive) {
        if (engine->useSemaphore) {
            if (sem_wait(sem)) {
                RKLog("Error. Failed in sem_wait(). errno = %d\n", errno);
            }
        } else {
            while (tic == me->tic && engine->state == RKMomentEngineStateActive) {
                usleep(1000);
            }
            tic = me->tic;
        }
        if (engine->state != RKMomentEngineStateActive) {
            break;
        }

        // Something happened
        gettimeofday(&t1, NULL);

        // Start of getting busy
        io = RKNextNModuloS(io, engine->coreCount, engine->rayBufferDepth);
        me->lag = fmodf((float)(*engine->pulseIndex + engine->pulseBufferDepth - me->pid) / engine->pulseBufferDepth, 1.0f);

        ray = RKGetRay(engine->rayBuffer, io);

        // Mark being processed so that the other thread will not override the length
        ray->header.s = RKRayStatusProcessing;
        ray->header.i = tag;

        // The index path of the source of this ray
        path = engine->momentSource[io];

        // Call the assigned moment processor if we are to process, is = indexStart, ie = indexEnd
        is = path.origin;
        ie = RKNextNModuloS(is, path.length, engine->pulseBufferDepth);

        // Latest rayStatusBufferIndex the other end should check (I know there is a horse raise here)
        engine->rayStatusBufferIndex = iu;

        // Start and end pulses to calculate this ray
        S = RKGetPulse(engine->pulseBuffer, is);
        E = RKGetPulse(engine->pulseBuffer, ie);

        // Set the ray headers
        ray->header.startTime       = S->header.time;
        ray->header.startTimeDouble = S->header.timeDouble;
        ray->header.startAzimuth    = S->header.azimuthDegrees;
        ray->header.startElevation  = S->header.elevationDegrees;
        ray->header.endTime         = E->header.time;
        ray->header.endTimeDouble   = E->header.timeDouble;
        ray->header.endAzimuth      = E->header.azimuthDegrees;
        ray->header.endElevation    = E->header.elevationDegrees;
        ray->header.configIndex     = S->header.configIndex;
        ray->header.gateSizeMeters  = S->header.gateSizeMeters;
        marker = RKMarkerNull;

        // Compute the range correction factor if needed.
        if (gateSizeMeters != ray->header.gateSizeMeters) {
            gateSizeMeters = ray->header.gateSizeMeters;
            RKLog("gateSize changed to %.2f\n", gateSizeMeters);
            RKFloat r = 0.0f;
            for (i = 0; i < ray->header.capacity; i++) {
                r = (RKFloat)i * gateSizeMeters;
                space->rcor[i] = 20.0f * log10f(r);
            }
        }

        // Duplicate a linear array for processor if we are to process; otherwise just skip this group
        if (path.length > 3) {
            k = 0;
            i = is;
            do {
                pulse = RKGetPulse(engine->pulseBuffer, i);
                marker |= pulse->header.marker;
                pulses[k++] = pulse;
                i = RKNextModuloS(i, engine->pulseBufferDepth);
            } while (k < path.length);
            if (ie != i) {
                RKLog("%s %s I detected a bug %d vs %d.\n", engine->name, name, ie, i);
            }
            // Call the processor
            k = engine->processor(space, pulses, path.length);
            if (k != path.length) {
                RKLog("%s %s processed %d samples, which is not expected (%d)\n", engine->name, name, k, path.length);
            }
            ray->header.s |= RKRayStatusProcessed;
        } else {
            ray->header.s |= RKRayStatusSkipped;
            if (engine->verbose) {
                RKLog("%s %s skipped a ray with %d sampples.\n", engine->name, name, path.length);
            }
        }

        // Fill in the ray
        makeRayFromScratch(space, ray, S->header.gateCount, stride);
        
        // Update the rest of the ray header
        RKConfig *config = &engine->configBuffer[S->header.configIndex];
        ray->header.sweepElevation = config->sweepElevation;
        ray->header.sweepAzimuth = config->sweepAzimuth;
        ray->header.marker = marker;
        ray->header.s ^= RKRayStatusProcessing;
        ray->header.s |= RKRayStatusReady;

        // Update processed index
        me->pid = ie;

        // Status of the ray
        iu = RKNextNModuloS(iu, engine->coreCount, RKBufferSSlotCount);
        string = engine->rayStatusBuffer[iu];
        i = io * (RKStatusBarWidth + 1) / engine->rayBufferDepth;
        memset(string, '#', i);
        memset(string + i, '.', RKStatusBarWidth - i);

        deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees,   E->header.azimuthDegrees);
        deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);
        snprintf(string + RKStatusBarWidth, RKMaximumStringLength - RKStatusBarWidth,
                 " %05lu | %s  %05lu...%05lu (%3d)  E%4.2f-%.2f (%4.2f)   A%6.2f-%6.2f (%4.2f)   M%05x %s%s",
                 (unsigned long)io, name, (unsigned long)is, (unsigned long)ie, path.length,
                 S->header.elevationDegrees, E->header.elevationDegrees, deltaElevation,
                 S->header.azimuthDegrees,   E->header.azimuthDegrees,   deltaAzimuth,
                 ray->header.marker,
                 ray->header.marker & RKMarkerSweepBegin ? sweepBeginMarker : "",
                 ray->header.marker & RKMarkerSweepEnd ? sweepEndMarker : "");
        
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

        tag += engine->coreCount;

        t2 = t0;
    }

    if (engine->verbose > 1) {
        RKLog("%s %s freeing reources ...\n", engine->name, name);
    }
    
    RKScratchFree(space);
    free(busyPeriods);
    free(fullPeriods);

    RKLog(">%s %s ended.\n", engine->name, name);

    return NULL;
}

void *pulseGatherer(void *in) {
    RKMomentEngine *engine = (RKMomentEngine *)in;

    int c, i, j, k, s;

    sem_t *sem[engine->coreCount];

    // Beam index at t = 0 and t = 1 (previous sample)
    int i0;
    int i1 = 0;
    int count = 0;
    int skipCounter = 0;
    float lag;
    struct timeval t0, t1;

    RKPulse *pulse;
    RKRay *ray;

    // Change the state to active so all the processing cores stay in the busy loop
    engine->state = RKMomentEngineStateActive;

    // Spin off N workers to process I/Q pulses
    for (c = 0; c < engine->coreCount; c++) {
        RKMomentWorker *worker = &engine->workers[c];
        snprintf(worker->semaphoreName, 16, "rk-mm-%02d", c);
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
        if (pthread_create(&worker->tid, NULL, momentCore, worker) != 0) {
            RKLog(">%s Error. Failed to start a moment core.\n", engine->name);
            return (void *)RKResultFailedToStartMomentCore;
        }
    }

    // Wait for the workers to increase the tic count once
    // Using sem_wait here could cause a stolen post within the worker
    // See RKPulseCompression.c
    for (c = 0; c < engine->coreCount; c++) {
        while (engine->workers[c].tic == 0) {
            usleep(1000);
        }
    }

    RKLog("%s started.   mem = %s B   pulseIndex = %d   rayIndex = %d\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex, *engine->rayIndex);
    
    // Increase the tic once to indicate the watcher is ready
    engine->tic++;

    gettimeofday(&t1, 0); t1.tv_sec -= 1;

    // Here comes the busy loop
    j = 0;   // ray index for workers
    k = 0;   // pulse index
    c = 0;   // core index
    while (engine->state == RKMomentEngineStateActive) {
        // The pulse
        pulse = RKGetPulse(engine->pulseBuffer, k);
        // Wait until the buffer is advanced
        s = 0;
        while (k == *engine->pulseIndex && engine->state == RKMomentEngineStateActive) {
            usleep(1000);
            // Timeout and say "nothing" on the screen
            if (++s % 1000 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.001f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        // At this point, three things are happening:
        // A separate thread has checked out a pulse, filling it with data (RKPulseStatusHasIQData);
        // A separate thread waits until it has data and time, then give it a position (RKPulseStatusHasPosition);
        // A separate thread applies matched filter to the data (RKPulseStatusProcessed).
        s = 0;
        while ((pulse->header.s & RKPulseStatusReadyForMoment) != RKPulseStatusReadyForMoment && engine->state == RKMomentEngineStateActive) {
            usleep(1000);
            if (++s % 200 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.001f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        if (engine->state == RKMomentEngineStateActive) {
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
                RKLog("%s Warning. Projected an overflow.  lags = %.2f | %.2f %.2f   j = %d   pulseIndex = %d vs %d\n",
                      engine->name, engine->lag, engine->workers[0].lag, engine->workers[1].lag, j, *engine->pulseIndex, k);
//                RKLog("%s Warning. Projected an overflow.", engine->name);
                // Skip the ray source length to 0 for those that are currenly being or have not been processed. Save the j-th source, which is current.
                i = j;
                do {
                    i = RKPreviousModuloS(i, engine->rayBufferDepth);
                    engine->momentSource[j].length = 0;
                    ray = RKGetRay(engine->rayBuffer, i);
                } while (!(ray->header.s & (RKRayStatusReady | RKRayStatusProcessing)));
            } else if (skipCounter > 0) {
                // Skip processing if we are in skipping mode
                if (--skipCounter == 0 && engine->verbose) {
                    RKLog(">%s Info. Skipped a chunk.   pulseIndex = %d vs %d\n", engine->name, *engine->pulseIndex, k);
                    for (i = 0; i < engine->coreCount; i++) {
                        engine->workers[i].lag = 0.0f;
                    }
                }
            } else {
                // Gather the start and end pulses and post a worker to process for a ray
                i0 = (int)floorf(pulse->header.azimuthDegrees);
                if (i1 != i0 || count == RKMaxPulsesPerRay) {
                    i1 = i0;
                    if (count > 0) {
                        // Number of samples in this ray
                        engine->momentSource[j].length = count;
                        if (engine->useSemaphore) {
                            if (sem_post(sem[c])) {
                                RKLog("%s Error. Failed in sem_post(), errno = %d\n", engine->name, errno);
                            }
                        } else {
                            engine->workers[c].tic++;
                        }
                        // Move to the next core, gather the next ray
                        c = RKNextModuloS(c, engine->coreCount);
                        j = RKNextModuloS(j, engine->rayBufferDepth);
                        // New origin for the next ray
                        engine->momentSource[j].origin = k;
                        ray = RKGetRay(engine->rayBuffer, j);
                        ray->header.s = RKRayStatusVacant;
                        count = 0;
                    } else {
                        // Just started, i0 could be any azimuth bin
                    }
                }
                // Keep counting up
                count++;
            }
            
            // Check finished rays
            ray = RKGetRay(engine->rayBuffer, *engine->rayIndex);
            while (ray->header.s & RKRayStatusReady) {
                *engine->rayIndex = RKNextModuloS(*engine->rayIndex, engine->rayBufferDepth);
                ray = RKGetRay(engine->rayBuffer, *engine->rayIndex);
            }

            // Log a message if it has been a while
            gettimeofday(&t0, NULL);
            if (RKTimevalDiff(t0, t1) > 0.05) {
                t1 = t0;
                engine->processedPulseIndex = k;
                RKMomentUpdateStatusString(engine);
            }
        }
        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->pulseBufferDepth);
    }

    // Wait for workers to return
    for (c = 0; c < engine->coreCount; c++) {
        RKMomentWorker *worker = &engine->workers[c];
        if (engine->useSemaphore) {
            sem_post(sem[c]);
        }
        pthread_join(worker->tid, NULL);
        sem_unlink(worker->semaphoreName);
    }

    return NULL;
}

#pragma mark - Life Cycle

RKMomentEngine *RKMomentEngineInit(void) {
    RKMomentEngine *engine = (RKMomentEngine *)malloc(sizeof(RKMomentEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a momment engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKMomentEngine));
    sprintf(engine->name, "%s<ProductGatherer>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKMomentEngineStateAllocated;
    engine->useSemaphore = true;
    engine->processor = &RKPulsePairHop;
    engine->processorLagCount = RKLagCount;
    engine->memoryUsage = sizeof(RKMomentEngine);
    pthread_mutex_init(&engine->coreMutex, NULL);
    return engine;
}

void RKMomentEngineFree(RKMomentEngine *engine) {
    if (engine->state == RKMomentEngineStateActive) {
        RKMomentEngineStop(engine);
    }
    free(engine->momentSource);
    free(engine);
}

#pragma mark - Properties

void RKMomentEngineSetVerbose(RKMomentEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *engine, RKRadarDesc *desc,
                                         RKConfig *configBuffer, uint32_t *configIndex, const uint32_t configBufferDepth,
                                         RKBuffer pulseBuffer, uint32_t *pulseIndex, const uint32_t pulseBufferDepth,
                                         RKBuffer rayBuffer,   uint32_t *rayIndex,   const uint32_t rayBufferDepth) {
    engine->radarDescription  = desc;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->configBufferDepth = configBufferDepth;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;
    engine->pulseBufferDepth  = pulseBufferDepth;
    engine->rayBuffer         = rayBuffer;
    engine->rayIndex          = rayIndex;
    engine->rayBufferDepth   = rayBufferDepth;
    engine->momentSource = (RKModuloPath *)malloc(rayBufferDepth * sizeof(RKModuloPath));
    if (engine->momentSource == NULL) {
        RKLog("Error. Unable to allocate momentSource.\n");
        exit(EXIT_FAILURE);
    }
    memset(engine->momentSource, 0, rayBufferDepth * sizeof(RKModuloPath));
    for (int i = 0; i < rayBufferDepth; i++) {
        engine->momentSource[i].modulo = pulseBufferDepth;
    }
}

void RKMomentEngineSetCoreCount(RKMomentEngine *engine, const int count) {
    if (engine->state == RKMomentEngineStateActive) {
        RKLog("Error. Core count cannot be changed when the engine is active.\n");
        return;
    }
    engine->coreCount = count;
}

void RKMomentEngineSetMomentProcessorToMultilag(RKMomentEngine *engine) {
    engine->processor = &RKMultiLag;
    engine->processorLagCount = RKLagCount;
}

void RKMomentEngineSetMomentProcessorToPulsePair(RKMomentEngine *engine) {
    engine->processor = &RKPulsePair;
    engine->processorLagCount = 3;
}

void RKMomentEngineSetMomentProcessorToPulsePairHop(RKMomentEngine *engine) {
    engine->processor = &RKPulsePairHop;
    engine->processorLagCount = 2;
}

#pragma mark - Interactions

int RKMomentEngineStart(RKMomentEngine *engine) {
    engine->state = RKMomentEngineStateActivating;
    if (engine->coreCount == 0) {
        engine->coreCount = 4;
    }
    if (engine->workers != NULL) {
        RKLog("Error. RKMomentEngine->workers should be NULL here.\n");
    }
    engine->workers = (RKMomentWorker *)malloc(engine->coreCount * sizeof(RKMomentWorker));
    memset(engine->workers, 0, engine->coreCount * sizeof(RKMomentWorker));
    RKLog("%s starting ...\n", engine->name);
    if (pthread_create(&engine->tidPulseGatherer, NULL, pulseGatherer, engine) != 0) {
        RKLog("Error. Failed to start a pulse watcher.\n");
        return RKResultFailedToStartPulseGatherer;
    }
    while (engine->tic == 0) {
        usleep(10000);
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
    if (engine->verbose > 1) {
        RKLog("%s stopping ...\n", engine->name);
    }
    engine->state = RKMomentEngineStateDeactivating;
    pthread_join(engine->tidPulseGatherer, NULL);
    RKLog("%s stopped.\n", engine->name);
    free(engine->workers);
    engine->workers = NULL;
    engine->state = RKMomentEngineStateNull;
    return RKResultNoError;
}

char *RKMomentEngineStatusString(RKMomentEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
