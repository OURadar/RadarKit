//
//  RKRadar.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include <RadarKit/RKRadar.h>

//
//
//

// More function definitions

typedef struct rk_task {
    RKRadar *radar;
    char command[RKNameLength];
} RKTaskInput;

#pragma mark - Helper Functions

void *radarCoPilot(void *in) {
    RKRadar *radar = (RKRadar *)in;
    RKMomentEngine *productGenerator = radar->momentEngine;
    int k = 0;
    int s = 0;

    RKLog("CoPilot started.  %d\n", productGenerator->rayStatusBufferIndex);
    
    while (radar->active) {
        s = 0;
        while (k == productGenerator->rayStatusBufferIndex && radar->active) {
            usleep(1000);
            if (++s % 200 == 0) {
                RKLog("Nothing ...\n");
            }
        }
        RKLog(productGenerator->rayStatusBuffer[k]);
        k = RKNextModuloS(k, RKBufferSSlotCount);
    }
    return NULL;
}

void *masterControllerExecuteInBackground(void *in) {
    RKTaskInput *task = (RKTaskInput *)in;
    if (task->radar->masterController) {
        task->radar->masterControllerExec(task->radar->masterController, task->command, NULL);
    }
    return NULL;
}

#pragma mark - Life Cycle

//
// Initialize a radar object
// Input:
//     RKRadarDesc desc - a description of the properties,
//     which are:
//         desc.initFlags - can be ORed togher from RKInitFlag enums
//         desc.pulseCapacity - the maximum number of samples for each pulse
//         desc.pulseToRayRatio - the down-sampling factor going from pulse to ray
//         desc.healthNodeCount - the number of user health node count
//         desc.healthBufferDepth - the depth of the cosolidated health buffer
//         desc.configBufferDepth - the depth of the operational configuration parameters
//         desc.positionBufferDepth - the depth of position readings
//         desc.pulseBufferDepth - the depth of pulse buffer
//         desc.rayBufferDepth - the depth of ray buffer
//         desc.controlCapacity - the maximum number of control
//         desc.expectedPulseRate - typical number of pulses per second (from the Transceiver)
//         desc.expectedPositionRate - typical number of positions per second (from the Pedestal)
//         desc.latitude - latitude in degrees
//         desc.longitude - longitude in degrees
//         desc.heading - heading in degrees
//         desc.radarHeight - radar height from the ground
//         desc.wavelength - radar wavelength in meters
//         desc.name[RKNameLength] - radar name
//         desc.filePrefix[RKNameLength] - file prefix user would like to use
//         desc.dataPath[RKMaximumPathLength] - the root path where data are stored
// output:
//     RKRadar *radar - an "object" radar. This is a reference of a radar system.
//
RKRadar *RKInitWithDesc(const RKRadarDesc desc) {
    RKRadar *radar;
    size_t bytes;
    int i, k;
    char name[RKNameLength];

    RKSetUseDailyLog(true);
    RKSetRootFolder(desc.dataPath);
	
    if (desc.initFlags & RKInitFlagVerbose) {
        RKLog("Initializing ... 0x%x", desc.initFlags);
    }
    // Allocate self
    bytes = sizeof(RKRadar);
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&radar, RKSIMDAlignSize, bytes))
    memset(radar, 0, bytes);

    // Get the number of CPUs
    radar->processorCount = (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
    if (desc.initFlags & RKInitFlagVerbose) {
        RKLog("Number of online CPUs = %ld\n", radar->processorCount);
    }

    // Set some non-zero variables
    sprintf(radar->name, "%s<MasterController>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(13) : "", rkGlobalParameters.showColor ? RKNoColor : "");
    radar->memoryUsage += bytes;

    // Copy over the input flags and constaint the capacity and depth to hard-coded limits
    radar->desc = desc;
    if (radar->desc.pulseBufferDepth > RKBuffer0SlotCount) {
        radar->desc.pulseBufferDepth = RKBuffer0SlotCount;
        RKLog("Info. Pulse buffer clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.pulseBufferDepth));
    } else if (radar->desc.pulseBufferDepth == 0) {
        radar->desc.pulseBufferDepth = 2000;
    }
    if (radar->desc.rayBufferDepth > RKBuffer2SlotCount) {
        radar->desc.rayBufferDepth = RKBuffer2SlotCount;
        RKLog("Info. Ray buffer clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.rayBufferDepth));
    } else if (radar->desc.rayBufferDepth == 0) {
        radar->desc.rayBufferDepth = 720;
    }
    if (radar->desc.pulseCapacity > RKGateCount) {
        radar->desc.pulseCapacity = RKGateCount;
        RKLog("Info. Pulse capacity clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
    } else if (radar->desc.pulseCapacity == 0) {
        radar->desc.pulseCapacity = 512;
    }
    radar->desc.pulseCapacity = ((radar->desc.pulseCapacity * sizeof(RKFloat) + RKSIMDAlignSize - 1) / RKSIMDAlignSize) * RKSIMDAlignSize / sizeof(RKFloat);
    if (radar->desc.pulseCapacity != desc.pulseCapacity) {
        RKLog("Info. Pulse capacity changed from %s to %s\n", RKIntegerToCommaStyleString(desc.pulseCapacity), RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
    }
    if (radar->desc.statusBufferDepth > RKBufferSSlotCount) {
        radar->desc.statusBufferDepth = RKBufferSSlotCount;
        RKLog("Info. Status buffer clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.statusBufferDepth));
    } else if (radar->desc.statusBufferDepth == 0) {
        radar->desc.statusBufferDepth = 90;
    }
    if (radar->desc.configBufferDepth > RKBufferCSlotCount) {
        radar->desc.configBufferDepth = RKBufferCSlotCount;
        RKLog("Info. Config buffer clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.configBufferDepth));
    } else if (radar->desc.configBufferDepth == 0) {
        radar->desc.configBufferDepth = 25;
    }
    if (radar->desc.healthBufferDepth > RKBufferHSlotCount) {
        radar->desc.healthBufferDepth = RKBufferHSlotCount;
        RKLog("Info. Health buffer clamped to %s\n", RKIntegerToCommaStyleString(radar->desc.healthBufferDepth));
    } else if (radar->desc.healthBufferDepth == 0) {
        radar->desc.healthBufferDepth = 25;
    }
    if (radar->desc.healthNodeCount == 0 || radar->desc.healthNodeCount > RKHealthNodeCount) {
        radar->desc.healthNodeCount = RKHealthNodeCount;
    }
    if (radar->desc.positionBufferDepth > RKBufferPSlotCount) {
        radar->desc.positionBufferDepth = RKBufferPSlotCount;
        RKLog("Info. Position buffer clamped to %s\n", radar->desc.positionBufferDepth);
    } else if (radar->desc.positionBufferDepth == 0) {
        radar->desc.positionBufferDepth = 500;
    }
    if (radar->desc.controlCapacity > RKMaxControlCount) {
        radar->desc.controlCapacity = RKMaxControlCount;
        RKLog("Info. Control count limited to %s\n", radar->desc.controlCapacity);
    } else if (radar->desc.controlCapacity == 0) {
        radar->desc.controlCapacity = RKMaxControlCount;
    }

    // Read in preference file here, override some values
    if (!strlen(radar->desc.name)) {
        sprintf(radar->desc.name, "Radar");
    }
    if (!strlen(radar->desc.filePrefix)) {
        sprintf(radar->desc.filePrefix, "RK");
    }
    RKLog("Radar name = '%s'  prefix = '%s'", radar->desc.name, radar->desc.filePrefix);
    radar->desc.latitude = 35.2550320;
    radar->desc.longitude = -97.4227810;
    if (strlen(desc.dataPath) == 0) {
        sprintf(radar->desc.dataPath, RKDefaultDataPath);
    }

    // Status buffer
    if (radar->desc.initFlags & RKInitFlagAllocStatusBuffer) {
        radar->state |= RKRadarStateStatusBufferAllocating;
        bytes = radar->desc.statusBufferDepth * sizeof(RKStatus);
        if (bytes == 0) {
            RKLog("Error. Zero storage for status buffer?\n");
            radar->desc.statusBufferDepth = 25;
            bytes = radar->desc.statusBufferDepth * sizeof(RKStatus);
        }
        radar->status = (RKStatus *)malloc(bytes);
        if (radar->status == NULL) {
            RKLog("Error. Unable to allocate memory for status buffer");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Status buffer occupies %s B  (%s sets)\n",
                  RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(radar->desc.statusBufferDepth));
        }
        memset(radar->status, 0, bytes);
        radar->memoryUsage += bytes;
        for (i = 0; i < radar->desc.statusBufferDepth; i++) {
            radar->status[i].i = (uint64_t)(-1) - radar->desc.statusBufferDepth + i;
        }
        radar->state ^= RKRadarStateStatusBufferAllocating;
        radar->state |= RKRadarStateStatusBufferInitialized;
    }

    // Config buffer
    if (radar->desc.initFlags & RKInitFlagAllocConfigBuffer) {
        radar->state |= RKRadarStateConfigBufferAllocating;
        bytes = radar->desc.configBufferDepth * sizeof(RKConfig);
        if (bytes == 0) {
            RKLog("Error. Zero storage for config buffer?\n");
            radar->desc.configBufferDepth = 25;
            bytes = radar->desc.configBufferDepth * sizeof(RKConfig);
        }
        radar->configs = (RKConfig *)malloc(bytes);
        if (radar->configs == NULL) {
            RKLog("Error. Unable to allocate memory for config buffer");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Config buffer occupies %s B  (%s sets)\n",
                  RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(radar->desc.configBufferDepth));
        }
        memset(radar->configs, 0, bytes);
        radar->memoryUsage += bytes;
        for (i = 0; i < radar->desc.configBufferDepth; i++) {
            radar->configs[i].i = (uint64_t)(-1) - radar->desc.configBufferDepth + i;
        }
        radar->state ^= RKRadarStateConfigBufferAllocating;
        radar->state |= RKRadarStateConfigBufferInitialized;
    }
    
    // Health buffer
    if (radar->desc.initFlags & RKInitFlagAllocHealthBuffer) {
        radar->state |= RKRadarStateHealthBufferAllocating;
        bytes = radar->desc.healthBufferDepth * sizeof(RKHealth);
        if (bytes == 0) {
            RKLog("Error. Zero storage for health buffer?\n");
            radar->desc.healthBufferDepth = 25;
            bytes = radar->desc.healthBufferDepth * sizeof(RKHealth);
        }
        radar->healths = (RKHealth *)malloc(bytes);
        if (radar->healths == NULL) {
            RKLog("Error. Unable to allocate memory for health status");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Health buffer occupies %s B  (%s sets)\n",
                  RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(radar->desc.healthBufferDepth));
        }
        memset(radar->healths, 0, bytes);
        radar->memoryUsage += bytes;
        for (i = 0; i < radar->desc.healthBufferDepth; i++) {
            radar->healths[i].i = (uint64_t)(-1) - radar->desc.healthBufferDepth + i;
        }
        radar->state ^= RKRadarStateHealthBufferAllocating;
        radar->state |= RKRadarStateHealthBufferInitialized;
    }
    
    // Health nodes
    if (radar->desc.initFlags & RKInitFlagAllocHealthNodes) {
        radar->state |= RKRadarStateHealthNodesAllocating;
        bytes = radar->desc.healthNodeCount * sizeof(RKNodalHealth);
        radar->healthNodes = (RKNodalHealth *)malloc(bytes);
        memset(radar->healthNodes, 0, bytes);
        radar->memoryUsage += bytes;
        bytes = radar->desc.healthBufferDepth * sizeof(RKHealth);
        if (bytes == 0) {
            RKLog("Error. Zero storage for health nodes?\n");
            radar->desc.healthBufferDepth = 25;
            bytes = radar->desc.healthBufferDepth * sizeof(RKHealth);
        }
        for (i = 0; i < radar->desc.healthNodeCount; i++) {
            radar->healthNodes[i].healths = (RKHealth *)malloc(bytes);
            memset(radar->healthNodes[i].healths, 0, bytes);
        }
        radar->memoryUsage += radar->desc.healthNodeCount * bytes;
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Nodal-health buffers occupy %s B  (%d nodes x %s sets)\n",
                  RKIntegerToCommaStyleString(radar->desc.healthNodeCount * bytes), radar->desc.healthNodeCount, RKIntegerToCommaStyleString(radar->desc.healthBufferDepth));
        }
        for (k = 0; k < radar->desc.healthNodeCount; k++) {
            for (i = 0; i < radar->desc.healthBufferDepth; i++) {
                radar->healthNodes[k].healths[i].i = (uint64_t)(-1) - radar->desc.healthBufferDepth + i;
            }
        }
        radar->state ^= RKRadarStateHealthNodesAllocating;
        radar->state |= RKRadarStateHealthNodesInitialized;
    }
    
    // Position buffer
    if (radar->desc.initFlags & RKInitFlagAllocPositionBuffer) {
        radar->state |= RKRadarStatePositionBufferAllocating;
        bytes = radar->desc.positionBufferDepth * sizeof(RKPosition);
        if (bytes == 0) {
            RKLog("Error. Zero storage for position buffer?\n");
            radar->desc.positionBufferDepth = 250;
            bytes = radar->desc.positionBufferDepth * sizeof(RKPosition);
        }
        radar->positions = (RKPosition *)malloc(bytes);
        if (radar->positions == NULL) {
            RKLog("Error. Unable to allocate memory for positions.");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Position buffer occupies %s B  (%s positions)\n",
                  RKIntegerToCommaStyleString(bytes), RKIntegerToCommaStyleString(radar->desc.positionBufferDepth));
        }
        radar->memoryUsage += bytes;
        for (i = 0; i < radar->desc.positionBufferDepth; i++) {
            radar->positions[i].i = (uint64_t)(-1) - radar->desc.positionBufferDepth + i;
        }
        radar->state ^= RKRadarStatePositionBufferAllocating;
        radar->state |= RKRadarStatePositionBufferInitialized;
    }

    // Pulse (IQ) buffer
    if (radar->desc.initFlags & RKInitFlagAllocRawIQBuffer) {
        radar->state |= RKRadarStateRawIQBufferAllocating;
        bytes = RKPulseBufferAlloc(&radar->pulses, radar->desc.pulseCapacity, radar->desc.pulseBufferDepth);
        if (bytes == 0 || radar->pulses == NULL) {
            RKLog("Error. Unable to allocate memory for I/Q pulses.\n");
            exit(EXIT_FAILURE);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Level I buffer occupies %s B  (%s pulses x %s gates)\n",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.pulseBufferDepth),
                  RKIntegerToCommaStyleString(radar->desc.pulseCapacity));
        }
        for (i = 0; i < radar->desc.pulseBufferDepth; i++) {
            RKPulse *pulse = RKGetPulse(radar->pulses, i);
            size_t offset = (size_t)pulse->data - (size_t)pulse;
            if (offset != RKPulseHeaderPaddedSize) {
                printf("Unexpected offset = %d != %d\n", (int)offset, RKPulseHeaderPaddedSize);
            }
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRawIQBufferAllocating;
        radar->state |= RKRadarStateRawIQBufferInitialized;
    }

    // Ray (moment) bufer
    if (radar->desc.initFlags & RKInitFlagAllocMomentBuffer) {
        radar->state |= RKRadarStateRayBufferAllocating;
        k = ((int)ceilf((float)(radar->desc.pulseCapacity / radar->desc.pulseToRayRatio) / (float)RKSIMDAlignSize)) * RKSIMDAlignSize;
        bytes = RKRayBufferAlloc(&radar->rays, k, radar->desc.rayBufferDepth);
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Level II buffer occupies %s B  (%s rays x %d products of %s gates)\n",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.rayBufferDepth),
                  RKMaxProductCount,
                  RKIntegerToCommaStyleString(k));
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateRayBufferAllocating;
        radar->state |= RKRadarStateRayBufferInitialized;
    }

    // Controls
    if (radar->desc.controlCapacity) {
        radar->state |= RKRadarStateControlsAllocating;
        bytes = radar->desc.controlCapacity * sizeof(RKControl);
        if (bytes == 0) {
            RKLog("Error. Zero storage for controls?\n");
            radar->desc.controlCapacity = 64;
            bytes = radar->desc.controlCapacity * sizeof(RKControl);
        }
        radar->controls = (RKControl *)malloc(bytes);
        if (radar->controls == NULL) {
            RKLog("Error. Unable to allocate memory for controls.\n");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < radar->desc.controlCapacity; i++) {
            RKControl *control = &radar->controls[i];
            control->uid = i;
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("Controls occupy %s B (%s units)",
                  RKIntegerToCommaStyleString(bytes),
                  RKIntegerToCommaStyleString(radar->desc.controlCapacity));
        }
        radar->memoryUsage += bytes;
        radar->state ^= RKRadarStateControlsAllocating;
        radar->state |= RKRadarStateControlsInitialized;
    }

    // File manager
    radar->fileManager = RKFileManagerInit();
    RKFileManagerSetInputOutputBuffer(radar->fileManager, &radar->desc);
    radar->memoryUsage += radar->fileManager->memoryUsage;
    radar->state |= RKRadarStateFileManagerInitialized;

    // Signal processor marries pulse and position data, process for moment, etc.
    if (radar->desc.initFlags & RKInitFlagSignalProcessor) {
        // Clocks
        //radar->pulseClock = RKClockInitWithSize(15000, 5000);
        if (radar->desc.pulseSmoothFactor > 0) {
            radar->pulseClock = RKClockInitWithSize(radar->desc.pulseSmoothFactor + 1000, radar->desc.pulseSmoothFactor);
        } else {
            radar->pulseClock = RKClockInit();
        }
        if (radar->desc.pulseTicsPerSecond > 0) {
            RKClockSetDuDx(radar->pulseClock, (double)radar->desc.pulseTicsPerSecond);
        }
        sprintf(name, "%s<PulseClock>%s",
                rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorClock) : "", RKNoColor);
        RKClockSetName(radar->pulseClock, name);
        radar->memoryUsage += sizeof(RKClock);
        
        //radar->positionClock = RKClockInit();
        if (radar->desc.positionSmoothFactor > 0) {
            radar->positionClock = RKClockInitWithSize(radar->desc.positionSmoothFactor + 1000, radar->desc.positionSmoothFactor);
        } else {
            radar->positionClock = RKClockInit();
        }
        if (radar->desc.positionTicsPerSecond > 0) {
            RKClockSetDuDx(radar->positionClock, (double)radar->desc.positionTicsPerSecond);
        }
        sprintf(name, "%s<PositionClock>%s",
                rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorClock) : "", RKNoColor);
        RKClockSetName(radar->positionClock, name);
        RKClockSetOffset(radar->positionClock, -radar->desc.positionLatency);
		RKLog("positionLatency = %.3e\n", radar->desc.positionLatency);
        radar->memoryUsage += sizeof(RKClock);
        
        // Pulse compression engine
        radar->pulseCompressionEngine = RKPulseCompressionEngineInit();
        RKPulseCompressionEngineSetInputOutputBuffers(radar->pulseCompressionEngine, &radar->desc,
                                                      radar->configs, &radar->configIndex,
                                                      radar->pulses, &radar->pulseIndex);
        radar->memoryUsage += radar->pulseCompressionEngine->memoryUsage;
        radar->state |= RKRadarStatePulseCompressionEngineInitialized;
        
        // Position engine
        radar->positionEngine = RKPositionEngineInit();
        RKPositionEngineSetInputOutputBuffers(radar->positionEngine, &radar->desc,
                                              radar->positions, &radar->positionIndex,
                                              radar->configs, &radar->configIndex,
                                              radar->pulses, &radar->pulseIndex);
        radar->memoryUsage += radar->positionEngine->memoryUsage;
        radar->state |= RKRadarStatePositionEngineInitialized;

        // Moment engine
        radar->momentEngine = RKMomentEngineInit();
        RKMomentEngineSetInputOutputBuffers(radar->momentEngine, &radar->desc,
                                            radar->configs, &radar->configIndex,
                                            radar->pulses, &radar->pulseIndex,
                                            radar->rays, &radar->rayIndex);
        radar->memoryUsage += radar->momentEngine->memoryUsage;
        radar->state |= RKRadarStateMomentEngineInitialized;

        // Health engine
        radar->healthEngine = RKHealthEngineInit();
        RKHealthEngineSetInputOutputBuffers(radar->healthEngine, &radar->desc, radar->healthNodes,
                                            radar->healths, &radar->healthIndex);
        radar->memoryUsage += radar->healthEngine->memoryUsage;
        radar->state |= RKRadarStateHealthEngineInitialized;
    } else {
        // Radar relay
        radar->radarRelay = RKRadarRelayInit();
        RKRadarRelaySetInputOutputBuffers(radar->radarRelay, &radar->desc, radar->fileManager,
                                          radar->status, &radar->statusIndex,
                                          radar->configs, &radar->configIndex,
                                          radar->healths, &radar->healthIndex,
                                          radar->pulses, &radar->pulseIndex,
                                          radar->rays, &radar->rayIndex);
        RKRadarRelaySetVerbose(radar->radarRelay, 2);
        radar->memoryUsage += radar->radarRelay->memoryUsage;
        radar->state |= RKRadarStateRadarRelayInitialized;
    }

    // Health logger (to be modified)
    radar->healthLogger = RKHealthLoggerInit();
    RKHealthLoggerSetInputOutputBuffers(radar->healthLogger, &radar->desc, radar->fileManager,
                                        radar->healths, &radar->healthIndex);
    radar->memoryUsage += radar->healthLogger->memoryUsage;
    radar->state |= RKRadarStateHealthLoggerInitialized;

    // Sweep engine (to be modified)
    radar->sweepEngine = RKSweepEngineInit();
    RKSweepEngineSetInputOutputBuffer(radar->sweepEngine, &radar->desc, radar->fileManager,
                                      radar->configs, &radar->configIndex,
                                      radar->rays, &radar->rayIndex);
    radar->memoryUsage += radar->sweepEngine->memoryUsage;
    radar->state |= RKRadarStateSweepEngineInitialized;
    
    // Raw data recorder
    radar->dataRecorder = RKDataRecorderInit();
    RKDataRecorderSetInputOutputBuffers(radar->dataRecorder, &radar->desc, radar->fileManager,
                                      radar->configs, &radar->configIndex,
                                      radar->pulses, &radar->pulseIndex);
    radar->memoryUsage += radar->dataRecorder->memoryUsage;
    radar->state |= RKRadarStateFileRecorderInitialized;

    // Total memory usage
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Radar initialized. Data buffers occupy \033[4m%s B\033[24m (%s GiB)\n",
              RKIntegerToCommaStyleString(radar->memoryUsage),
              RKFloatToCommaStyleString((double)radar->memoryUsage / 1073741824.0));
    }

    return radar;
}

