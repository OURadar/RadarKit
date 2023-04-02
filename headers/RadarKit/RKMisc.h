//
//  RKMisc.h
//  RadarKit
//
//  Technically these functions are not part of the RadarKit framework. They can be
//  used independently outside of RadarKit.
//
//  Created by Boonleng Cheong on 11/4/15.
//
//

#ifndef __RadarKit_Misc__
#define __RadarKit_Misc__

#define __STDC_WANT_LIB_EXT1__ 1

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
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/resource.h>
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
(errno == EHOSTDOWN    ? "EHOSTDOWN"    : \
(errno == EHOSTUNREACH ? "EHOSTUNREACH" : \
(errno == EACCES       ? "EACCES"       : \
(errno == EIO          ? "EIO"          : "OTHERS"))))))))))

#define POSIX_MEMALIGN_CHECK(x)        if (x) { RKLog("Could not allocate memory.\n"); exit(EXIT_FAILURE); }

#define MAKE_FUNCTION_NAME(x) \
RKName x; \
sprintf(x, "%s()", __FUNCTION__);

#define SHOW_FUNCTION_NAME \
int _fn_len = strlen(__FUNCTION__); \
char _fn_str[RKNameLength]; \
memset(_fn_str, '=', _fn_len); \
sprintf(_fn_str + _fn_len, "\n%s\n", __FUNCTION__); \
memset(_fn_str + 2 * _fn_len + 2, '=', _fn_len); \
_fn_str[3 * _fn_len + 2] = '\0'; \
printf("%s\n", _fn_str);

#define SHOW_SIZE(x) \
printf(RKDeepPinkColor "sizeof" RKNoColor "(" RKSkyBlueColor #x RKNoColor ") = " RKLimeColor "%s" RKNoColor "\n", \
RKUIntegerToCommaStyleString(sizeof(x)));

#define SHOW_SIZE_SIMD(x) \
printf(RKDeepPinkColor "sizeof" RKNoColor "(" RKSkyBlueColor #x RKNoColor ") = " RKLimeColor "%s" RKNoColor \
"   \033[40G" RKOrangeColor "SIMDAlign" RKNoColor " = " RKPurpleColor "%s" RKNoColor "  %s\n", \
RKUIntegerToCommaStyleString(sizeof(x)), \
sizeof(x) % RKMemoryAlignSize == 0 ? "True" : "False", \
sizeof(x) % RKMemoryAlignSize != 0 ? RKWarningColor " WARNING " RKNoColor : "");

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

#define RKMiscStringLength  1024

enum RKJSONObjectType {
    RKJSONObjectTypeUnknown,
    RKJSONObjectTypePlain,
    RKJSONObjectTypeString,
    RKJSONObjectTypeArray,
    RKJSONObjectTypeObject
};

//
// Color Output
//

// Returns an escape sequence to produce colored output on terminal (random)
char *RKGetColor(void);

// Same as before but you get to choose the index
char *RKGetColorOfIndex(const int);

// Returns an escape sequence to produce colored background on terminal (random)
char *RKGetBackgroundColor(void);

// Same as before but you get to choose the index
char *RKGetBackgroundColorOfIndex(const int);

// Same as before but indexing in a cube-like coordinte, see rkutil -T1
char *RKGetBackgroundColorOfCubeIndex(const int);

//
// JSON (RadarKit 2)
//
// The following is a set of JSON functions that are *very* limited
// They are made for efficiency with many assumptions
//

// Extract a string into type, key, and vlaue
char *RKExtractJSON(char *ks, uint8_t *type, char *key, char *value);

// Get the value of a specific key
char *RKGetValueOfKey(const char *string, const char *key);

// Replace all values of a key
void RKReplaceAllValuesOfKey(char *string, const char *key, int value);

// Repalce enum of a key
void RKReplaceEnumOfKey(char *string, const char *key, int value);

// Revise logical values like "true" to true
void RKReviseLogicalValues(char *);

//
// JSON (RadarKit 3)
//
// The following is a set of JSON functions that are made to handle
// somewhat generic forms. More than previous verions but still a work
// in progress by Jan 2022. Will add functionality as the need arises
// Single quotes (') are used to denote the boundary of the char array
//

