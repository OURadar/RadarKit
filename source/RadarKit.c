//
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/14/16.
//
//

#include <RadarKit.h>

// Global variables
RKGlobalParamters rkGlobalParameters = {
    .program = {"radar"},
    .logfile = {RKDefaultLogfile},
    .rootDataFolder = RKDefaultDataPath,
    .dailyLog = false,
    .showColor = true,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .stream = NULL
};

#define N(x) #x,
const char * const rkResultStrings[] = { RKResultNames };
#undef N