RKRadar *RKInitQuiet(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocEverythingQuiet | RKInitFlagSignalProcessor;
    desc.pulseCapacity = 4096;
    desc.pulseToRayRatio = 1;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.positionBufferDepth = RKBufferPSlotCount;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitLean(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocEverything | RKInitFlagSignalProcessor;
    desc.pulseCapacity = 2048;
    desc.pulseToRayRatio = 1;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.positionBufferDepth = RKBufferPSlotCount;
    desc.pulseBufferDepth = 10000;
    desc.rayBufferDepth = 1080;
    return RKInitWithDesc(desc);
}

RKRadar *RKInitMean(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocEverything | RKInitFlagSignalProcessor;
    desc.pulseCapacity = 8192;
    desc.pulseToRayRatio = 2;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.positionBufferDepth = RKBufferPSlotCount;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

//
// Initializes a radar object with all features
// Input:
//     None
// Output:
//     An RKRadar object
//
RKRadar *RKInitFull(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocEverything | RKInitFlagSignalProcessor;
    desc.pulseCapacity = RKGateCount;
    desc.pulseToRayRatio = 8;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.positionBufferDepth = RKBufferPSlotCount;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

//
// Initializes a radar object with all features
// Input:
//     None
// Output:
//     An RKRadar object
//
RKRadar *RKInit(void) {
    return RKInitFull();
}

//
// Initializes a radar object as a relay
// Input:
//     None
// Output:
//     An RKRadar object
//
RKRadar *RKInitAsRelay(void) {
    RKRadarDesc desc;
    memset(&desc, 0, sizeof(RKRadarDesc));
    desc.initFlags = RKInitFlagAllocRawIQBuffer | RKInitFlagAllocMomentBuffer;
    desc.pulseCapacity = RKGateCount;
    desc.pulseToRayRatio = 8;
    desc.configBufferDepth = RKBufferCSlotCount;
    desc.healthBufferDepth = RKBufferHSlotCount;
    desc.pulseBufferDepth = RKBuffer0SlotCount;
    desc.rayBufferDepth = RKBuffer2SlotCount;
    return RKInitWithDesc(desc);
}

//
// Free up the radar object
// Input:
//     None
// Output:
//     RKResultSuccess if no errors
//
int RKFree(RKRadar *radar) {
	int k = radar->desc.initFlags & RKInitFlagVerbose;
    if (radar->active) {
        RKStop(radar);
    }
    // Various RadarKit engines
    if (radar->state & RKRadarStateFileManagerInitialized) {
        RKFileManagerFree(radar->fileManager);
    }
    if (radar->pulseClock) {
        RKClockFree(radar->pulseClock);
        radar->pulseClock = NULL;
    }
    if (radar->positionClock) {
        RKClockFree(radar->positionClock);
        radar->positionClock = NULL;
    }
    if (radar->state & RKRadarStatePulseCompressionEngineInitialized) {
        RKPulseCompressionEngineFree(radar->pulseCompressionEngine);
    }
    if (radar->state & RKRadarStatePositionEngineInitialized) {
        RKPositionEngineFree(radar->positionEngine);
    }
    if (radar->state & RKRadarStateMomentEngineInitialized) {
        RKMomentEngineFree(radar->momentEngine);
    }
    if (radar->state & RKRadarStateHealthEngineInitialized) {
        RKHealthEngineFree(radar->healthEngine);
    }
    if (radar->state & RKRadarStateRadarRelayInitialized) {
        RKRadarRelayFree(radar->radarRelay);
    }
    if (radar->state & RKRadarStateHealthLoggerInitialized) {
        RKHealthLoggerFree(radar->healthLogger);
    }
    if (radar->state & RKRadarStateSweepEngineInitialized) {
        RKSweepEngineFree(radar->sweepEngine);
    }
    if (radar->state & RKRadarStateFileRecorderInitialized) {
        RKDataRecorderFree(radar->dataRecorder);
    }
    // Transceiver, pedestal & health relay
    if (radar->pedestal) {
        radar->pedestalFree(radar->pedestal);
    }
    if (radar->transceiver) {
        radar->transceiverFree(radar->transceiver);
    }
    if (radar->healthRelay) {
        radar->healthRelayFree(radar->healthRelay);
    }
    // Buffers
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Freeing radar '%s' ...\n", radar->desc.name);
    }
    if (radar->state & RKRadarStateStatusBufferInitialized) {
        free(radar->status);
    }
    if (radar->state & RKRadarStateConfigBufferInitialized) {
        free(radar->configs);
    }
    if (radar->state & RKRadarStateHealthBufferInitialized) {
        free(radar->healths);
    }
    if (radar->state & RKRadarStateHealthNodesInitialized) {
        free(radar->healthNodes);
    }
    if (radar->state & RKRadarStatePositionBufferInitialized) {
        free(radar->positions);
    }
    if (radar->state & RKRadarStateRawIQBufferInitialized) {
        free(radar->pulses);
    }
    if (radar->state & RKRadarStateRayBufferInitialized) {
        free(radar->rays);
    }
    if (radar->state & RKRadarStateControlsInitialized) {
        free(radar->controls);
    }
    free(radar);
	if (k) {
		RKLog("Done.");
	}
    return RKResultSuccess;
}

#pragma mark - Hardware Hooks

//
// The three major compenents: Digital transceiver, pedestal and general health relay
//
// Input:
//     RKRadar *radar - object of the radar
//     void *input - user defined input structure / object, de-referenced to void * here
//     RKTransceiver/RKPedestal/RKHealthRelay initRoutine(RKRadar *, void *) - init routine
//     int execRoutine(RKTransceiver, const char *, char *) - command execution routine
//     int freeRoutine(RKTransceiver)) - de-allocation routine
// Output:
//     RKResultSuccess if everything was successful
//
int RKSetTransceiver(RKRadar *radar,
                     void *initInput,
                     RKTransceiver initRoutine(RKRadar *, void *),
                     int execRoutine(RKTransceiver, const char *, char *),
                     int freeRoutine(RKTransceiver)) {
    radar->transceiverInitInput = initInput;
    radar->transceiverInit = initRoutine;
    radar->transceiverExec = execRoutine;
    radar->transceiverFree = freeRoutine;
    return RKResultNoError;
}

int RKSetPedestal(RKRadar *radar,
                  void *initInput,
                  RKPedestal initRoutine(RKRadar *, void *),
                  int execRoutine(RKPedestal, const char *, char *),
                  int freeRoutine(RKPedestal)) {
    radar->pedestalInitInput = initInput;
    radar->pedestalInit = initRoutine;
    radar->pedestalExec = execRoutine;
    radar->pedestalFree = freeRoutine;
    return RKResultNoError;
}

int RKSetHealthRelay(RKRadar *radar,
                     void *initInput,
                     RKHealthRelay initRoutine(RKRadar *, void *),
                     int execRoutine(RKHealthRelay, const char *, char *),
                     int freeRoutine(RKHealthRelay)) {
    radar->healthRelayInitInput = initInput;
    radar->healthRelayInit = initRoutine;
    radar->healthRelayExec = execRoutine;
    radar->healthRelayFree = freeRoutine;
    return RKResultNoError;
}

#pragma mark - Properties

//
// Property setters are self-explanatory so minimal descriptions are provided here
//
int RKSetVerbose(RKRadar *radar, const int verbose) {
    if (verbose) {
        RKLog("Setting verbose level to %d ...\n", verbose);
    }
    if (verbose == 1) {
        radar->desc.initFlags |= RKInitFlagVerbose;
    } else if (verbose == 2) {
        radar->desc.initFlags |= RKInitFlagVeryVerbose;
    } else if (verbose >= 3) {
        radar->desc.initFlags |= RKInitFlagVeryVeryVerbose;
    }
    if (radar->fileManager) {
        RKFileManagerSetVerbose(radar->fileManager, verbose);
    }
    if (radar->pulseClock) {
        RKClockSetVerbose(radar->pulseClock, verbose);
    }
    if (radar->positionClock) {
        RKClockSetVerbose(radar->positionClock, verbose);
    }
    if (radar->positionEngine) {
        RKPositionEngineSetVerbose(radar->positionEngine, verbose);
    }
    if (radar->pulseCompressionEngine) {
        RKPulseCompressionEngineSetVerbose(radar->pulseCompressionEngine, verbose);
    }
    if (radar->momentEngine) {
        RKMomentEngineSetVerbose(radar->momentEngine, verbose);
    }
    if (radar->healthEngine) {
        RKHealthEngineSetVerbose(radar->healthEngine, verbose);
    }
    if (radar->radarRelay) {
        RKRadarRelaySetVerbose(radar->radarRelay, verbose);
    }
    if (radar->healthLogger) {
        RKHealthLoggerSetVerbose(radar->healthLogger, verbose);
    }
    if (radar->sweepEngine) {
        RKSweepEngineSetVerbose(radar->sweepEngine, verbose);
    }
    if (radar->dataRecorder) {
        RKDataRecorderSetVerbose(radar->dataRecorder, verbose);
    }
    return RKResultNoError;
}

int RKSetDataPath(RKRadar *radar, const char *path) {
    memcpy(radar->desc.dataPath, path, RKMaximumPathLength - 1);
    RKSetRootFolder(path);
    return RKResultNoError;
}

int RKSetDataUsageLimit(RKRadar *radar, const size_t limit) {
    radar->fileManager->usagelimit = limit;
    RKLog("Usage limit %s B\n", RKIntegerToCommaStyleString(radar->fileManager->usagelimit));
    return RKResultNoError;
}

int RKSetDoNotWrite(RKRadar *radar, const bool doNotWrite) {
    RKHealthLoggerSetDoNotWrite(radar->healthLogger, doNotWrite);
    RKSweepEngineSetDoNotWrite(radar->sweepEngine, doNotWrite);
    RKDataRecorderSetDoNotWrite(radar->dataRecorder, doNotWrite);
    return RKResultNoError;
}

int RKSetDataRecorder(RKRadar *radar, const bool record) {
    radar->dataRecorder->doNotWrite = !record;
    return RKResultNoError;
}

int RKToggleDataRecorder(RKRadar *radar) {
    radar->dataRecorder->doNotWrite = !radar->dataRecorder->doNotWrite;
    return RKResultNoError;
}

int RKSetWaveform(RKRadar *radar, RKWaveform *waveform) {
    if (radar->pulseCompressionEngine == NULL) {
        RKLog("Error. No pulse compression engine.\n");
        return RKResultNoPulseCompressionEngine;
    }
    int j, k;
    RKPulseCompressionResetFilters(radar->pulseCompressionEngine);
    for (k = 0; k < waveform->count; k++) {
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            RKComplex *filter = waveform->samples[k] + waveform->filterAnchors[k][j].origin;
            RKPulseCompressionSetFilter(radar->pulseCompressionEngine,
                                        filter,
                                        waveform->filterAnchors[k][j],
                                        k,
                                        j);
        }
    }
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKPulseCompressionFilterSummary(radar->pulseCompressionEngine);
    }
    RKClockReset(radar->pulseClock);
    return RKResultNoError;
}

