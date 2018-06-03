//
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/14/16.
//
//

#include <RadarKit.h>

// Global variables
RKGlobalParamters rkGlobalParameters = {{"radar"}, {RKDefaultLogfile}, {""}, false, true, PTHREAD_MUTEX_INITIALIZER, NULL};

#define N(x) #x,
const char * const rkResultStrings[] = { RKResultNames };
#undef N
