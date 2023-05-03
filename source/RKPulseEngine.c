//
//  RKPulseEngine.c
//  RadarKit
//
//  Created by Boonleng Cheong on 3/18/15.
//  Copyright (c) 2015-2017 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKPulseEngine.h>

#define RKPulseEngineConvertMethod   1
#define RKPulseEngineMultiplyMethod  1

// Internal Functions

static void RKPulseEngineUpdateStatusString(RKPulseEngine *);
static void *pulseEngineCore(void *);
static void *pulseWatcher(void *);

#pragma mark - Helper Functions

static void RKPulseEngineUpdateStatusString(RKPulseEngine *engine) {
    int i, c;
    char *string = engine->statusBuffer[engine->statusBufferIndex];
    const bool useCompact = engine->coreCount >= 4;

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

    // Use RKStatusBarWidth characters to draw a bar
    i = *engine->pulseIndex * RKStatusBarWidth / engine->radarDescription->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'C';

    // Engine lag
    i = RKStatusBarWidth + snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth, " %s%02.0f%s :%s",
                                    rkGlobalParameters.statusColor ? RKColorLag(engine->lag) : "",
                                    99.49f * engine->lag,
                                    rkGlobalParameters.statusColor ? RKNoColor : "",
                                    useCompact ? " " : "");

    RKPulseWorker *worker;

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
    for (c = 0; c < engine->coreCount && i < RKStatusStringLength - RKStatusBarWidth - 5; c++) {
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

    // Almost full count
    //i += snprintf(string + i, RKStatusStringLength - i, " [%d]", engine->almostFull);

    // Concluding string
    if (i > RKStatusStringLength - RKStatusBarWidth - 5) {
        memset(string + i, '#', RKStatusStringLength - i - 1);
    }
    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);

    // Status of the pulse
    string = engine->pulseStatusBuffer[engine->pulseStatusBufferIndex];
    c = *engine->pulseIndex;
    i = c * (RKStatusBarWidth + 1) / engine->radarDescription->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = '#';

    RKPulse *pulse = RKGetPulseFromBuffer(engine->pulseBuffer, c);
    RKConfig *config = &engine->configBuffer[pulse->header.configIndex];
    snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth,
             " %05u | C%2d/E%5.2f/A%6.2f   E%5.2f   A%6.2f   G%s   M%04x",
             (unsigned int)c,
             pulse->header.configIndex, config->sweepElevation, config->sweepAzimuth,
             pulse->header.elevationDegrees, pulse->header.azimuthDegrees, RKIntegerToCommaStyleString(pulse->header.gateCount),
             pulse->header.marker);

    engine->pulseStatusBufferIndex = RKNextModuloS(engine->pulseStatusBufferIndex, RKBufferSSlotCount);
}

static void RKPulseEngineVerifyWiring(RKPulseEngine *engine) {
    if (engine->radarDescription == NULL ||
        engine->configBuffer == NULL || engine->configIndex == NULL ||
        engine->pulseBuffer == NULL || engine->pulseIndex == NULL ||
        engine->fftModule == NULL) {
        engine->state &= ~RKEngineStateProperlyWired;
        return;
    }
    engine->state |= RKEngineStateProperlyWired;
}

#pragma mark - Delegate Workers

void RKBuiltInConfigChangeCallback(RKCompressionScratch *scratch) {
    RKWaveform *waveform = scratch->config->waveform;
    RKLog("%s Config changed. waveform @ %p   %s", scratch->name, waveform,
        waveform == NULL ? "" : RKVariableInString("waveform->count", &waveform->count, RKValueTypeUInt8));
}

