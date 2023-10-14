//
//  RKMomentEngine.c
//  RadarKit
//
//  Created by Boonleng Cheong on 9/20/15.
//  Copyright (c) 2015-2022 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKMomentEngine.h>

// Internal Functions

static void RKMomentEngineUpdateStatusString(RKMomentEngine *);
static void zeroOutRay(RKRay *);
static void *momentCore(void *);
static void *pulseGatherer(void *);

// Private Functions (accessible for tests)

int downSamplePulses(RKPulse **pulses, const uint16_t count, const int stride);

#pragma mark - Helper Functions

static void RKMomentEngineUpdateStatusString(RKMomentEngine *engine) {
    int i, c;
    char *string = engine->statusBuffer[engine->statusBufferIndex];
    bool useCompact = engine->coreCount >= 4;

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

    // Use b characters to draw a bar
    i = engine->processedPulseIndex * RKStatusBarWidth / engine->radarDescription->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'M';

    // Engine lead-lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth, " %s%02.0f%s :%s",
                                    rkGlobalParameters.statusColor ? RKColorLag(engine->lag) : "",
                                    99.49f * engine->lag,
                                    rkGlobalParameters.statusColor ? RKNoColor : "",
                                    useCompact ? " " : "");

    RKMomentWorker *worker;

    // State: 0 - green, 1 - yellow, 2 - red
    int s1 = -1, s0 = 0;

    // Lag from each core
    for (c = 0; c < engine->coreCount; c++) {
        worker = &engine->workers[c];
        s0 = (worker->lag > RKLagRedThreshold ? 2 : (worker->lag > RKLagOrangeThreshold ? 1 : 0));
        if (s1 != s0 && rkGlobalParameters.statusColor) {
            s1 = s0;
            i += snprintf(string + i, RKStatusStringLength - i, "%s",
                          s0 == 2 ? RKBaseRedColor : (s0 == 1 ? RKBaseYellowColor : RKBaseGreenColor));
        }
        if (useCompact) {
            i += snprintf(string + i, RKStatusStringLength - i, "%01.0f", 9.49f * worker->lag);
        } else {
            i += snprintf(string + i, RKStatusStringLength - i, " %02.0f", 99.49f * worker->lag);
        }
    }

    // Put a separator
    i += snprintf(string + i, RKStatusStringLength - i, " ");
    // Duty cycle of each core
    for (c = 0; c < engine->coreCount && i < RKStatusStringLength - RKStatusBarWidth - 20; c++) {
        worker = &engine->workers[c];
        s0 = (worker->dutyCycle > RKDutyCyleRedThreshold ? 2 : (worker->dutyCycle > RKDutyCyleOrangeThreshold ? 1 : 0));
        if (s1 != s0 && rkGlobalParameters.statusColor) {
            s1 = s0;
            i += snprintf(string + i, RKStatusStringLength - i, "%s",
                          s0 == 2 ? RKBaseRedColor : (s0 == 1 ? RKBaseYellowColor : RKBaseGreenColor));
        }
        if (useCompact) {
            i += snprintf(string + i, RKStatusStringLength - i, "%01.0f", 9.49f * worker->dutyCycle);
        } else {
            i += snprintf(string + i, RKStatusStringLength - i, " %02.0f", 99.49f * worker->dutyCycle);
        }
    }
    if (rkGlobalParameters.statusColor) {
        i += snprintf(string + i, RKStatusStringLength - i, "%s", RKNoColor);
    }

    // Concluding string
    if (i > RKStatusStringLength - RKStatusBarWidth - 20) {
        memset(string + i, '#', RKStatusStringLength - i - 1);
    }
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

static void RKMomentEngineCheckWiring(RKMomentEngine *engine) {
    if (engine->radarDescription == NULL ||
        engine->configBuffer == NULL || engine->configIndex == NULL ||
        engine->pulseBuffer == NULL || engine->pulseIndex == NULL ||
        engine->rayBuffer == NULL || engine->rayIndex == NULL ||
        engine->fftModule == NULL) {
        engine->state &= ~RKEngineStateProperlyWired;
        return;
    }
    engine->state |= RKEngineStateProperlyWired;
}

