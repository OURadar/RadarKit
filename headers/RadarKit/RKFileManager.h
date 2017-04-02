//
//  RKFileManager.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_FileManager__
#define __RadarKit_FileManager__

#include <RadarKit/RKFoundation.h>

#define RKFileManagerDefaultUsageLimit   (size_t)256 * 1024 * 1024 * 1024

typedef struct rk_file_manager {
    // User set variables
    char                   name[RKNameLength];
    RKRadarDesc            *radarDescription;
    uint8_t                verbose;
    char                   dataPath[RKMaximumPathLength];
    size_t                 rawDataUsagelimit;
    
    // Program set variables
    pthread_t              tidFileWatcher;

    // Status / health
    RKEngineState          state;
    uint32_t               memoryUsage;
} RKFileManager;

RKFileManager *RKFileManagerInit(void);
void RKFileManagerFree(RKFileManager *);
void RKFileManagerSetVerbose(RKFileManager *, const int);
void RKFileManagerSetInputOutputBuffer(RKFileManager *, RKRadarDesc *);

void RKFileManagerSetPathToMonitor(RKFileManager *, const char *);

int RKFileManagerStart(RKFileManager *);
int RKFileManagerStop(RKFileManager *);
int RKFileManagerAddFile(RKFileManager *, const char *);

#endif
