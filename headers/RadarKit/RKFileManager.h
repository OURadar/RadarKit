//
//  RKFileManager.h
//  RadarKit
//
//  Remove old files so that disk usage does not exceed a user-set limit.
//  Expected file structure from RadarKit:
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
//  .../rootDataFolder/health/20170401/RK-20170402-2250.txt
//                                     RK-20170402-2251.txt
//                                     RK-20170402-2252.txt
//                                             :
//  .../rootDataFolder/health/20170402/RK-20170402-2250.txt
//                                     RK-20170402-2251.txt
//                                     RK-20170402-2252.txt
//                                             :
//
//  iq : moment : health ~ 3,000 : 100 : 1
//
//  Created by Boon Leng Cheong on 3/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_FileManager__
#define __RadarKit_FileManager__

#include <RadarKit/RKFoundation.h>

#define RKFileManagerDefaultUsageLimit   (size_t)256 * 1024 * 1024 * 1024

typedef struct rk_file_remover RKFileRemover;
typedef struct rk_file_manager RKFileManager;

struct rk_file_remover {
    int                    id;
    pthread_t              tid;
    char                   path[RKMaximumPathLength];
    size_t                 usageLimit;
    RKFileManager          *parent;
};

struct rk_file_manager {
    // User set variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;
    uint8_t                verbose;
    char                   dataPath[RKMaximumPathLength];
    size_t                 rawDataUsagelimit;
    
    // Program set variables
    int                    workerCount;
    RKFileRemover          *workers;
    pthread_t              tidFileWatcher;
    pthread_mutex_t        mutex;

    // Status / health
    RKEngineState          state;
    uint32_t               memoryUsage;
};

RKFileManager *RKFileManagerInit(void);
void RKFileManagerFree(RKFileManager *);
void RKFileManagerSetVerbose(RKFileManager *, const int);
void RKFileManagerSetInputOutputBuffer(RKFileManager *, RKRadarDesc *);

void RKFileManagerSetPathToMonitor(RKFileManager *, const char *);

int RKFileManagerStart(RKFileManager *);
int RKFileManagerStop(RKFileManager *);
int RKFileManagerAddFile(RKFileManager *, const char *);

#endif