void RKBuiltInCompressor(RKCompressionScratch *scratch) {

    int i, p;
    RKPulse *pulse = scratch->pulse;
    RKComplex *filter = scratch->filter;
    RKFilterAnchor *filterAnchor = scratch->filterAnchor;
    fftwf_complex *in = scratch->inBuffer;
    fftwf_complex *out = scratch->outBuffer;
    unsigned int inBound, outBound;

    inBound = MIN(pulse->header.gateCount - filterAnchor->inputOrigin, filterAnchor->inputOrigin + filterAnchor->maxDataLength + filterAnchor->length);
    outBound = MIN(pulse->header.gateCount - filterAnchor->outputOrigin, filterAnchor->maxDataLength);

    #if DEBUG_PULSE_COMPRESSION

    if (pulse->header.i % 1000 == 0) {
        RKLog("---- filter %d   o= %d   gateCount = %d   bound = %d\n", filterAnchor->name, filterAnchor->inputOrigin, pulse->header.gateCount, inBound);
    }

    #endif

    for (p = 0; p < 2; p++) {
       // Copy and convert the samples
        RKInt16C *X = RKGetInt16CDataFromPulse(pulse, p);
        X += filterAnchor->inputOrigin;
        RKSIMD_Int2Complex(X, (RKComplex *)in, inBound);

        // Zero pad the input; a filter is always zero-padded in the setter function.
        if (scratch->planSize > inBound) {
            memset(in + inBound, 0, (scratch->planSize - inBound) * sizeof(fftwf_complex));
        }

        fftwf_execute_dft(scratch->planForwardInPlace, in, in);

        //printf("dft(in) =\n"); RKPulseEngineShowBuffer(in, 8);

        fftwf_execute_dft(scratch->planForwardOutPlace, (fftwf_complex *)filter, out);

        //printf("dft(filt[%d][%d]) =\n", gid, j); RKPulseEngineShowBuffer(out, 8);

        #if RKPulseEngineMultiplyMethod == 1

        // In-place SIMD multiplication using the interleaved format (hand tuned, this should be the fastest)
        RKSIMD_iymulc((RKComplex *)in, (RKComplex *)out, scratch->planSize);

        #elif RKPulseEngineMultiplyMethod == 2

        // In-place SIMD multiplication using two seperate SIMD calls (hand tune, second fastest)
        RKSIMD_iyconj((RKComplex *)out, planSize);
        RKSIMD_iymul((RKComplex *)in, (RKComplex *)out, planSize);

        #elif RKPulseEngineMultiplyMethod == 3

        // Deinterleave the RKComplex data into RKIQZ format, multiply using SIMD, then interleave the result back to RKComplex format
        RKSIMD_Complex2IQZ((RKComplex *)in, scratch->zi, planSize);
        RKSIMD_Complex2IQZ((RKComplex *)out, scratch->zo, planSize);
        RKSIMD_izmul(zi, zo, planSize, true);
        RKSIMD_IQZ2Complex(zo, (RKComplex *)out, planSize);

        #else

        // Regular multiplication and let compiler optimize with either -O1 -O2 or -Os
        RKSIMD_iyconj((RKComplex *)in, planSize);
        RKSIMD_iymul_reg((RKComplex *)in, (RKComplex *)out, planSize);

        #endif

        //printf("in * out =\n"); RKPulseEngineShowBuffer(out, 8);

        fftwf_execute_dft(scratch->planBackwardInPlace, out, out);

        //printf("idft(out) =\n"); RKPulseEngineShowBuffer(out, 8);

        // Scaling due to a net gain of planSize from forward + backward DFT, plus the waveform gain
        RKSIMD_iyscl((RKComplex *)out, 1.0f / scratch->planSize, scratch->planSize);

        //printf("idft(out) =\n"); RKPulseEngineShowBuffer(out, 8);

        RKComplex *Y = RKGetComplexDataFromPulse(pulse, p);
        RKIQZ Z = RKGetSplitComplexDataFromPulse(pulse, p);
        Y += filterAnchor->outputOrigin;
        Z.i += filterAnchor->outputOrigin;
        Z.q += filterAnchor->outputOrigin;
        fftwf_complex *o = out;
        for (i = 0; i < outBound; i++) {
            Y->i = (*o)[0];
            Y++->q = (*o)[1];
            *Z.i++ = (*o)[0];
            *Z.q++ = (*o)[1];
            o++;
        }

        #ifdef DEBUG_PULSE_COMPRESSION

        pthread_mutex_lock(&engine->mutex);
        Y = RKGetComplexDataFromPulse(pulse, p);
        printf("Y [i0 = %d   p = %d   j = %d] =\n", i0, p, j);
        RKPulseEngineShowBuffer((fftwf_complex *)Y, 8);

        Z = RKGetSplitComplexDataFromPulse(pulse, p);
        RKShowArray(Z.i, "Zi", 8, 1);
        RKShowArray(Z.q, "Zq", 8, 1);
        pthread_mutex_unlock(&engine->mutex);

        #endif

    } // for (p = 0; ...
}

