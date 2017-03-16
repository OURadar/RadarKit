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

#define RKWaveformDefaultDepth       1024
#define RKWaveformDigitalAmplitude   32767.0

typedef uint32_t RKWaveformType;
enum RKWaveformType {
    RKWaveformTypeNone,
    RKWaveformTypeSingle,
    RKWaveformTypeFrequencyHopping,
    RKWaveformTypeTimeFrequencyMultiplex
};

typedef struct rk_waveform {
    int             count;                                      // Number of groups
    int             depth;                                      // Maximum number of samples
    RKWaveformType  type;                                       // Various type of waveforms
    int             name[RKMaxFilterGroups];                    // Name of the sub-carrier
    double          omega[RKMaxFilterGroups];                   // Sub-carrier of RKWaveformTypeFrequencyHopping
    RKComplex       *samples[RKMaxFilterGroups];                // Samples up to amplitude of 1.0
    RKInt16C        *iSamples[RKMaxFilterGroups];               // 16-bit full-scale equivalent of the waveforms
} RKWaveform;

RKWaveform *RKWaveformInitWithCountAndDepth(const int count, const int depth);
RKWaveform *RKWaveformInit(void);
void RKWaveformFree(RKWaveform *);

void RKWaveformOnes(RKWaveform *waveform);
void RKWaveformHops(RKWaveform *waveform, const double fs, const double bandwidth);
void RKWaveformConjuate(RKWaveform *waveform);
void RKWaveformDecimate(RKWaveform *waveform, const int decimate);
void RKWaveformRead(RKWaveform *waveform, const char *filename);

#endif
