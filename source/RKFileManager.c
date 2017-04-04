//
//  RKFileManager.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKFileManager.h>

#define RKFileManagerFolderListCapacity  60
#define RKFileManagerFileListCapacity    100

typedef char RKPathname[RKNameLength];
typedef struct _rk_indexed_stat {
    int      index;
    time_t   time;
    size_t   size;
} RKIndexedStat;

#pragma mark - Helper Functions

// Compare two unix time in RKIndexedStat
static int struct_cmp_by_time(const void *a, const void *b) {
    RKIndexedStat *x = (RKIndexedStat *)a;
    RKIndexedStat *y = (RKIndexedStat *)b;
    return (int)(x->time - y->time);
}

// Compare two filenames in the pattern of YYYYMMDD, e.g., 20170402
static int string_cmp_by_pseudo_time(const void *a, const void *b) {
    return strncmp((char *)a, (char *)b, 8);
}

static int listElementsInFolder(RKPathname *list, const int maximumCapacity, const char *path, uint8_t type) {
    int k = 0;
    struct dirent *dir;
    DIR *did = opendir(path);
    if (did == NULL) {
        fprintf(stderr, "Unable to list folder '%s'\n", path);
        return -1;
    }
    while ((dir = readdir(did)) != NULL && k < maximumCapacity) {
        if (dir->d_type == type) {
            sprintf(list[k], "%s/%s", path, dir->d_name);
            k++;
        }
    }
    return k;
}

static int listFoldersInFolder(RKPathname *list, const int maximumCapacity, const char *path) {
    return listElementsInFolder(list, maximumCapacity, path, DT_DIR);
}

static int listFilesInFolder(RKPathname *list, const int maximumCapacity, const char *path) {
    return listElementsInFolder(list, maximumCapacity, path, DT_REG);
}

#pragma mark - Delegate Workers

static void refreshFileList(RKFileRemover *me) {
    int k;
    
    RKPathname *folders = (RKPathname *)me->folders;
    RKPathname *filenames = (RKPathname *)me->filenames;
    RKIndexedStat *indexedStats = (RKIndexedStat *)me->indexedStats;

    me->index = 0;
    me->count = 0;
    me->usage = 0;

    // Go through all folders
    int folderCount = listFoldersInFolder(folders, RKFileManagerFolderListCapacity, me->path);
    
    // Sort the folders by name (should be in time numbers)
    qsort(folders, folderCount, sizeof(RKPathname), string_cmp_by_pseudo_time);

    // Go through all files in the folders
    for (k = 0; k < folderCount && me->count < RKFileManagerFileListCapacity; k++) {
        me->count += listFilesInFolder(&filenames[me->count], RKFileManagerFileListCapacity - me->count, folders[k]);
    }
    
    // Gather the stats of the files
    struct stat fileStat;
    for (k = 0; k < me->count; k++) {
        stat(filenames[k], &fileStat);
        indexedStats[k].index = k;
        indexedStats[k].time = fileStat.st_ctime;
        indexedStats[k].size = fileStat.st_size;
        me->usage += fileStat.st_size;
    }

    // We can operate in a circular buffer fashion if all the files are accounted
    if (me->count < RKFileManagerFileListCapacity) {
        me->reusable = true;
    } else if (me->usage < me->limit) {
        RKLog("%s Warning. No files may be erased in '%s'.\n", me->parent->name, me->path);
    }

    // Sort the files by time
    qsort(indexedStats, me->count, sizeof(RKIndexedStat), struct_cmp_by_time);
}

