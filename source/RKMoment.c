//
//  RKMoment.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/20/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKMoment.h>

// Internal Functions

static void RKMomentUpdateStatusString(RKMomentEngine *);
static void zeroOutRay(RKRay *);
static void *momentCore(void *);
static void *pulseGatherer(void *);

// Private Functions (accessible for tests)

int nullProcessor(RKScratch *space, RKPulse **pulses, const uint16_t count);
int downSamplePulses(RKPulse **pulses, const uint16_t count, const int stride);
int makeRayFromScratch(RKScratch *, RKRay *, const int gateCount);

#pragma mark -
#pragma mark Helper Functions

static void RKMomentUpdateStatusString(RKMomentEngine *engine) {
    int i, c;
    char *string = engine->statusBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

    // Use b characters to draw a bar
    i = engine->processedPulseIndex * RKStatusBarWidth / engine->radarDescription->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'M';

    // Engine lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth, " %s%02.0f%s :",
                                    rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
                                    99.49f * engine->lag,
                                    rkGlobalParameters.showColor ? RKNoColor : "");

    RKMomentWorker *worker;

    // Lag from each core
    for (c = 0; c < engine->coreCount; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKStatusStringLength - i, " %s%02.0f%s",
                      rkGlobalParameters.showColor ? RKColorLag(worker->lag) : "",
                      99.49f * worker->lag,
                      rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Put a separator
    i += snprintf(string + i, RKStatusStringLength - i, " ");
    // Duty cycle of each core
    for (c = 0; c < engine->coreCount && i < RKStatusStringLength - RKStatusBarWidth - 20; c++) {
        worker = &engine->workers[c];
        i += snprintf(string + i, RKStatusStringLength - i, " %s%02.0f%s",
                      rkGlobalParameters.showColor ? RKColorDutyCycle(worker->dutyCycle) : "",
                      99.49f * worker->dutyCycle,
                      rkGlobalParameters.showColor ? RKNoColor : "");
    }
    // Almost full count
    //i += snprintf(string + i, RKStatusStringLength - i, " [%d]", engine->almostFull);
    if (i > RKStatusStringLength - RKStatusBarWidth - 20) {
        memset(string + i, '#', RKStatusStringLength - i - 1);
    }
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

int nullProcessor(RKScratch *space, RKPulse **pulses, const uint16_t count) {
    return 0;
}

int downSamplePulses(RKPulse **pulses, const uint16_t count, const int stride) {
    int i, j, k, p;

    // Get the start pulse to know the capacity
    RKPulse *pulse = pulses[0];
    const uint32_t capacity = pulse->header.capacity;
    const uint32_t gateCount = pulse->header.gateCount;

    if (gateCount > capacity) {
        RKLog("Error. gateCount > capacity: %d > %d\n", gateCount, capacity);
    }

    for (i = 0; i < count; i++) {
        RKPulse *pulse = pulses[i];
        if (stride > 1) {
            for (p = 0; p < 2; p++) {
                RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulse, p);
                for (j = 0, k = 0; k < gateCount; j++, k += stride) {
                    Xn.i[j] = Xn.i[k];
                    Xn.q[j] = Xn.q[k];
                }
                if (j != (gateCount + stride - 1) / stride) {
                    RKLog("Equation (314) does not work.  gateCount = %d  stride = %d  i = %d vs %d\n",
                          gateCount, stride, j, (gateCount + stride - 1) / stride);
                }
            }
        }
        pulse->header.gateCount = (gateCount + stride - 1) / stride;
        pulse->header.gateSizeMeters *= (float)stride;
        pulse->header.s |= RKPulseStatusDownSampled;
    }
    return pulse->header.gateCount;
}

