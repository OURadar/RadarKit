//
//  RKWaveform.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/19/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKWaveform.h>

RKWaveform *RKWaveformInitWithCountAndDepth(const int count, const int depth) {
    int k;
    RKWaveform *waveform = (RKWaveform *)malloc(sizeof(RKWaveform));
    memset(waveform, 0, sizeof(RKWaveform));
    waveform->count = count;
    waveform->depth = depth;
    if (count > RKMaxMatchedFilterGroupCount) {
        waveform->count = RKMaxMatchedFilterGroupCount;
        RKLog("Warning. Waveform count is clamped to %s\n", RKIntegerToCommaStyleString(waveform->count));
    }
    for (k = 0; k < waveform->count; k++) {
        waveform->samples[k] = (RKComplex *)malloc(waveform->depth * sizeof(RKComplex));
        waveform->iSamples[k] = (RKInt16C *)malloc(waveform->depth * sizeof(RKInt16C));
        if (waveform->samples[k] == NULL || waveform->iSamples[k] == NULL) {
            RKLog("Error. Unable to allocate memory.\n");
            exit(EXIT_FAILURE);
        }
        memset(waveform->samples[k], 0, waveform->depth * sizeof(RKComplex));
        memset(waveform->iSamples[k], 0, waveform->depth * sizeof(RKInt16C));
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

//
// This is actually hop pairs: f0, f0, f1, f1, f2, f2, ...
//
void RKWaveformMakeHops(RKWaveform *waveform, const double fs, const double bandwidth) {
    int i, k;
    double f, omega;
    RKComplex *x;
    RKInt16C *w;

    waveform->type = RKWaveformTypeFrequencyHopping;

    double stride = bandwidth / (double)((waveform->count / 2) - 1);

    for (k = 0; k < waveform->count; k++) {
        f = stride * (double)(k / 2) - 0.5 * bandwidth;
        omega = 2.0 * M_PI * f / fs;
        //RKLog(">f[%d] = %+.1f MHz   omega = %.3f", k, 1.0e-6 * f, omega);
        x = waveform->samples[k];
        w = waveform->iSamples[k];
        for (i = 0; i < waveform->depth; i++) {
            x->i = cos(omega * i);
            x->q = sin(omega * i);
            w->i = (int16_t)(32767.0 * x->i);
            w->q = (int16_t)(32767.0 * x->q);
            x++;
            w++;
        }
    }
}
