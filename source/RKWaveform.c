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
    if (count > RKMaxFilterGroups) {
        waveform->count = RKMaxFilterGroups;
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

RKWaveform *RKWaveformInitFromFile(const char *filename) {
    int j, k;
    FILE *fid = fopen(filename, "r");
    if (fid == NULL) {
        RKLog("Error. Unable to read wave file %s\n", filename);
        return NULL;
    }
    RKWaveFileHeader fileHeader;
    RKWaveFileGroup waveGroup;
    fread(&fileHeader, sizeof(RKWaveFileHeader), 1, fid);
    
    RKLog(">Waveform '%s'   groupCount = %d   depth = %d\n", fileHeader.name, fileHeader.groupCount, fileHeader.depth);
    
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(fileHeader.groupCount, fileHeader.depth);
    //
    for (k = 0; k < fileHeader.groupCount; k++) {
        fread(&waveGroup, sizeof(RKWaveFileGroup), 1, fid);
        if (waveform->depth < waveGroup.depth) {
            RKLog("Error. Unable to fit waveform %s into supplied buffer. (%d vs %d)\n", filename, waveform->depth, waveGroup.depth);
            return NULL;
        }
        waveform->type = waveGroup.type;
        waveform->filterCounts[k] = waveGroup.filterCounts;
        fread(waveform->filterAnchors[k], sizeof(RKFilterAnchor), waveform->filterCounts[k], fid);
        for (j = 0; j < waveGroup.filterCounts; j++) {
            RKLog("> - Filter[%2d][%d/%d] @ %d %+6.3f\n", k, j, waveGroup.filterCounts, waveGroup.depth, waveform->filterAnchors[k][j].subCarrierFrequency);
        }
        fread(waveform->samples[k], sizeof(RKComplex), waveform->depth, fid);
        fread(waveform->iSamples[k], sizeof(RKInt16C), waveform->depth, fid);
    }
    fclose(fid);
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

#pragma mark - Waveforms

void RKWaveformOnes(RKWaveform *waveform) {
    int i, k;
    RKComplex *x;
    RKInt16C *w;

    waveform->type = RKWaveformTypeSingle;
    double amplitudeScale = 1.0 / sqrt((double)waveform->depth);

    for (k = 0; k < waveform->count; k++) {
        x = waveform->samples[k];
        w = waveform->iSamples[k];
        for (i = 0; i < waveform->depth; i++) {
            w->i = (int16_t)RKWaveformDigitalAmplitude;
            w->q = 0;
            x->i = amplitudeScale;
            x->q = 0.0f;
            x++;
            w++;
        }
    }
}

//
// This is actually hop pairs: f0, f0, f1, f1, f2, f2, ...
//
void RKWaveformHops(RKWaveform *waveform, const double fs, const double bandwidth) {
    int i, k, n;
    double f, omega, psi;
    RKComplex *x;
    RKInt16C *w;
    RKFloat gain;

    const bool sequential = false;

    waveform->type = RKWaveformTypeFrequencyHopping;

    const double delta = waveform->count <= 2 ? 0.0 : bandwidth / (double)((waveform->count / 2) - 1);

    int stride = RKBestStrideOfHops(waveform->count / 2, false);

    n = 0;
    for (k = 0; k < waveform->count; k++) {
        f = delta * (double)n - 0.5 * bandwidth;
        omega = 2.0 * M_PI * f / fs;
        psi = omega * (double)(waveform->depth / 2);
        waveform->filterCounts[k] = 1;
        waveform->filterAnchors[k][0].name = n;
        waveform->filterAnchors[k][0].origin = 0;
        waveform->filterAnchors[k][0].length = waveform->depth;
        waveform->filterAnchors[k][0].maxDataLength = RKGateCount;  // Replace with actual depth later
        waveform->filterAnchors[k][0].subCarrierFrequency = omega;
        //RKLog(">f[%d] = %+.1f MHz   omega = %.3f", k, 1.0e-6 * f, waveform->omega);
        x = waveform->samples[k];
        w = waveform->iSamples[k];
        gain = 0.0f;
        for (i = 0; i < waveform->depth; i++) {
            x->i = cos(omega * i - psi);
            x->q = sin(omega * i - psi);
            w->i = (int16_t)(RKWaveformDigitalAmplitude * x->i);
            w->q = (int16_t)(RKWaveformDigitalAmplitude * x->q);
            gain += (x->i * x->i + x->q * x->q);
            x++;
            w++;
        }
        waveform->filterAnchors[k][0].gain = gain;
        // This equation still needs to be checked.
        gain = sqrtf(gain);
        x = waveform->samples[k];
        for (i = 0; i < waveform->depth; i++) {
            x->i /= gain;
            x->q /= gain;
            x++;
        }
        // Get ready for the next frequency when we are odd index
        if (k % 2 == 1) {
            if (sequential) {
                n = ((k + 1) / 2);
            } else {
                n = RKNextNModuloS(n, stride, waveform->count / 2);
            }
        }
    }
}

RKWaveform *RKWaveformTimeFrequencyMultiplexing(const double fs, const double bandwidth, const double stride, const int filterCount) {
    int i, j;
    const int longPulseWidth = 400;
    const int shortPulseWidth = 25;
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, longPulseWidth + shortPulseWidth);
    
    waveform->type = RKWaveformTypeTimeFrequencyMultiplexing;
    
    waveform->count = 1;
    waveform->filterCounts[0] = 2;
    
    // Long pulse
    waveform->filterAnchors[0][0].name = 0;
    waveform->filterAnchors[0][0].origin = longPulseWidth;
    waveform->filterAnchors[0][0].length = longPulseWidth;
    waveform->filterAnchors[0][0].maxDataLength = RKGateCount;
    waveform->filterAnchors[0][0].subCarrierFrequency = -0.25f;
    
    // Short pulse
    waveform->filterAnchors[0][1].name = 1;
    waveform->filterAnchors[0][1].origin = 0;
    waveform->filterAnchors[0][1].length = shortPulseWidth;
    waveform->filterAnchors[0][1].maxDataLength = longPulseWidth + shortPulseWidth;
    waveform->filterAnchors[0][1].subCarrierFrequency = +0.25f;
    
    RKFloat a;
    RKInt16C *w;
    RKComplex *x;
    for (j = 0; j < waveform->filterCounts[0]; j++) {
        a = 2.0f / sqrtf(waveform->filterAnchors[0][j].length);
        x = &waveform->samples[0][waveform->filterAnchors[0][j].origin];
        w = &waveform->iSamples[0][waveform->filterAnchors[0][j].origin];
        RKLog("> - Filter[0][%d] - %s\n", j, RKIntegerToCommaStyleString(waveform->filterAnchors[0][j].length));
        const float omega = 2.0f * M_PI * waveform->filterAnchors[0][j].subCarrierFrequency;
        for (i = 0; i < waveform->filterAnchors[0][j].length; i++) {
            x->i = a * cosf(omega * i);
            x->q = a * sinf(omega * i);
            x++;
            w->i = (int16_t)(RKWaveformDigitalAmplitude * x->i);
            w->q = (int16_t)(RKWaveformDigitalAmplitude * x->q);
            w++;
        }
    }
    
    return waveform;
}

void RKWaveformConjuate(RKWaveform *waveform) {
    int i, k;
    RKComplex *x;
    RKInt16C *w;
    for (k = 0; k < waveform->count; k++) {
        x = waveform->samples[k];
        w = waveform->iSamples[k];
        for (i = 0; i < waveform->depth; i++) {
            x->q = -x->q;
            w->q = -w->q;
            x++;
            w++;
        }
    }
}

void RKWaveformDecimate(RKWaveform *waveform, const int stride) {
    int i, j, k;
    waveform->depth /= stride;
    RKComplex *x;
    RKInt16C *w;
    RKFloat gainAdjust = 1.0f / (RKFloat)stride;
    for (k = 0; k < waveform->count; k++) {
        waveform->filterAnchors[k][0].origin /= stride;
        waveform->filterAnchors[k][0].length /= stride;
        waveform->filterAnchors[k][0].gain /= stride;
        x = waveform->samples[k];
        w = waveform->iSamples[k];
        for (j = 0, i = 0; j < waveform->depth; j++, i += stride) {
            x[j].i = gainAdjust * x[i].i;
            x[j].q = gainAdjust * x[i].q;
            w[j] = w[i];
        }
    }
}

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
//  - depth (may be different in the future)
//  - filter count
// ----
//
// ----
//  - (RKFilterAnchor) x filter count
//  - depth x sizeof(RKComplex)
//  - depth x sizeof(RKInt16C)
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
//  - depth x sizeof(RKComplex)
//  - depth x sizeof(RKInt16C)
// ---
//

void RKWaveformWrite(RKWaveform *waveform, const char *filename) {
    int k;
    RKWaveFileHeader fileHeader;
    RKWaveFileGroup groupHeader;

    memset(&fileHeader, 0, sizeof(RKWaveFileHeader));
    memset(&groupHeader, 0, sizeof(RKWaveFileGroup));

    // Get the path created if it doesn't exist
    RKPreparePath(filename);
    FILE *fid = fopen(filename, "w");
    if (fid == NULL) {
        RKLog("Error. Unable to write wave file %s\n", filename);
        return;
    }
    char *lastPart = strchr(filename, '/');
    if (lastPart == NULL) {
        strcpy(fileHeader.name, filename);
    } else {
        strcpy(fileHeader.name, lastPart + 1);
    }
    // File header
    fileHeader.groupCount = waveform->count;
    fileHeader.depth = waveform->depth;
    fwrite(&fileHeader, sizeof(RKWaveFileHeader), 1, fid);
    for (k = 0; k < waveform->count; k++) {
        // Group header
        groupHeader.type = waveform->type;
        groupHeader.depth = waveform->depth;
        groupHeader.filterCounts = waveform->filterCounts[k];
        fwrite(&groupHeader, sizeof(RKWaveFileGroup), 1, fid);
        // Filter anchors
        fwrite(waveform->filterAnchors[k], sizeof(RKFilterAnchor), groupHeader.filterCounts, fid);
        for (int j = 0; j < waveform->filterCounts[k]; j++) {
            RKLog("> - Filter[%2d][%d/%d] @ %s %+6.3f\n",
                  k, j, groupHeader.filterCounts, RKIntegerToCommaStyleString(groupHeader.depth), waveform->filterAnchors[k][j].subCarrierFrequency);
        }
        // Waveform samples
        fwrite(waveform->samples[k], sizeof(RKComplex), groupHeader.depth, fid);
        fwrite(waveform->iSamples[k], sizeof(RKInt16C), groupHeader.depth, fid);
    }
    fclose(fid);
}

void RKWaveformCalculateGain(RKWaveform *waveform) {
    // Normalize the peak to get the peak transmit power
    int i, j, k;
    for (k = 0; k < waveform->count; k++) {
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            RKComplex *w = waveform->samples[k] + waveform->filterAnchors[k][j].origin;
            RKFloat maxSq = 0.0f;
            for (i = 0; i < waveform->filterAnchors[k][j].length; i++) {
                maxSq = MAX(maxSq, w->i * w->i + w->q * w->q);
                w++;
            }
            waveform->filterAnchors[k][j].gain = 1.0 / maxSq * waveform->filterCounts[k];
        }
    }
}
