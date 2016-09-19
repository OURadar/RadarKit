//
//  RKMisc.h
//
//  Created by Boon Leng Cheong on 11/4/15.
//
//

#ifndef __RadarKit_RKMisc__
#define __RadarKit_RKMisc__

#include <RadarKit/RKTypes.h>


typedef struct RKGlobalParameterStruct {
    char program[RKMaximumStringLength];
    char logfile[RKMaximumStringLength];
    char showColor;
    FILE *stream;
} RKGlobalParamters;

extern RKGlobalParamters rkGlobalParameters;


// some RK functions here

char *RKNow();
char *RKIntegerToCommaStyleString(long num);
void RKSetWantColor(const bool showColor);
int RKSetProgramName(const char *name);
int RKSetLogfile(const char *file);
int RKLog(const char *whatever, ...);
double RKTimevalDiff(const struct timeval, const struct timeval);

#endif /* rk_misc_h */
