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
#include <dirent.h>

#ifdef __MACH__
#include <mach/mach.h>
#include <mach/clock.h>
#endif

#define RKErrnoString(A)  \
(errno == EAGAIN       ? "EAGAIN"       : \
(errno == EBADF        ? "EBADF"        : \
(errno == EFAULT       ? "EFAULT"       : \
(errno == EINTR        ? "EINTR"        : \
(errno == EINVAL       ? "EINVAL"       : \
(errno == ECONNREFUSED ? "ECONNREFUSED" : \
(errno == EIO          ? "EIO"          : "OTHERS")))))))

#define POSIX_MEMALIGN_CHECK(x)        if (x) { RKLog("Could not allocate memory.\n"); exit(EXIT_FAILURE); }

void stripTrailingUnwanted(char *str);

char *RKNow();
char *RKGetColor();
char *RKGetColorOfIndex(const int i);
char *RKGetBackgroundColor();
char *RKGetBackgroundColorOfIndex(const int i);

char *RKIntegerToCommaStyleString(const long);
char *RKFloatToCommaStyleString(const double);

double RKTimevalDiff(const struct timeval miuend, const struct timeval subtrahend);
double RKTimespecDiff(const struct timespec miuend, const struct timespec subtrahend);
void RKUTCTime(struct timespec *);
bool RKFilenameExists(const char *);
void RKPreparePath(const char *filename);
char *RKSignalString(const int);

#endif /* rk_misc_h */