// Returns the pointer to the first non-space character of the input source
char *RKJSONSkipWhiteSpaces(const char *);

// Scan and copy a constituent until a delimiter
// Delimiter within double quotes, single quotes, or brackets are passed through
// The special combination \" within double quotes is passed through
// In the copy process, unnecessary empty spaces are eliminated, except those within quotes
char *RKJSONScanPassed(char *, const char *, const char);

// Get the current element of an array
char *RKJSONGetElement(char *, const char *);

// Extract a like '"key": "value"' string into key string and value string
// e.g., '"name": "startgate"' -> key = 'name' (no double quotes), value = '"stargate"'
char *RKJSONKeyValueFromElement(char *, char *, const char *);

// Scan a source to an element where the key == name and return the position in source as
// (char *) memories keyValue, key, and value must be provided to hold expected output
// The provided key name is case insensitive
// e.g., source: '{"tag": "shoe", "id": 123}', name = 'tag' produces
//       output: keyValue = '"tag": "show"', key = 'tag', value = '"shoe"'
// e.g., source: = {"tag": "shoe", "id": 123},
//       output: keyValue = '"id": 123', key = 'id', value = '123'
char *RKJSONGetValueOfKey(char *keyValueString, char *key, char *value, const char *name, const char *source);

//
//
//

// Returns a string of an unsigned long long with thousands separator
char *RKUIntegerToCommaStyleString(const unsigned long long);

// Returns a string of a long long with thousands separator
char *RKIntegerToCommaStyleString(const long long);

// Returns a string of hexadecimal representation
char *RKIntegerToHexStyleString(const long long);

// Returns a string of a double with thousands separator
char *RKFloatToCommaStyleString(const double);

//
//
//

// Returns a time string now
char *RKNow(void);

// Computes the time difference between to struct timeval
double RKTimevalDiff(const struct timeval minuend, const struct timeval subtrahend);

// Computes the time difference between to struct timespec
double RKTimespecDiff(const struct timespec minuend, const struct timespec subtrahend);

// Gets the current UTC time in struct timespec
void RKUTCTime(struct timespec *);

//
//
//

bool RKFilenameExists(const char *);
void RKPreparePath(const char *);
long RKCountFilesInPath(const char *);
char *RKFolderOfFilename(const char *);
char *RKFileExtension(const char *);
char *RKLastPartOfPath(const char *);
char *RKLastTwoPartsOfPath(const char *);
char *RKLastNPartsOfPath(const char *, const int n);
char *RKPathStringByExpandingTilde(const char *);
void RKReplaceFileExtension(char *filename, const char *pattern, const char *replacement);

//
//
//

char *RKSignalString(const int);

//
//
//

int RKStripTail(char *);
int RKUnquote(char *);
int RKIndentCopy(char *dst, char *src, const int width);
int RKStringCenterized(char *dst, const char *src, const int width);
char *RKNextNoneWhite(const char *);
char *RKLastLine(const char *);
char *RKStripEscapeSequence(const char *line);

//
//
//

float RKMinDiff(const float minuend, const float subtrahend);
float RKUMinDiff(const float minuend, const float subtrahend);
float RKModulo360Diff(const float minuend, const float subtrahend);
bool RKAngularCrossOver(const float a1, const float a2, const float crossover);

//
//
//

long RKGetCPUIndex(void);
long RKGetMemoryUsage(void);

//
//
//

char *RKCountryFromPosition(const double latitude, const double longitude);

//
//
//

char *RKGetNextKeyValue(char *json, char *key, char *value);
int RKMergeColumns(char *, const char *, const char *, const int);

//
//
//

char *RKBinaryString(char *dst, void *src, const size_t count);
void RKHeadTailBinaryString(char *dst, void *src, const size_t count);
char *RKBytesInHex(char *dst, void *src, const size_t count);
void RKHeadTailBytesInHex(char *dst, void *src, const size_t count);
void RKRadarHubPayloadString(char *dst, void *src, const size_t count);

//
//
//

char *RKStringLower(char *string);

#endif /* rk_misc_h */
