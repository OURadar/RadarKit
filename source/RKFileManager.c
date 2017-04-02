//
//  RKFileManager.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKFileManager.h>

#define RKFileManagerCapacity  10000

typedef char RKFilename[RKNameLength];
typedef struct _rk_indexed_stat {
    int      index;
    time_t   time;
    size_t   size;
} RKIndexedStat;

#pragma mark - Helper Functions

static int struct_cmp_by_time(const void *a, const void *b) {
    RKIndexedStat *x = (RKIndexedStat *)a;
    RKIndexedStat *y = (RKIndexedStat *)b;
    return (int)(x->time - y->time);
}

#pragma mark - Delegate Workers

static void *fileWatcher(void *in) {
    RKFileManager *engine = (RKFileManager *)in;
    
    int k = 0;
    
    //struct stat st;
    struct dirent *dir;
    char path[RKMaximumPathLength];
    path[RKMaximumPathLength - 1] = '\0';
    
    RKLog("%s Started.   mem = %s B  state = %x\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), engine->state);
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    // Raw data path
    if (engine->radarDescription != NULL && strlen(engine->radarDescription->dataPath)) {
        snprintf(path, RKMaximumPathLength - 1, "%s/iq", engine->radarDescription->dataPath);
    } else if (strlen(engine->dataPath)) {
        snprintf(path, RKMaximumPathLength - 1, "%s/iq", engine->dataPath);
    } else {
        snprintf(path, RKMaximumPathLength - 1, "iq");
    }
    
    RKLog("%s Monitoring path '%s'\n", engine->name, path);
    DIR *did = opendir(path);
    if (did == NULL) {
        RKLog("%s Error. Unable to access director '%s'\n", engine->name, path);
        return (void *)1;
    }
    
    RKFilename *filenames = (RKFilename *)malloc(RKFileManagerCapacity * sizeof(RKFilename));
    if (filenames == NULL) {
        RKLog("%s Error. unable to allocate space for filenames.\n", engine->name);
    }
    memset(filenames, 0, RKFileManagerCapacity * sizeof(RKFilename));
    RKIndexedStat *indexedStats = (RKIndexedStat *)malloc(RKFileManagerCapacity * sizeof(RKIndexedStat));
    if (indexedStats == NULL) {
        RKLog("%s Error. unable to allocate space for indexed stats.\n", engine->name);
    }
    memset(indexedStats, 0, RKFileManagerCapacity * sizeof(RKIndexedStat));
    
    // List all the files. Keep a copy of the list
    k = 0;
    struct stat fileStat;
    size_t totalUsage = 0;
    while ((dir = readdir(did)) != NULL) {
        if (dir->d_type == DT_REG) {
            sprintf(filenames[k], "%s/%s", path, dir->d_name);
            stat(filenames[k], &fileStat);
            // Get creation time
            indexedStats[k].index = k;
            indexedStats[k].time = fileStat.st_birthtimespec.tv_sec;
            indexedStats[k].size = fileStat.st_size;
            totalUsage += fileStat.st_size;
            printf("%s %12s B  %ld\n", filenames[k], RKIntegerToCommaStyleString(indexedStats[k].size), indexedStats[k].time);
            k++;
        }
    }
    printf("Total usage: %s B\n", RKIntegerToCommaStyleString(totalUsage));
    
    qsort(indexedStats, k, sizeof(RKIndexedStat), struct_cmp_by_time);
    
    while (k > 0) {
        k--;
        printf("%d %zu -> %s\n", indexedStats[k].index, indexedStats[k].time, filenames[indexedStats[k].index]);
    }
    
    while (engine->state & RKEngineStateActive) {
        k = 0;
        while (k++ < 10 && engine->state & RKEngineStateActive) {
            usleep(100000);
        }
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

void RKFileManagerSetInputOutputBuffer(RKFileManager *engine, RKRadarDesc *desc) {
    engine->radarDescription = desc;
}

void RKFileManagerSetPathToMonitor(RKFileManager *engine, const char *path) {
    strncpy(engine->dataPath, path, RKMaximumPathLength - 1);
}

#pragma mark - Interactions

int RKFileManagerStart(RKFileManager *engine) {
    // File manager is always assumed wired
    engine->state |= RKEngineStateProperlyWired;
    if (engine->rawDataUsagelimit == 0) {
        engine->rawDataUsagelimit = RKFileManagerDefaultUsageLimit;
        RKLog("%s Usage limit not set.  Use default = %s GB\n", engine->name, RKIntegerToCommaStyleString(engine->rawDataUsagelimit / 1073741824));
    }
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidFileWatcher, NULL, fileWatcher, engine) != 0) {
        RKLog("Error. Failed to start a ray gatherer.\n");
        return RKResultFailedToStartFileManager;
    }
    while (!(engine->state & RKEngineStateActive)) {
        usleep(10000);
    }
    return RKResultSuccess;
}

int RKFileManagerStop(RKFileManager *engine) {
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
    pthread_join(engine->tidFileWatcher, NULL);
    engine->state ^= RKEngineStateDeactivating;
    if (engine->verbose) {
        RKLog("%s Stopped.\n", engine->name);
    }
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->state);
    }
    return RKResultSuccess;
}
