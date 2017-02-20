//
//  RKWaveform.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/19/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKWaveform.h>

RKWaveform *RKWaveformInitWithCountAndDepth(const int count, const int depth) {
    RKWaveform *waveform = (RKWaveform *)malloc(sizeof(RKWaveform));
    memset(waveform, 0, sizeof(RKWaveform));
    if (count > RKMaxMatchedFilterGroupCount) {
        waveform->count = RKMaxMatchedFilterGroupCount;
        RKLog("Warning. Waveform count is clamped to %s\n", RKIntegerToCommaStyleString(waveform->count));
    } else {
        waveform->count = count;
    }
    waveform->depth = depth;
    for (int k = 0; k < count; k++) {
        waveform->samples[k] = (RKComplex *)malloc(waveform->depth * sizeof(RKComplex));
        waveform->iSamples[k] = (RKInt16C *)malloc(waveform->depth * sizeof(RKInt16C));
        if (waveform->samples[k] == NULL || waveform->iSamples[k] == NULL) {
            RKLog("Error. Unable to allocate memory.\n");
            exit(EXIT_FAILURE);
        }
        memset(waveform->samples, 0, waveform->depth * sizeof(RKComplex));
        memset(waveform->iSamples, 0, waveform->depth * sizeof(RKInt16C));
    }
    return waveform;
}

RKWaveform *RKWaveformInit() {
    return RKWaveformInitWithCountAndDepth(1, RKWaveformDefaultDepth);
}

void RKWaveformFree(RKWaveform *waveform) {
    int k;
    for (k = 0; k < waveform->count; k++) {
        free(waveform->samples[k]);
        free(waveform->iSamples[k]);
    }
    free(waveform);
}

void RKWaveformMakeHops(RKWaveform *waveform, const double fs, const double bandwidth) {
    int i, k;
    RKLog("Frequency Hoping @ %d hops over %s MHz", waveform->count, RKFloatToCommaStyleString(1.0e-6 * bandwidth));
    double stride = bandwidth / (double)(waveform->count - 1);
    waveform->type = RKWaveformTypeFrequencyHopping;
    for (k = 0; k < waveform->count; k++) {
        double f = stride * (double)k - 0.5 * bandwidth;
        RKLog(">f = %+.1f MHz", 1.0e-6 * f);
        double omega = 2.0 * M_PI * f / fs;
        RKComplex *x = waveform->samples[k];
        RKInt16C *y = waveform->iSamples[k];
        for (i = 0; i < waveform->depth; i++) {
            x->i = cos(omega);
            x->q = sin(omega);
            y->i = (int16_t)(32767 * x->i);
            y->q = (int16_t)(32767 * x->q);
            x++;
            y++;
        }
    }
}
