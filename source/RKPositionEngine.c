//
//  RKPosition.c
//  RadarKit
//
//  Created by Boonleng Cheong on 1/3/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKPositionEngine.h>

// Internal functions

static void RKPositionnEngineUpdateStatusString(RKPositionEngine *);
static void *pulseTagger(void *);

// Implementations

#pragma mark - Helper Functions

#define RKPositionAzimuthFlagColor(x)                           \
(x & RKPositionFlagAzimuthError ? "\033[91m" :                  \
(x & RKPositionFlagAzimuthEnabled ? "\033[92m" : "\033[93m"))

#define RKPositionElevationFlagColor(x)                         \
(x & RKPositionFlagElevationError ? "\033[91m" :                \
(x & RKPositionFlagElevationEnabled ? "\033[92m" : "\033[93m"))

static void RKPositionnEngineUpdateStatusString(RKPositionEngine *engine) {
    int i;
    char *string;

    // Status string
    string = engine->statusBuffer[engine->statusBufferIndex];
    
    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

    // Use b characters to draw a bar
    i = engine->processedPulseIndex * RKStatusBarWidth / engine->radarDescription->pulseBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = 'P';

    // Engine lag
    snprintf(string + RKStatusBarWidth, RKStatusStringLength - RKStatusBarWidth, " %s%02.0f%s",
             rkGlobalParameters.showColor ? RKColorLag(engine->lag) : "",
             99.49f * engine->lag,
             rkGlobalParameters.showColor ? RKNoColor : "");
    
    // Position string
    string = engine->positionStringBuffer[engine->statusBufferIndex];

    // Always terminate the end of string buffer
    string[RKStatusStringLength - 1] = '\0';
    string[RKStatusStringLength - 2] = '#';

    // Same as previous status, use b characters to draw a bar
    i = *engine->positionIndex * RKStatusBarWidth / engine->radarDescription->positionBufferDepth;
    memset(string, '.', RKStatusBarWidth);
    string[i] = '#';
    i = RKStatusBarWidth + sprintf(string + RKStatusBarWidth, " %04d |", *engine->positionIndex);
    RKPosition *position = &engine->positionBuffer[RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth)];
    snprintf(string + i,RKStatusStringLength - i, " %010lu  %sAZ%s %6.2f° @ %+7.2f°/s [%6.2f°]   %sEL%s %6.2f° @ %+6.2f°/s [%6.2f°]  %08x",
             (unsigned long)position->i,
             rkGlobalParameters.showColor ? RKPositionAzimuthFlagColor(position->flag) : "",
             rkGlobalParameters.showColor ? RKNoColor : "",
             position->azimuthDegrees,
             position->azimuthVelocityDegreesPerSecond,
             position->sweepAzimuthDegrees,
             rkGlobalParameters.showColor ? RKPositionElevationFlagColor(position->flag) : "",
             rkGlobalParameters.showColor ? RKNoColor : "",
             position->elevationDegrees,
             position->elevationVelocityDegreesPerSecond,
             position->sweepElevationDegrees,
             position->flag);

    engine->statusBufferIndex = RKNextModuloS(engine->statusBufferIndex, RKBufferSSlotCount);
}

#pragma mark - Delegate Workers

