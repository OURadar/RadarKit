//
//  RKFoundation.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_Foundation__
#define __RadarKit_Foundation__

#include <RadarKit/RKTypes.h>
#include <RadarKit/RKMisc.h>

#define RKDefaultLogfile                 "messages.log"
#define RKEOL                            "\r\n"

// Compute the next/previous N-stride location with size S
#define RKNextNModuloS(i, N, S)          ((i) >= (S) - (N) ? (i) + (N) - (S) : (i) + (N))
#define RKPreviousNModuloS(i, N, S)      ((i) < (N) ? (S) - (N) + (i) : (i) - (N))

// Compute the next/previous location with size S
#define RKNextModuloS(i, S)              ((i) == (S) - 1 ? 0 : (i) + 1)
#define RKPreviousModuloS(i, S)          ((i) == 0 ? (S) - 1 : (i) - 1)

typedef struct RKGlobalParameterStruct {
    char             program[RKNameLength];                      // Name of the program in log
    char             logfile[RKNameLength];                      // Name of the log file. This is ignored when dailyLog = true
    char             rootDataFolder[RKNameLength];               // Root folder where iq, moment health and log files are stored
    bool             dailyLog;                                   // Daily mode where log file is /{rootDataFolder}/log/YYYYMMDD.log
    bool             showColor;                                  // Show colors
    pthread_mutex_t  mutex;                                      // Mutual exclusive access
    FILE             *stream;                                    // Secondary output stream, can be NULL
} RKGlobalParamters;

extern RKGlobalParamters rkGlobalParameters;

typedef uint32_t RKValueType;
enum RKValueType {
    RKValueTypeInt32,
    RKValueTypeUInt32,
    RKValueTypeFloat,
    RKValueTypeDouble,
    RKValueTypeString
};

#pragma mark -

int RKLog(const char *, ...);

void RKSetWantColor(const bool);
void RKSetWantScreenOutput(const bool);
void RKSetUseDailyLog(const bool);
int RKSetProgramName(const char *);
int RKSetRootFolder(const char *);
int RKSetLogfile(const char *);
int RKSetLogfileToDefault(void);

void RKShowTypeSizes(void);
void RKShowVecFloat(const char *name, const float *p, const int n);
void RKShowVecIQZ(const char *name, const RKIQZ *p, const int n);

void RKZeroOutFloat(RKFloat *data, const uint32_t capacity);
void RKZeroOutIQZ(RKIQZ *data, const uint32_t capacity);
void RKZeroTailFloat(RKFloat *data, const uint32_t capacity, const uint32_t origin);
void RKZeroTailIQZ(RKIQZ *data, const uint32_t capacity, const uint32_t origin);

size_t RKPulseBufferAlloc(RKBuffer *, const uint32_t capacity, const uint32_t pulseCount);
void RKPulseBufferFree(RKBuffer);
RKPulse *RKGetPulse(RKBuffer, const uint32_t pulseIndex);
RKInt16C *RKGetInt16CDataFromPulse(RKPulse *, const uint32_t channelIndex);
RKComplex *RKGetComplexDataFromPulse(RKPulse *, const uint32_t channelIndex);
RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *, const uint32_t channelIndex);

size_t RKRayBufferAlloc(RKBuffer *, const uint32_t capacity, const uint32_t rayCount);
void RKRayBufferFree(RKBuffer);
RKRay *RKGetRay(RKRay *, const uint32_t rayIndex);
uint8_t *RKGetUInt8DataFromRay(RKRay *, const uint32_t productIndex);
float *RKGetFloatDataFromRay(RKRay *, const uint32_t productIndex);

size_t RKScratchAlloc(RKScratch **space, const uint32_t capacity, const uint8_t lagCount, const bool);
void RKScratchFree(RKScratch *);

void RKParseCommaDelimitedValues(void *, RKValueType , const size_t, const char *);
void RKParseQuotedStrings(const char *source, ...);
void RKMakeJSONStringFromControls(char *, RKControl *, uint32_t count);

RKFileMonitor *RKFileMonitorInit(const char *filename, void (*)(void *));
int RKFileMonitorFree(RKFileMonitor *engine);

#endif /* defined(__RadarKit_RKFoundation__) */