static void *pulseEngineCore(void *_in) {
    RKPulseWorker *me = (RKPulseWorker *)_in;
    RKPulseEngine *engine = me->parent;

    int i, j, k, p;
    struct timeval t0, t1, t2;

    const int c = me->id;
    const int ci = engine->radarDescription->initFlags & RKInitFlagManuallyAssignCPU ? engine->coreOrigin + c : -1;

    uint32_t blindGateCount = 0;

    // Find the semaphore
    sem_t *sem = sem_open(me->semaphoreName, O_RDWR);
    if (sem == SEM_FAILED) {
        RKLog("Error. Unable to retrieve semaphore %d\n", c);
        return (void *)RKResultFailedToRetrieveSemaphore;
    };

    // Initiate my name
    if (rkGlobalParameters.showColor) {
        pthread_mutex_lock(&engine->mutex);
        k = snprintf(me->name, RKShortNameLength - 1, "%s", rkGlobalParameters.showColor ? RKGetColor() : "");
        pthread_mutex_unlock(&engine->mutex);
    } else {
        k = 0;
    }
    if (engine->coreCount > 9) {
        k += sprintf(me->name + k, "P%02d", c);
    } else {
        k += sprintf(me->name + k, "P%d", c);
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

    RKBuffer localPulseBuffer;
    RKPulseBufferAlloc(&localPulseBuffer, engine->radarDescription->pulseCapacity, 1);

    const size_t nfft = 1 << (int)ceilf(log2f((float)MIN(RKMaximumGateCount, engine->radarDescription->pulseCapacity)));

    RKPulse *pulseCopy = RKGetPulseFromBuffer(localPulseBuffer, 0);

    // Allocate local resources, use k to keep track of the total allocation
    // Avoid fftwf_malloc() here so that non-avx-enabled libfftw is compatible
    RKCompressionScratch *scratch = (RKCompressionScratch *)malloc(sizeof(RKCompressionScratch));
    if (scratch == NULL) {
        RKLog("%s Error. Unable to allocate a scratch space.\n", engine->name);
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    sprintf(scratch->name, "%s %s", engine->name, me->name);
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&scratch->inBuffer, RKMemoryAlignSize, nfft * sizeof(fftwf_complex)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&scratch->outBuffer, RKMemoryAlignSize, nfft * sizeof(fftwf_complex)))
    if (scratch->inBuffer == NULL || scratch->outBuffer == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    size_t mem = 2 * nfft * sizeof(fftwf_complex);
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&scratch->zi, RKMemoryAlignSize, nfft * sizeof(RKFloat)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&scratch->zo, RKMemoryAlignSize, nfft * sizeof(RKFloat)))
    if (scratch->zi == NULL || scratch->zo == NULL) {
        RKLog("Error. Unable to allocate resources for FFTW.\n");
        return (void *)RKResultFailedToAllocateFFTSpace;
    }
    mem += 2 * nfft * sizeof(RKFloat);

    double *busyPeriods, *fullPeriods;
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&busyPeriods, RKMemoryAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&fullPeriods, RKMemoryAlignSize, RKWorkerDutyCycleBufferDepth * sizeof(double)))
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

    // The last index of the pulse buffer for this core (i.e., increment by one will get c)
    uint32_t i0 = engine->radarDescription->pulseBufferDepth - engine->coreCount + c;

    // The latest index in the dutyCycle buffer
    int d0 = 0;

    // DFT plan index of the FFT module
    int planIndex;

    // Log my initial state
    pthread_mutex_lock(&engine->mutex);
    engine->memoryUsage += mem;

    RKLog(">%s %s Started.   mem = %s B   i0 = %s   nfft = %s   ci = %d\n",
          engine->name, me->name, RKIntegerToCommaStyleString(mem), RKIntegerToCommaStyleString(i0), RKIntegerToCommaStyleString(nfft), ci);

    pthread_mutex_unlock(&engine->mutex);

    // Increase the tic once to indicate this processing core is created.
    me->tic++;

    //
    // free   busy       free   busy
    // .......|||||||||||.......|||||||||
    // t2 --- t1 --- t0/t2 --- t1 --- t0
    //        [ t0 - t1 ]
    // [    t0 - t2     ]
    //
    uint64_t tic = me->tic;
    uint16_t configIndex = -1;
    scratch->config = &engine->configBuffer[0];

    while (engine->state & RKEngineStateWantActive) {
        if (engine->useSemaphore) {
            #ifdef DEBUG_IQ
            RKLog(">%s sem_wait()\n", coreName);
            #endif
            if (sem_wait(sem)) {
                RKLog("%s %s Error. Failed in sem_wait(). errno = %d\n", engine->name, me->name, errno);
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
        i0 = RKNextNModuloS(i0, engine->coreCount, engine->radarDescription->pulseBufferDepth);

        RKPulse *pulse = RKGetPulseFromBuffer(engine->pulseBuffer, i0);

        // Update configIndex when it no longer matches the latest pulse
        if (configIndex != pulse->header.configIndex) {
            configIndex = pulse->header.configIndex;
            scratch->config = &engine->configBuffer[configIndex];
            scratch->filter = engine->filters[0][0];
            scratch->filterAnchor = &engine->filterAnchors[0][0];
            if (engine->compressorInit) {
                engine->compressorInit(scratch);
            }
        }

        #ifdef DEBUG_IQ
        RKLog(">%s i0 = %d  stat = %d\n", coreName, i0, input->header.s);
        #endif

        // Filter group id
        const int gid = engine->filterGid[i0];
        //printf("pulse i = %u   gid = %d\n", (uint32_t)pulse->header.i, gid);
        pulse->parameters.gid = gid;

        // Now we process / skip
        if (gid < 0 || gid >= engine->filterGroupCount || engine->state & RKEngineStateMemoryChange) {
            pulse->parameters.planSizes[0][0] = 0;
            pulse->parameters.planSizes[1][0] = 0;
            pulse->parameters.filterCounts[0] = 0;
            pulse->parameters.filterCounts[1] = 0;
            pulse->header.s |= RKPulseStatusSkipped;
            if (engine->verbose > 1) {
                RKLog("%s pulse skipped. header->i = %d   gid = %d\n", engine->name, pulse->header.i, gid);
            }
        } else {
            // Do some work with this pulse
            // DFT of the raw data is stored in *in
            // DFT of the filter is stored in *out
            // Their product is stored in *out using in-place multiplication: out[i] = conj(out[i]) * in[i]
            // Then, the inverse DFT is performed to get out back to time domain, which is the compressed pulse

            // Go through all the filters in this filter group
            blindGateCount = 0;
            for (j = 0; j < engine->filterCounts[gid]; j++) {
                // Get the plan index and size from parent engine
                planIndex = engine->planIndices[i0][j];
                blindGateCount += engine->filterAnchors[gid][j].length;

                // Compression
                scratch->pulse = pulse;
                scratch->filter = engine->filters[gid][j];
                scratch->filterAnchor = &engine->filterAnchors[gid][j];
                scratch->planForwardInPlace = engine->fftModule->plans[planIndex].forwardInPlace;
                scratch->planForwardOutPlace = engine->fftModule->plans[planIndex].forwardOutPlace;
                scratch->planBackwardInPlace = engine->fftModule->plans[planIndex].backwardInPlace;
                scratch->planBackwardOutPlace = engine->fftModule->plans[planIndex].backwardOutPlace;
                scratch->planSize = engine->fftModule->plans[planIndex].size;
                scratch->waveformGroupdId = gid;
                scratch->waveformFilterId = j;

                // Now we actually compress
                engine->compressorExec(scratch);

                // Copy over the parameters used
                for (p = 0; p < 2; p++) {
                    pulse->parameters.planIndices[p][j] = planIndex;
                    pulse->parameters.planSizes[p][j] = engine->fftModule->plans[planIndex].size;
                }
            } // for (j=0; j < engine->filterCount ...
            pulse->parameters.filterCounts[0] = j;
            pulse->parameters.filterCounts[1] = j;
            pulse->header.pulseWidthSampleCount = blindGateCount;
            #ifdef DEBUG_IQ
            if (pulse->header.i % 1000 == 0) {
                RKLog("-- %d --> %d\n", pulse->header.gateCount, pulse->header.gateCount - blindGateCount);
            }
            #endif
            pulse->header.gateCount -= blindGateCount;
            pulse->header.s |= RKPulseStatusCompressed;
        }

        // Down-sampling regardless if the pulse was compressed or skipped
        int stride = MAX(1, engine->radarDescription->pulseToRayRatio);
        if (stride > 1) {
            pulse->header.downSampledGateCount = (pulse->header.gateCount + stride - 1) / stride;
            // The tail part can be emptied but we are going to use it to store the compressed response prior to down-sampling for AScope viewing
            for (p = 0; p < 2; p++) {
                RKComplex *YCopy = RKGetComplexDataFromPulse(pulseCopy, p);
                RKComplex *Y = RKGetComplexDataFromPulse(pulse, p);
                RKIQZ Z = RKGetSplitComplexDataFromPulse(pulse, p);
                memcpy(YCopy, Y, (pulse->header.gateCount - pulse->header.downSampledGateCount) * sizeof(RKComplex));
                for (i = 0, j = 0; j < pulse->header.gateCount; i++, j+= stride) {
                    Y[i].i = Y[j].i;
                    Y[i].q = Y[j].q;
                    Z.i[i] = Z.i[j];
                    Z.q[i] = Z.q[j];
                }
                memcpy(&Y[i], YCopy, (pulse->header.gateCount - pulse->header.downSampledGateCount) * sizeof(RKComplex));
            }
        } else {
            pulse->header.downSampledGateCount = pulse->header.gateCount;
        }
        pulse->header.s |= RKPulseStatusDownSampled | RKPulseStatusProcessed;

        // Record down the latest processed pulse index
        me->pid = i0;
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

        t2 = t0;
    }

    // Clean up
    if (engine->verbose > 1) {
        RKLog("%s %s Freeing reources ...\n", engine->name, me->name);
    }

    if (engine->compressorFree) {
        engine->compressorFree(scratch);
    }
    free(scratch->zi);
    free(scratch->zo);
    free(scratch->inBuffer);
    free(scratch->outBuffer);
    free(scratch);
    free(busyPeriods);
    free(fullPeriods);
    RKPulseBufferFree(localPulseBuffer);

    RKLog(">%s %s Stopped.\n", engine->name, me->name);

    return NULL;
}

static void *pulseWatcher(void *_in) {
    RKPulseEngine *engine = (RKPulseEngine *)_in;

    int c, i, j, k, s;
    struct timeval t0, t1;
    float lag;

    sem_t *sem[engine->coreCount];

    unsigned int gid;
    unsigned int planIndex = 0;
    unsigned int skipCounter = 0;

    if (engine->coreCount == 0) {
        RKLog("Error. No processing core?\n");
        return NULL;
    }

    RKPulse *pulse;
    RKPulse *pulseToSkip;

    // Update the engine state
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;

    // Spin off N workers to process I/Q pulses
    memset(sem, 0, engine->coreCount * sizeof(sem_t *));
    for (c = 0; c < engine->coreCount; c++) {
        RKPulseWorker *worker = &engine->workers[c];
        snprintf(worker->semaphoreName, 32, "rk-iq-%03d", c);
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
        if (pthread_create(&worker->tid, NULL, pulseEngineCore, worker) != 0) {
            RKLog(">%s Error. Failed to start a compression core.\n", engine->name);
            return (void *)RKResultFailedToStartCompressionCore;
        }
    }

    // Wait for the workers to increase the tic count once
    // Using sem_wait here could cause a stolen post within the worker
    // Tested and removed on 9/29/2016
    engine->state |= RKEngineStateSleep0;
    for (c = 0; c < engine->coreCount; c++) {
        while (engine->workers[c].tic == 0) {
            usleep(10000);
        }
    }
    engine->state ^= RKEngineStateSleep0;
    engine->state |= RKEngineStateActive;

    RKLog("%s Started.   mem = %s B   pulseIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);

    // Increase the tic once to indicate the engine is ready
    engine->tic = 1;

    gettimeofday(&t1, NULL); t1.tv_sec -= 1;

    // Here comes the busy loop
    // i  anonymous
    // j  filter index
    k = 0;   // pulse index
    c = 0;   // core index
    while (engine->state & RKEngineStateWantActive) {
        // The pulse
        pulse = RKGetPulseFromBuffer(engine->pulseBuffer, k);

        // Wait until the engine index move to the next one for storage, which is also the time pulse has data.
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->pulseIndex && engine->state & RKEngineStateWantActive) {
            usleep(200);
            if (++s % 1000 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.0002f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state |= RKEngineStateSleep2;
        // Wait until the pulse has position so that this engine won't compete with the tagger to set the status.
        s = 0;
        while (!(pulse->header.s & RKPulseStatusHasIQData) && engine->state & RKEngineStateWantActive) {
            usleep(200);
            if (++s % 1000 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.0002f, k , *engine->pulseIndex, pulse->header.s);
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
            RKLog("%s Warning. Projected an I/Q Buffer overflow.\n", engine->name);
            i = *engine->pulseIndex;
            do {
                i = RKPreviousModuloS(i, engine->radarDescription->pulseBufferDepth);
                engine->filterGid[i] = -1;
                pulseToSkip = RKGetPulseFromBuffer(engine->pulseBuffer, i);
            } while (!(pulseToSkip->header.s & RKPulseStatusProcessed));
        } else if (skipCounter > 0) {
            // Skip processing if the buffer is getting full (avoid hitting SEM_VALUE_MAX)
            engine->filterGid[k] = -1;
            engine->planIndices[k][0] = 0;
            if (--skipCounter == 0) {
                RKLog(">%s Info. Skipped a chunk.\n", engine->name);
            }
        } else {
            // Compute the filter group id to use
            engine->filterGid[k] = (gid = pulse->header.i % engine->filterGroupCount);
            //printf("pulse->header.i = %d   gid = %d\n", (uint32_t)pulse->header.i, gid);

            // Find the right plan; create it if it does not exist
            for (j = 0; j < engine->filterCounts[gid]; j++) {
                planIndex = (unsigned int)ceilf(log2f((float)MIN(pulse->header.gateCount - engine->filterAnchors[gid][j].inputOrigin,
                                                                 engine->filterAnchors[gid][j].maxDataLength + engine->filterAnchors[gid][j].length)));
                engine->planIndices[k][j] = planIndex;
                engine->fftModule->plans[planIndex].count++;

            }
        }

        // The pulse is considered "inspected" whether it will be skipped / compressed by the desingated worker
        pulse->header.s |= RKPulseStatusInspected;

        // Now we post
        #ifdef DEBUG_IQ
        RKLog("%s posting core-%d for pulse %d w/ %d gates\n", engine->name, c, k, pulse->header.gateCount);
        #endif
        if (engine->useSemaphore) {
            if (sem_post(sem[c])) {
                RKLog("Error. Failed in sem_post(), errno = %d\n", errno);
            }
        } else {
            engine->workers[c].tic++;
        }
        c = RKNextModuloS(c, engine->coreCount);

        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            RKPulseEngineUpdateStatusString(engine);
        }

        engine->tic++;

        // Update k to catch up for the next watch
        k = RKNextModuloS(k, engine->radarDescription->pulseBufferDepth);
    }

    // Wait for workers to return
    for (c = 0; c < engine->coreCount; c++) {
        RKPulseWorker *worker = &engine->workers[c];
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

RKPulseEngine *RKPulseEngineInit(void) {
    RKPulseEngine *engine = (RKPulseEngine *)malloc(sizeof(RKPulseEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a pulse compression engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKPulseEngine));
    sprintf(engine->name, "%s<PulseCompressor>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPulseCompressionEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->useSemaphore = true;
    engine->configChangeCallback = &RKBuiltInConfigChangeCallback;
    engine->compressorExec = &RKBuiltInCompressor;
    engine->memoryUsage = sizeof(RKPulseEngine);
    pthread_mutex_init(&engine->mutex, NULL);
    return engine;
}

void RKPulseEngineFree(RKPulseEngine *engine) {
    if (engine->state & RKEngineStateWantActive) {
        RKPulseEngineStop(engine);
    }
    for (int i = 0; i < engine->filterGroupCount; i++) {
        for (int j = 0; j < engine->filterCounts[i]; j++) {
            if (engine->filters[i][j] != NULL) {
                free(engine->filters[i][j]);
            }
        }
    }
    pthread_mutex_destroy(&engine->mutex);
    free(engine->filterGid);
    free(engine->planIndices);
    free(engine);
}

#pragma mark - Properties

void RKPulseEngineSetVerbose(RKPulseEngine *engine, const int verb) {
    engine->verbose = verb;
}

void RKPulseEngineSetFilterChangeCallback(RKPulseEngine *engine, void (*callback)(RKCompressionScratch *)) {
    engine->configChangeCallback = callback;
}

//
// RKPulseEngineSetInputOutputBuffers
//
// Input:
// engine - the pulse compression engine
// desc - the description of the radar
// configBuffer - config buffer (for status display only)
// configIndex - index to retrieve the up-to-date config
// pulseBuffer - the raw data buffer
// pulseIndex - the reference index watch, *pulseIndex is the latest reading in *pulseBuffer
//
void RKPulseEngineSetInputOutputBuffers(RKPulseEngine *engine, const RKRadarDesc *desc,
                                        RKConfig *configBuffer, uint32_t *configIndex,
                                        RKBuffer pulseBuffer,   uint32_t *pulseIndex) {
    engine->radarDescription = (RKRadarDesc *)desc;
    engine->configBuffer     = configBuffer;
    engine->configIndex      = configIndex;
    engine->pulseBuffer      = pulseBuffer;
    engine->pulseIndex       = pulseIndex;

    size_t bytes;

    if (engine->filterGid != NULL) {
        free(engine->filterGid);
    }
    bytes = engine->radarDescription->pulseBufferDepth * sizeof(int);
    engine->filterGid = (int *)malloc(bytes);
    if (engine->filterGid == NULL) {
        RKLog("%s Error. Unable to allocate RKPulseEngine->filterGid.\n", engine->name);
        exit(EXIT_FAILURE);
    }
    engine->memoryUsage += bytes;

    if (engine->planIndices != NULL) {
        free(engine->planIndices);
    }
    bytes = engine->radarDescription->pulseBufferDepth * sizeof(RKPulseEnginePlanIndex);
    engine->planIndices = (RKPulseEnginePlanIndex *)malloc(bytes);
    if (engine->planIndices == NULL) {
        RKLog("%s Error. Unable to allocate RKPulseEngine->planIndices.\n", engine->name);
        exit(EXIT_FAILURE);
    }
    engine->memoryUsage += bytes;
    size_t filterLength = 1 << (int)ceilf(log2f((float)engine->radarDescription->pulseCapacity));
    bytes = filterLength * sizeof(RKComplex);
    engine->state |= RKEngineStateMemoryChange;
    for (int g = 0; g < RKMaximumWaveformCount; g++) {
        for (int i = 0; i < RKMaximumFilterCount; i++) {
            POSIX_MEMALIGN_CHECK(posix_memalign((void **)&engine->filters[g][i], RKMemoryAlignSize, bytes))
            engine->memoryUsage += bytes;
        }
    }
    engine->state ^= RKEngineStateMemoryChange;
    RKPulseEngineVerifyWiring(engine);
}

void RKPulseEngineSetFFTModule(RKPulseEngine *engine, RKFFTModule *module) {
    engine->fftModule = module;
    RKPulseEngineVerifyWiring(engine);
}

void RKPulseEngineSetCoreCount(RKPulseEngine *engine, const uint8_t count) {
    if (engine->state & RKEngineStateWantActive) {
        RKLog("%s Error. Core count cannot change when the engine is active.\n", engine->name);
        return;
    }
    engine->coreCount = count;
}

void RKPulseEngineSetCoreOrigin(RKPulseEngine *engine, const uint8_t origin) {
    if (engine->state & RKEngineStateWantActive) {
        RKLog("%s Error. Core origin cannot change when the engine is active.\n", engine->name);
        return;
    }
    engine->coreOrigin = origin;
}

int RKPulseEngineResetFilters(RKPulseEngine *engine) {
    // If engine->filterGroupCount is set to 0, gid may be undefined segmentation fault
    engine->filterGroupCount = 1;
    engine->filterCounts[0] = 1;
    for (int k = 1; k < RKMaximumFilterCount; k++) {
        engine->filterCounts[k] = 0;
    }
    return RKResultSuccess;
}

int RKPulseEngineSetFilterCountOfGroup(RKPulseEngine *engine, const int group, const int count) {
    engine->filterCounts[group] = count;
    return RKResultSuccess;
}

int RKPulseEngineSetFilterGroupCount(RKPulseEngine *engine, const int groupCount) {
    engine->filterGroupCount = groupCount;
    return RKResultSuccess;
}

// Input:
//     engine - the compression engine
// Output:
//     filter - filter coefficients, always start at index 0
//     anchor - origin - not used
//            - length - length of filter coefficients to use
//            - inputOrigin - origin of the intput data to process
//            - outputOrigin - origin of the output data to deliver
//            - maxDataLength - maximum length of the output data to deliver
//            - subCarrierFrequency - not used
//            - gain - filter gain in dB
//     group - filter group to assign to
//     index - index within the group to assign to
int RKPulseEngineSetGroupFilter(RKPulseEngine *engine, const RKComplex *filter, const RKFilterAnchor anchor, const int group, const int index) {
    if (engine->pulseBuffer == NULL) {
        RKLog("Warning. Pulse buffer has not been set.\n");
        return RKResultNoPulseBuffer;
    }
    if (group >= RKMaximumWaveformCount) {
        RKLog("Error. Filter group %d is invalid.\n", group);
        return RKResultFailedToSetFilter;
    }
    if (index >= RKMaximumFilterCount) {
        RKLog("Error. Filter index %d is invalid.\n", index);
        return RKResultFailedToSetFilter;
    }
    // Use the first pulse of the buffer to determine the capacity
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    // Check if filter anchor is valid
    if (engine->verbose > 1) {
        RKLog(">%s Setting filter group %d index %d ...\n", engine->name, group, index);
    }
    if (anchor.inputOrigin >= pulse->header.capacity) {
        RKLog("%s Error. Pulse capacity %s   Filter X @ (i:%s) invalid.\n", engine->name,
              RKIntegerToCommaStyleString(pulse->header.capacity),
              RKIntegerToCommaStyleString(anchor.inputOrigin));
        return RKResultFailedToSetFilter;
    }
    if (anchor.outputOrigin >= pulse->header.capacity) {
        RKLog("%s Error. Pulse capacity %s   Filter X @ (o:%s) invalid.\n", engine->name,
              RKIntegerToCommaStyleString(pulse->header.capacity),
              RKIntegerToCommaStyleString(anchor.outputOrigin));
        return RKResultFailedToSetFilter;
    }
    // Check if this filter works with my capacity & nfft
    const size_t nfft = 1 << (int)ceilf(log2f((float)MIN(RKMaximumGateCount, pulse->header.capacity)));
    if (anchor.length > nfft) {
        RKLog("%s Error. NFFT %s   Filter X @ (d:%s) invalid.\n", engine->name,
              RKIntegerToCommaStyleString(nfft),
              RKIntegerToCommaStyleString(anchor.length));
        return RKResultFailedToSetFilter;
    }
    if (engine->filters[group][index] == NULL) {
        RKLog("%s Error. Filter memory not allocated.\n", engine->name);
    }
    memset(engine->filters[group][index], 0, nfft * sizeof(RKComplex));
    memcpy(engine->filters[group][index], filter, anchor.length * sizeof(RKComplex));
    memcpy(&engine->filterAnchors[group][index], &anchor, sizeof(RKFilterAnchor));
    engine->filterAnchors[group][index].length = (uint32_t)MIN(nfft, anchor.length);
    engine->filterGroupCount = MAX(engine->filterGroupCount, group + 1);
    engine->filterCounts[group] = MAX(engine->filterCounts[group], index + 1);
    if (engine->state & RKEngineStateMemoryChange) {
        engine->state ^= RKEngineStateMemoryChange;
    }
    return RKResultSuccess;
}

int RKPulseEngineSetFilter(RKPulseEngine *engine, const RKComplex *filter, const RKFilterAnchor anchor) {
    return RKPulseEngineSetGroupFilter(engine, filter, anchor, 0, 0);
}

int RKPulseEngineSetFilterToImpulse(RKPulseEngine *engine) {
    if (engine->verbose > 1) {
        RKLog("%s Setting impulse filter...", engine->name);
    }
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    if (pulse == NULL) {
        RKLog("%s Error. RKPulseEngineSetFilterToImpulse() should be called after pulse buffer is set\n", engine->name);
        return RKResultNoPulseBuffer;
    }
    RKPulseEngineResetFilters(engine);
    RKComplex filter[] = {{1.0f, 0.0f}};
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    anchor.length = sizeof(filter) / sizeof(RKComplex);
    anchor.maxDataLength = pulse->header.capacity;
    anchor.subCarrierFrequency = 0.0f;
    anchor.filterGain = 0.0f;
    return RKPulseEngineSetFilter(engine, filter, anchor);
}

int RKPulseEngineSetFilterTo12321(RKPulseEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}, {2.0f, 0.0f}, {3.0f, 0.0f}, {2.0f, 0.0f}, {1.0f, 0.0f}};
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    if (pulse == NULL) {
        RKLog("%s Error. RKPulseEngineSetFilterTo*() should be called after pulse buffer is set\n", engine->name);
        return RKResultNoPulseBuffer;
    }
    RKPulseEngineResetFilters(engine);
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    anchor.length = sizeof(filter) / sizeof(RKComplex);
    anchor.maxDataLength = pulse->header.capacity;
    anchor.subCarrierFrequency = 0.0f;
    anchor.filterGain = 12.8f;
    return RKPulseEngineSetFilter(engine, filter, anchor);
}

int RKPulseEngineSetFilterTo121(RKPulseEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}, {2.0f, 0.0f}, {1.0f, 0.0f}};
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    if (pulse == NULL) {
        RKLog("%s Error. RKPulseEngineSetFilterTo*() should be called after pulse buffer is set\n", engine->name);
        return RKResultNoPulseBuffer;
    }
    RKPulseEngineResetFilters(engine);
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    anchor.length = sizeof(filter) / sizeof(RKComplex);
    anchor.maxDataLength = pulse->header.capacity;
    anchor.subCarrierFrequency = 0.0f;
    anchor.filterGain = 7.78f;
    return RKPulseEngineSetFilter(engine, filter, anchor);
}

int RKPulseEngineSetFilterTo11(RKPulseEngine *engine) {
    RKComplex filter[] = {{1.0f, 0.0f}, {1.0f, 0.0f}};
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    if (pulse == NULL) {
        RKLog("%s Error. RKPulseEngineSetFilterTo*() should be called after pulse buffer is set\n", engine->name);
        return RKResultNoPulseBuffer;
    }
    RKPulseEngineResetFilters(engine);
    RKFilterAnchor anchor = RKFilterAnchorDefault;
    anchor.length = sizeof(filter) / sizeof(RKComplex);
    anchor.maxDataLength = pulse->header.capacity;
    anchor.subCarrierFrequency = 0.0f;
    anchor.filterGain = 3.01f;
    return RKPulseEngineSetFilter(engine, filter, anchor);
}

#pragma mark - Interactions

int RKPulseEngineStart(RKPulseEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.  0x%08x\n", engine->name, engine->state);
        return RKResultEngineNotWired;
    }
    if (engine->filterGroupCount == 0) {
        // Set to default impulse as matched filter
        RKPulseEngineSetFilterToImpulse(engine);
    }
    if (engine->coreCount == 0) {
        engine->coreCount = 8;
    }
    if (engine->coreOrigin == 0) {
        engine->coreOrigin = 1;
    }
    if (engine->workers != NULL) {
        RKLog("%s Error. workers should be NULL here.\n", engine->name);
    }
    engine->workers = (RKPulseWorker *)malloc(engine->coreCount * sizeof(RKPulseWorker));
    engine->memoryUsage += engine->coreCount * sizeof(RKPulseWorker);
    memset(engine->workers, 0, engine->coreCount * sizeof(RKPulseWorker));
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidPulseWatcher, NULL, pulseWatcher, engine) != 0) {
        RKLog("%s Error. Failed to start.\n", engine->name);
        return RKResultFailedToStartPulseWatcher;
    }
    while (engine->tic == 0) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKPulseEngineStop(RKPulseEngine *engine) {
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
    if (engine->tidPulseWatcher) {
        pthread_join(engine->tidPulseWatcher, NULL);
        engine->tidPulseWatcher = (pthread_t)0;
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

char *RKPulseEngineStatusString(RKPulseEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}

char *RKPulseEnginePulseString(RKPulseEngine *engine) {
    return engine->pulseStatusBuffer[RKPreviousModuloS(engine->pulseStatusBufferIndex, RKBufferSSlotCount)];
}

void RKPulseEngineFilterSummary(RKPulseEngine *engine) {
    RKLog("%s I/Q filter set.  group count = %d\n", engine->name, engine->filterGroupCount);
    int i, j;
    char format[1024];
    RKPulse *pulse = (RKPulse *)engine->pulseBuffer;
    size_t nfft = 1 << (int)ceilf(log2f((float)MIN(pulse->header.capacity, engine->filterAnchors[0][0].maxDataLength)));
    for (i = 0; i < engine->filterGroupCount; i += 2) {
        int w0 = 0, w1 = 0, w2 = 0, w3 = 0;
        for (j = 0; j < engine->filterCounts[i]; j++) {
            w0 = MAX(w0, (int)log10f((float)engine->filterAnchors[i][j].length));
            w1 = MAX(w1, (int)log10f((float)engine->filterAnchors[i][j].inputOrigin));
            w2 = MAX(w2, (int)log10f((float)engine->filterAnchors[i][j].outputOrigin));
            w3 = MAX(w3, (int)log10f((float)engine->filterAnchors[i][j].maxDataLength));
        }
        w0 += (w0 / 3);
        w1 += (w1 / 3);
        w2 += (w2 / 3);
        w3 += (w3 / 3);
        sprintf(format, ">%%s - Filter[%%%dd][%%%dd/%%%dd] @ (%%%ds / %%%ds)   g:%%s%%+5.2f dB%%s, i:%%%ds, o:%%%ds, d:%%%ds\n",
                (int)log10f((float)engine->filterGroupCount) + 1,
                (int)log10f((float)engine->filterCounts[i]) + 1,
                (int)log10f((float)engine->filterCounts[i]) + 1,
                w0 + 1,
                (int)log10f(nfft) + 1,
                w1 + 1,
                w2 + 1,
                w3 + 1);
        for (j = 0; j < engine->filterCounts[i]; j++) {
            RKLog(format,
                  engine->name, i, j, engine->filterCounts[i],
                  RKIntegerToCommaStyleString(engine->filterAnchors[i][j].length),
                  RKIntegerToCommaStyleString(nfft),
                  rkGlobalParameters.showColor && fabs(engine->filterAnchors[i][j].filterGain) > 0.1 ? RKGetColorOfIndex(0) : "",
                  engine->filterAnchors[i][j].filterGain,
                  rkGlobalParameters.showColor ? RKNoColor : "",
                  RKIntegerToCommaStyleString(engine->filterAnchors[i][j].inputOrigin),
                  RKIntegerToCommaStyleString(engine->filterAnchors[i][j].outputOrigin),
                  RKIntegerToCommaStyleString(engine->filterAnchors[i][j].maxDataLength));
        }
    }
}

void RKPulseEngineShowBuffer(fftwf_complex *in, const int n) {
    for (int k = 0; k < n; k++) {
        printf("    %6.2f %s %6.2fi\n", in[k][0], in[k][1] < 0 ? "-" : "+", fabsf(in[k][1]));
    }
}