static void *fileRemover(void *in) {
    RKFileRemover *me = (RKFileRemover *)in;
    RKFileManager *engine = me->parent;
    
    int k;

    const int c = me->id;
    
    char name[RKNameLength];
    char command[RKMaximumPathLength];
    struct timeval time = {0, 0};

    if (rkGlobalParameters.showColor) {
        pthread_mutex_lock(&engine->mutex);
        k = snprintf(name, RKNameLength - 1, "%s", rkGlobalParameters.showColor ? RKGetColor() : "");
        pthread_mutex_unlock(&engine->mutex);
    } else {
        k = 0;
    }
    if (engine->workerCount > 9) {
        k += sprintf(name + k, "F%02d", c);
    } else {
        k += sprintf(name + k, "F%d", c);
    }
    if (rkGlobalParameters.showColor) {
        sprintf(name + k, RKNoColor);
    }
    
    size_t mem = 0;
    RKPathname *folders = (RKPathname *)malloc(RKFileManagerFolderListCapacity * sizeof(RKPathname));
    if (folders == NULL) {
        RKLog("%s %s Error. Unable to allocate space for folder list.\n", engine->name, name);
    }
    memset(folders, 0, RKFileManagerFolderListCapacity * sizeof(RKPathname));
    mem += RKFileManagerFolderListCapacity * sizeof(RKPathname);
    RKPathname *filenames = (RKPathname *)malloc(RKFileManagerFileListCapacity * sizeof(RKPathname));
    if (filenames == NULL) {
        RKLog("%s %s Error. Unable to allocate space for file list.\n", engine->name, name);
    }
    memset(filenames, 0, RKFileManagerFileListCapacity * sizeof(RKPathname));
    mem += RKFileManagerFileListCapacity * sizeof(RKPathname);
    RKIndexedStat *indexedStats = (RKIndexedStat *)malloc(RKFileManagerFileListCapacity * sizeof(RKIndexedStat));
    if (indexedStats == NULL) {
        RKLog("%s %s Error. Unable to allocate space for indexed stats.\n", engine->name, name);
    }
    memset(indexedStats, 0, RKFileManagerFileListCapacity * sizeof(RKIndexedStat));
    mem += RKFileManagerFileListCapacity * sizeof(RKIndexedStat);
    
    me->folders = folders;
    me->filenames = filenames;
    me->indexedStats = indexedStats;
    
    pthread_mutex_lock(&engine->mutex);
    engine->memoryUsage += mem;
    
    // Gather the initial file list in the folders
    refreshFileList(me);

    RKLog(">%s %s Started.   mem = %s B   path = '%s'\n",
          engine->name, name, RKIntegerToCommaStyleString(mem), me->path);
    
    pthread_mutex_unlock(&engine->mutex);

    if (me->usage > 10 * 1024 * 1024) {
        RKLog("%s %s Listed.  count = %7s   usage = %7s / %7s MB\n", engine->name, name,
              RKIntegerToCommaStyleString(me->count), RKIntegerToCommaStyleString(me->usage / 1024 / 1024), RKIntegerToCommaStyleString(me->limit / 1024 / 1024));
    } else if (me->usage > 10 * 1024) {
        RKLog("%s %s Listed.  count = %7s   usage = %7s / %7s KB\n", engine->name, name,
              RKIntegerToCommaStyleString(me->count), RKIntegerToCommaStyleString(me->usage / 1024), RKIntegerToCommaStyleString(me->limit / 1024));
    } else {
        RKLog("%s %s Listed.  count = %7s   usage = %7s / %7s B\n", engine->name, name,
              RKIntegerToCommaStyleString(me->count), RKIntegerToCommaStyleString(me->usage), RKIntegerToCommaStyleString(me->limit));
    }

    // Just wait a little
    usleep(30000);
    
    me->index = 0;
    while (engine->state & RKEngineStateActive) {
        while (me->usage > me->limit && engine->state & RKEngineStateActive) {
            if (engine->verbose) {
                RKLog("%s %s Removing '%s' %d/%d", engine->name, name, filenames[indexedStats[me->index].index], me->index, indexedStats[me->index].index);
            }
            sprintf(command, "rm -f %s", filenames[indexedStats[me->index].index]);
            system(command);
            me->usage -= indexedStats[me->index].size;
            me->index++;
            if (me->index == RKFileManagerFileListCapacity) {
                me->index = 0;
                if (me->reusable) {
                    if (engine->verbose) {
                        RKLog("%s %s Reusable file list rotated to 0.\n", engine->name, name);
                    }
                } else {
                    if (engine->verbose) {
                        RKLog("%s %s Refreshing file list.\n", engine->name, name);
                    }
                    refreshFileList(me);
                }
            }
        }
        k = 0;
        while ((k++ < 10 || RKTimevalDiff(time, me->latestTime) < 0.3) && engine->state & RKEngineStateActive) {
            gettimeofday(&time, NULL);
            usleep(100000);
        }
    }
    
    free(filenames);
    free(indexedStats);
    
    RKLog(">%s %s Stopped.\n", engine->name, name);
    
    return NULL;
}