static void *pulseTagger(void *_in) {
    RKPositionEngine *engine = (RKPositionEngine *)_in;
    
    int i, j, k, s;
    uint16_t c0, c1;
	struct timeval t0, t1;

	RKPulse *pulse;
    RKPosition *positionBefore;
    RKPosition *positionAfter;
    double timeBefore;
    double timeAfter;
    double timeLatest;
    double alpha;
	RKMarker marker0;
	RKMarker marker1 = RKMarkerSweepEnd;
    bool hasSweepEnd;

	// Update the engine state
	engine->state |= RKEngineStateWantActive;
	engine->state ^= RKEngineStateActivating;

	// If multiple workers are needed, here will be the time to launch them.

    RKLog("%s Started.   mem = %s B   pulseIndex = %d\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), *engine->pulseIndex);

	// Increase the tic once to indicate the engine is ready
	engine->tic = 1;

    gettimeofday(&t1, NULL); t1.tv_sec -= 1;

    // Wait until there is something ingested
    s = 0;
    engine->state |= RKEngineStateSleep0;
    while (*engine->positionIndex < 2 && engine->state & RKEngineStateWantActive) {
        usleep(1000);
        if (++s % 200 == 0 && engine->verbose > 1) {
            RKLog("%s sleep 0/%.1f s\n", engine->name, (float)s * 0.001f);
        }
    }
    engine->state ^= RKEngineStateSleep0;
    engine->state |= RKEngineStateActive;

    c0 = *engine->configIndex;
    c1 = RKPreviousModuloS(c0, engine->radarDescription->configBufferDepth);

    // Set the pulse to have position
    j = 0;   // position index
    k = 0;   // pulse index;
    while (engine->state & RKEngineStateWantActive) {
        // Get the latest pulse
        pulse = RKGetPulseFromBuffer(engine->pulseBuffer, k);
        
        // Wait until a thread check out this pulse.
        engine->state |= RKEngineStateSleep1;
        s = 0;
        while (k == *engine->pulseIndex && engine->state & RKEngineStateWantActive) {
            usleep(1000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 1/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.001f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep1;
        engine->state ^= RKEngineStateSleep2;
        // Wait until the pulse has data & processed. Otherwise, the time stamp is no good and there is a horse raise with the pulse compression engine (setting flag).
        s = 0;
        while (!(pulse->header.s & RKPulseStatusRingProcessed) && engine->state & RKEngineStateWantActive) {
            usleep(1000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 2/%.1f s   k = %d   pulseIndex = %d   header.s = 0x%02x\n",
                      engine->name, (float)s * 0.001f, k , *engine->pulseIndex, pulse->header.s);
            }
        }
        engine->state ^= RKEngineStateSleep2;
        engine->state |= RKEngineStateSleep3;
        // Wait until we have a position newer than pulse time.
        s = 0;
        i = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
        while ((!(engine->positionBuffer[i].flag & RKPositionFlagReady) || engine->positionBuffer[i].timeDouble <= pulse->header.timeDouble) && engine->state & RKEngineStateWantActive) {
            usleep(1000);
            if (++s % 100 == 0 && engine->verbose > 1) {
                RKLog("%s sleep 3/%.1f s   k = %d   positionTime = %s %s %s = pulseTime\n",
                      engine->name, (float)s * 0.001f, k,
                      RKFloatToCommaStyleString(engine->positionBuffer[i].timeDouble),
                      engine->positionBuffer[i].timeDouble <= pulse->header.timeDouble ? "<=" : ">",
                      RKFloatToCommaStyleString(pulse->header.timeDouble));
            }
            i = RKPreviousModuloS(*engine->positionIndex, engine->radarDescription->positionBufferDepth);
        }
        engine->state ^= RKEngineStateSleep3;
        
        // Record down the latest time
        timeLatest = engine->positionBuffer[i].timeDouble;
        
        if (!(engine->state & RKEngineStateWantActive)) {
            break;
        }
        
        // Lag of the engine
        engine->lag = fmodf(((float)*engine->pulseIndex + engine->radarDescription->pulseBufferDepth - k) / engine->radarDescription->pulseBufferDepth, 1.0f);
        
        // Search until the time just after the pulse was acquired.
        i = 0;
        hasSweepEnd = false;
        while (engine->positionBuffer[j].timeDouble <= pulse->header.timeDouble && i < engine->radarDescription->pulseBufferDepth && engine->state & RKEngineStateWantActive) {
            hasSweepEnd |= engine->positionBuffer[j].flag & (RKPositionFlagAzimuthComplete | RKPositionFlagElevationComplete);
            j = RKNextModuloS(j, engine->radarDescription->positionBufferDepth);
            i++;
        }
        if (i == engine->radarDescription->pulseBufferDepth) {
            if (engine->verbose > 2) {
                RKLog("Could not find an appropriate position.  %.2f %s %.2f",
                      pulse->header.timeDouble,
                      pulse->header.timeDouble < timeLatest ? "<" : ">=",
                      timeLatest);
            }
            // Update pulseIndex for the next watch
            RKLog("%s Warning. Skipping forward ...\n", engine->name);
            k = RKPreviousModuloS(*engine->pulseIndex, engine->radarDescription->pulseBufferDepth);
            continue;
        }
        positionAfter  = &engine->positionBuffer[j];   timeAfter  = positionAfter->timeDouble;
        positionAfter->flag |= RKPositionFlagUsed;
        
        // Roll back one slot, which should be the position just before the pulse was acquired.
        s = RKPreviousModuloS(j, engine->radarDescription->positionBufferDepth);
        positionBefore = &engine->positionBuffer[s];   timeBefore = positionBefore->timeDouble;
        positionBefore->flag |= RKPositionFlagUsed;
        
        // Linear interpololation : V_interp = V_before + alpha * (V_after - V_before)
        alpha = (pulse->header.timeDouble - timeBefore) / (timeAfter - timeBefore);
        pulse->header.azimuthDegrees = RKInterpolatePositiveAngles(positionBefore->azimuthDegrees,
                                                                   positionAfter->azimuthDegrees,
                                                                   alpha);
        pulse->header.elevationDegrees = RKInterpolateAngles(positionBefore->elevationDegrees,
                                                             positionAfter->elevationDegrees,
                                                             alpha);
        pulse->header.azimuthVelocityDegreesPerSecond = positionBefore->azimuthVelocityDegreesPerSecond;
        pulse->header.elevationVelocityDegreesPerSecond = positionBefore->elevationVelocityDegreesPerSecond;
        pulse->header.rawAzimuth = positionBefore->rawAzimuth;
        pulse->header.rawElevation = positionBefore->rawElevation;

        // Consolidate markers from the positions
        marker0 = RKMarkerNull;
        
        // First set of logics are purely from position
        if (positionBefore->flag & RKPositionFlagScanActive) {
            marker0 |= RKMarkerSweepMiddle;
        }
        if ((positionBefore->flag & RKPositionFlagElevationPoint) && (positionBefore->flag & RKPositionFlagAzimuthSweep)) {
            marker0 |= RKMarkerScanTypePPI;
        } else if ((positionBefore->flag & RKPositionFlagAzimuthPoint) && (positionBefore->flag & RKPositionFlagElevationSweep)) {
            marker0 |= RKMarkerScanTypeRHI;
        } else if ((positionBefore->flag & RKPositionFlagAzimuthPoint) && (positionBefore->flag & RKPositionFlagElevationPoint)) {
            marker0 |= RKMarkerScanTytpePoint;
        }
        
        // Second set of logics are derived from marker change
        // NOTE: hasComplete indicates that a sweep complete has been encountered during the search, which may be prior to positionBefore
        // NOTE: i = 0 means this loop is still using the same pair of positionBefore and positionAfter
        //       i = 1 means the next pair was valid, this is the case when pulses come in equal or faster than positions
        //       i > 1 means the next pair was invalid, this is the case when pulses come in slower than positions
        if (i == 1 && (positionBefore->flag & (RKPositionFlagAzimuthComplete | RKPositionFlagElevationComplete))) {
            marker0 |= RKMarkerSweepEnd;
        } else if (i > 1 && hasSweepEnd && !(marker1 & RKMarkerSweepEnd)) {
            marker0 |= RKMarkerSweepEnd;
        }
        if (!(marker1 & RKMarkerSweepMiddle) && (marker0 & RKMarkerSweepMiddle)) {
            marker0 |= RKMarkerSweepBegin;
        }
        if ((marker1 & RKMarkerSweepMiddle) && !(marker0 & RKMarkerSweepMiddle)) {
            marker0 |= RKMarkerSweepEnd;
        }
        if ((marker1 & RKMarkerSweepEnd) && (marker0 & RKMarkerSweepMiddle)) {
            marker0 |= RKMarkerSweepBegin;
        }

		// Mode change also indicates a start
		if ((marker1 & RKPositionFlagScanModeMask) != (marker0 & RKPositionFlagScanModeMask)) {
			marker0 |= RKMarkerSweepBegin;
		}

        if (marker0 & RKMarkerSweepBegin || (marker0 & RKMarkerScanTypeMask) != (marker1 & RKMarkerScanTypeMask)) {
            if (engine->verbose) {
                RKLog("%s C%02d New sweep   EL %.2f°   AZ %.2f°\n", engine->name,
                      *engine->configIndex, positionAfter->sweepElevationDegrees, positionAfter->sweepAzimuthDegrees);
            }
            // Add another configuration
            RKConfigAdvanceEllipsis(engine->configBuffer, engine->configIndex, engine->radarDescription->configBufferDepth,
                                    RKConfigKeySweepElevation, (double)positionAfter->sweepElevationDegrees,
                                    RKConfigKeySweepAzimuth, (double)positionAfter->sweepAzimuthDegrees,
                                    RKConfigKeyPulseGateSize, pulse->header.gateSizeMeters,
                                    RKConfigKeyPulseGateCount, pulse->header.gateCount,
                                    RKConfigKeyPositionMarker, marker0,
                                    RKConfigKeyNull);
        }

        if (c0 != *engine->configIndex) {
            c0 = *engine->configIndex;
            c1 = RKPreviousModuloS(c0, engine->radarDescription->configBufferDepth);
        }

        pulse->header.marker = marker0;
        pulse->header.configIndex = c1;
        
        marker1 = marker0;

        // Log a message if it has been a while
        gettimeofday(&t0, NULL);
        if (RKTimevalDiff(t0, t1) > 0.05) {
            t1 = t0;
            engine->processedPulseIndex = k;
            RKPositionnEngineUpdateStatusString(engine);
        }

        if (engine->verbose > 2) {
            RKLog("%s pulse[%04lu]  T [ %.4f %s %.4f %s %.4f ]   A [ %6.2f < %6.2f < %6.2f ]   E [ %.2f < %.2f < %.2f ] %s %08x < \033[3%dm%08x\033[0m < %08x (%d / %d)\n",
                  engine->name,
                  (unsigned long)pulse->header.i,
                  timeBefore,
                  timeBefore <= pulse->header.timeDouble ? "<" : ">=",
                  pulse->header.timeDouble,
                  pulse->header.timeDouble <= timeAfter ? "<" : ">=",
                  timeAfter,
                  positionBefore->azimuthDegrees,
                  pulse->header.azimuthDegrees,
                  positionAfter->azimuthDegrees,
                  positionBefore->elevationDegrees,
                  pulse->header.elevationDegrees,
                  positionAfter->elevationDegrees,
                  hasSweepEnd ? "E" : "-",
                  positionBefore->flag,
                  marker0 & 0x7,
                  marker0,
                  positionAfter->flag,
                  i, j);
        }
        
        pulse->header.s |= RKPulseStatusHasPosition;

		engine->tic++;

		// Update pulseIndex for the next watch
        k = RKNextModuloS(k, engine->radarDescription->pulseBufferDepth);
    }
    engine->state ^= RKEngineStateActive;
    return NULL;
}

#pragma mark - Life Cycle

RKPositionEngine *RKPositionEngineInit() {
    RKPositionEngine *engine = (RKPositionEngine *)malloc(sizeof(RKPositionEngine));
    memset(engine, 0, sizeof(RKPositionEngine));
    sprintf(engine->name, "%s<PulsePositioner>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPositionEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->memoryUsage = sizeof(RKPositionEngine);
    engine->state = RKEngineStateAllocated;
    return engine;
}

void RKPositionEngineFree(RKPositionEngine *engine) {
    free(engine);
}

#pragma mark - Properties

void RKPositionEngineSetVerbose(RKPositionEngine *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKPositionEngineSetInputOutputBuffers(RKPositionEngine *engine, const RKRadarDesc *desc,
                                           RKPosition *positionBuffer, uint32_t *positionIndex,
                                           RKConfig   *configBuffer,   uint32_t *configIndex,
                                           RKPulse    *pulseBuffer,    uint32_t *pulseIndex) {
    engine->radarDescription    = (RKRadarDesc *)desc;
    engine->positionBuffer      = positionBuffer;
    engine->positionIndex       = positionIndex;
    engine->configBuffer        = configBuffer;
    engine->configIndex         = configIndex;
    engine->pulseBuffer         = pulseBuffer;
    engine->pulseIndex          = pulseIndex;
    engine->state |= RKEngineStateProperlyWired;
}

#pragma mark - Interactions

int RKPositionEngineStart(RKPositionEngine *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->threadId, NULL, pulseTagger, engine)) {
        RKLog("Error. Unable to start position engine.\n");
        return RKResultFailedToStartPedestalWorker;
    }
	while (engine->tic == 0) {
        usleep(10000);
    }
    struct timeval t;
    gettimeofday(&t, NULL);
    engine->startTime = (double)t.tv_sec + 1.0e-6 * (double)t.tv_usec;
    return RKResultSuccess;
}

int RKPositionEngineStop(RKPositionEngine *engine) {
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
	if (engine->threadId) {
		pthread_join(engine->threadId, NULL);
		engine->threadId = (pthread_t)0;
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

char *RKPositionEngineStatusString(RKPositionEngine *engine) {
    return engine->statusBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}

char *RKPositionEnginePositionString(RKPositionEngine *engine) {
    return engine->positionStringBuffer[RKPreviousModuloS(engine->statusBufferIndex, RKBufferSSlotCount)];
}
