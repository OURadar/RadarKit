//
//  RKFileManager.h
//  RadarKit
//
//  Remove old files so that disk usage does not exceed a user-set limit.
//  Expected file structure from RadarKit:
//
//
//  Three major folders (IQ, moment, and health) with structure A:
//
//  .../rootDataFolder/iq/20170401/RK-20170401-012345-E1.0.rkr
//                                 RK-20170401-012346-E2.0.rkr
//                                 RK-20170401-012347-E3.0.rkr
//                                             :
//  .../rootDataFolder/iq/20170402/RK-20170402-012345-E1.0.rkr
//                                 RK-20170402-012346-E2.0.rkr
//                                 RK-20170402-012347-E3.0.rkr
//                                             :
//
//  .../rootDataFolder/moments/20170401/RK-20170401-012345-E1.0-Z.nc
//                                      RK-20170401-012345-E1.0-V.nc
//                                      RK-20170401-012345-E1.0-W.nc
//                                                  :
//  .../rootDataFolder/moments/20170402/RK-20170402-012347-E3.0-Z.nc
//                                      RK-20170402-012347-E3.0-V.nc
//                                      RK-20170402-012347-E3.0-W.nc
//                                                  :
//
//  .../rootDataFolder/health/20170401/RK-20170402-2250.json
//                                     RK-20170402-2251.json
//                                     RK-20170402-2252.json
//                                             :
//  .../rootDataFolder/health/20170402/RK-20170402-2250.json
//                                     RK-20170402-2251.json
//                                     RK-20170402-2252.json
//                                             :
//
//
//  Log folders have structure B:
//
//  .../rootDataFolder/log/trxd-20170401.log
//                         trxd-20170402.log
//                         trxd-20170403.log
//                                      :
//
//
//  Created by Boonleng Cheong on 3/11/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_FileManager__
#define __RadarKit_FileManager__

#include <RadarKit/RKFoundation.h>

#define RKFileManagerDefaultUsageLimit     (size_t)1024 * 1024 * 1024 * 1024 * 12 / 10
#define RKFileManagerRawDataRatio          8
#define RKFileManagerMomentDataRatio       5
#define RKFileManagerHealthDataRatio       1
#define RKFileManagerDefaultLogAgeInDays   30

typedef struct rk_file_remover RKFileRemover;
typedef struct rk_file_manager RKFileManager;

struct rk_file_remover {
    RKShortName                      name;
    int                              id;
    uint64_t                         tic;
    pthread_t                        tid;
    RKFileManager                    *parent;

    int                              index;                               // Index to get sorted index for removal
    int                              count;                               // Dual use index: count and index to add (reusable buffer)
    int                              capacity;                            // Capcity of *folders, *filenames and *indexedStats
    size_t                           usage;
    size_t                           limit;
    char                             path[RKMaximumFolderPathLength + 32];

    void                             *folders;
    void                             *filenames;
    void                             *indexedStats;
    bool                             reusable;

    struct timeval                   latestTime;
};

struct rk_file_manager {
    // User set variables
    RKName                           name;
    RKRadarDesc                      *radarDescription;                   // This takes precedence over dataPath[] if both are set
    uint8_t                          verbose;                             // Verbosity level
    char                             dataPath[RKMaximumFolderPathLength]; // Can be empty. In this case all folders are relative to the path where it is executed
    size_t                           usagelimit;                          // Overall usage limit in bytes if no user limits are set
    int                              maximumLogAgeInDays;                 // Maximum number of days to keep logs in .../rootDataFolder/log/
    size_t                           userRawDataUsageLimit;               // User set raw data usage limit
    size_t                           userMomentDataUsageLimit;            // User set moment data usage limit
    size_t                           userHealthDataUsageLimit;            // User set health data usage limit

    // Program set variables
    uint64_t                         tic;
    int                              workerCount;
    RKFileRemover                    *workers;
    pthread_t                        tidFileWatcher;
    pthread_mutex_t                  mutex;
    char                             scratch[RKMaximumStringLength];

    // Status / health
    RKEngineState                    state;
    uint32_t                         memoryUsage;
};

RKFileManager *RKFileManagerInit(void);
void RKFileManagerFree(RKFileManager *);

void RKFileManagerSetVerbose(RKFileManager *, const int);
void RKFileManagerSetInputOutputBuffer(RKFileManager *, RKRadarDesc *);
void RKFileManagerSetPathToMonitor(RKFileManager *, const char *);
void RKFileManagerSetDiskUsageLimit(RKFileManager *, const size_t);
void RKFileManagerSetMaximumLogAgeInDays(RKFileManager *, const int age);
void RKFileManagerSetRawDataLimit(RKFileManager *, const size_t);
void RKFileManagerSetMomentDataLimit(RKFileManager *, const size_t);
void RKFileManagerSetHealthDataLimit(RKFileManager *, const size_t);

int RKFileManagerStart(RKFileManager *);
int RKFileManagerStop(RKFileManager *);

int RKFileManagerAddFile(RKFileManager *, const char *, RKFileType);

#endif