int makeRayFromScratch(RKScratch *space, RKRay *ray, const int gateCount) {
    int k;
    // Grab the data from scratch space.
    float *Si = space->S[0];
    float *Zi = space->Z[0],  *Zo = RKGetFloatDataFromRay(ray, RKBaseMomentIndexZ);
    float *Vi = space->V[0],  *Vo = RKGetFloatDataFromRay(ray, RKBaseMomentIndexV);
    float *Wi = space->W[0],  *Wo = RKGetFloatDataFromRay(ray, RKBaseMomentIndexW);
    float *Di = space->ZDR,   *Do = RKGetFloatDataFromRay(ray, RKBaseMomentIndexD);
    float *Pi = space->PhiDP, *Po = RKGetFloatDataFromRay(ray, RKBaseMomentIndexP);
    float *Ki = space->KDP,   *Ko = RKGetFloatDataFromRay(ray, RKBaseMomentIndexK);
    float *Ri = space->RhoHV, *Ro = RKGetFloatDataFromRay(ray, RKBaseMomentIndexR);
    float SNR;
    float SNRThreshold = powf(10.0f, 0.1f * space->SNRThreshold);
    // Masking based on SNR
    for (k = 0; k < gateCount; k++) {
        SNR = *Si / space->noise[0];
        if (SNR > SNRThreshold) {
            *Zo++ = *Zi;
            *Vo++ = *Vi;
            *Wo++ = *Wi;
            *Do++ = *Di;
            *Po++ = *Pi;
            *Ko++ = *Ki;
            *Ro++ = *Ri;
        } else {
            *Zo++ = NAN;
            *Vo++ = NAN;
            *Wo++ = NAN;
            *Do++ = NAN;
            *Po++ = NAN;
            *Ko++ = NAN;
            *Ro++ = NAN;
        }
        Si++;
        Zi++;
        Vi++;
        Wi++;
        Di++;
        Pi++;
        Ki++;
        Ri++;
    }
    // Record down the down-sampled gate count
    ray->header.gateCount = k;
    // Convert float to color representation (0.0 - 255.0) using M * (value) + A; RhoHV is special
    RKFloat lhma[4];
    int K = (ray->header.gateCount * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKZLHMAC   RKVec zl = _rk_mm_set1_pf(lhma[0]);  RKVec zh = _rk_mm_set1_pf(lhma[1]);  RKVec zm = _rk_mm_set1_pf(lhma[2]);  RKVec za = _rk_mm_set1_pf(lhma[3]);
    RKV2LHMAC  RKVec vl = _rk_mm_set1_pf(lhma[0]);  RKVec vh = _rk_mm_set1_pf(lhma[1]);  RKVec vm = _rk_mm_set1_pf(lhma[2]);  RKVec va = _rk_mm_set1_pf(lhma[3]);
    RKWLHMAC   RKVec wl = _rk_mm_set1_pf(lhma[0]);  RKVec wh = _rk_mm_set1_pf(lhma[1]);  RKVec wm = _rk_mm_set1_pf(lhma[2]);  RKVec wa = _rk_mm_set1_pf(lhma[3]);
    RKDLHMAC   RKVec dl = _rk_mm_set1_pf(lhma[0]);  RKVec dh = _rk_mm_set1_pf(lhma[1]);  RKVec dm = _rk_mm_set1_pf(lhma[2]);  RKVec da = _rk_mm_set1_pf(lhma[3]);
    RKPLHMAC   RKVec pl = _rk_mm_set1_pf(lhma[0]);  RKVec ph = _rk_mm_set1_pf(lhma[1]);  RKVec pm = _rk_mm_set1_pf(lhma[2]);  RKVec pa = _rk_mm_set1_pf(lhma[3]);
    RKKLHMAC   RKVec kl = _rk_mm_set1_pf(lhma[0]);  RKVec kh = _rk_mm_set1_pf(lhma[1]);  RKVec km = _rk_mm_set1_pf(lhma[2]);  RKVec ka = _rk_mm_set1_pf(lhma[3]);
    RKRLHMAC   RKVec rl = _rk_mm_set1_pf(lhma[0]);  RKVec rh = _rk_mm_set1_pf(lhma[1]);
    RKVec *Zi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseMomentIndexZ);  RKVec *Zo_pf = (RKVec *)space->Z[0];
    RKVec *Vi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseMomentIndexV);  RKVec *Vo_pf = (RKVec *)space->V[0];
    RKVec *Wi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseMomentIndexW);  RKVec *Wo_pf = (RKVec *)space->W[0];
    RKVec *Di_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseMomentIndexD);  RKVec *Do_pf = (RKVec *)space->ZDR;
    RKVec *Pi_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseMomentIndexP);  RKVec *Po_pf = (RKVec *)space->PhiDP;
    RKVec *Ki_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseMomentIndexK);  RKVec *Ko_pf = (RKVec *)space->KDP;
    RKVec *Ri_pf = (RKVec *)RKGetFloatDataFromRay(ray, RKBaseMomentIndexR);  RKVec *Ro_pf = (RKVec *)space->RhoHV;
    for (k = 0; k < K; k++) {
        *Zo_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Zi_pf++, zl), zh), zm), za);
        *Vo_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Vi_pf++, vl), vh), vm), va);
        *Wo_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Wi_pf++, wl), wh), wm), wa);
        *Do_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Di_pf++, dl), dh), dm), da);
        *Po_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Pi_pf++, pl), ph), pm), pa);
        *Ko_pf++ = _rk_mm_add_pf(_rk_mm_mul_pf(_rk_mm_min_pf(_rk_mm_max_pf(*Ki_pf++, kl), kh), km), ka);
        *Ro_pf++ = _rk_mm_min_pf(_rk_mm_max_pf(*Ri_pf++, rl), rh);
    }
    // Convert to uint8 type
    Zi = space->Z[0];  uint8_t *zu = RKGetUInt8DataFromRay(ray, RKBaseMomentIndexZ);
    Vi = space->V[0];  uint8_t *vu = RKGetUInt8DataFromRay(ray, RKBaseMomentIndexV);
    Wi = space->W[0];  uint8_t *wu = RKGetUInt8DataFromRay(ray, RKBaseMomentIndexW);
    Di = space->ZDR;   uint8_t *du = RKGetUInt8DataFromRay(ray, RKBaseMomentIndexD);
    Pi = space->PhiDP; uint8_t *pu = RKGetUInt8DataFromRay(ray, RKBaseMomentIndexP);
    Ki = space->KDP;   uint8_t *ku = RKGetUInt8DataFromRay(ray, RKBaseMomentIndexK);
    Ri = space->RhoHV; uint8_t *ru = RKGetUInt8DataFromRay(ray, RKBaseMomentIndexR);
    for (k = 0; k < ray->header.gateCount; k++) {
        if (isfinite(*Zi)) {
            *zu++ = (uint8_t)*Zi;
            *vu++ = (uint8_t)*Vi;
            *wu++ = (uint8_t)*Wi;
            *du++ = (uint8_t)*Di;
            *pu++ = (uint8_t)*Pi;
            *ku++ = (uint8_t)*Ki;
            *ru++ = (uint8_t)RKRho2Uint8(*Ri);
        } else {
            // Uint8 = 0 = transparent color
            *zu++ = 0;
            *vu++ = 0;
            *wu++ = 0;
            *du++ = 0;
            *pu++ = 0;
            *ku++ = 0;
            *ru++ = 0;
        }
        Zi++;
        Vi++;
        Wi++;
        Di++;
        Pi++;
        Ki++;
        Ri++;
    }
    // If the space has been used for the same gateCount calculations, it should remain zero
    if (*Zi != 0.0 || *zu != 0) {
        memset(zu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(vu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(wu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(du, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(pu, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(ku, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(ru, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(uint8_t));
        memset(Zi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Vi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Wi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Di, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Pi, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Ki, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        memset(Ri, 0, (ray->header.capacity - ray->header.gateCount) * sizeof(RKFloat));
        ray->header.marker |= RKMarkerMemoryManagement;
    }
    ray->header.baseMomentList = RKBaseMomentListProductZVWDPRK;
    return k;
}

static void zeroOutRay(RKRay *ray) {
    memset(ray->data, 0, RKBaseMomentCount * ray->header.capacity * (sizeof(uint8_t) + sizeof(float)));
}

#pragma mark -
#pragma mark Delegate Workers

static void *momentCore(void *in) {
    RKMomentWorker *me = (RKMomentWorker *)in;
    RKMomentEngine *engine = me->parent;

    int i, k, p;
    struct timeval t0, t1, t2;

    // My ID that is suppose to be constant
    const int c = me->id;
    const int ci = engine->radarDescription->initFlags & RKInitFlagManuallyAssignCPU ? engine->coreOrigin + c : -1;

    // A tag for header identification, will increase by engine->coreCount later
    uint32_t tag = c;

    // Grab the semaphore
    sem_t *sem = me->sem;

    // Initiate my name
    if (rkGlobalParameters.showColor) {
        pthread_mutex_lock(&engine->mutex);
        k = snprintf(me->name, RKShortNameLength - 1, "%s", rkGlobalParameters.showColor ? RKGetColor() : "");
        pthread_mutex_unlock(&engine->mutex);
    } else {
        k = 0;
    }
    if (engine->coreCount > 9) {
        k += sprintf(me->name + k, "M%02d", c);
    } else {
        k += sprintf(me->name + k, "M%d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(me->name + k, RKNoColor);
    }
    
#if defined(_GNU_SOURCE)
    
    if (engine->radarDescription->initFlags & RKInitFlagManuallyAssignCPU) {
        // Set my CPU core
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(ci, &cpuset);
        sched_setaffinity(0, sizeof(cpuset), &cpuset);
        pthread_setaffinity_np(me->tid, sizeof(cpu_set_t), &cpuset);
    }

#endif

    RKRay *ray;
    RKPulse *pulse;
    RKConfig *config;
    RKScratch *space;

    // Allocate local resources and keep track of the total allocation
    pulse = RKGetPulse(engine->pulseBuffer, 0);
    uint32_t capacity = (uint32_t)ceilf((float)pulse->header.capacity * sizeof(RKFloat) / RKSIMDAlignSize) * RKSIMDAlignSize / sizeof(RKFloat);
    size_t mem = RKScratchAlloc(&space, capacity, engine->processorLagCount, engine->verbose > 3);
    if (space == NULL) {
        RKLog("Error. Unable to allocate resources for duty cycle calculation\n");
        exit(EXIT_FAILURE);
    }
	if (engine->userLagChoice != 0) {
		space->userLagChoice = engine->userLagChoice;
	}
    double *busyPeriods, *fullPeriods;
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&busyPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&fullPeriods, RKSIMDAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    if (busyPeriods == NULL || fullPeriods == NULL) {
        RKLog("Error. Unable to allocate resources for duty cycle calculation\n");
        exit(EXIT_FAILURE);
    }
    mem += 2 * RKWorkerDutyCycleBufferDepth * sizeof(double);
    memset(busyPeriods, 0, RKWorkerDutyCycleBufferDepth * sizeof(double));
    memset(fullPeriods, 0, RKWorkerDutyCycleBufferDepth * sizeof(double));
    double allBusyPeriods = 0.0, allFullPeriods = 0.0;

    // Initialize some end-of-loop variables
    gettimeofday(&t0, NULL);
    gettimeofday(&t2, NULL);

    // Output index for current ray
    uint32_t io = engine->radarDescription->rayBufferDepth - engine->coreCount + c;

    // Update index of the status for current ray
    uint32_t iu = RKBufferSSlotCount - engine->coreCount + c;

    // Start and end indices of the input pulses
    uint32_t is;
    uint32_t ie;

    // Configuration index
    uint32_t ic = 0;

    // Gate size in meters for range correction
    RKFloat gateSizeMeters = 0.0f;

    // The latest index in the dutyCycle buffer
    int d0 = 0;

    // Log my initial state
    pthread_mutex_lock(&engine->mutex);
    engine->memoryUsage += mem;
    
    RKLog(">%s %s Started.   mem = %s B   i0 = %s   ci = %d\n",
          engine->name, me->name, RKUIntegerToCommaStyleString(mem), RKIntegerToCommaStyleString(io), ci);

    pthread_mutex_unlock(&engine->mutex);

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
    uint64_t tic = me->tic;

    RKModuloPath path;
    RKPulse *S, *E, *pulses[RKMaximumPulsesPerRay];
    RKMarker marker = RKMarkerNull;
    float deltaAzimuth, deltaElevation;
    char *string;

    char sweepBeginMarker[20] = "S", sweepEndMarker[20] = "E";
    if (rkGlobalParameters.showColor) {
        sprintf(sweepBeginMarker, "%sS%s", RKGetColorOfIndex(3), RKNoColor);
        sprintf(sweepEndMarker, "%sE%s", RKGetColorOfIndex(2), RKNoColor);
    }

    while (engine->state & RKEngineStateWantActive) {
        if (engine->useSemaphore) {
            if (sem_wait(sem)) {
                RKLog("Error. Failed in sem_wait(). errno = %d\n", errno);
            }
        } else {
            while (tic == me->tic && engine->state & RKEngineStateWantActive) {
                usleep(1000);
            }
            tic = me->tic;
        }
        if (!(engine->state & RKEngineStateWantActive)) {
            break;
        }

        // Something happened
        gettimeofday(&t1, NULL);

        // Start of getting busy
        io = RKNextNModuloS(io, engine->coreCount, engine->radarDescription->rayBufferDepth);

        // The index path of the source of this ray
        path = engine->momentSource[io];
        if (path.origin > engine->radarDescription->pulseBufferDepth || path.length > engine->radarDescription->pulseBufferDepth) {
            RKLog("%s Warning. Unexpected path->origin = %d   path->length = %d   io = %d\n", engine->name, path.origin, path.length, io);
            path.origin = 0;
            path.length = 1;
        }

        // Call the assigned moment processor if we are to process, is = indexStart, ie = indexEnd
        is = path.origin;
        ie = RKNextNModuloS(is, path.length, engine->radarDescription->pulseBufferDepth);

        // Latest rayStatusBufferIndex the other end should check (I know there is a horse raise here)
        engine->rayStatusBufferIndex = iu;

        // Start and end pulses to calculate this ray
        S = RKGetPulse(engine->pulseBuffer, is);
        E = RKGetPulse(engine->pulseBuffer, ie);

        // Beamwidth
        deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees,   E->header.azimuthDegrees);
        deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);

        // My ray
        ray = RKGetRay(engine->rayBuffer, io);

        // Mark being processed so that the other thread will not override the length
        ray->header.s = RKRayStatusProcessing;
        ray->header.i = tag;

        // Set the ray headers
        ray->header.startTime       = S->header.time;
        ray->header.startTimeDouble = S->header.timeDouble;
        ray->header.startAzimuth    = S->header.azimuthDegrees;
        ray->header.startElevation  = S->header.elevationDegrees;
        ray->header.endTime         = E->header.time;
        ray->header.endTimeDouble   = E->header.timeDouble;
        ray->header.endAzimuth      = E->header.azimuthDegrees;
        ray->header.endElevation    = E->header.elevationDegrees;
        ray->header.configIndex     = E->header.configIndex;
        ray->header.gateCount       = S->header.downSampledGateCount;
        ray->header.gateSizeMeters  = S->header.gateSizeMeters * engine->radarDescription->pulseToRayRatio;

        config = &engine->configBuffer[E->header.configIndex];

        // Compute the range correction factor if needed.
        if (ic != E->header.configIndex) {
            ic = E->header.configIndex;
            // At this point, gateSizeMeters is no longer the spacing of raw pulse, it has been down-sampled according to pulseToRayRatio
            gateSizeMeters = E->header.gateSizeMeters;
            if (engine->verbose > 1) {
                RKLog("%s %s RCor @ filterCount = %d   capacity = %s   C%02d\n",
                      engine->name, me->name,
                      config->filterCount,
                      RKIntegerToCommaStyleString(ray->header.capacity),
                      ic);
            }
            // Because the pulse-compression engine uses unity noise gain filters, there is an inherent gain difference at different sampling rate
            // The gain difference is compensated here with a calibration factor if raw-sampling is at 1-MHz (150-m)
            // The number 60 is for conversion of range from meters to kilometers in the range correction term.
            RKFloat f = 10.0f * log10f(gateSizeMeters / (150.0f * engine->radarDescription->pulseToRayRatio)) + 60.0f;
            RKFloat r = 0.0f;
            for (k = 0; k < config->filterCount; k++) {
                for (i = config->filterAnchors[k].outputOrigin; i < MIN(config->filterAnchors[k].outputOrigin + config->filterAnchors[k].maxDataLength, ray->header.gateCount); i++) {
                    r = (RKFloat)i * gateSizeMeters;
                    for (p = 0; p < 2; p++) {
                        space->rcor[p][i] = 20.0f * log10f(r) + config->systemZCal[p] + config->ZCal[k][p] - config->filterAnchors[k].sensitivityGain - f;
                    }
                    space->dcal[i] = config->systemDCal + config->DCal[k];
                    space->pcal[i] = RKSingleWrapTo2PI(config->systemPCal + config->PCal[k]);
                }
                if (engine->verbose > 1) {
                    for (p = 0; p < 2; p++) {
                        RKLog(">%s %s ZCal[%d][%s] = %.2f + %.2f - %.2f - %.2f = %.2f dB @ %d ..< %d\n",
                              engine->name, me->name, k,
                              p == 0 ? "H" : (p == 1 ? "V" : "-"),
                              config->systemZCal[p],
                              config->ZCal[k][p],
                              config->filterAnchors[k].sensitivityGain,
                              f,
                              config->ZCal[k][p] + config->systemZCal[p] - config->filterAnchors[k].sensitivityGain - f,
                              config->filterAnchors[k].outputOrigin, config->filterAnchors[k].outputOrigin + config->filterAnchors[k].maxDataLength);
                    }
                }
            }
            space->noise[0] = config->noise[0];
            space->noise[1] = config->noise[1];
            space->SNRThreshold = config->SNRThreshold;
            space->velocityFactor = 0.25f * engine->radarDescription->wavelength * config->prf[0] / M_PI;
            space->widthFactor = engine->radarDescription->wavelength * config->prf[0] / (2.0f * sqrtf(2.0f) * M_PI);
            space->KDPFactor = 1.0f / S->header.gateSizeMeters;
        }

        // Consolidate the pulse marker into ray marker
        marker = RKMarkerNull;
        i = is;
        k = 0;
        do {
            pulse = RKGetPulse(engine->pulseBuffer, i);
            marker |= pulse->header.marker;
            pulses[k++] = pulse;
            i = RKNextModuloS(i, engine->radarDescription->pulseBufferDepth);
        } while (k < path.length);
        
        // Duplicate a linear array for processor if we are to process; otherwise just skip this group
        if (path.length > 3 && deltaAzimuth < 3.0f && deltaElevation < 3.0f) {
            if (ie != i) {
                RKLog("%s %s I detected a bug %d vs %d.\n", engine->name, me->name, ie, i);
            }
            // Call the processor
            k = engine->processor(space, pulses, path.length);
            if (k != path.length) {
                RKLog("%s %s processed %d samples, which is not expected (%d)\n", engine->name, me->name, k, path.length);
            }
            // Fill in the ray
            makeRayFromScratch(space, ray, ray->header.gateCount);
            for (k = 0; k < path.length; k++) {
                pulse = pulses[k];
                pulse->header.s |= RKPulseStatusUsedForMoments;
            }
            ray->header.s |= RKRayStatusProcessed;
        } else {
            // Zero out the ray
            zeroOutRay(ray);
            if (engine->verbose > 1) {
                RKLog("%s %s Skipped a ray with %d sample%s   deltaAz = %.2f   deltaEl = %.2f.\n", engine->name, me->name,
                      path.length, path.length > 1 ? "s": "", deltaAzimuth, deltaElevation);
            }
            ray->header.s |= RKRayStatusSkipped;
        }

        // Update the rest of the ray header
        ray->header.sweepElevation = config->sweepElevation;
        ray->header.sweepAzimuth = config->sweepAzimuth;
        ray->header.pulseCount = path.length;
        ray->header.marker = marker;
        ray->header.s ^= RKRayStatusProcessing;
        ray->header.s |= RKRayStatusReady;

        // Status of the ray
        iu = RKNextNModuloS(iu, engine->coreCount, RKBufferSSlotCount);
        string = engine->rayStatusBuffer[iu];
        i = io * (RKStatusBarWidth + 1) / engine->radarDescription->rayBufferDepth;
        memset(string, '.', RKStatusBarWidth);
        string[i] = '#';

        // Summary of this ray
        snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth,
                 " %05u %s | %05u - %05u (%3d)  [C%02d %s E%.2f A%.2f]   %s%6.2f-%6.2f (%4.2f)  G%s  M%05x %s%s",
                 (unsigned int)io, me->name, (unsigned int)is, (unsigned int)ie, path.length,
                 ray->header.configIndex,
                 RKMarkerScanTypeShortString(ray->header.marker),
                 engine->configBuffer[ray->header.configIndex].sweepElevation, engine->configBuffer[ray->header.configIndex].sweepAzimuth,
                 (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "E" : "A",
                 (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? S->header.elevationDegrees : S->header.azimuthDegrees,
                 (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? E->header.elevationDegrees : E->header.azimuthDegrees,
                 (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? deltaElevation : deltaAzimuth,
                 RKIntegerToCommaStyleString(ray->header.gateCount),
                 ray->header.marker,
                 ray->header.marker & RKMarkerSweepBegin ? sweepBeginMarker : "",
                 ray->header.marker & RKMarkerSweepEnd ? sweepEndMarker : "");

       // Update processed index
        me->pid = ie;
        me->lag = fmodf((float)(*engine->pulseIndex + engine->radarDescription->pulseBufferDepth - me->pid) / engine->radarDescription->pulseBufferDepth, 1.0f);

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
        RKLog("%s %s Freeing reources ...\n", engine->name, me->name);
    }
    
    RKScratchFree(space);
    free(busyPeriods);
    free(fullPeriods);

    RKLog(">%s %s Stopped.\n", engine->name, me->name);

    return NULL;
}

static void *pulseGatherer(void *_in) {
    RKMomentEngine *engine = (RKMomentEngine *)_in;

    int c, i, j, k, s;
	struct timeval t0, t1;
	float lag;

	sem_t *sem[engine->coreCount];

	unsigned int skipCounter = 0;

	// Beam index at t = 0 and t = 1 (previous sample)
    int i0;
    int i1 = 0;
    int count = 0;

	RKPulse *pulse;
    RKRay *ray;
	RKMarker marker;

    // Show the selected moment processor
    if (engine->verbose) {
        if (engine->processor == &RKMultiLag) {
            RKLog(">%s Method = RKMultiLag @ %d\n", engine->name, engine->userLagChoice);
        } else if (engine->processor == &RKPulsePairHop) {
            RKLog(">%s Method = RKPulsePairHop()\n", engine->name);
        } else if (engine->processor == &nullProcessor) {
            RKLog(">%s Warning. No moment processor.\n", engine->name);
        } else {
            RKLog(">%s Method %p not recognized\n", engine->name, engine->processor);
        }
    }

	// Update the engine state
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;

    // Spin off N workers to process I/Q pulses
    memset(sem, 0, engine->coreCount * sizeof(sem_t *));
    for (c = 0; c < engine->coreCount; c++) {
        RKMomentWorker *worker = &engine->workers[c];
        snprintf(worker->semaphoreName, 32, "rk-mm-%02d", c);
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
        worker->parent = engine;
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
    engine->state |= RKEngineStateSleep0;
    for (c = 0; c < engine->coreCount; c++) {
        while (engine->workers[c].tic == 0) {
            usleep(1000);
        }
    }
    engine->state ^= RKEngineStateSleep0;
    engine->state |= RKEngineStateActive;

    RKLog("%s Started.   mem = %s B   pulseIndex = %d   rayIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex, *engine->rayIndex);

	// Increase the tic once to indicate the watcher is ready
	engine->tic = 1;

    gettimeofday(&t1, NULL); t1.tv_sec -= 1;

    // Here comes the busy loop
    j = 0;   // ray index for workers
    k = 0;   // pulse index
    c = 0;   // core index
    while (engine->state & RKEngineStateWantActive) {
        // The pulse
        pulse = RKGetPulse(engine->pulseBuffer, k);
        
        // Wait until the buffer is advanced
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->pulseIndex && engine->state & RKEngineStateWantActive) {
            usleep(1000);
            // Timeout and say "nothing" on the screen
            if (++s % 1000 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.001f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // At this point, three things are happening:
        // A separate thread has checked out a pulse, filling it with data (RKPulseStatusHasIQData);
        // A separate thread waits until it has data and time, then give it a position (RKPulseStatusHasPosition);
        // A separate thread applies matched filter to the data (RKPulseStatusProcessed).
        s = 0;
        while ((pulse->header.s & RKPulseStatusReadyForMoments) != RKPulseStatusReadyForMoments && engine->state & RKEngineStateWantActive) {
            usleep(1000);
            if (++s % 200 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.001f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateWantActive)) {
            break;
        }

        // Lag of the engine
        engine->lag = fmodf(((float)*engine->pulseIndex + engine->radarDescription->pulseBufferDepth - k) / engine->radarDescription->pulseBufferDepth, 1.0f);

        // Assess the lag of the workers
        lag = engine->workers[0].lag;
        for (i = 1; i < engine->coreCount; i++) {
            lag = MAX(lag, engine->workers[i].lag);
        }
        if (skipCounter == 0 && lag > 0.9f) {
            engine->almostFull++;
            skipCounter = engine->radarDescription->pulseBufferDepth / 10;
            RKLog("%s Warning. Projected an overflow.  lags = %.2f | %.2f %.2f   j = %d   pulseIndex = %d vs %d\n",
                  engine->name, engine->lag, engine->workers[0].lag, engine->workers[1].lag, j, *engine->pulseIndex, k);
            // Skip the ray: set source length to 0 for those that are currenly being or have not been processed. Save the j-th source, which is current.
            i = j;
            do {
                i = RKPreviousModuloS(i, engine->radarDescription->rayBufferDepth);
                engine->momentSource[i].length = 0;
                ray = RKGetRay(engine->rayBuffer, i);
            } while (!(ray->header.s & RKRayStatusReady) && engine->state & RKEngineStateWantActive);
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
            marker = engine->configBuffer[pulse->header.configIndex].startMarker;
            if ((marker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI) {
                i0 = (int)floorf(pulse->header.azimuthDegrees);
            } else if ((marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI) {
                i0 = (int)floorf(pulse->header.elevationDegrees);
            } else {
                i0 = 360 * (int)floorf(pulse->header.elevationDegrees - 0.25f) + (int)floorf(pulse->header.azimuthDegrees);
            }
            if (i1 != i0 || count == RKMaximumPulsesPerRay) {
                i1 = i0;
                if (count > 0) {
                    // Number of samples in this ray
                    engine->momentSource[j].length = count;
                    //printf("%s k = %d --> momentSource[%d] = %d / %d / %d\n", engine->name, k, j, engine->momentSource[j].origin, engine->momentSource[j].length, engine->momentSource[j].modulo);
                    if (engine->useSemaphore) {
                        if (sem_post(sem[c])) {
                            RKLog("%s Error. Failed in sem_post(), errno = %d\n", engine->name, errno);
                        }
                    } else {
                        engine->workers[c].tic++;
                    }
                    // Move to the next core, gather pulses for the next ray
                    c = RKNextModuloS(c, engine->coreCount);
                    j = RKNextModuloS(j, engine->radarDescription->rayBufferDepth);
                    // New origin for the next ray
                    engine->momentSource[j].origin = k;
                    ray = RKGetRay(engine->rayBuffer, j);
                    ray->header.s = RKRayStatusVacant;
                    count = 0;
                } else {
                    // Just started, i0 could refer to any azimuth bin
                }
            }
            // Keep counting up
            count++;
        }
        
        // Check finished rays
        ray = RKGetRay(engine->rayBuffer, *engine->rayIndex);
        while (ray->header.s & RKRayStatusReady && engine->state & RKEngineStateWantActive) {
            *engine->rayIndex = RKNextModuloS(*engine->rayIndex, engine->radarDescription->rayBufferDepth);
            ray = RKGetRay(engine->rayBuffer, *engine->rayIndex);
        }

        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            engine->processedPulseIndex = k;
            RKMomentUpdateStatusString(engine);
        }

		engine->tic++;

        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->radarDescription->pulseBufferDepth);
    }

    // Wait for workers to return
    for (c = 0; c < engine->coreCount; c++) {
        RKMomentWorker *worker = &engine->workers[c];
        if (engine->useSemaphore) {
            sem_post(worker->sem);
        }
        pthread_join(worker->tid, NULL);
        sem_unlink(worker->semaphoreName);
    }

    engine->state ^= RKEngineStateActive;
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
    sprintf(engine->name, "%s<MomentGenerator>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorMomentEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->useSemaphore = true;
    engine->processor = &RKPulsePairHop;
    engine->processorLagCount = RKMaximumLagCount;
    engine->memoryUsage = sizeof(RKMomentEngine);
    pthread_mutex_init(&engine->mutex, NULL);
    return engine;
}

void RKMomentEngineFree(RKMomentEngine *engine) {
    if (engine->state & RKEngineStateWantActive) {
        RKMomentEngineStop(engine);
    }
    free(engine->momentSource);
    free(engine);
}

#pragma mark - Properties

void RKMomentEngineSetVerbose(RKMomentEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKMomentEngineSetInputOutputBuffers(RKMomentEngine *engine, const RKRadarDesc *desc,
                                         RKConfig *configBuffer, uint32_t *configIndex,
                                         RKBuffer pulseBuffer,   uint32_t *pulseIndex,
                                         RKBuffer rayBuffer,     uint32_t *rayIndex) {
    engine->radarDescription  = (RKRadarDesc *)desc;
    engine->configBuffer      = configBuffer;
    engine->configIndex       = configIndex;
    engine->pulseBuffer       = pulseBuffer;
    engine->pulseIndex        = pulseIndex;
    engine->rayBuffer         = rayBuffer;
    engine->rayIndex          = rayIndex;
    
    size_t bytes = engine->radarDescription->rayBufferDepth * sizeof(RKModuloPath);

    engine->momentSource = (RKModuloPath *)malloc(bytes);
    if (engine->momentSource == NULL) {
        RKLog("Error. Unable to allocate momentSource.\n");
        exit(EXIT_FAILURE);
    }
    memset(engine->momentSource, 0, bytes);
    for (int i = 0; i < engine->radarDescription->rayBufferDepth; i++) {
        engine->momentSource[i].modulo = engine->radarDescription->pulseBufferDepth;
    }
    engine->state |= RKEngineStateProperlyWired;
}

void RKMomentEngineSetCoreCount(RKMomentEngine *engine, const uint8_t count) {
    if (engine->state & RKEngineStateWantActive) {
        RKLog("Error. Core count cannot be changed when the engine is active.\n");
        return;
    }
    engine->coreCount = count;
}

void RKMomentEngineSetCoreOrigin(RKMomentEngine *engine, const uint8_t origin) {
    if (engine->state & RKEngineStateWantActive) {
        RKLog("Error. Core origin cannot be changed when the engine is active.\n");
        return;
    }
    engine->coreOrigin = origin;
}

#pragma mark - Interactions

int RKMomentEngineStart(RKMomentEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    if (engine->coreCount == 0) {
        engine->coreCount = 4;
    }
    if (engine->workers != NULL) {
        RKLog("Error. RKMomentEngine->workers should be NULL here.\n");
    }
    engine->workers = (RKMomentWorker *)malloc(engine->coreCount * sizeof(RKMomentWorker));
    memset(engine->workers, 0, engine->coreCount * sizeof(RKMomentWorker));
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidPulseGatherer, NULL, pulseGatherer, engine) != 0) {
        RKLog("Error. Failed to start a pulse watcher.\n");
        return RKResultFailedToStartPulseGatherer;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKMomentEngineStop(RKMomentEngine *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose > 1) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
	if (!(engine->state & RKEngineStateWantActive)) {
		RKLog("%s Not active.\n", engine->name);
		return RKResultEngineDeactivatedMultipleTimes;
	}
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateWantActive;
    if (engine->tidPulseGatherer) {
        pthread_join(engine->tidPulseGatherer, NULL);
		engine->tidPulseGatherer = (pthread_t)0;
        free(engine->workers);
        engine->workers = NULL;
	} else {
		RKLog("%s Invalid thread ID.\n", engine->name);
    }
    engine->state ^= RKEngineStateDeactivating;
    RKLog("%s Stopped.\n", engine->name);
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

char *RKMomentEngineStatusString(RKMomentEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