int RKSetMomentProcessorToMultiLag(RKRadar *radar, const uint8_t lagChoice) {
	radar->momentEngine->processor = &RKMultiLag;
	radar->momentEngine->processorLagCount = RKLagCount;
	if (lagChoice < 0 || lagChoice > 4) {
		RKLog("Error. Invalid lag choice (%d) for multi-lag method.\n", lagChoice);
		return RKResultInvalidMomentParameters;
	}
	radar->momentEngine->userLagChoice = lagChoice;
	return RKResultNoError;
}

int RKSetMomentProcessorToPulsePair(RKRadar *radar) {
	radar->momentEngine->processor = &RKPulsePair;
	radar->momentEngine->processorLagCount = 3;
	return RKResultNoError;
}

int RKSetMomentProcessorToPulsePairHop(RKRadar *radar) {
	radar->momentEngine->processor = &RKPulsePairHop;
	radar->momentEngine->processorLagCount = 2;
	return RKResultNoError;
}

int RKSetMomentProcessorRKPulsePairStaggeredPRT(RKRadar *radar) {
	radar->momentEngine->processorLagCount = 2;
	return RKResultNoError;
}

//
// Sets the waveform from a pre-defined file that specifies the digital samples for an
// arbitrary waveform generator.
//
int RKSetWaveformByFilename(RKRadar *radar, const char *filename) {
    if (radar->pulseCompressionEngine == NULL) {
        RKLog("Error. No pulse compression engine.\n");
        return RKResultNoPulseCompressionEngine;
    }
    // Load in the waveform
    RKWaveform *waveform = RKWaveformInitFromFile(filename);
    return RKSetWaveform(radar, waveform);
}

