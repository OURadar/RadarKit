//
//  RKFileManager.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKFileManager.h>

#define RKFileManagerFolderListCapacity      60
#define RKFileManagerLogFileListCapacity     1000

// The way RadarKit names the files should be relatively short:
// Folder: YYYYMMDD
// IQData: XXXXXX-YYYYMMDD-HHMMSS-EX.X.rkr  (31 chars)
// Moment: XXXXXX-YYYYMMDD-HHMMSS-EX.X-Z.nc (32 chars)
// Health: XXXXXX-YYYYMMDD-HHMMSS.json      (27 chars)
// Log   : XXXXXX-YYYYMMDD.log              (19 chars)
#define RKFileManagerFilenameLength          36

typedef char RKPathname[RKFileManagerFilenameLength];
typedef struct _rk_indexed_stat {
    int      index;
    int      folderId;
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
    struct stat status;
    DIR *did = opendir(path);
    char pathname[RKMaximumPathLength];
    if (did == NULL) {
        if (errno != ENOENT) {
            // It is possible that the root storage folder is empty, in this case errno = ENOENT is okay.
            fprintf(stderr, "Error opening directory %s  errno = %d\n", path, errno);
        }
        return -1;
    }
    while ((dir = readdir(did)) != NULL && k < maximumCapacity) {
        if (dir->d_type == DT_UNKNOWN && dir->d_name[0] != '.') {
            //sprintf(pathname, "%s/%s", path, dir->d_name);
            int r = sprintf(pathname, "%s/", path);
            (void)strncpy(pathname + r, dir->d_name, RKMaximumPathLength - r - 1);
            lstat(pathname, &status);
            if ((type == DT_REG && S_ISREG(status.st_mode)) ||
                (type == DT_DIR && S_ISDIR(status.st_mode))) {
                //snprintf(list[k], RKFileManagerFilenameLength - 1, "%s", dir->d_name);
                (void)strncpy(list[k], dir->d_name, RKFileManagerFilenameLength - 1);
                k++;
            }
        } else if (dir->d_type == type && dir->d_name[0] != '.') {
            //snprintf(list[k], RKFileManagerFilenameLength - 1, "%s", dir->d_name);
            (void)strncpy(list[k], dir->d_name, RKFileManagerFilenameLength - 1);
            k++;
        }
    }
    closedir(did);
    return k;
}

static int listFoldersInFolder(RKPathname *list, const int maximumCapacity, const char *path) {
    return listElementsInFolder(list, maximumCapacity, path, DT_DIR);
}

static int listFilesInFolder(RKPathname *list, const int maximumCapacity, const char *path) {
    return listElementsInFolder(list, maximumCapacity, path, DT_REG);
}

static bool isFolderEmpty(const char *path) {
    struct dirent *dir;
    DIR *did = opendir(path);
    if (did == NULL) {
        fprintf(stderr, "Error opening directory %s\n", path);
        return false;
    }
    while ((dir = readdir(did)) != NULL) {
        if (dir->d_type == DT_REG) {
            closedir(did);
            return false;
        }
    }
    closedir(did);
    return true;
}

#pragma mark - Delegate Workers

