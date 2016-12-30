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

#define RKEOL                            "\r\n"

// Compute the next/previous location in Buffer0
#define RKNextBuffer0Slot(i)             ((i) == (RKBuffer0SlotCount - 1) ? 0 : (i) + 1)
#define RKPreviousBuffer0Slot(i)         ((i) == 0 ? (RKBuffer0SlotCount - 1) : (i) - 1)

// Compute the next/previous N-stride location in Buffer0
#define RKNextNBuffer0Slot(i, N)         ((i) >= RKBuffer0SlotCount - (N) ? (i) + (N) - RKBuffer0SlotCount : (i) + (N))
#define RKPreviousNBuffer0Slot(i, N)     ((i) < (N) ? RKBuffer0SlotCount - (N) + (i) : (i) - (N))

// Compute the next/previous N-stride location with size S
#define RKNextNModuloS(i, N, S)          ((i) >= (S) - (N) ? (i) + (N) - (S) : (i) + (N))
#define RKPreviousNModuloS(i, N, S)      ((i) < (N) ? (S) - (N) + (i) : (i) - (N))

// Compute the next/previous location with size S
#define RKNextModuloS(i, S)              ((i) == (S) - 1 ? 0 : (i) + 1)
#define RKPreviousModuloS(i, S)          ((i) == 0 ? (S) - 1 : (i) - 1)



void stripTrailingUnwanted(char *str);

size_t RKPulseBufferAlloc(void **, const int, const int);
RKPulse *RKGetPulse(void *, const int);
RKInt16C *RKGetInt16DataFromPulse(RKPulse *, const int);
RKComplex *RKGetComplexDataFromPulse(RKPulse *, const int);
RKIQZ RKGetSplitComplexDataFromPulse(RKPulse *, const int);

size_t RKRayBufferAlloc(void **, const int, const int);
RKRay *RKGetRay(void *, const int);
int16_t *RKGetInt16DataFromRay(RKRay *, const int);
float *RKGetFloatDataFromRay(RKRay *, const int);

RKScratch *RKScratchInit(const size_t);
void RKScratchFree(RKScratch *);

#endif /* defined(__RadarKit_RKFoundation__) */