int RKSetWaveformToImpulse(RKRadar *radar) {
    if (radar->pulseCompressionEngine == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    return RKPulseCompressionSetFilterToImpulse(radar->pulseCompressionEngine);
}

int RKSetWaveformTo121(RKRadar *radar) {
    if (radar->pulseCompressionEngine == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    return RKPulseCompressionSetFilterTo121(radar->pulseCompressionEngine);
}

int RKSetProcessingCoreCounts(RKRadar *radar,
                              const unsigned int pulseCompressionCoreCount,
                              const unsigned int momentProcessorCoreCount) {
    if (radar->pulseCompressionEngine == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    if (radar->state & RKRadarStateLive) {
        return RKResultUnableToChangeCoreCounts;
    }
    RKPulseCompressionEngineSetCoreCount(radar->pulseCompressionEngine, pulseCompressionCoreCount);
    RKMomentEngineSetCoreCount(radar->momentEngine, momentProcessorCoreCount);
    return RKResultNoError;
}

int RKSetPRF(RKRadar *radar, const uint32_t prf) {
    RKAddConfig(radar, RKConfigKeyPRF, prf, RKConfigKeyNull);
    return RKResultNoError;
}

uint32_t RKGetPulseCapacity(RKRadar *radar) {
    if (radar->pulses == NULL) {
        return RKResultNoPulseCompressionEngine;
    }
    RKPulse *pulse = RKGetPulse(radar->pulses, 0);
    return pulse->header.capacity;
}

void RKSetPulseTicsPerSeconds(RKRadar *radar, const double delta) {
    RKClockSetDxDu(radar->pulseClock, 1.0 / delta);
}

void RKSetPositionTicsPerSeconds(RKRadar *radar, const double delta) {
    RKClockSetDxDu(radar->positionClock, 1.0 / delta);
}

void RKAddControl(RKRadar *radar, const char *label, const char *command) {
    uint8_t index = radar->controlIndex++;
    if (index >= radar->desc.controlCapacity) {
        RKLog("Cannot add anymore controls.\n");
        return;
    }
    RKUpdateControl(radar, index, label, command);
}

void RKUpdateControl(RKRadar *radar, uint8_t index, const char *label, const char *command) {
    if (index >= radar->desc.controlCapacity) {
        RKLog("Error. Unable to update control.\n");
        return;
    }
    RKControl *control = &radar->controls[index];
    strncpy(control->label, label, RKNameLength - 1);
    strncpy(control->command, command, RKMaximumStringLength - 1);
}

void RKClearControls(RKRadar *radar) {
	radar->controlIndex = 0;
}

void RKConcludeControls(RKRadar *radar) {
	radar->controlSetIndex++;
}

#pragma mark - Developer Access

void RKGetRegisterValue(RKRadar *radar, void *value, const unsigned long registerOffset, size_t size) {
    memcpy(value, (void *)radar + registerOffset, size);
}

void RKSetRegisterValue(RKRadar *radar, void *value, const unsigned long registerOffset, size_t size) {
    memcpy((void *)radar + registerOffset, value, size);
}

void RKShowOffsets(RKRadar *radar) {
    printf("radar->active       @ %ld\n", (unsigned long)((void *)radar - (void *)&radar->active));
    printf("radar->configIndex  @ %ld\n", (unsigned long)((void *)radar - (void *)&radar->configIndex));
    printf("radar->pulseCompressionEngine  @ %ld\n", (unsigned long)((void *)radar - (void *)radar->pulseCompressionEngine));
    printf("radar->dataRecorder->doNotWrite  @ %ld\n", (unsigned long)((void *)radar - (void *)radar->dataRecorder->doNotWrite));
}

#pragma mark - Interaction / State Change

//
// Set the radar to go live. All the engines will be ignited with this function, going into infinite
// loops waiting for incoming samples or events, which come from network connections.
// Input:
//     RKRadar *radar - object of the radar
// Output:
//     Always RKResultSuccess
//
int RKGoLive(RKRadar *radar) {
    radar->active = true;

    // Offset the pre-allocated memory
    if (radar->desc.initFlags & RKInitFlagSignalProcessor) {
        radar->memoryUsage -= radar->pulseCompressionEngine->memoryUsage;
        radar->memoryUsage -= radar->positionEngine->memoryUsage;
        radar->memoryUsage -= radar->momentEngine->memoryUsage;
        radar->memoryUsage -= radar->healthEngine->memoryUsage;
    }
    radar->memoryUsage -= radar->healthLogger->memoryUsage;
    radar->memoryUsage -= radar->dataRecorder->memoryUsage;
    radar->memoryUsage -= radar->sweepEngine->memoryUsage;
    radar->memoryUsage -= radar->fileManager->memoryUsage;
    
    // Start the engines
    RKFileManagerStart(radar->fileManager);
    if (radar->desc.initFlags & RKInitFlagSignalProcessor) {
        // Main thread uses 1 CPU. Start the others from 1.
        uint8_t o = 1;
        if (o + radar->pulseCompressionEngine->coreCount + radar->momentEngine->coreCount > radar->processorCount) {
            RKLog("Info. Not enough physical cores. Core counts will be adjusted.\n");
            RKPulseCompressionEngineSetCoreCount(radar->pulseCompressionEngine, radar->processorCount / 2);
            RKMomentEngineSetCoreCount(radar->momentEngine, radar->processorCount / 2 - 1);
        }
        RKPulseCompressionEngineSetCoreOrigin(radar->pulseCompressionEngine, o);
        RKMomentEngineSetCoreOrigin(radar->momentEngine, o + radar->pulseCompressionEngine->coreCount);
        // Now, we start the engines
        RKPulseCompressionEngineStart(radar->pulseCompressionEngine);
        RKPositionEngineStart(radar->positionEngine);
        RKMomentEngineStart(radar->momentEngine);
        RKHealthEngineStart(radar->healthEngine);
    } else {
        RKRadarRelayStart(radar->radarRelay);
    }
    RKHealthLoggerStart(radar->healthLogger);
    RKDataRecorderStart(radar->dataRecorder);
    RKSweepEngineStart(radar->sweepEngine);

    // Get the post-allocated memory
    if (radar->desc.initFlags & RKInitFlagSignalProcessor) {
        radar->memoryUsage += radar->pulseCompressionEngine->memoryUsage;
        radar->memoryUsage += radar->positionEngine->memoryUsage;
        radar->memoryUsage += radar->momentEngine->memoryUsage;
        radar->memoryUsage += radar->healthEngine->memoryUsage;
    }
    radar->memoryUsage += radar->healthLogger->memoryUsage;
    radar->memoryUsage += radar->dataRecorder->memoryUsage;
    radar->memoryUsage += radar->sweepEngine->memoryUsage;
    radar->memoryUsage += radar->fileManager->memoryUsage;

    // Show the udpated memory usage
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("Radar live. All data buffers occupy \033[4m%s B\033[24m (%s GiB)\n",
              RKIntegerToCommaStyleString(radar->memoryUsage),
              RKFloatToCommaStyleString((double)radar->memoryUsage / 1073741824.0));
    }

    // Add a dummy config to get things started if there hasn't been one
    if (radar->configIndex == 0) {
        RKAddConfig(radar,
                    RKConfigKeyZCal, -47.0, -47.0,
                    RKConfigKeyDCal, -1.0,
                    RKConfigKeyNoise, 1.0, 1.0,
                    RKConfigKeyNull);
    }

    // Pedestal
    if (radar->pedestalInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
            RKLog("Initializing pedestal ...");
        }
        if (radar->pedestalFree == NULL || radar->pedestalExec == NULL) {
            RKLog("Error. Pedestal incomplete.");
			RKStop(radar);
			return RKResultIncompletePedestal;
        }
        radar->pedestal = radar->pedestalInit(radar, radar->pedestalInitInput);
		if (radar->pedestal == NULL) {
			RKStop(radar);
			return RKResultFailedToStartPedestal;
		}
        radar->state |= RKRadarStatePedestalInitialized;
    }

    // Health Relay
    if (radar->healthRelayInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
            RKLog("Initializing health relay ...");
        }
        if (radar->healthRelayFree == NULL || radar->healthRelayExec == NULL) {
            RKLog("Error. Health relay incomplete.");
			RKStop(radar);
			return RKResultIncompleteHealthRelay;
        }
        radar->healthRelay = radar->healthRelayInit(radar, radar->healthRelayInitInput);
		if (radar->healthRelay == NULL) {
			RKStop(radar);
			return RKResultFailedToStartHealthRelay;
		}
        radar->state |= RKRadarStateHealthRelayInitialized;
    }
    
    // Transceiver
    if (radar->transceiverInit != NULL) {
        if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
            RKLog("Initializing transceiver ...");
        }
        if (radar->transceiverFree == NULL || radar->transceiverExec == NULL) {
            RKLog("Error. Transceiver incomplete.");
			RKStop(radar);
			return RKResultIncompleteTransceiver;
        }
        radar->transceiver = radar->transceiverInit(radar, radar->transceiverInitInput);
		if (radar->transceiver == NULL) {
			RKStop(radar);
			return RKResultFailedToStartTransceiver;
		}
        radar->state |= RKRadarStateTransceiverInitialized;
    }

    // For now, the transceiver is the master controller
    radar->masterController = radar->transceiver;
    radar->masterControllerExec = radar->transceiverExec;
    
    // Update memory usage
    if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
        RKLog("Radar Live. Memory usage = %s B (%s GiB)\n",
              RKIntegerToCommaStyleString(radar->memoryUsage),
              RKFloatToCommaStyleString((double)radar->memoryUsage / 1073741824.0));
    }

    radar->state |= RKRadarStateLive;
    return RKResultSuccess;
}

