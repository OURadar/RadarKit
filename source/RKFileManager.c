//
//  RKFileManager.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKFileManager.h>

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

static void *fileWatcher(void *in) {
    RKFileManager *engine = (RKFileManager *)in;
    
    int k = 0;
    
    RKLog("%s Started.   mem = %s B\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage));
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    struct stat st;
    struct dirent *dir;
    char path[RKMaximumPathLength];
    path[RKMaximumPathLength - 1] = '\0';

    // Raw data path
    if (strlen(engine->radarDescription->dataPath)) {
        snprintf(path, RKMaximumPathLength - 1, "%s/iq", engine->radarDescription->dataPath);
    } else {
        snprintf(path, RKMaximumPathLength - 1, "iq");
    }
    DIR *did = opendir(path);
    
    // List all the files. Keep a copy of the list
    while ((dir = readdir(did)) != NULL) {
        if (dir->d_type == DT_REG) {
            stat(dir->d_name, &st);
            printf("%s %d %lld\n", dir->d_name, dir->d_type, st.st_size);
        }
    }

    while (engine->state & RKEngineStateActive) {
//        if ((dir = readdir(did)) != NULL) {
//            stat(dir->d_name, &st);
//            printf("%s %d %lld\n", dir->d_name, dir->d_type, st.st_size);
//        }
        sleep(1);
        k++;
    }

    return NULL;
}

#pragma mark - Life Cycle

RKFileManager *RKFileManagerInit() {
    RKFileManager *engine = (RKFileManager *)malloc(sizeof(RKFileManager));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a file manager.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKFileManager));
    sprintf(engine->name, "%s<DataFileManager>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(2) : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->memoryUsage = sizeof(RKFileManager);
    return engine;
}

void RKFileManagerFree(RKFileManager *engine) {
    if (engine->state & RKEngineStateActive) {
        RKFileManagerStop(engine);
    }
    free(engine);
}

#pragma mark - Properties

void RKFileManagerSetVerbose(RKFileManager *engine, const int verbose) {
    engine->verbose = verbose;
}

void RKFileManagerSSetInputOutputBuffer(RKFileManager *engine, RKRadarDesc *desc) {
    engine->radarDescription = desc;
    engine->state |= RKEngineStateProperlyWired;
}

#pragma mark - Interactions

int RKFileManagerStart(RKFileManager *engine) {
    if (!(engine->state & RKEngineStateProperlyWired)) {
        RKLog("%s Error. Not properly wired.\n", engine->name);
        return RKResultEngineNotWired;
    }
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidFileWatcher, NULL, fileWatcher, engine) != 0) {
        RKLog("Error. Failed to start a ray gatherer.\n");
        return RKResultFailedToStartRayGatherer;
    }
    while (!(engine->state & RKEngineStateActive)) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKFileManagerStop(RKFileManager *engine) {
    if (engine->state & RKEngineStateDeactivating) {
        if (engine->verbose > 1) {
            RKLog("%s Info. Engine is being or has been deactivated.\n", engine->name);
        }
        return RKResultEngineDeactivatedMultipleTimes;
    }
    if (engine->verbose) {
        RKLog("%s Stopping ...\n", engine->name);
    }
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateActive;
    pthread_join(engine->tidFileWatcher, NULL);
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    engine->state = RKEngineStateAllocated;
    return RKResultSuccess;
}
