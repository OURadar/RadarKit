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

enum RKJSONObjectType {
    RKJSONObjectTypeUnknown,
    RKJSONObjectTypePlain,
    RKJSONObjectTypeString,
    RKJSONObjectTypeArray,
    RKJSONObjectTypeObject
};

void stripTrailingUnwanted(char *str);

char *RKNow();
char *RKGetColor();
char *RKGetColorOfIndex(const int i);
char *RKGetBackgroundColor();
char *RKGetBackgroundColorOfIndex(const int i);
char *RKGetValueOfKey(const char *string, const char *key);
int RKIndentCopy(char *dst, char *src);

char *RKIntegerToCommaStyleString(const long);
char *RKFloatToCommaStyleString(const double);

double RKTimevalDiff(const struct timeval minuend, const struct timeval subtrahend);
double RKTimespecDiff(const struct timespec minuend, const struct timespec subtrahend);
void RKUTCTime(struct timespec *);
bool RKFilenameExists(const char *);
void RKPreparePath(const char *filename);
char *RKSignalString(const int);
int RKStripTail(char *);
float RKUMinDiff(const float minuend, const float subtrahend);

char *RKExtractJSON(char *ks, uint8_t *type, char *key, char *value);
void RKGoThroughKeywords(const char *string);
void RKReplaceKeyValue(char *string, const char *key, int value);

#endif /* rk_misc_h */
