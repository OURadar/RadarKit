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

typedef struct rk_file_manager {
    // User set variables
    char                   name[RKNameLength];
    RKEngineState          state;
    uint8_t                verbose;
    uint32_t               memoryUsage;
} RKFileManager;

RKFileManager *RKFileManagerInit(void);
void RKFileManagerFree(RKFileManager *);
void RKFileManagerSetVerbose(RKFileManager *, const int);

#endif
