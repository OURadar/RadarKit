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
	RKWaveformTypeLinearFrequencyModulation,
    RKWaveformTypeTimeFrequencyMultiplexing
};

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
        char            name[RKNameLength];
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

typedef struct rk_waveform {
    int             count;                                                 // Number of groups
    int             depth;                                                 // Maximum number of samples
    double          fc;                                                    // Carrier frequency (Hz)
    double          fs;                                                    // Sampling frequency (Hz)
    RKWaveformType  type;                                                  // Various type of waveforms
    char            name[RKNameLength];                                    // Waveform name in plain string
    RKComplex       *samples[RKMaxFilterGroups];                           // Samples up to amplitude of 1.0
    RKInt16C        *iSamples[RKMaxFilterGroups];                          // 16-bit full-scale equivalence of the waveforms
    uint32_t        filterCounts[RKMaxFilterGroups];                       // Number of filters to applied to each waveform, see filterAnchors
    RKFilterAnchor  filterAnchors[RKMaxFilterGroups][RKMaxFilterCount];    // Filter anchors of each sub-waveform for de-multiplexing
} RKWaveform;

RKWaveform *RKWaveformInitWithCountAndDepth(const int count, const int depth);
RKWaveform *RKWaveformInitFromFile(const char *filename);
RKWaveform *RKWaveformInit(void);
void RKWaveformFree(RKWaveform *);

RKWaveform *RKWaveformInitAsTimeFrequencyMultiplexing(const double fs, const double bandwidth, const double stride, const int filterCount);
RKWaveform *RKWaveformInitAsLinearFrequencyModulation(const double fs, const double fc, const double pulsewidth, const double bandwidth);

void RKWaveformOnes(RKWaveform *waveform);
void RKWaveformHops(RKWaveform *waveform, const double fs, const double fc, const double bandwidth);
void RKWaveformLinearFrequencyModulation(RKWaveform *waveform, const double fs, const double fc, const double pulsewidth, const double bandwidth);

void RKWaveformConjuate(RKWaveform *waveform);
void RKWaveformDecimate(RKWaveform *waveform, const int decimate);
void RKWaveformDownConvert(RKWaveform *waveform, const double omega);

void RKWaveformWrite(RKWaveform *waveform, const char *filename);
void RKWaveformNormalizeNoiseGain(RKWaveform *waveform);
void RKWaveformCalculateGain(RKWaveform *waveform);
void RKWaveformSummary(RKWaveform *waveform);

#endif
