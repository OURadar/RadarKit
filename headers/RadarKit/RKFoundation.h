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
    char showColor;
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

void RKZeroOutIQZ(RKIQZ *data, const uint32_t capacity);

size_t RKPulseBufferAlloc(void **, const int, const int);
RKPulse *RKGetPulse(void *, const int);
RKInt16C *RKGetInt16CDataFromPulse(RKPulse *, const int);
RKComplex *RKGetComplexDataFromPulse(RKPulse *, const int);
RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *, const int);

size_t RKRayBufferAlloc(void **, const int, const int);
RKRay *RKGetRay(void *, const int);
int16_t *RKGetInt16DataFromRay(RKRay *, const int);
float *RKGetFloatDataFromRay(RKRay *, const int);

RKScratch *RKScratchInit(const size_t);
void RKScratchFree(RKScratch *);

#endif /* defined(__RadarKit_RKFoundation__) */