static void *folderWatcher(void *in) {
    RKFileManager *engine = (RKFileManager *)in;
    
    int k;
    
    RKLog("%s Started.   mem = %s B  state = %x\n", engine->name, RKIntegerToCommaStyleString(engine->memoryUsage), engine->state);
    
    engine->state |= RKEngineStateActive;
    engine->state ^= RKEngineStateActivating;
    
    // Three major data folders that have structure A
    char folders[][RKNameLength] = {
        RKDataFolderIQ,
        RKDataFolderMoment,
        RKDataFolderHealth
    };
    size_t limits[] = {
        RKFileManagerRawDataPercentage,
        RKFileManagerMomentDataPercentage,
        RKFileManagerHealthDataPercentage
    };
    
    engine->workerCount = sizeof(folders) / RKNameLength;
    
    engine->workers = (RKFileRemover *)malloc(RKFileTypeCount * sizeof(RKFileRemover));
    if (engine->workers == NULL) {
        RKLog(">%s Error. Unable to allocate an RKFileRemover.\n", engine->name);
        return (void *)RKResultFailedToCreateFileRemover;
    }
    memset(engine->workers, 0, engine->workerCount * sizeof(RKFileRemover));
    
    for (k = 0; k < engine->workerCount; k++) {
        RKFileRemover *worker = &engine->workers[k];
        
        worker->id = k;
        worker->parent = engine;
        if (engine->radarDescription != NULL && strlen(engine->radarDescription->dataPath)) {
            snprintf(worker->path, RKMaximumPathLength - 1, "%s/%s", engine->radarDescription->dataPath, folders[k]);
        } else if (strlen(engine->dataPath)) {
            snprintf(worker->path, RKMaximumPathLength - 1, "%s/%s", engine->dataPath, folders[k]);
        } else {
            snprintf(worker->path, RKMaximumPathLength - 1, "%s", folders[k]);
        }
        worker->limit = engine->usagelimit * limits[k] / 1000;

        // Workers to actually remove the files
        if (pthread_create(&worker->tid, NULL, fileRemover, worker) != 0) {
            RKLog(">%s Error. Failed to start a file remover.\n", engine->name);
            return (void *)RKResultFailedToStartFileRemover;
        }
    }

    // Wait here while the engine should stay active
    while (engine->state & RKEngineStateActive) {
        k = 0;
        while (k++ < 10 && engine->state & RKEngineStateActive) {
            usleep(100000);
        }
    }

    for (k = 0; k < engine->workerCount; k++) {
        pthread_join(engine->workers[k].tid, NULL);
    }
    free(engine->workers);
    
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
    pthread_mutex_init(&engine->mutex, NULL);
    return engine;
}

void RKFileManagerFree(RKFileManager *engine) {
    if (engine->state & RKEngineStateActive) {
        RKFileManagerStop(engine);
    }
    pthread_mutex_destroy(&engine->mutex);
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
    if (engine->usagelimit == 0) {
        engine->usagelimit = RKFileManagerDefaultUsageLimit;
        RKLog("%s Usage limit not set.  Use default = %s GB\n", engine->name, RKIntegerToCommaStyleString(engine->usagelimit / 1073741824));
    }
    if (engine->verbose) {
        RKLog("%s Starting ...\n", engine->name);
    }
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidFileWatcher, NULL, folderWatcher, engine) != 0) {
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

int RKFileManagerAddFile(RKFileManager *engine, const char *filename, RKFileType type) {
    RKFileRemover *me = &engine->workers[type];
    if (me->reusable == false) {
        return RKResultFileManagerBufferNotResuable;
    }
    
    RKPathname *filenames = (RKPathname *)me->filenames;
    RKIndexedStat *indexedStats = (RKIndexedStat *)me->indexedStats;
    
    pthread_mutex_lock(&engine->mutex);
    
    uint32_t k = me->count;
    
    strcpy(filenames[k], filename);

    struct stat fileStat;
    stat(filenames[k], &fileStat);
    indexedStats[k].index = k;
    indexedStats[k].time = fileStat.st_ctime;
    indexedStats[k].size = fileStat.st_size;
    
    if (engine->verbose > 2) {
        RKLog("%s Added file '%s' %d\n", engine->name, filename, k);
    }
    
    me->usage += indexedStats[k].size;
    me->count = RKNextModuloS(me->count, RKFileManagerFileListCapacity);
    
    gettimeofday(&me->latestTime, NULL);
    
    pthread_mutex_unlock(&engine->mutex);

    //for (k = me->count - 3; k < me->count; k++) {
    //    printf("%3d -> %3d. %s  %s\n", k, indexedStats[k].index, filenames[indexedStats[k].index], RKIntegerToCommaStyleString(indexedStats[k].size));
    //}
    return RKResultSuccess;
}
