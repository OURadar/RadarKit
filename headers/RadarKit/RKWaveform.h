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
#define RKWaveformDigitalAmplitude   32000.0

// ----
//  File header
//  - name
//  - group count
//  - global depth
// ----
//
// ----
//  RKWaveFileGroup (0)
//  - type
//  - depth
//  - filter count
// ----
//
// ----
//  - (RKFilterAnchor) x filter count
//  - depth x [sizeof(RKComplex) + sizeof(RKIntC)]
// ---
//
// ----
//  RKWaveFileGroup (1)
//  - type
//  - depth
//  - filter count
// ----
//
// ----
//  - (RKFilterAnchor) x filter count
//  - depth x [sizeof(RKComplex) + sizeof(RKIntC)]
// ---
//

#pragma pack(push, 1)

typedef union rk_wave_file_header {
    struct {
        char            name[256];
        uint8_t         groupCount;
        uint32_t        depth;
        double          fc;
        double          fs;
    };
    char bytes[512];
} RKWaveFileHeader;

typedef union rk_wave_file_group {
    struct {
        RKWaveformType  type;
        uint32_t        depth;
        uint32_t        filterCounts;
    };
    char bytes[32];
} RKWaveFileGroup;

#pragma pack(pop)

RKWaveform *RKWaveformInitWithCountAndDepth(const int count, const int depth);
RKWaveform *RKWaveformInitFromFile(const char *filename);
RKWaveform *RKWaveformInit(void);
void RKWaveformFree(RKWaveform *);

RKWaveform *RKWaveformCopy(RKWaveform *);

RKWaveform *RKWaveformInitAsImpulse(void);
RKWaveform *RKWaveformInitAsSingleTone(const double fs, const double fc, const double pulsewidth);
RKWaveform *RKWaveformInitAsLinearFrequencyModulation(const double fs, const double fc, const double pulsewidth, const double bandwidth);
RKWaveform *RKWaveformInitAsFrequencyHops(const double fs, const double fc, const double pulsewidth, const double bandwidth, const int count);
RKWaveform *RKWaveformInitAsFakeTimeFrequencyMultiplexing(const double fs, const double bandwidth, const double stride, const int filterCount);
RKWaveform *RKWaveformInitAsTimeFrequencyMultiplexing(const double fs, const double fc, const double bandwidth, const double pulsewidth);
RKWaveform *RKWaveformInitAsFrequencyHoppingChirp(const double fs, const double fc, const double bandwidth, const double pulsewidth, const int count);

RKResult RKWaveformAppendWaveform(RKWaveform *, const RKWaveform *appendix, const uint32_t transitionSamples);
RKResult RKWaveformApplyWindow(RKWaveform *waveform, const RKWindowType type, ...);

void RKWaveformOnes(RKWaveform *);
void RKWaveformSingleTone(RKWaveform *, const double fs, const double fc);
void RKWaveformLinearFrequencyModulation(RKWaveform *, const double fs, const double fc, const double pulsewidth, const double bandwidth);
void RKWaveformFrequencyHops(RKWaveform *, const double fs, const double fc, const double bandwidth);
void RKWaveformFrequencyHoppingChirp(RKWaveform *, const double fs, const double fc, const double bandwidth);

void RKWaveformDecimate(RKWaveform *, const int);
void RKWaveformConjuate(RKWaveform *);
void RKWaveformDownConvert(RKWaveform *);

void RKWaveformWrite(RKWaveform *, const char *);
void RKWaveformNormalizeNoiseGain(RKWaveform *);
void RKWaveformSummary(RKWaveform *);

#endif
