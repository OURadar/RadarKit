//
//  RKMisc.h
//
//  Created by Boon Leng Cheong on 11/4/15.
//
//

#ifndef __RadarKit_RKMisc__
#define __RadarKit_RKMisc__

#include <RadarKit/RKTypes.h>

#ifdef __MACH__
#include <mach/mach.h>
#include <mach/clock.h>
#endif



typedef struct RKGlobalParameterStruct {
    char program[RKMaximumStringLength];
    char logfile[RKMaximumStringLength];
    char showColor;
    FILE *stream;
} RKGlobalParamters;

extern RKGlobalParamters rkGlobalParameters;

// some RK functions here

char *RKNow();
char *RKIntegerToCommaStyleString(const long num);
char *RKFloatToCommaStyleString(const float num);
void RKSetWantColor(const bool showColor);
void RKSetWantScreenOutput(const bool yes);
int RKSetProgramName(const char *name);
int RKSetLogfile(const char *file);
int RKLog(const char *whatever, ...);
double RKTimevalDiff(const struct timeval, const struct timeval);
double RKTimespecDiff(const struct timespec, const struct timespec);
void RKUTCTime(struct timespec *);

#endif /* rk_misc_h */