//
// Wait indefinitely until the radar engines are killed
// Input:
//     RKRadar *radar - the radar object of the system
// Output:
//     None
//
int RKWaitWhileActive(RKRadar *radar) {
    int k;
    int s = 0;
    uint32_t pulseIndex = radar->pulseIndex;
    uint32_t positionIndex = radar->positionIndex;
    uint32_t healthIndex = radar->desc.initFlags & RKInitFlagSignalProcessor ? radar->healthNodes[RKHealthNodeTweeta].index : 0;
    bool transceiverOkay;
    bool pedestalOkay;
    bool healthOkay;

    while (radar->active) {
        if (radar->desc.initFlags & RKInitFlagSignalProcessor) {
            if (s++ == 3) {
                s = 0;
                // General Health
                transceiverOkay = pulseIndex == radar->pulseIndex ? false : true;
                pedestalOkay = positionIndex == radar->positionIndex ? false : true;
                healthOkay = healthIndex == radar->healthNodes[RKHealthNodeTweeta].index ? false : true;

                RKConfig *config = RKGetLatestConfig(radar);
                RKHealth *health = RKGetVacantHealth(radar, RKHealthNodeRadarKit);
                sprintf(health->string, "{"
                        "\"Transceiver\":{\"Value\":%s,\"Enum\":%d},"
                        "\"Pedestal\":{\"Value\":%s,\"Enum\":%d},"
                        "\"Health Relay\":{\"Value\":%s,\"Enum\":%d},"
                        "\"Network\":{\"Value\":true,\"Enum\":0},"
                        "\"Recorder\":{\"Value\":%s,\"Enum\":%d},"
                        "\"Noise\":[%.3f,%.3f],"
                        "\"DFTPlanUsage\":[%d,%d,%d,%d,%d]"
                        "}",
                        transceiverOkay ? "true" : "false", transceiverOkay ? RKStatusEnumNormal : RKStatusEnumFault,
                        pedestalOkay ? "true" : "false", pedestalOkay ? RKStatusEnumNormal : RKStatusEnumFault,
                        healthOkay ? "true" : "false", healthOkay ? RKStatusEnumNormal : RKStatusEnumFault,
                        radar->dataRecorder->doNotWrite ? "false" : "true", radar->dataRecorder->doNotWrite ? RKStatusEnumStandby: RKStatusEnumNormal,
                        config->noise[0], config->noise[1],
                        radar->pulseCompressionEngine->planUseCount[0],
                        radar->pulseCompressionEngine->planUseCount[1],
                        radar->pulseCompressionEngine->planUseCount[2],
                        radar->pulseCompressionEngine->planUseCount[3],
                        radar->pulseCompressionEngine->planUseCount[4]
                        );
                RKSetHealthReady(radar, health);

                //printf("radarkitnode %d\n", radar->healthNodes[RKHealthNodeRadarKit].index);
                
                pulseIndex = radar->pulseIndex;
                positionIndex = radar->positionIndex;
                healthIndex = radar->healthNodes[RKHealthNodeTweeta].index;
            }
            // Put together a system status
            RKStatus *status = RKGetVacantStatus(radar);
            status->pulseMonitorLag = radar->pulseCompressionEngine->lag * 100 / radar->desc.pulseBufferDepth;
            for (k = 0; k < MIN(RKProcessorStatusPulseCoreCount, radar->pulseCompressionEngine->coreCount); k++) {
                status->pulseCoreLags[k] = (uint8_t)(99.5f * radar->pulseCompressionEngine->workers[k].lag);
                status->pulseCoreUsage[k] = (uint8_t)(99.5 * radar->pulseCompressionEngine->workers[k].dutyCycle);
            }
            status->rayMonitorLag = radar->momentEngine->lag * 100 / radar->desc.rayBufferDepth;
            for (k = 0; k < MIN(RKProcessorStatusRayCoreCount, radar->momentEngine->coreCount); k++) {
                status->rayCoreLags[k] = (uint8_t)(99.5f * radar->momentEngine->workers[k].lag);
                status->rayCoreUsage[k] = (uint8_t)(99.5 * radar->momentEngine->workers[k].dutyCycle);
            }
            status->recorderLag = radar->dataRecorder->lag;
            RKSetStatusReady(radar, status);
        }
        usleep(100000);
    }
    return RKResultSuccess;
}

