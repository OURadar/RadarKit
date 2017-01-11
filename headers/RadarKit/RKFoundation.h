//
//  RKFoundation.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKFoundation__
#define __RadarKit_RKFoundation__

#include <RadarKit/RKTypes.h>
#include <RadarKit/RKMisc.h>
#include <RadarKit/RKSIMD.h>
#include <RadarKit/RKClock.h>

#define RKDefaultLogfile                 "messages.log"
#define RKEOL                            "\r\n"

// Compute the next/previous N-stride location with size S
#define RKNextNModuloS(i, N, S)          ((i) >= (S) - (N) ? (i) + (N) - (S) : (i) + (N))
#define RKPreviousNModuloS(i, N, S)      ((i) < (N) ? (S) - (N) + (i) : (i) - (N))

// Compute the next/previous location with size S
#define RKNextModuloS(i, S)              ((i) == (S) - 1 ? 0 : (i) + 1)
#define RKPreviousModuloS(i, S)          ((i) == 0 ? (S) - 1 : (i) - 1)

typedef struct RKGlobalParameterStruct {
    char program[RKMaximumStringLength];
    char logfile[RKMaximumStringLength];
    bool showColor;
    FILE *stream;
} RKGlobalParamters;

extern RKGlobalParamters rkGlobalParameters;

#pragma mark -

int RKLog(const char *, ...);

void RKSetWantColor(const bool);
void RKSetWantScreenOutput(const bool);
int RKSetProgramName(const char *);
int RKSetLogfile(const char *);
int RKSetLogfileToDefault(void);

void RKShowTypeSizes(void);
void RKShowVecFloat(const char *name, const float *p, const int n);
void RKShowVecIQZ(const char *name, const RKIQZ *p, const int n);

void RKZeroOutFloat(RKFloat *data, const uint32_t capacity);
void RKZeroOutIQZ(RKIQZ *data, const uint32_t capacity);
void RKZeroTailFloat(RKFloat *data, const uint32_t capacity, const uint32_t origin);
void RKZeroTailIQZ(RKIQZ *data, const uint32_t capacity, const uint32_t origin);

size_t RKPulseBufferAlloc(RKBuffer *, const uint32_t, const uint32_t);
RKPulse *RKGetPulse(RKBuffer, const uint32_t);
RKInt16C *RKGetInt16CDataFromPulse(RKPulse *, const uint32_t);
RKComplex *RKGetComplexDataFromPulse(RKPulse *, const uint32_t);
RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *, const uint32_t);

size_t RKRayBufferAlloc(RKBuffer *, const uint32_t, const uint32_t);
RKRay *RKGetRay(RKRay *, const uint32_t);
uint8_t *RKGetUInt8DataFromRay(RKRay *, const uint32_t);
float *RKGetFloatDataFromRay(RKRay *, const uint32_t);

size_t RKScratchAlloc(RKScratch **, const uint32_t, const uint8_t, const bool);
void RKScratchFree(RKScratch *);

#endif /* defined(__RadarKit_RKFoundation__) */
