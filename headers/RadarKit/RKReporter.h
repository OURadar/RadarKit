//
//  RKReporter.h
//  RadarKit
//
//  Created by Boonleng Cheong on 2/9/22.
//  Copyright (c) 2017-2022 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Reporter__
#define __RadarKit_Reporter__

#include <RadarKit/RKRadar.h>

typedef struct rk_reporter {
    // User set variables
    RKName                           name;
    int                              verbose;
    RKRadar                          *radar;

    // Program set variables
    pthread_mutex_t                  mutex;

    // Status / health
    size_t                           memoryUsage;
} RKReporter;

RKReporter *RKReporterInit(void);
void RKReporterFree(RKReporter *);

void RKReporterStart(RKReporter *);
void RKReporterStop(RKReporter *);

#endif
