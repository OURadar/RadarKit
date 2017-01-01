//
//  RKMisc.h
//  RadarKit
//
//  Technically these functions are not part of radar kit. They can be
//  used independently outside of radar kit.
//
//  Created by Boon Leng Cheong on 11/4/15.
//
//

#ifndef __RadarKit_RKMisc__
#define __RadarKit_RKMisc__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __MACH__
#include <mach/mach.h>
#include <mach/clock.h>
#endif

void stripTrailingUnwanted(char *str);

char *RKNow();

char *RKIntegerToCommaStyleString(const long);
char *RKFloatToCommaStyleString(const float);

double RKTimevalDiff(const struct timeval, const struct timeval);
double RKTimespecDiff(const struct timespec, const struct timespec);
void RKUTCTime(struct timespec *);
bool RKFilenameExists(const char *);
char *RKSignalString(const int);

#endif /* rk_misc_h */
