//
//  RKHostMonitor.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/24/18.
//  Copyright © Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKHostMonitor.h>

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

#pragma mark - Life Cycle

RKHostMonitor *RKHostMonitorInit(void) {
    RKHostMonitor *engine = (RKHostMonitor *)malloc(sizeof(RKHostMonitor));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a host monitor.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKHostMonitor));
    sprintf(engine->name, "%s<NetworInspector>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHostMonitor) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->hosts = (RKUnitHost *)malloc(sizeof(RKUnitHost));
    memset(engine->hosts, 0, sizeof(RKUnitHost));
    pthread_mutex_init(&engine->mutex, NULL);
    RKHostMonitorAddHost(engine, "8.8.8.8");
    return engine;
}

void RKHostMonitorFree(RKHostMonitor *engine) {
    if (engine->state & RKEngineStateActive) {
        RKHostMonitorStop(engine);
    }
    pthread_mutex_destroy(&engine->mutex);
    free(engine);
}

#pragma mark - Properties

void RKHostMonitorSetVerbose(RKHostMonitor *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKHostMonitorAddHost(RKHostMonitor *engine, const char *address) {
    int k = engine->workerCount++;
    engine->hosts = realloc(engine->hosts, engine->workerCount * sizeof(RKUnitHost));
    strncpy(engine->hosts[k], address, RKNameLength - 1);
    engine->memoryUsage = sizeof(RKHostMonitor) + engine->workerCount * sizeof(RKUnitHost);
}

#pragma mark - Interactions

int RKHostMonitorStart(RKHostMonitor *engine) {
    RKLog("%s %d %s", engine->name, engine->workerCount, engine->hosts[0]);
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    return RKResultSuccess;
}

int RKHostMonitorStop(RKHostMonitor *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    // pthread_join ...
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}
