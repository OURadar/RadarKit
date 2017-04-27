//
//  RKFileManager.h
//  RadarKit
//
//  Remove old files so that disk usage does not exceed a user-set limit.
//  Expected file structure from RadarKit:
//
//
//  Three major folders with structure A:
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
//  Created by Boon Leng Cheong on 3/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_FileManager__
#define __RadarKit_FileManager__

#include <RadarKit/RKFoundation.h>

#define RKFileManagerDefaultUsageLimit     (size_t)1024 * 1024 * 1024 * 1024 * 15 / 10
#define RKFileManagerRawDataRatio          500
#define RKFileManagerMomentDataRatio       1000
#define RKFileManagerHealthDataRatio       2
#define RKFileManagerLogDataRatio          1
#define RKFileManagerTotalRatio            (RKFileManagerRawDataRatio + RKFileManagerMomentDataRatio + RKFileManagerHealthDataRatio + RKFileManagerLogDataRatio)
#define RKFileManagerDefaultLogAgeInDays   30

typedef struct rk_file_remover RKFileRemover;
typedef struct rk_file_manager RKFileManager;

struct rk_file_remover {
    int                    id;
    int                    tic;
    pthread_t              tid;
    int                    index;                               // Index to get sorted index for removal
    int                    count;                               // Dual use index: count and index to add (reusable buffer)
    int                    capacity;                            // Capcity of *folders, *filenames and *indexedStats
    size_t                 usage;
    size_t                 limit;
    char                   path[RKMaximumPathLength];
    RKFileManager          *parent;
    
    void                   *folders;
    void                   *filenames;
    void                   *indexedStats;
    bool                   reusable;
    
    struct timeval         latestTime;
};

struct rk_file_manager {
    // User set variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;                   // This takes precedence over dataPath[] if both are set
    uint8_t                verbose;                             // Verbosity level
    char                   dataPath[RKMaximumPathLength];       // Can be empty. In this case all folders are relative to the path where it is executed
    size_t                 usagelimit;                          // Overall usage limit in bytes
    int                    maximumLogAgeInDays;                 // Maximum number of days to keep logs in .../rootDataFolder/log/
    
    // Program set variables
    int                    tic;
    int                    workerCount;
    RKFileRemover          *workers;
    pthread_t              tidFileWatcher;
    pthread_mutex_t        mutex;
    char                   scratch[RKMaximumStringLength];

    // Status / health
    RKEngineState          state;
    uint32_t               memoryUsage;
};

RKFileManager *RKFileManagerInit(void);
void RKFileManagerFree(RKFileManager *);

void RKFileManagerSetVerbose(RKFileManager *, const int);
void RKFileManagerSetInputOutputBuffer(RKFileManager *, RKRadarDesc *);
void RKFileManagerSetPathToMonitor(RKFileManager *, const char *);
void RKFileManagerSetMaximumLogAgeInDays(RKFileManager *engine, const int age);

int RKFileManagerStart(RKFileManager *);
int RKFileManagerStop(RKFileManager *);

int RKFileManagerAddFile(RKFileManager *, const char *, RKFileType);

#endif
