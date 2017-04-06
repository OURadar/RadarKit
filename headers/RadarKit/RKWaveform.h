//
//  RKWaveform.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/19/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Waveform__
#define __RadarKit_Waveform__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

#define RKWaveformDefaultDepth       1024
#define RKWaveformDigitalAmplitude   32767.0

typedef uint32_t RKWaveformType;
enum RKWaveformType {
    RKWaveformTypeNone,
    RKWaveformTypeSingle,
    RKWaveformTypeFrequencyHopping,
    RKWaveformTypeTimeFrequencyMultiplexing
};

typedef struct rk_waveform {
    int             count;                                                 // Number of groups
    int             depth;                                                 // Maximum number of samples
    RKWaveformType  type;                                                  // Various type of waveforms
    RKComplex       *samples[RKMaxFilterGroups];                           // Samples up to amplitude of 1.0
    RKInt16C        *iSamples[RKMaxFilterGroups];                          // 16-bit full-scale equivalent of the waveforms
    uint32_t        filterCounts[RKMaxFilterGroups];                       // Number of filters to applied to each waveform, see filterAnchors
    RKFilterAnchor  filterAnchors[RKMaxFilterGroups][RKMaxFilterCount];    // Filter anchors of each sub-waveform for de-multiplexing
} RKWaveform;

RKWaveform *RKWaveformInitWithCountAndDepth(const int count, const int depth);
RKWaveform *RKWaveformInit(void);
void RKWaveformFree(RKWaveform *);

void RKWaveformOnes(RKWaveform *waveform);
void RKWaveformHops(RKWaveform *waveform, const double fs, const double bandwidth);
void RKWaveformConjuate(RKWaveform *waveform);
void RKWaveformDecimate(RKWaveform *waveform, const int decimate);
void RKWaveformRead(RKWaveform *waveform, const char *filename);

void RKWaveformCalculateGain(RKWaveform *waveform);

#endif
