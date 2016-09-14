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
    char program[64];
    char logfile[64];
    char showColor;
    FILE *stream;
} RKGlobalParamters;

extern RKGlobalParamters rkGlobalParameters;

// some RK functions here

char *RKNow();
void RKSetWantColor(const bool showColor);
int RKLog(const char *whatever, ...);
char *RKIntegerToCommaStyleString(long num);

#endif /* rk_misc_h */
