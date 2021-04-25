//
//  RadarKit
//
//  Created by Boonleng Cheong on 9/14/16.
//
//

#include <RadarKit.h>

// Global variables
RKGlobalParameters rkGlobalParameters = {
    .program = {"radar"},
    .logfile = {RKDefaultLogfile},
    .logFolder = {""},
    .rootDataFolder = RKDefaultDataPath,
    .dailyLog = false,
    .showColor = true,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .stream = NULL
};

#define N(x) #x,
const char * const rkResultStrings[] = { RKResultNames };
#undef N
