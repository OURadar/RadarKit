//
//  RKMisc.h
//
//  Created by Boon Leng Cheong on 11/4/15.
//
//

#ifndef __RadarKit_RKMisc__
#define __RadarKit_RKMisc__

#include <RadarKit/RKTypes.h>
#include <signal.h>

#ifdef __MACH__
#include <mach/mach.h>
#include <mach/clock.h>
#endif

#define RKDefaultLogfile   "messages.log"

typedef struct RKGlobalParameterStruct {
    char program[RKMaximumStringLength];
    char logfile[RKMaximumStringLength];
    char showColor;
    FILE *stream;
} RKGlobalParamters;

extern RKGlobalParamters rkGlobalParameters;

// some RK functions here

char *RKNow();
char *RKIntegerToCommaStyleString(const long);
char *RKFloatToCommaStyleString(const float);
void RKSetWantColor(const bool);
void RKSetWantScreenOutput(const bool);
int RKSetProgramName(const char *);
int RKSetLogfile(const char *);
int RKSetLogfileToDefault(void);
int RKLog(const char *, ...);
double RKTimevalDiff(const struct timeval, const struct timeval);
double RKTimespecDiff(const struct timespec, const struct timespec);
void RKUTCTime(struct timespec *);
bool RKFilenameExists(const char *);
char *RKSignalString(const int);
void RKShowTypeSizes(void);

#endif /* rk_misc_h */