//
// Stops the engines. The radar engines will all be killed.
// Input:
//     RKRadar object of the radar
// Ouput:
//     Always RKResultSuccess
//
int RKStop(RKRadar *radar) {
    if (radar->active == false) {
        return RKResultEngineDeactivatedMultipleTimes;
    }
    radar->active = false;
    if (radar->state & RKRadarStatePedestalInitialized) {
        if (radar->pedestalExec != NULL) {
            radar->pedestalExec(radar->pedestal, "disconnect", radar->pedestalResponse);
        }
        radar->state ^= RKRadarStatePedestalInitialized;
    }
    if (radar->state & RKRadarStateTransceiverInitialized) {
        if (radar->transceiverExec != NULL) {
            radar->transceiverExec(radar->transceiver, "disconnect", radar->transceiverResponse);
        }
        radar->state ^= RKRadarStateTransceiverInitialized;
    }
    if (radar->state & RKRadarStateHealthRelayInitialized) {
        if (radar->healthRelayExec != NULL) {
            radar->healthRelayExec(radar->healthRelay, "disconnect", radar->healthRelayResponse);
        }
        radar->state ^= RKRadarStateHealthRelayInitialized;
    }
    if (radar->state & RKRadarStateFileManagerInitialized) {
        RKFileManagerStop(radar->fileManager);
        radar->state ^= RKRadarStateFileManagerInitialized;
    }
    if (radar->state & RKRadarStatePulseCompressionEngineInitialized) {
        RKPulseCompressionEngineStop(radar->pulseCompressionEngine);
        radar->state ^= RKRadarStatePulseCompressionEngineInitialized;
    }
    if (radar->state & RKRadarStatePositionEngineInitialized) {
        RKPositionEngineStop(radar->positionEngine);
        radar->state ^= RKRadarStatePositionEngineInitialized;
    }
    if (radar->state & RKRadarStateMomentEngineInitialized) {
        RKMomentEngineStop(radar->momentEngine);
        radar->state ^= RKRadarStateMomentEngineInitialized;
    }
    if (radar->state & RKRadarStateHealthEngineInitialized) {
        RKHealthEngineStop(radar->healthEngine);
        radar->state ^= RKRadarStateHealthEngineInitialized;
    }
	if (radar->state & RKRadarStateHealthLoggerInitialized) {
		RKHealthLoggerStop(radar->healthLogger);
		radar->state ^= RKRadarStateHealthLoggerInitialized;
	}
    if (radar->state & RKRadarStateRadarRelayInitialized) {
        RKRadarRelayStop(radar->radarRelay);
        radar->state ^= RKRadarStateRadarRelayInitialized;
    }
    if (radar->state & RKRadarStateFileRecorderInitialized) {
        RKDataRecorderStop(radar->dataRecorder);
        radar->state ^= RKRadarStateFileRecorderInitialized;
    }
    if (radar->state & RKRadarStateSweepEngineInitialized) {
        RKSweepEngineStop(radar->sweepEngine);
        radar->state ^= RKRadarStateSweepEngineInitialized;
    }
    if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
        RKLog("radar->state = 0x%x\n", radar->state);
    }
    return RKResultSuccess;
}