static void refreshFileList(RKFileRemover *me) {
    int j, k;
    struct stat fileStat;
    char string[RKMaximumStringLength];

    RKPathname *folders = (RKPathname *)me->folders;
    RKPathname *filenames = (RKPathname *)me->filenames;
    RKIndexedStat *indexedStats = (RKIndexedStat *)me->indexedStats;

    me->index = 0;
    me->count = 0;
    me->usage = 0;

    // Go through all folders
    int folderCount = listFoldersInFolder(folders, RKFileManagerFolderListCapacity, me->path);
    if (folderCount <= 0) {
        return;
    }

    // Sort the folders by name (should be in time numbers)
    qsort(folders, folderCount, sizeof(RKPathname), string_cmp_by_pseudo_time);

    // Go through all files in the folders
    for (j = 0, k = 0; k < folderCount && me->count < me->capacity; k++) {
        sprintf(string, "%s/%s", me->path, folders[k]);
        int count = listFilesInFolder(&filenames[me->count], me->capacity - me->count, string);
        if (count < 0) {
            RKLog("%s Error. Unable to list files in %s\n", me->parent->name, string);
            return;
        }
        me->count += count;
        for (; j < me->count; j++) {
            sprintf(string, "%s/%s/%s", me->path, folders[k], filenames[j]);
            stat(string, &fileStat);
            indexedStats[j].index = j;
            indexedStats[j].folderId = k;
            indexedStats[j].time = fileStat.st_ctime;
            indexedStats[j].size = fileStat.st_size;
            me->usage += fileStat.st_size;
        }
    }

    // Sort the files by time
    qsort(indexedStats, me->count, sizeof(RKIndexedStat), struct_cmp_by_time);

#if defined(DEBUG_FILE_MANAGER)

    for (k = 0; k < me->count; k++) {
        printf("%s/%s/%s\n", me->path, folders[indexedStats[k].folderId], filenames[indexedStats[k].index]);
    }
    
#endif
    
    // We can operate in a circular buffer fashion if all the files are accounted
    if (me->count < me->capacity) {
        me->reusable = true;
    } else {
        RKLog("%s Info. Experimental mode for '%s'.\n", me->parent->name, me->path);
        // Re-calculate the usage
        me->usage = 0;
        for (k = 0; k < folderCount; k++) {
            struct dirent *dir;
            sprintf(string, "%s/%s/%s", me->path, folders[k], filenames[j]);
            DIR *did = opendir(string);
            if (did == NULL) {
                fprintf(stderr, "Unable to list folder '%s'\n", folders[k]);
                continue;
            }
            while ((dir = readdir(did)) != NULL) {
                sprintf(string, "%s/%s/%s", me->path, folders[k], dir->d_name);
                stat(string, &fileStat);
                me->usage += fileStat.st_size;
            }
            closedir(did);
        }
        RKLog("%s Truncated list with total usage = %s B\n", me->parent->name, RKUIntegerToCommaStyleString(me->usage));
    }
}