static void RKMomentEngineUpdateMinMaxWorkerLag(RKMomentEngine *engine) {
    int i;
    float minWorkerLag = engine->workers[0].lag;
    float maxWorkerLag = engine->workers[0].lag;
    for (i = 1; i < engine->coreCount; i++) {
        minWorkerLag = MIN(minWorkerLag, engine->workers[i].lag);
        maxWorkerLag = MAX(maxWorkerLag, engine->workers[i].lag);
    }
    engine->minWorkerLag = minWorkerLag;
    engine->maxWorkerLag = maxWorkerLag;
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

static void zeroOutRay(RKRay *ray) {
    //memset(ray->data, 0, RKBaseProductCount * ray->header.capacity * (sizeof(uint8_t) + sizeof(float)));
    RKFloat *f = RKGetFloatDataFromRay(ray, RKBaseProductIndexZ);
    for (int k = 0; k < RKBaseProductCount * ray->header.capacity; k++) {
        *f++ = NAN;
    }
    uint8_t *u = RKGetUInt8DataFromRay(ray, RKBaseProductIndexZ);
    memset(u, 0, RKBaseProductCount * sizeof(uint8_t));
}

int RKNoiseFromConfig(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount) {
    space->noise[0] = space->config->noise[0];
    space->noise[1] = space->config->noise[1];
    return RKResultSuccess;
}

#pragma mark - Delegate Workers

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
    RKShortName name;
    if (rkGlobalParameters.showColor) {
        pthread_mutex_lock(&engine->mutex);
        k = snprintf(name, RKShortNameLength, "%s", rkGlobalParameters.showColor ? RKGetColor() : "");
        pthread_mutex_unlock(&engine->mutex);
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
    snprintf(me->name, RKChildNameLength, "%s %s", engine->name, name);

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

    // Business calculation
    double *busyPeriods, *fullPeriods;
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&busyPeriods, RKMemoryAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&fullPeriods, RKMemoryAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    if (busyPeriods == NULL || fullPeriods == NULL) {
        RKLog("%s Error. Unable to allocate resources for duty cycle calculation\n", me->name);
        exit(EXIT_FAILURE);
    }
    memset(busyPeriods, 0, RKWorkerDutyCycleBufferDepth * sizeof(double));
    memset(fullPeriods, 0, RKWorkerDutyCycleBufferDepth * sizeof(double));
    double allBusyPeriods = 0.0, allFullPeriods = 0.0;

    RKConfig *previousConfig = (RKConfig *)malloc(sizeof(RKConfig));
    memset(previousConfig, 0, sizeof(RKConfig));

    size_t mem = 2 * RKWorkerDutyCycleBufferDepth * sizeof(double) + sizeof(RKConfig);

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

    // The latest index in the dutyCycle buffer
    int d0 = 0;

    // Log my initial state
    pthread_mutex_lock(&engine->mutex);

    // Allocate local resources and keep track of the total allocation
    RKMomentScratch *space;
    const uint32_t capacity = engine->radarDescription->pulseCapacity / engine->radarDescription->pulseToRayRatio;
    engine->memoryUsage += RKMomentScratchAlloc(&space, capacity, engine->verbose, me->name);
    if (space == NULL || mem == 0) {
        exit(EXIT_FAILURE);
    }

    // Pass down other parameters in scratch space
    space->config = &engine->configBuffer[0];
    space->fftModule = engine->fftModule;
    space->userLagChoice = engine->userLagChoice;

    engine->memoryUsage += mem;

    RKLog(">%s Started.   mem = %s B   fftOrder = %d   i0 = %s   ci = %d\n",
          me->name, RKUIntegerToCommaStyleString(mem), engine->processorFFTOrder, RKIntegerToCommaStyleString(io), ci);

    pthread_mutex_unlock(&engine->mutex);

    // Increase the tic once to indicate this processing core is created.
    me->tic++;

    //
    // Same as in RKPulseEngine.c
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
                RKLog("%s Error. Failed in sem_wait(). errno = %d\n", me->name, errno);
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
            RKLog("%s Warning. Unexpected path->origin = %d   path->length = %d   io = %d\n", me->name, path.origin, path.length, io);
            path.origin = 0;
            path.length = 1;
        }

        // Call the assigned moment processor if we are to process, is = indexStart, ie = indexEnd
        is = path.origin;
        ie = RKNextNModuloS(is, path.length, engine->radarDescription->pulseBufferDepth);

        // Latest rayStatusBufferIndex the other end should check (I know there is a horse raise here)
        engine->rayStatusBufferIndex = iu;

        // Start and end pulses to calculate this ray
        S = RKGetPulseFromBuffer(engine->pulseBuffer, is);
        E = RKGetPulseFromBuffer(engine->pulseBuffer, ie);

        // Beamwidth
        deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees,   E->header.azimuthDegrees);
        deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);

        // My ray
        ray = RKGetRayFromBuffer(engine->rayBuffer, io);

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
        ray->header.gateCount       = E->header.downSampledGateCount;
        ray->header.gateSizeMeters  = E->header.gateSizeMeters * engine->radarDescription->pulseToRayRatio;

        config = &engine->configBuffer[E->header.configIndex];

        // Compute the range correction factor if needed.
        if (ic != E->header.configIndex || ray->header.gateCount != space->gateCount) {
            ic = E->header.configIndex;
            // At this point, gateCount and gateSizeMeters is no longer of the raw pulses, they have been adjusted according to pulseToRayRatio
            space->gateCount = ray->header.gateCount;
            space->gateSizeMeters = ray->header.gateSizeMeters;
            // Because the pulse-compression engine uses unity noise gain filters, there is an inherent gain difference at different sampling rate
            // The gain difference is compensated here with a calibration factor if raw-sampling is at 1-MHz (150-m)
            // The number 60 is for conversion of range from meters to kilometers in the range correction term.
            space->samplingAdjustment = 10.0f * log10f(space->gateSizeMeters / (150.0f * engine->radarDescription->pulseToRayRatio)) + 60.0f;
            // Check if the config is identical to the previous one it has seen
            bool configHasChanged = false;
            configHasChanged |= previousConfig->systemDCal != config->systemDCal;
            configHasChanged |= previousConfig->systemPCal != config->systemPCal;
            configHasChanged |= previousConfig->SNRThreshold != config->SNRThreshold;
            configHasChanged |= previousConfig->SQIThreshold != config->SQIThreshold;
            for (p = 0; p < 2; p++) {
                configHasChanged |= previousConfig->prt[p] != config->prt[p];
                configHasChanged |= previousConfig->noise[p] != config->noise[p];
                configHasChanged |= previousConfig->systemZCal[p] != config->systemZCal[p];
            }
            configHasChanged |= previousConfig->waveformDecimate != config->waveformDecimate;
            // The rest of the constants
            space->velocityFactor = 0.25f * engine->radarDescription->wavelength / config->prt[0] / M_PI;
            space->widthFactor = engine->radarDescription->wavelength / config->prt[0] / (2.0f * sqrtf(2.0f) * M_PI);
            space->KDPFactor = 1.0f / S->header.gateSizeMeters;
            space->config = config;
            // Show the info only if config has changed
            if (engine->verbose && configHasChanged) {
                pthread_mutex_lock(&rkGlobalParameters.lock);

                RKFilterAnchor *filterAnchors = config->waveform->filterAnchors[0];
                RKLog("%s systemZCal = %.2f   ZCal = %.2f  sensiGain[0] = %.2f   samplingAdj = %.2f\n",
                      me->name,
                      config->systemZCal[0], config->ZCal[0][0], filterAnchors[0].sensitivityGain, space->samplingAdjustment);
                RKLog(">%s RCor @ filterCount = %d   capacity = %s   C%02d\n",
                      me->name,
                      config->waveform->count,
                      RKIntegerToCommaStyleString(ray->header.capacity),
                      ic);
                RKLog(">%s PRF = %s Hz -> Va = %.2f m/s/rad\n",
                      me->name,
                      RKFloatToCommaStyleString(1.0f / config->prt[0]), space->velocityFactor);
                if (engine->verbose > 1) {
                    for (p = 0; p < 2; p++) {
                        for (k = 0; k < config->waveform->count; k++) {
                            RKLog(">%s ZCal[%d][%s] = %.2f + %.2f - %.2f - %.2f = %.2f dB @ %d ..< %d\n",
                                  me->name, k,
                                  p == 0 ? "H" : (p == 1 ? "V" : "-"),
                                  config->systemZCal[p],
                                  config->ZCal[k][p],
                                  filterAnchors[k].sensitivityGain,
                                  space->samplingAdjustment,
                                  config->ZCal[k][p] + config->systemZCal[p] - filterAnchors[k].sensitivityGain - space->samplingAdjustment,
                                  filterAnchors[k].outputOrigin, filterAnchors[k].outputOrigin + filterAnchors[k].maxDataLength);
                        }
                    }
                }
                pthread_mutex_unlock(&rkGlobalParameters.lock);
            }
            memcpy(previousConfig, config, sizeof(RKConfig));
            // Call the calibrator to derive range calibration, ZCal and DCal
            engine->calibrator(engine->userModule, space);
        }

        // Consolidate the pulse marker into ray marker
        marker = RKMarkerNull;
        i = is;
        k = 0;
        do {
            pulse = RKGetPulseFromBuffer(engine->pulseBuffer, i);
            marker |= pulse->header.marker;
            pulses[k++] = pulse;
            i = RKNextModuloS(i, engine->radarDescription->pulseBufferDepth);
        } while (k <= path.length);
        // printf("k = %d   is = %u   ie = %u\n", k, is, ie);

        // Duplicate a linear array for processor if we are to process; otherwise, just skip this group
        if (path.length > 3 && deltaAzimuth < 3.0f && deltaElevation < 3.0f) {
            // Initialize the scratch space
            prepareScratch(space);
            // Call the noise estimator
            // RKLog("%s noiseEstimator on.\n", me->name);
            k = engine->noiseEstimator(space, pulses, path.length);
            // RKLog("%s noiseEstimator off.\n", me->name);
            if (k != RKResultSuccess) {
                RKNoiseFromConfig(space, pulses, path.length);
            }
            // RKLog("%s noise = %.4f %.4f \n", me->name, space->noise[0], space->noise[1]);
            // Call the moment processor
            k = engine->momentProcessor(space, pulses, path.length);
            // RKLog("%s Processed %d samples\n", me->name, k);
            if (k != path.length) {
                RKLog("%s Processed %d samples, which is not expected (%d)\n", me->name, k, path.length);
            }
            // Fill in the ray with SNR and SQI censoring, 16-bit and 8-bit data
            makeRayFromScratch(space, ray);
            ray->header.s |= RKRayStatusProcessed;
        } else {
            // Zero out the ray
            zeroOutRay(ray);
            if (engine->verbose > 1) {
                RKLog("%s Skipped a ray with %d sample%s   deltaAz = %.2f   deltaEl = %.2f.\n", me->name,
                      path.length, path.length > 1 ? "s": "", deltaAzimuth, deltaElevation);
            }
            ray->header.s |= RKRayStatusSkipped;
        }
        for (k = 0; k < path.length; k++) {
            pulse = pulses[k];
            pulse->header.s |= RKPulseStatusUsedForMoments;
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
                 " %05u %s | %05u - %05u (%3d)  [C%02d %s E%.2f A%.2f]   %s%6.2f-%6.2f (%4.2f)  G%s  M%05x  %s  %s%s",
                 (unsigned int)io, name, (unsigned int)is, (unsigned int)ie, path.length,
                 ray->header.configIndex,
                 RKMarkerScanTypeShortString(ray->header.marker),
                 engine->configBuffer[ray->header.configIndex].sweepElevation, engine->configBuffer[ray->header.configIndex].sweepAzimuth,
                 (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "E" : "A",
                 (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? S->header.elevationDegrees : S->header.azimuthDegrees,
                 (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? E->header.elevationDegrees : E->header.azimuthDegrees,
                 (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? deltaElevation : deltaAzimuth,
                 RKIntegerToCommaStyleString(ray->header.gateCount),
                 ray->header.marker,
                 RKIntegerToCommaStyleString(ray->header.fftOrder),
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

        #if defined(RAY_GATHERER)
        RKLog("%s Ray E%.1f-%.1f A%.1f-%.1f done\n", me->name, ray->header.startElevation, ray->header.endElevation, ray->header.startAzimuth, ray->header.endAzimuth);
        #endif
    }

    if (engine->verbose > 1) {
        RKLog("%s Freeing reources ...\n", me->name);
    }

    RKMomentScratchFree(space);
    free(previousConfig);
    free(busyPeriods);
    free(fullPeriods);

    RKLog(">%s Stopped.\n", me->name);

    return NULL;
}

static void *pulseGatherer(void *_in) {
    RKMomentEngine *engine = (RKMomentEngine *)_in;

    int c, i, j, k, s;
    struct timeval t0, t1;

    sem_t *sem[engine->coreCount];

    unsigned int skipCounter = 0;

    // Beam index at t = 0 and t = 1 (previous sample)
    int i0;
    int i1 = -999999;
    int count = 0;

    RKPulse *pulse;
    RKRay *ray;
    RKMarker marker;

    // Show the selected noise estimator & moment processor
    if (engine->verbose) {
        if (engine->noiseEstimator == &RKNoiseFromConfig) {
            RKLog(">%s Noise method = RKNoiseEstimator\n", engine->name);
        } else if (engine->momentProcessor == NULL) {
            RKLog(">%s Warning. No moment processor.\n", engine->name);
        } else {
            RKLog(">%s Noise method @ %p not recognized\n", engine->name, engine->momentProcessor);
        }
        if (engine->momentProcessor == &RKMultiLag) {
            RKLog(">%s Moment method = RKMultiLag @ %d\n", engine->name, engine->userLagChoice);
        } else if (engine->momentProcessor == &RKPulsePairHop) {
            RKLog(">%s Moment method = RKPulsePairHop\n", engine->name);
        } else if (engine->momentProcessor == &RKPulsePair) {
            RKLog(">%s Moment method = RKPulsePair\n", engine->name);
        } else if (engine->momentProcessor == &RKSpectralMoment) {
            RKLog(">%s Moment method = RKSpectralMoment\n", engine->name);
        } else if (engine->momentProcessor == &RKPulseATSR) {
            RKLog(">%s Moment method = RKPulseATS\n", engine->name);
        } else if (engine->momentProcessor == NULL || engine->momentProcessor == &RKNullProcessor) {
            RKLog(">%s Warning. No moment processor.\n", engine->name);
        } else {
            RKLog(">%s Moment method @ %p not recognized\n", engine->name, engine->momentProcessor);
        }
    }

    // Update the engine state
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;

    // Spin off N workers to process I/Q pulses
    memset(sem, 0, engine->coreCount * sizeof(sem_t *));
    for (c = 0; c < engine->coreCount; c++) {
        RKMomentWorker *worker = &engine->workers[c];
        snprintf(worker->semaphoreName, sizeof(worker->semaphoreName), "rk-mm-%02d", c);
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
    // See RKPulseEngine.c
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
        pulse = RKGetPulseFromBuffer(engine->pulseBuffer, k);

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
            RKMomentEngineUpdateMinMaxWorkerLag(engine);
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
            RKMomentEngineUpdateMinMaxWorkerLag(engine);
        }
        engine->state ^= RKEngineStateSleep2;

        if (!(engine->state & RKEngineStateWantActive)) {
            break;
        }

        // Lag of the engine
        engine->lag = fmodf(((float)*engine->pulseIndex + engine->radarDescription->pulseBufferDepth - k) / engine->radarDescription->pulseBufferDepth, 1.0f);

        if (skipCounter == 0 && engine->maxWorkerLag > 0.9f) {
            engine->almostFull++;
            skipCounter = engine->radarDescription->pulseBufferDepth / 10;
            RKLog("%s Warning. Projected an overflow.   lead-min-max-lag = %.2f / %.2f / %.2f   j = %d   pulseIndex = %d vs %d\n",
                  engine->name, engine->lag, engine->minWorkerLag, engine->maxWorkerLag, j, *engine->pulseIndex, k);
            // Skip the ray: set source length to 0 for those that are currenly being or have not been processed. Save the j-th source, which is current.
            i = j;
            do {
                i = RKPreviousModuloS(i, engine->radarDescription->rayBufferDepth);
                engine->momentSource[i].length = 0;
                engine->business--;
                ray = RKGetRayFromBuffer(engine->rayBuffer, i);
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
            // printf("k%4u   i = %d %d %s\n", k, i0, i1, pulse->header.marker & RKMarkerSweepEnd ? "E" : "");
            if (pulse->header.marker & RKMarkerSweepBegin) {
                engine->momentSource[j].origin = k;
                count = 0;
                i1 = i0;
            }
            if (i1 != i0 || count == RKMaximumPulsesPerRay || pulse->header.marker & RKMarkerSweepEnd) {
                i1 = i0;
                if (engine->excludeBoundaryPulses) {
                    if (pulse->header.marker & RKMarkerSweepEnd) {
                        count++;
                    }
                    if (count > 1) {
                        count--;
                    }
                }
                if (count > 0) {
                    // Number of samples in this ray and the correct plan index
                    #if defined(DEBUG_RAY_GATHERER)
                    int ii = RKNextNModuloS(engine->momentSource[j].origin, count, engine->radarDescription->pulseBufferDepth);
                    RKPulse *s = RKGetPulseFromBuffer(engine->pulseBuffer, engine->momentSource[j].origin);
                    RKPulse *e = RKGetPulseFromBuffer(engine->pulseBuffer, ii);
                    RKLog("%s MM Ray E%.1f-%.1f   %d ... %d (%u)", engine->name,
                          s->header.elevationDegrees, e->header.elevationDegrees,
                          engine->momentSource[j].origin, ii, count + 1);
                    #endif
                    engine->momentSource[j].length = count;
                    engine->momentSource[j].planIndex = (uint32_t)ceilf(log2f((float)count));

                    //printf("%s k = %d --> momentSource[%d] = %d / %d / %d\n", engine->name, k, j, engine->momentSource[j].origin, engine->momentSource[j].length, engine->momentSource[j].modulo);

                    if (engine->useSemaphore) {
                        if (sem_post(sem[c])) {
                            RKLog("%s Error. Failed in sem_post(), errno = %d\n", engine->name, errno);
                        }
                    } else {
                        engine->workers[c].tic++;
                    }
                    engine->business++;
                    // Move to the next core, gather pulses for the next ray
                    c = RKNextModuloS(c, engine->coreCount);
                    j = RKNextModuloS(j, engine->radarDescription->rayBufferDepth);
                    // New origin for the next ray
                    engine->momentSource[j].origin = k;
                    ray = RKGetRayFromBuffer(engine->rayBuffer, j);
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
        ray = RKGetRayFromBuffer(engine->rayBuffer, *engine->rayIndex);
        while (ray->header.s & RKRayStatusReady && engine->state & RKEngineStateWantActive) {
            *engine->rayIndex = RKNextModuloS(*engine->rayIndex, engine->radarDescription->rayBufferDepth);
            ray = RKGetRayFromBuffer(engine->rayBuffer, *engine->rayIndex);
            engine->business--;
        }

        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            engine->processedPulseIndex = k;
            RKMomentEngineUpdateStatusString(engine);
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
    engine->noiseEstimator = &RKNoiseFromConfig;
    engine->momentProcessor = &RKPulsePairHop;
    // engine->momentProcessor = &RKMultiLag;
    // engine->userLagChoice = 3;
    engine->calibrator = &RKCalibratorSimple;
    engine->processorLagCount = RKMaximumLagCount;
    engine->processorFFTOrder = (uint8_t)ceilf(log2f((float)RKMaximumPulsesPerRay));
    engine->memoryUsage = sizeof(RKMomentEngine);
    pthread_mutex_init(&engine->mutex, NULL);
    return engine;
}

void RKMomentEngineFree(RKMomentEngine *engine) {
    if (engine->state & RKEngineStateWantActive) {
        RKMomentEngineStop(engine);
    }
    if (engine->momentSource) {
        free(engine->momentSource);
    }
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

    size_t bytes;

    engine->state |= RKEngineStateMemoryChange;
    bytes = engine->radarDescription->rayBufferDepth * sizeof(RKModuloPath);
    engine->momentSource = (RKModuloPath *)malloc(bytes);
    if (engine->momentSource == NULL) {
        RKLog("Error. Unable to allocate momentSource.\n");
        exit(EXIT_FAILURE);
    }
    memset(engine->momentSource, 0, bytes);
    for (int i = 0; i < engine->radarDescription->rayBufferDepth; i++) {
        engine->momentSource[i].modulo = engine->radarDescription->pulseBufferDepth;
    }
    engine->state ^= RKEngineStateMemoryChange;
    RKMomentEngineCheckWiring(engine);
}

void RKMomentEngineSetFFTModule(RKMomentEngine *engine, RKFFTModule *module) {
    engine->fftModule = module;
    RKMomentEngineCheckWiring(engine);
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

void RKMomentEngineSetExcludeBoundaryPulses(RKMomentEngine *engine, const bool exclude) {
    engine->excludeBoundaryPulses = exclude;
}

void RKMomentEngineSetNoiseEstimator(RKMomentEngine *engine, int (*routine)(RKMomentScratch *, RKPulse **, const uint16_t)) {
    engine->noiseEstimator = routine;
}

void RKMomentEngineSetMomentProcessor(RKMomentEngine *engine, int (*routine)(RKMomentScratch *, RKPulse **, const uint16_t)) {
    engine->momentProcessor = routine;
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
    engine->memoryUsage += engine->coreCount * sizeof(RKMomentWorker);
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

RKRay *RKMomentEngineGetProcessedRay(RKMomentEngine *engine, const bool blocking) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired for RKPulseEngineGetProcessedPulse()\n", engine->name);
        return NULL;
    }
    RKRay *ray = RKGetRayFromBuffer(engine->rayBuffer, engine->doneIndex);
    if (blocking) {
        uint32_t s = 0;
        while (!(ray->header.s & RKRayStatusReady) && engine->state & RKEngineStateWantActive && s++ < 1000) {
            usleep(1000);
        }
    } else {
        if (!(ray->header.s & RKRayStatusReady)) {
            return NULL;
        }
    }
    if (!(engine->state & RKEngineStateWantActive)) {
        return NULL;
    }
    ray->header.s |= RKRayStatusConsumed;
    engine->doneIndex = RKNextModuloS(engine->doneIndex, engine->radarDescription->rayBufferDepth);
    return ray;
}

void RKMomentEngineWaitWhileBusy(RKMomentEngine *engine) {
    int k;
    RKRay *ray;

    k = 0;
    while (engine->business > 0 && k++ < 2000) {
        if (engine->state & RKEngineStateSleep2) {
            // Check finished rays
            #if defined(DEBUG_MOMENT_ENGINE_WAIT)
            RKLog("%s Revising business  k = %d\n", engine->name, k);
            #endif
            ray = RKGetRayFromBuffer(engine->rayBuffer, *engine->rayIndex);
            while (ray->header.s & RKRayStatusReady && engine->state & RKEngineStateWantActive) {
                *engine->rayIndex = RKNextModuloS(*engine->rayIndex, engine->radarDescription->rayBufferDepth);
                ray = RKGetRayFromBuffer(engine->rayBuffer, *engine->rayIndex);
                engine->business--;
            }
        }
        usleep(1000);
    }
}

char *RKMomentEngineStatusString(RKMomentEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