int RKResetEngines(RKRadar *radar) {
    RKClockReset(radar->pulseClock);
    RKClockReset(radar->positionClock);
    return RKResultSuccess;
}

//
// Performs a function in the background.
// This function is intended for handling commands that may recurse to itself. Instead of causing a block
// call, this function make the functional call as a separate call eliminating the blocking behavior.
// Input:
//     RKRadar *radar - object of the radar
//     char *command - a string of the command
// Ouput:
//     None
//
void RKPerformMasterTaskInBackground(RKRadar *radar, const char *command) {
    if (radar->masterController == NULL) {
        RKLog("Master controller is not valid.\n");
        return;
    }
    RKTaskInput taskInput;
    taskInput.radar = radar;
    strncpy(taskInput.command, command, RKNameLength - 1);
    pthread_t tid;
    pthread_create(&tid, NULL, &masterControllerExecuteInBackground, &taskInput);
}

#pragma mark - General

//
// Measure noise from the latest 200 pulses
// Input:
//     RKRadar *radar - object of the radar
// Ouput:
//     None
//
void RKMeasureNoise(RKRadar *radar) {
	int k = 0;
    RKFloat noise[2];
    RKFloat noiseAverage[2] = {0.0f, 0.0f};
    uint32_t index = RKPreviousModuloS(radar->pulseIndex, radar->desc.pulseBufferDepth);
    RKPulse *pulse = RKGetPulse(radar->pulses, index);
    while (!(pulse->header.s & RKPulseStatusCompressed) && k++ < radar->desc.pulseBufferDepth) {
        index = RKPreviousModuloS(index, radar->desc.pulseBufferDepth);
        pulse = RKGetPulse(radar->pulses, index);
    }
	// Avoid data before this offset to exclude the transcient effect right after transmit pulse
	int origin = 0;
	for (k = 0; k < radar->pulseCompressionEngine->filterCounts[0]; k++) {
		origin += radar->pulseCompressionEngine->filterAnchors[0][k].length;
	}
	origin *= 2;
    for (k = 0; k < RKPulseCountForNoiseMeasurement; k++) {
        index = RKPreviousModuloS(index, radar->desc.pulseBufferDepth);
        pulse = RKGetPulse(radar->pulses, index);
        RKMeasureNoiseFromPulse(noise, pulse, origin);
        noiseAverage[0] += noise[0];
        noiseAverage[1] += noise[1];
    }
    noiseAverage[0] /= (RKFloat)k;
    noiseAverage[1] /= (RKFloat)k;
	if (!isfinite(noiseAverage[0])) {
		noiseAverage[0] = 0.001f;
	}
	if (!isfinite(noiseAverage[1])) {
		noiseAverage[1] = 0.001f;
	}
    RKAddConfig(radar, RKConfigKeyNoise, noiseAverage[0], noiseAverage[1], RKConfigKeyNull);
}

void RKSetSNRThreshold(RKRadar *radar, const RKFloat threshold) {
    RKAddConfig(radar, RKConfigKeySNRThreshold, threshold, RKConfigKeyNull);
}

#pragma mark - Status

//
// Get a vacant slot to fill in system status
// This method is usually used by the RadarKit internally to report
// system status.
// Input:
//     RKRadar *radar - object of the radar
// Output:
//     None
//
RKStatus *RKGetVacantStatus(RKRadar *radar) {
    RKStatus *status = &radar->status[radar->statusIndex];
    status->i += radar->desc.statusBufferDepth;
    if (status->flag != RKStatusFlagVacant) {
        RKLog("Error. radar->status[%d] should be vacant.\n", radar->statusIndex);
        status->flag = RKStatusFlagVacant;
    }
    radar->statusIndex = RKNextModuloS(radar->statusIndex, radar->desc.statusBufferDepth);
    radar->status[radar->statusIndex].flag = RKStatusFlagVacant;
    return status;
}

void RKSetStatusReady(RKRadar *radar, RKStatus *status) {
    status->flag |= RKStatusFlagReady;
}

#pragma mark - Configs

//
// Add a configuration to change the operational setting.
// This is an internally used function, shouldn't be used by
// user program.
// Input:
//     RKConfigKey key - the key that describes what comes next
//     value(s) - values of the key
// Output:
//     None
// Note:
//     The last key must be RKConfigKeyNull
// Example:
//     RKConfigAdd(radar, RKConfigKeyPRF, 1000, RKConfigNull) to set PRF
//     RKConfigAdd(radar, RKConfigKeyNoise, 0.3, 0.2, RKConfigNull) to set noise
//
// Users normally don't have to deal with these
//
void RKAddConfig(RKRadar *radar, ...) {
    va_list args;
    va_start(args, radar);
    if (radar->desc.initFlags & RKInitFlagVeryVerbose) {
        RKLog("RKAddConfig() ...\n");
    }
    return RKConfigAdvance(radar->configs, &radar->configIndex, radar->desc.configBufferDepth, args);
}

RKConfig *RKGetLatestConfig(RKRadar *radar) {
    return &radar->configs[radar->configIndex];
}

#pragma mark - Healths

//
// Request a health reporting node.
// Input:
//     RKRadar radar - the radar
// Output:
//     An RKHealthNode to identifity that node
//
RKHealthNode RKRequestHealthNode(RKRadar *radar) {
    RKHealthNode node = RKHealthNodeUser1 + radar->healthNodeCount;
    if (node == RKHealthNodeCount) {
        RKLog("Error. No more health node available.\n");
        node = (RKHealthNode)-1;
    }
    radar->healthNodeCount++;
    return node;
}