static void *fileRemover(void *in) {
    RKFileRemover *me = (RKFileRemover *)in;
    RKFileManager *engine = me->parent;
    
    int k;

    const int c = me->id;
    
    char path[RKMaximumPathLength];
    char command[RKMaximumPathLength];
    char parentFolder[RKFileManagerFilenameLength] = "";
    struct timeval time = {0, 0};

	// Initiate a variable to store my name
	RKName name;
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
    
    size_t bytes;
    size_t mem = 0;
    
    bytes = RKFileManagerFolderListCapacity * sizeof(RKPathname);
    RKPathname *folders = (RKPathname *)malloc(bytes);
    if (folders == NULL) {
        RKLog("%s %s Error. Unable to allocate space for folder list.\n", engine->name, name);
    }
    memset(folders, 0, bytes);
    mem += bytes;
    
    bytes = me->capacity * sizeof(RKPathname);
    RKPathname *filenames = (RKPathname *)malloc(bytes);
    if (filenames == NULL) {
        RKLog("%s %s Error. Unable to allocate space for file list.\n", engine->name, name);
    }
    memset(filenames, 0, bytes);
    mem += bytes;
    
    bytes = me->capacity * sizeof(RKIndexedStat);
    RKIndexedStat *indexedStats = (RKIndexedStat *)malloc(bytes);
    if (indexedStats == NULL) {
        RKLog("%s %s Error. Unable to allocate space for indexed stats.\n", engine->name, name);
    }
    memset(indexedStats, 0, bytes);
    mem += bytes;
    
    me->folders = folders;
    me->filenames = filenames;
    me->indexedStats = indexedStats;
    
    pthread_mutex_lock(&engine->mutex);
    engine->memoryUsage += mem;
    
    // Gather the initial file list in the folders
    refreshFileList(me);

    // Use the first folder as the initial value of parentFolder
    strcpy(parentFolder, folders[0]);

    RKLog(">%s %s Started.   mem = %s B   capacity = %s\n",
          engine->name, name, RKUIntegerToCommaStyleString(mem), RKUIntegerToCommaStyleString(me->capacity));
    RKLog(">%s %s Path = %s\n", engine->name, name, me->path);
    if (me->usage > 10 * 1024 * 1024) {
        RKLog(">%s %s Listed.  count = %s   usage = %s / %s MB (%.2f %%)\n", engine->name, name,
              RKIntegerToCommaStyleString(me->count),
              RKUIntegerToCommaStyleString(me->usage / 1024 / 1024),
              RKUIntegerToCommaStyleString(me->limit / 1024 / 1024),
              100.0f * me->usage / me->limit);
    } else if (me->usage > 10 * 1024) {
        RKLog(">%s %s Listed.  count = %s   usage = %s / %s KB (%.2f %%)\n", engine->name, name,
              RKIntegerToCommaStyleString(me->count),
              RKUIntegerToCommaStyleString(me->usage / 1024),
              RKUIntegerToCommaStyleString(me->limit / 1024),
              100.0f * me->usage / me->limit);
    } else {
        RKLog(">%s %s Listed.  count = %s   usage = %s / %s B (%.2f %%)\n", engine->name, name,
              RKIntegerToCommaStyleString(me->count),
              RKUIntegerToCommaStyleString(me->usage),
              RKUIntegerToCommaStyleString(me->limit),
              100.0f * me->usage / me->limit);
    }

    me->tic++;
    
    pthread_mutex_unlock(&engine->mutex);

    engine->state |= RKEngineStateActive;

    me->index = 0;
    while (engine->state & RKEngineStateWantActive) {

        pthread_mutex_lock(&engine->mutex);

        // Removing files
        while (me->usage > me->limit) {
            // Build the complete path from various components
            sprintf(path, "%s/%s/%s", me->path, folders[indexedStats[me->index].folderId], filenames[indexedStats[me->index].index]);
            if (engine->verbose) {
				if (indexedStats[me->index].size > 1.0e9) {
					RKLog("%s %s Removing %s (%s GB)", engine->name, name, path, RKFloatToCommaStyleString(1.0e-9f * indexedStats[me->index].size));
				} else if (indexedStats[me->index].size > 1.0e6) {
					RKLog("%s %s Removing %s (%s MB)", engine->name, name, path, RKFloatToCommaStyleString(1.0e-6f * indexedStats[me->index].size));
				} else if (indexedStats[me->index].size > 1.0e3) {
					RKLog("%s %s Removing %s (%s KB)", engine->name, name, path, RKFloatToCommaStyleString(1.0e-3f * indexedStats[me->index].size));
				} else {
					RKLog("%s %s Removing %s (%s B)", engine->name, name, path, RKUIntegerToCommaStyleString(indexedStats[me->index].size));
				}
            }
            remove(path);
            me->usage -= indexedStats[me->index].size;
            me->index++;
			if (engine->verbose > 1) {
				RKLog("%s %s Usage -> %s B / %s B\n", engine->name, name, RKUIntegerToCommaStyleString(me->usage), RKUIntegerToCommaStyleString(me->limit));
			}

            // Get the parent folder, if it is different than before, check if it is empty, and remove it if so.
            if (strcmp(parentFolder, folders[indexedStats[me->index].folderId])) {
                sprintf(path, "%s/%s", me->path, parentFolder);
                if (isFolderEmpty(path)) {
                    RKLog("%s %s Removing folder %s that is empty.\n", engine->name, name, path);
                    sprintf(command, "rm -rf %s", path);
                    k = system(command);
                    if (k) {
                        RKLog("Error. system(%s) -> %d   errno = %d\n", command, k, errno);
                    }
                }
                RKLog("%s %s New parentFolder = %s -> %s\n", engine->name, name, parentFolder, folders[indexedStats[me->index].folderId]);
                strcpy(parentFolder, folders[indexedStats[me->index].folderId]);
            }

            // Update the index for next check or refresh it entirely if there are too many files
            if (me->index == me->capacity) {
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

        pthread_mutex_unlock(&engine->mutex);

        // Now we wait
        k = 0;
        while ((k++ < 10 || RKTimevalDiff(time, me->latestTime) < 0.2) && engine->state & RKEngineStateWantActive) {
            gettimeofday(&time, NULL);
            usleep(100000);
        }
    }
    
    free(filenames);
    free(indexedStats);
    
    RKLog(">%s %s Stopped.\n", engine->name, name);

    engine->state ^= RKEngineStateActive;
    return NULL;
}

static void *folderWatcher(void *in) {
    RKFileManager *engine = (RKFileManager *)in;
    
    int k;
    DIR *did;
    struct dirent *dir;
    struct stat fileStat;
    
    engine->state |= RKEngineStateWantActive;
    engine->state ^= RKEngineStateActivating;
    
    // Three major data folders that have structure A
    const char folders[][RKNameLength] = {
        RKDataFolderIQ,
        RKDataFolderMoment,
        RKDataFolderHealth,
        ""
    };

#if defined(DEBUG_FILE_MANAGER)

    const int capacities[] = {
        12,
        12,
        12,
        0
    };
    const size_t limits[] = {
        1,
        1,
        1,
        0
    };

#else

    const int capacities[] = {
        24 * 3600 / 2 * 2,                // Assume a file every 2 seconds, 2 folders
        24 * 3600 / 2 * 8 * 2,            // Assume 8 files every 2 seconds, 2 folders
        24 * 60 * 3,                      // Assume a file every minute, 3 folders
        0
    };
    const size_t limits[] = {
        RKFileManagerRawDataRatio,
        RKFileManagerMomentDataRatio,
        RKFileManagerHealthDataRatio,
        0
    };

#endif

    engine->workerCount = 3;
    
    engine->workers = (RKFileRemover *)malloc(engine->workerCount * sizeof(RKFileRemover));
    if (engine->workers == NULL) {
        RKLog(">%s Error. Unable to allocate an RKFileRemover.\n", engine->name);
        return (void *)RKResultFailedToCreateFileRemover;
    }
    memset(engine->workers, 0, engine->workerCount * sizeof(RKFileRemover));
    engine->memoryUsage += engine->workerCount * sizeof(RKFileRemover);
    
    for (k = 0; k < engine->workerCount; k++) {
        RKFileRemover *worker = &engine->workers[k];
        
        worker->id = k;
        worker->parent = engine;
        worker->capacity = capacities[k];
        if (engine->radarDescription != NULL && strlen(engine->radarDescription->dataPath)) {
            snprintf(worker->path, RKMaximumFolderPathLength + 32, "%s/%s", engine->radarDescription->dataPath, folders[k]);
        } else if (strlen(engine->dataPath)) {
            snprintf(worker->path, RKMaximumFolderPathLength + 32, "%s/%s", engine->dataPath, folders[k]);
        } else {
            snprintf(worker->path, RKMaximumFolderPathLength + 32, "%s", folders[k]);
        }
        worker->limit = engine->usagelimit * limits[k] / RKFileManagerTotalRatio;

        // Workers that actually remove the files (and folders)
        if (pthread_create(&worker->tid, NULL, fileRemover, worker) != 0) {
            RKLog(">%s Error. Failed to start a file remover.\n", engine->name);
            return (void *)RKResultFailedToStartFileRemover;
        }
        
        while (worker->tic == 0 && engine->state & RKEngineStateWantActive) {
            usleep(10000);
        }
    }

    // Log path has structure B
    char string[RKMaximumPathLength + 16];
    char logPath[RKMaximumPathLength + 16] = RKLogFolder;
    if (engine->radarDescription != NULL && strlen(engine->radarDescription->dataPath)) {
        snprintf(logPath, RKMaximumPathLength + 15, "%s/" RKLogFolder, engine->radarDescription->dataPath);
    } else if (strlen(engine->dataPath)) {
        snprintf(logPath, RKMaximumPathLength + 15, "%s/" RKLogFolder, engine->dataPath);
    }
    
    RKLog("%s Started.   mem = %s B  state = %x\n", engine->name, RKUIntegerToCommaStyleString(engine->memoryUsage), engine->state);

	// Increase the tic once to indicate the engine is ready
	engine->tic++;

    // Wait here while the engine should stay active
    time_t now;
    time_t longTime = engine->maximumLogAgeInDays * 86400;
    while (engine->state & RKEngineStateWantActive) {
        // Take care of super slow changing files, like the daily logs
        time(&now);
        if ((did = opendir(logPath)) != NULL) {
            while ((dir = readdir(did)) != NULL) {
                if (dir->d_name[0] == '.') {
                    continue;
                }
                snprintf(string, RKMaximumPathLength - 1, "%s/%s", logPath, dir->d_name);
                lstat(string, &fileStat);
                if (!S_ISREG(fileStat.st_mode)) {
                    continue;
                }
                if (fileStat.st_ctime < now - longTime) {
                    RKLog("%s Removing %s ...\n", engine->name, string);
                    //snprintf(string, RKMaximumPathLength - 1, "rm -f %s/%s", logPath, dir->d_name);
                    //system(string);
                    remove(string);
                }
            }
            closedir(did);
        }
        // Wait one minute, do it with multiples of 0.1s for a responsive exit
        k = 0;
        while (k++ < 600 && engine->state & RKEngineStateWantActive) {
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
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorFileManager) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->maximumLogAgeInDays = RKFileManagerDefaultLogAgeInDays;
    engine->memoryUsage = sizeof(RKFileManager);
    pthread_mutex_init(&engine->mutex, NULL);
    return engine;
}

void RKFileManagerFree(RKFileManager *engine) {
    if (engine->state & RKEngineStateWantActive) {
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
    strncpy(engine->dataPath, path, RKMaximumFolderPathLength - 1);
}

void RKFileManagerSetDiskUsageLimit(RKFileManager *engine, const size_t limit) {
    engine->usagelimit = limit;
}

void RKFileManagerSetMaximumLogAgeInDays(RKFileManager *engine, const int age) {
    engine->maximumLogAgeInDays = age;
}

#pragma mark - Interactions

int RKFileManagerStart(RKFileManager *engine) {
    // File manager is always assumed wired, dataPath may be empty
    engine->state |= RKEngineStateProperlyWired;
    if (engine->usagelimit == 0) {
        engine->usagelimit = RKFileManagerDefaultUsageLimit;
        RKLog("%s Usage limit not set, use default %s%s B%s (%s %s)\n",
              engine->name,
              rkGlobalParameters.showColor ? "\033[4m" : "",
              RKUIntegerToCommaStyleString(engine->usagelimit),
              rkGlobalParameters.showColor ? "\033[24m" : "",
              RKFloatToCommaStyleString((double)engine->usagelimit / (engine->usagelimit > 1073741824 ? 1099511627776 : 1073741824)),
              engine->usagelimit > 1073741824 ? "TiB" : "GiB");
    }
    RKLog("%s Starting ...\n", engine->name);
    engine->tic = 0;
    engine->state |= RKEngineStateActivating;
    if (pthread_create(&engine->tidFileWatcher, NULL, folderWatcher, engine) != 0) {
        RKLog("Error. Failed to start a ray gatherer.\n");
        return RKResultFailedToStartFileManager;
    }
	//RKLog("tidFileWatcher = %lu\n", (unsigned long)engine->tidFileWatcher);
	while (engine->tic == 0) {
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
    RKLog("%s Stopping ...\n", engine->name);
    engine->state |= RKEngineStateDeactivating;
    engine->state ^= RKEngineStateWantActive;
    pthread_join(engine->tidFileWatcher, NULL);
    engine->state ^= RKEngineStateDeactivating;
    RKLog("%s Stopped.\n", engine->name);
    if (engine->state != (RKEngineStateAllocated | RKEngineStateProperlyWired)) {
        RKLog("%s Inconsistent state 0x%04x\n", engine->name, engine->state);
    }
    return RKResultSuccess;
}

int RKFileManagerAddFile(RKFileManager *engine, const char *filename, RKFileType type) {
    RKFileRemover *me = &engine->workers[type];

    struct stat fileStat;
    stat(filename, &fileStat);

    gettimeofday(&me->latestTime, NULL);

    // For non-reusable type, just add the size
    if (me->reusable == false) {
        pthread_mutex_lock(&engine->mutex);
        me->usage += fileStat.st_size;
        pthread_mutex_unlock(&engine->mutex);
        return RKResultFileManagerBufferNotResuable;
    }
    
    RKPathname *folders = (RKPathname *)me->folders;
    RKPathname *filenames = (RKPathname *)me->filenames;
    RKIndexedStat *indexedStats = (RKIndexedStat *)me->indexedStats;
    
    // Copy over the filename and stats to the internal buffer
    uint32_t k = me->count;
    
    // Extract out the date portion
    char *lastPart = strrchr(filename, '/');
    if (lastPart) {
        strncpy(filenames[k], lastPart + 1, RKFileManagerFilenameLength - 1);
    } else {
        if (strlen(filename) > RKFileManagerFilenameLength - 1) {
            RKLog("%s Warning. Filename is too long.\n", engine->name);
        }
        strncpy(filenames[k], filename, RKFileManagerFilenameLength - 1);
    }
    
    if (strncmp(me->path, filename, strlen(me->path))) {
        RKLog("%s File %s does not belong here.\n", engine->name, filename);
    }
    
    // [me->path]/YYYYMMDD/RK-YYYYMMDD-...
    
    int folderId = 0;
    char *folder = engine->scratch;
    strcpy(folder, filename + strlen(me->path) + 1);
    char *e = strrchr(folder, '/');
    *e = '\0';   
    if (strlen(folder) > RKFileManagerFilenameLength - 1) {
        RKLog("%s Warning. Folder name is too long.\n", engine->name);
    }
    if (k == 0) {
        // The data folder is empty
        folderId = 0;
        strcpy(folders[0], folder);
    } else {
        folderId = indexedStats[k - 1].folderId;
        // If same folder as the previous entry, no need to add another folder in the list. Add one otherwise.
        if (strcmp(folder, folders[indexedStats[k - 1].folderId])) {
            folderId++;
            strcpy(folders[indexedStats[k - 1].folderId + 1], folder);
        }
    }
    //printf("%s --> %s / %s (%d)\n", filename, folder, filenames[k], folderId);

    pthread_mutex_lock(&engine->mutex);

    indexedStats[k].index = k;
    indexedStats[k].folderId = folderId;
    indexedStats[k].time = fileStat.st_ctime;
    indexedStats[k].size = fileStat.st_size;
    
    me->usage += fileStat.st_size;
    
    if (engine->verbose > 2) {
        RKLog("%s Added '%s'   %s B  k%d\n", engine->name, filename, RKUIntegerToCommaStyleString(indexedStats[k].size), k);
    }
    
    me->count = RKNextModuloS(me->count, me->capacity);
    
    pthread_mutex_unlock(&engine->mutex);

    //for (k = me->count - 3; k < me->count; k++) {
    //    printf("%3d -> %3d. %s  %s\n", k, indexedStats[k].index, filenames[indexedStats[k].index], RKUIntegerToCommaStyleString(indexedStats[k].size));
    //}
    return RKResultSuccess;
}
