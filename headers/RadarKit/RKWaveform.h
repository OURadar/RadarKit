//
//  RKWaveform.h
//  RadarKit
//
//  Created by Boonleng Cheong on 2/19/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
//

#ifndef __RadarKit_Waveform__
#define __RadarKit_Waveform__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKDSP.h>

#define RKWaveformDefaultDepth       1024
#define RKWaveformDigitalAmplitude   32000.0

RKWaveform *RKWaveformInitWithCountAndDepth(const uint8_t, const uint32_t);
RKWaveform *RKWaveformInitFromSamples(const RKComplex *samples, const uint32_t depth, const char * _Nullable name);
RKWaveform *RKWaveformInitFromSampleArrays(const RKComplex **samples, const uint8_t count, const uint32_t depth, const char * _Nullable name);
RKWaveform *RKWaveformInitFromFile(const char *);
RKWaveform *RKWaveformInit(void);
void RKWaveformFree(RKWaveform *);

RKWaveform *RKWaveformCopy(RKWaveform *);

RKWaveform *RKWaveformInitAsImpulse(void);
RKWaveform *RKWaveformInitAsSingleTone(const double fs, const double fc, const double pw);
RKWaveform *RKWaveformInitAsLinearFrequencyModulation(const double fs, const double fc, const double pw, const double bw);
RKWaveform *RKWaveformInitAsFrequencyHops(const double fs, const double fc, const double pw, const double bw, const int hops);
RKWaveform *RKWaveformInitAsFakeTimeFrequencyMultiplexing(void);
RKWaveform *RKWaveformInitAsTimeFrequencyMultiplexing(const double fs, const double fc, const double bw, const double pw);
RKWaveform *RKWaveformInitAsFrequencyHoppingChirp(const double fs, const double fc, const double bw, const double pw, const int count);
RKWaveform *RKWaveformInitFromString(const char *);

RKResult RKWaveformAppendWaveform(RKWaveform *, const RKWaveform *appendix, const uint32_t transitionSamples);
RKResult RKWaveformApplyWindow(RKWaveform *waveform, const RKWindowType type, ...);

void RKWaveformOnes(RKWaveform *);
void RKWaveformSingleTone(RKWaveform *, const double fs, const double fc);
void RKWaveformLinearFrequencyModulation(RKWaveform *, const double fs, const double fc, const double bw);
void RKWaveformFrequencyHops(RKWaveform *, const double fs, const double fc, const double bw);
void RKWaveformFrequencyHoppingChirp(RKWaveform *, const double fs, const double fc, const double bw);

void RKWaveformDecimate(RKWaveform *, const int);
void RKWaveformConjugate(RKWaveform *);
void RKWaveformDownConvert(RKWaveform *);

void RKWaveformNormalizeNoiseGain(RKWaveform *);
void RKWaveformSummary(RKWaveform *);

size_t RKWaveformWriteToReference(RKWaveform *, FILE *);
RKResult RKWaveformWriteFile(RKWaveform *, const char *);
RKWaveform *RKWaveformReadFromReference(FILE *);

#endif
