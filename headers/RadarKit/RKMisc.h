//
//  RKMisc.h
//  RadarKit
//
//  Technically these functions are not part of RadarKit. They can be
//  used independently outside of radar kit.
//
//  Created by Boon Leng Cheong on 11/4/15.
//
//

#ifndef __RadarKit_Misc__
#define __RadarKit_Misc__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <dirent.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <resolv.h>

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

#if defined(__APPLE__)

#define SYSCTL_CORE_COUNT   "machdep.cpu.core_count"

typedef struct cpu_set {
    uint32_t    count;
} cpu_set_t;

static inline void
CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }

static inline void
CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }

static inline int
CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }

int pthread_setaffinity_np(pthread_t thread, size_t cpu_size, cpu_set_t *cpu_set);

#endif

enum RKJSONObjectType {
    RKJSONObjectTypeUnknown,
    RKJSONObjectTypePlain,
    RKJSONObjectTypeString,
    RKJSONObjectTypeArray,
    RKJSONObjectTypeObject
};

void stripTrailingUnwanted(char *str);

char *RKNow(void);
char *RKGetColor(void);
char *RKGetColorOfIndex(const int i);
char *RKGetBackgroundColor(void);
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

long RKCountFilesInPath(const char *);
char *RKLastTwoPartsOfPath(const char *);
char *RKPathStringByExpandingTilde(const char *);
void RKReplaceFileExtension(char *filename, const char *pattern, const char *replacement);

#endif /* rk_misc_h */
