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
void RKWaveformLinearFrequencyModulation(RKWaveform *, const double fs, const double fc, const double bandwidth);
void RKWaveformFrequencyHops(RKWaveform *, const double fs, const double fc, const double bandwidth);
void RKWaveformFrequencyHoppingChirp(RKWaveform *, const double fs, const double fc, const double bandwidth);

void RKWaveformDecimate(RKWaveform *, const int);
void RKWaveformConjuate(RKWaveform *);
void RKWaveformDownConvert(RKWaveform *);

void RKWaveformNormalizeNoiseGain(RKWaveform *);
void RKWaveformSummary(RKWaveform *);

size_t RKWaveformWriteToReference(RKWaveform *, FILE *);
RKResult RKWaveformWriteFile(RKWaveform *, const char *);
RKWaveform *RKWaveformReadFromReference(FILE *);

#endif