//
// Get a health slot for a specific node.
// Input:
//     RKRadar radar - the radar
//     RKHealthNode node - the node to report the health later
// Output
//     A vacant slot to fill in health information
//
RKHealth *RKGetVacantHealth(RKRadar *radar, const RKHealthNode node) {
    if (radar->healthEngine == NULL) {
        RKLog("Error. Health engine has not started.\n");
        //exit(EXIT_FAILURE);
        return NULL;
    }
    radar->healthNodes[node].active = true;
    uint32_t index = radar->healthNodes[node].index;
    RKHealth *health = &radar->healthNodes[node].healths[index];
    health->i += radar->desc.healthBufferDepth;
    index = RKNextModuloS(index, radar->desc.healthBufferDepth);
    radar->healthNodes[node].healths[index].flag = RKHealthFlagVacant;
    radar->healthNodes[node].healths[index].string[0] = '\0';
    radar->healthNodes[node].index = index;
    return health;
}

void RKSetHealthReady(RKRadar *radar, RKHealth *health) {
    gettimeofday(&health->time, NULL);
    health->timeDouble = (double)health->time.tv_sec + 1.0e-6 * (double)health->time.tv_usec;
    health->flag = RKHealthFlagReady;
}

RKHealth *RKGetLatestHealth(RKRadar *radar) {
    uint32_t index = RKPreviousModuloS(radar->healthIndex, radar->desc.healthBufferDepth);
    return &radar->healths[index];
}

int RKGetEnumFromLatestHealth(RKRadar *radar, const char *keyword) {
    RKHealth *health = RKGetLatestHealth(radar);
    char *stringObject = RKGetValueOfKey(health->string, keyword);
    if (stringObject) {
        char *stringEnum = RKGetValueOfKey(stringObject, "enum");
        if (stringEnum) {
            return atoi(stringEnum);
        }
    }
    return RKStatusEnumInvalid;
}

#pragma mark - Positions

//
// Get a vacant slot to fill in position data from the pedestal.
// Input:
//     RKRadar *radar - object of the radar
// Output:
//     None
//
RKPosition *RKGetVacantPosition(RKRadar *radar) {
    if (radar->positionEngine == NULL) {
        RKLog("Error. Pedestal engine has not started.\n");
        return NULL;
    }
    RKPosition *position = &radar->positions[radar->positionIndex];
    position->i += radar->desc.positionBufferDepth;
    position->flag = RKPositionFlagVacant;
    return position;
}

void RKSetPositionReady(RKRadar *radar, RKPosition *position) {
    if (position->flag & ~RKPositionFlagHardwareMask) {
        RKLog("Error. Ingested a position with a flag (0x%08x) outside of allowable value.\n", position->flag);
    }
    position->timeDouble = RKClockGetTime(radar->positionClock, (double)position->tic, &position->time);
	if ((radar->desc.initFlags & RKInitFlagShowClockOffset) && (position->tic % 5 == 0)) {
		struct timeval t;
		gettimeofday(&t, NULL);
        printf("\033[38;5;222mposition\033[0m @ %+14.4f %+14.4f            x0 = %10.4f  u0 = %10.0f  dx/du = %.3e  k = %u\n", position->timeDouble,
			   position->timeDouble - ((double)t.tv_sec + 1.0e-6 * (double)t.tv_usec - radar->positionClock->initDay),
			   radar->positionClock->x0, radar->positionClock->u0, radar->positionClock->dx, radar->positionClock->index);
	}
    position->flag |= RKPositionFlagReady;
    radar->positionIndex = RKNextModuloS(radar->positionIndex, radar->desc.positionBufferDepth);
    return;
}

RKPosition *RKGetLatestPosition(RKRadar *radar) {
    uint32_t index = RKPreviousModuloS(radar->positionIndex, radar->desc.positionBufferDepth);
    return &radar->positions[index];
}

float RKGetPositionUpdateRate(RKRadar *radar) {
	uint32_t n = radar->desc.positionBufferDepth / 2;
	uint32_t i = RKPreviousModuloS(radar->positionIndex, radar->desc.positionBufferDepth);
	uint32_t o = RKPreviousNModuloS(radar->positionIndex, n, radar->desc.positionBufferDepth);
	return (float)n / (radar->positions[i].timeDouble - radar->positions[o].timeDouble);
}

#pragma mark - Pulses

//
// Get a vacant slot to fill in pulse data from the digital transceiver.
// Input:
//     RKRadar *radar - object of the radar
// Output:
//     None
//
RKPulse *RKGetVacantPulse(RKRadar *radar) {
    if (radar->pulses == NULL) {
        RKLog("Error. Buffer for raw pulses has not been allocated.\n");
        exit(EXIT_FAILURE);
    }
    RKPulse *pulse = RKGetPulse(radar->pulses, radar->pulseIndex);
    pulse->header.s = RKPulseStatusVacant;
    pulse->header.i += radar->desc.pulseBufferDepth;
    pulse->header.timeDouble = 0.0;
    pulse->header.time.tv_sec = 0;
    pulse->header.time.tv_usec = 0;
    pulse->header.configIndex = radar->configIndex;
    radar->pulseIndex = RKNextModuloS(radar->pulseIndex, radar->desc.pulseBufferDepth);
    return pulse;
}

void RKSetPulseHasData(RKRadar *radar, RKPulse *pulse) {
    if (pulse->header.i == 0) {
        RKClockReset(radar->pulseClock);
    }
    if (pulse->header.timeDouble == 0.0 && pulse->header.time.tv_sec == 0) {
        pulse->header.timeDouble = RKClockGetTime(radar->pulseClock, (double)pulse->header.t, &pulse->header.time);
    }
    if (pulse->header.gateCount > pulse->header.capacity) {
        RKLog("Error. gateCount = %s > capacity = %s\n",
              RKIntegerToCommaStyleString(pulse->header.gateCount), RKIntegerToCommaStyleString(pulse->header.capacity));
        pulse->header.gateCount = pulse->header.capacity;
    }
	if ((radar->desc.initFlags & RKInitFlagShowClockOffset) && (pulse->header.i % 100 == 0)) {
		struct timeval t;
		gettimeofday(&t, NULL);
		printf("           %+14.4f %+14.4f @ \033[38;5;118mpulse\033[0m    x0 = %10.4f  u0 = %10.0f  dx/du = %.3e\n", pulse->header.timeDouble,
			   pulse->header.timeDouble - ((double)t.tv_sec + 1.0e-6 * (double)t.tv_usec - radar->pulseClock->initDay),
			   radar->pulseClock->x0, radar->pulseClock->u0, radar->pulseClock->dx);
	}
    pulse->header.s = RKPulseStatusHasIQData;
    return;
}

void RKSetPulseReady(RKRadar *radar, RKPulse *pulse) {
    pulse->header.s = RKPulseStatusHasIQData | RKPulseStatusHasPosition;
}

RKPulse *RKGetLatestPulse(RKRadar *radar) {
    uint32_t index = RKPreviousModuloS(radar->pulseIndex, radar->desc.pulseBufferDepth);
    RKPulse *pulse = RKGetPulse(radar->pulses, index);
    int k = 0;
    while (!(pulse->header.s & RKPulseStatusCompressed) && k++ < radar->desc.pulseBufferDepth) {
        index = RKPreviousModuloS(index, radar->desc.pulseBufferDepth);
        pulse = RKGetPulse(radar->pulses, index);
    }
    return pulse;
}

#pragma mark - Rays

//
// Get a vacant slot to fill in ray data.
// MomentEngine doesn't rely on this function.
// This function may be used for a relay to fill the buffer.
// Input:
//     RKRadar *radar - object of the radar
// Output:
//     None
//
RKRay *RKGetVacanRay(RKRadar *radar) {
    if (radar->rays == NULL) {
        RKLog("Error. Buffer for rays has not been allocated.\n");
        exit(EXIT_FAILURE);
    }
    RKRay *ray = RKGetRay(radar->rays, radar->rayIndex);
    ray->header.s = RKRayStatusVacant;
    ray->header.i += radar->desc.rayBufferDepth;
    ray->header.startTime.tv_sec = 0;
    ray->header.startTime.tv_usec = 0;
    ray->header.endTime.tv_sec = 0;
    ray->header.endTime.tv_usec = 0;
    radar->rayIndex = RKNextModuloS(radar->rayIndex, radar->desc.rayBufferDepth);
    return ray;
}

void RKSetRayReady(RKRadar *radar, RKRay *ray) {
    ray->header.s |= RKRayStatusReady;
}
