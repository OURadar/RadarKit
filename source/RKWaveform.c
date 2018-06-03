//
//  RKWaveform.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/19/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKWaveform.h>

typedef uint8_t RKWaveformGain;
enum RKWaveformGain {
    RKWaveformGainNull          = 0,
    RKWaveformGainNoise         = 1,
    RKWaveformGainFullScale     = 1 << 1,
    RKWaveformGainSensitivity   = 1 << 2,
    RKWaveformGainAll          = (RKWaveformGainNoise | RKWaveformGainFullScale | RKWaveformGainSensitivity)
};

static void RKWaveformCalculateGain(RKWaveform *waveform, RKWaveformGain gainCalculationFlag) {
    // Calculate the noise gain
    int i, j, k;
    RKFloat a, f, g, h;
    RKComplex *w;
    for (i = 0; i < waveform->count; i++) {
        for (j = 0; j < waveform->filterCounts[i]; j++) {
            f = 0.0f;
            g = 0.0f;
            h = 0.0f;
            w = waveform->samples[i] + waveform->filterAnchors[i][j].origin;
            for (k = 0; k < waveform->filterAnchors[i][j].length; k++) {
                a = (w->i * w->i);
                if (waveform->type & RKWaveformTypeIsComplex) {
                    a += (w->q * w->q);
                    f = MAX(f, w->i * w->i + w->q * w->q);
                } else {
                    f = MAX(f, fabsf(w->i));
                }
                h = MAX(h, a);
                g += a;
                w++;
            }
            if (waveform->type & RKWaveformTypeIsComplex) {
                f = sqrtf(f);
            } else {
                // For waveforms that are not complex at this stage, the down-convert process doubles the noise gain later.
                g *= 2.0f;
            }
            if (gainCalculationFlag & RKWaveformGainFullScale) {
                waveform->filterAnchors[i][j].fullScale = (RKFloat)RKWaveformDigitalAmplitude / f;
            }
            if (gainCalculationFlag & RKWaveformGainNoise) {
                waveform->filterAnchors[i][j].filterGain = 10.0f * log10f(g);
            }
            if (gainCalculationFlag & RKWaveformGainSensitivity) {
                waveform->filterAnchors[i][j].sensitivityGain = 10.0f * log10f(g / (h * 1.0e-6 * waveform->fs));
            }
        }
    }
}

#pragma mark - Life Cycle

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
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&waveform->samples[k], RKSIMDAlignSize, waveform->depth * sizeof(RKComplex)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&waveform->iSamples[k], RKSIMDAlignSize, waveform->depth * sizeof(RKInt16C)));
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
    size_t r;
    FILE *fid = fopen(filename, "r");
    if (fid == NULL) {
        RKLog("Error. Unable to read wave file %s\n", filename);
        return NULL;
    }
    RKWaveFileHeader fileHeader;
    RKWaveFileGroup groupHeader;
    r = fread(&fileHeader, sizeof(RKWaveFileHeader), 1, fid);
    if (r == 0) {
        RKLog("Error. No file header from %s.\n", filename);
    }
    
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(fileHeader.groupCount, fileHeader.depth);
    //RKLog("fileHeader.groupCount = %d   fileHeader.depth = %d\n", fileHeader.groupCount, fileHeader.depth);

    waveform->fc = fileHeader.fc;
    waveform->fs = fileHeader.fs;
    strncpy(waveform->name, fileHeader.name, RKNameLength);
    
    for (k = 0; k < fileHeader.groupCount; k++) {
        // Read in the waveform of each group
        r = fread(&groupHeader, sizeof(RKWaveFileGroup), 1, fid);
        if (r == 0) {
            RKLog("Error. Failed reading group header from %s.\n", filename);
        }
        if (waveform->depth < groupHeader.depth) {
            RKLog("Error. Unable to fit %s into supplied buffer. (%s B < %s B)\n", filename,
                  RKIntegerToCommaStyleString(waveform->depth), RKIntegerToCommaStyleString(groupHeader.depth));
            RKWaveformFree(waveform);
            fclose(fid);
            return NULL;
        }
        waveform->type = groupHeader.type;
        waveform->filterCounts[k] = groupHeader.filterCounts;
        r = fread(waveform->filterAnchors[k], sizeof(RKFilterAnchor), waveform->filterCounts[k], fid);
        if (r == 0) {
            RKLog("Error. Unable to read filter anchors from %s\n", filename);
        }
        // Output data from the first filter is allowed up to the maximum
        waveform->filterAnchors[k][0].maxDataLength = RKGateCount;
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            if (waveform->filterAnchors[k][j].length > waveform->depth) {
                RKLog("Error. This waveform is invalid.  length = %s > depth %s\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].length),
                      RKIntegerToCommaStyleString(waveform->depth));
                RKWaveformFree(waveform);
                fclose(fid);
                return NULL;
            }
            if (waveform->filterAnchors[k][j].origin > RKGateCount) {
                RKLog("Error. This waveform is invalid.  origin = %s > %s (RKGateCount)\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].origin),
                      RKIntegerToCommaStyleString(RKGateCount));
                RKWaveformFree(waveform);
                fclose(fid);
                return NULL;
            }
            if (waveform->filterAnchors[k][j].inputOrigin > RKGateCount) {
                RKLog("Error. This waveform is invalid.  inputOrigin = %s > %s (RKGateCount)\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].inputOrigin),
                      RKIntegerToCommaStyleString(RKGateCount));
                RKWaveformFree(waveform);
                fclose(fid);
                return NULL;
            }
            if (waveform->filterAnchors[k][j].outputOrigin > RKGateCount) {
                RKLog("Error. This waveform is invalid.  outputOrigin = %s > %s (RKGateCount)\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].outputOrigin),
                      RKIntegerToCommaStyleString(RKGateCount));
                RKWaveformFree(waveform);
                fclose(fid);
                return NULL;
            }
        }
        r = fread(waveform->samples[k], sizeof(RKComplex), waveform->depth, fid);
        if (r == 0) {
            RKLog("Error. Failed reading complex samples from %s.\n", filename);
        }
        r = fread(waveform->iSamples[k], sizeof(RKInt16C), waveform->depth, fid);
        if (r == 0) {
            RKLog("Error. Frailed reading int16 samples from %s.\n", filename);
        }
    }
    fclose(fid);

    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
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

RKWaveform *RKWaveformCopy(RKWaveform *waveform) {
    int i, k;
    RKWaveform *waveformCopy = RKWaveformInitWithCountAndDepth(waveform->count, waveform->depth);
    waveformCopy->fc = waveform->fc;
    waveformCopy->fs = waveform->fs;
    waveformCopy->type = waveform->type;
    strcpy(waveformCopy->name, waveform->name);
    for (k = 0; k < waveform->count; k++) {
        memcpy(waveformCopy->samples[k], waveform->samples[k], waveform->depth * sizeof(RKComplex));
        memcpy(waveformCopy->iSamples[k], waveform->iSamples[k], waveform->depth * sizeof(RKInt16C));
        waveformCopy->filterCounts[k] = waveform->filterCounts[k];
        for (i = 0; i < waveformCopy->filterCounts[k]; i++) {
            memcpy(&waveformCopy->filterAnchors[k][i], &waveform->filterAnchors[k][i], sizeof(RKFilterAnchor));
        }
    }
    return waveformCopy;
}

RKWaveform *RKWaveformInitAsImpulse(void) {
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, 1);
    waveform->fs = 1.0;
    waveform->type = RKWaveformTypeSingleTone;
    sprintf(waveform->name, "P01");
    
    // Single filter
    waveform->filterCounts[0] = 1;
    
    // Everything simple
    waveform->filterAnchors[0][0].name = 0;
    waveform->filterAnchors[0][0].origin = 0;
    waveform->filterAnchors[0][0].length = 1;
    waveform->filterAnchors[0][0].inputOrigin = 0;
    waveform->filterAnchors[0][0].outputOrigin = 0;
    waveform->filterAnchors[0][0].maxDataLength = RKGateCount;
    waveform->filterAnchors[0][0].subCarrierFrequency = 0.0f;
    
    // Only a one. No need to set components that are 0's
    waveform->samples[0][0].i = 1.0f;
    waveform->iSamples[0][0].i = RKWaveformDigitalAmplitude;

    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
    return waveform;
}

RKWaveform *RKWaveformInitAsTimeFrequencyMultiplexing(const double fs, const double bandwidth, const double stride, const int filterCount) {
    int i, j;
    const uint32_t longPulseWidth = 300;
    const uint32_t shortPulseWidth = 10;
    const uint32_t transitionWidth = 30;
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, longPulseWidth + shortPulseWidth);
    
    waveform->fs = fs;
    waveform->type = RKWaveformTypeIsComplex | RKWaveformTypeTimeFrequencyMultiplexing;
    sprintf(waveform->name, "tfm-fake");
    
    // Two filters per waveform
    waveform->filterCounts[0] = 2;
    
    // Long pulse
    waveform->filterAnchors[0][0].name = 0;
    waveform->filterAnchors[0][0].origin = 0;
    waveform->filterAnchors[0][0].length = longPulseWidth;
    waveform->filterAnchors[0][0].inputOrigin = 0;
    waveform->filterAnchors[0][0].outputOrigin = longPulseWidth + shortPulseWidth + transitionWidth;
    waveform->filterAnchors[0][0].maxDataLength = RKGateCount - longPulseWidth - shortPulseWidth - transitionWidth;
    waveform->filterAnchors[0][0].subCarrierFrequency = 0.15f;
    
    // Short pulse
    waveform->filterAnchors[0][1].name = 1;
    waveform->filterAnchors[0][1].origin = longPulseWidth;
    waveform->filterAnchors[0][1].length = shortPulseWidth;
    waveform->filterAnchors[0][1].inputOrigin = longPulseWidth;
    waveform->filterAnchors[0][1].outputOrigin = 0;
    waveform->filterAnchors[0][1].maxDataLength = longPulseWidth + shortPulseWidth + transitionWidth;
    waveform->filterAnchors[0][1].subCarrierFrequency = 0.0f;
    
    RKFloat a;
    RKInt16C *w;
    RKComplex *x;
    for (j = 0; j < waveform->filterCounts[0]; j++) {
        a = 2.0f / sqrtf(waveform->filterAnchors[0][j].length);
        x = &waveform->samples[0][waveform->filterAnchors[0][j].origin];
        w = &waveform->iSamples[0][waveform->filterAnchors[0][j].origin];
        const float omega = 2.0f * M_PI * waveform->filterAnchors[0][j].subCarrierFrequency;
        for (i = 0; i < waveform->filterAnchors[0][j].length; i++) {
            x->i = a * cosf(omega * i);
            x->q = a * sinf(omega * i);
            w->i = (int16_t)(RKWaveformDigitalAmplitude * x->i);
            w->q = (int16_t)(RKWaveformDigitalAmplitude * x->q);
            x++;
            w++;
        }
    }
    
    RKWaveformNormalizeNoiseGain(waveform);
    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
    return waveform;
}

RKWaveform *RKWaveformInitAsLinearFrequencyModulation(const double fs, const double fc, const double pulsewidth, const double bandwidth) {
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, (uint32_t)round(pulsewidth * fs));
    RKWaveformLinearFrequencyModulation(waveform, fs, fc, pulsewidth, bandwidth);
    return waveform;
}

RKWaveform *RKWaveformInitAsFrequencyHops(const double fs, const double fc, const double pulsewidth, const double bandwidth, const int count) {
    uint32_t depth = (uint32_t)round(pulsewidth * fs);
    if (fabs(depth / fs - pulsewidth) / pulsewidth > 0.1) {
        RKLog("Info. Waveform depth %.2f us --> %.2f us due to fs = %.2f MHz.\n", 1.0e6 * pulsewidth, 1.0e6 * depth / fs, 1.0e-6 * fs);
    }
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(count == 1 ? 1 : 2 * count, depth);
    RKWaveformHops(waveform, fs, fc, bandwidth);
    return waveform;
}

#pragma mark - Waveforms

void RKWaveformOnes(RKWaveform *waveform) {
    int i, k;
    RKComplex *x;
    RKInt16C *w;

    waveform->type = RKWaveformTypeSingleTone;
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
    RKWaveformNormalizeNoiseGain(waveform);
    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
}

//
// This is actually hop pairs: f0, f0, f1, f1, f2, f2, ... around fc
//
void RKWaveformHops(RKWaveform *waveform, const double fs, const double fc, const double bandwidth) {
    int i, k, n;
    double f, omega, psi, pw;
    RKComplex *x;
    RKInt16C *w;

    const bool sequential = false;

    waveform->fs = fs;
    waveform->type = RKWaveformTypeIsComplex | RKWaveformTypeFrequencyHopping;
    k = sprintf(waveform->name, "h%02.0f%02d", 1.0e-6 * bandwidth, waveform->count == 1 ? 1 : waveform->count / 2);
    pw = 1.0e6 * waveform->depth / waveform->fs;
    if (pw < 1.0) {
        sprintf(waveform->name + k, ".%.0f", round(10.0 * pw));
    } else if (pw > 1.000001) {
        sprintf(waveform->name + k, "%.0f", pw);
    }

    const double delta = waveform->count <= 2 ? 0.0 : bandwidth / (double)((waveform->count / 2) - 1);

    // A good sequence can be achieved through a modulo sequence.
    int stride = RKBestStrideOfHops(waveform->count / 2, false);

    n = 0;
    for (k = 0; k < waveform->count; k++) {
        f = delta * (double)n - 0.5 * bandwidth + fc;
        omega = 2.0 * M_PI * f / fs;
        psi = omega * (double)(waveform->depth / 2);
        waveform->filterCounts[k] = 1;
        waveform->filterAnchors[k][0].name = n;
        waveform->filterAnchors[k][0].origin = 0;
        waveform->filterAnchors[k][0].length = waveform->depth;
        waveform->filterAnchors[k][0].maxDataLength = RKGateCount;   // Can be replaced with actual depth later
        waveform->filterAnchors[k][0].subCarrierFrequency = omega;
        //RKLog(">f[%d] = %+.1f MHz   omega = %.3f   n = %d", k, 1.0e-6 * f, omega, n);
        x = waveform->samples[k];
        w = waveform->iSamples[k];
        for (i = 0; i < waveform->depth; i++) {
            x->i = cos(omega * i - psi);
            x->q = sin(omega * i - psi);
            w->i = (int16_t)(RKWaveformDigitalAmplitude * x->i);
            w->q = (int16_t)(RKWaveformDigitalAmplitude * x->q);
            x++;
            w++;
        }
        // Get ready for the next frequency when we are in odd index
        if (k % 2 == 1) {
            if (sequential) {
                n = ((k + 1) / 2);
            } else {
                n = RKNextNModuloS(n, stride, waveform->count / 2);
            }
        }
    }

    RKWaveformNormalizeNoiseGain(waveform);
    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
}

void RKWaveformLinearFrequencyModulation(RKWaveform *waveform, const double fs, const double fc, const double pulsewidth, const double bandwidth) {
    int i;

    // Other parameters
    waveform->fc = fc;
    waveform->fs = fs;
    waveform->type = RKWaveformTypeIsComplex | RKWaveformTypeLinearFrequencyModulation;
    if (bandwidth == 0.0) {
        sprintf(waveform->name, "s%02d", (int)(1.0e6 * pulsewidth));
    } else if (bandwidth >= 1.0e6 && bandwidth < 100.0e6f) {
        sprintf(waveform->name, "q%02d", (int)(1.0e6 * pulsewidth));
    } else {
        sprintf(waveform->name, "lfm");
    }

    waveform->filterCounts[0] = 1;

    // Filter parameters
    waveform->filterAnchors[0][0].name = 0;
    waveform->filterAnchors[0][0].origin = 0;
    waveform->filterAnchors[0][0].length = waveform->depth;
    waveform->filterAnchors[0][0].inputOrigin = 0;
    waveform->filterAnchors[0][0].outputOrigin = 0;
    waveform->filterAnchors[0][0].maxDataLength = RKGateCount - waveform->depth;
    waveform->filterAnchors[0][0].subCarrierFrequency = 2.0f * M_PI * (fc + 0.5 * bandwidth) / fs;

    RKInt16C *w = waveform->iSamples[0];
    RKComplex *x = waveform->samples[0];
    double omega_c = 2.0 * M_PI * fc / fs;
    double k = 2.0 * M_PI * bandwidth / pulsewidth / (fs * fs);
    for (i = 0; i < waveform->depth; i++) {
        x->i = cosf(omega_c * i + 0.5 * k * i * i);
        x->q = sinf(omega_c * i + 0.5 * k * i * i);
        w->i = (int16_t)(RKWaveformDigitalAmplitude * x->i);
        w->q = (int16_t)(RKWaveformDigitalAmplitude * x->q);
        x++;
        w++;
    }
    RKWaveformNormalizeNoiseGain(waveform);
    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
    return;
}

#pragma mark - Generic Manipulation

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
    int i, j, k, l;
    waveform->fs /= stride;
    waveform->depth /= stride;
    RKComplex *x;
    RKInt16C *w;
    RKFloat gainAdjust = sqrtf(stride);
    for (l = 0; l < waveform->count; l++) {
        for (k = 0; k < waveform->filterCounts[l]; k++) {
            waveform->filterAnchors[l][k].origin /= stride;
            waveform->filterAnchors[l][k].length /= stride;
            waveform->filterAnchors[l][k].inputOrigin /= stride;
            waveform->filterAnchors[l][k].outputOrigin /= stride;
            // Account for the odd length ended in another range
            if (waveform->filterAnchors[l][k].maxDataLength % 2 != 0) {
                waveform->filterAnchors[l][k].maxDataLength++;
            }
            waveform->filterAnchors[l][k].maxDataLength /= stride;
        }
        x = waveform->samples[l];
        w = waveform->iSamples[l];
        for (j = 0, i = 0; j < waveform->depth; j++, i += stride) {
            x[j].i = gainAdjust * x[i].i;
            x[j].q = gainAdjust * x[i].q;
            w[j] = w[i];
        }
    }
    RKWaveformCalculateGain(waveform, RKWaveformGainNoise);
}

void RKWaveformDownConvert(RKWaveform *waveform) {
    int i, j, k;
    RKFloat a, f, g;
    RKFloat *w;
    RKComplex *s;

    int nfft = (int)powf(2.0f, ceilf(log2f((float)waveform->depth)));

    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&w, RKSIMDAlignSize, nfft * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&s, RKSIMDAlignSize, nfft * sizeof(RKComplex)));

    // Generate an SSB equivalent so that a real-valued function cos(omega t) becomes exp(i omage t)
    if (!(waveform->type & RKWaveformTypeIsComplex)) {
        for (i = 0; i < waveform->count; i++) {
            for (j = 0; j < waveform->depth; j++) {
                w[j] = waveform->samples[i][j].i;
            }
            memset(&w[j], 0, (nfft - j) * sizeof(RKFloat));
            // Technically, this is a function that generates X = Xi + j Xq from Xi
            RKHilbertTransform(w, waveform->samples[i], waveform->depth);
        }
        waveform->type |= RKWaveformTypeIsComplex;
    }

    // Demodulation tone
    const RKFloat omega = -2.0f * M_PI * waveform->fc / waveform->fs;
    const RKFloat m = 1.0f;
    for (j = 0; j < waveform->depth; j++) {
        // There is another convention to flip the sign: u[i].q = -u[i].q;
        s[j].i = m * cosf(omega * j);
        s[j].q = m * sinf(omega * j);
    }

    RKInt16C *ic;
    RKComplex *fc;

    // Copy over the float samples to int16_t samples and adjust the amplitude so that it represents the intended transmit envelope
    // This adjustment will not produce the transmit waveform that takes full advantage of the DAC range
    // The normalization factor to get the waveform to unity noise gain is no longer known here.
    // All filters should have a unity noise gain so an equivalent sensitivity gain vs a 1-us pulse can be derived.
    for (i = 0; i < waveform->count; i++) {
        // Go through the waveform samples (filters)
        RKSIMD_iymul(s, waveform->samples[i], waveform->depth);
        float x = 0.0f;
        for (j = 0; j < waveform->filterCounts[i]; j++) {
            // Go through it once to figure out the peak sample. Could be different than RKWaveformCalculateGain()
            f = 0.0f;
            g = 0.0f;
            fc = waveform->samples[i] + waveform->filterAnchors[i][j].origin;
            for (k = 0; k < waveform->filterAnchors[i][j].length; k++) {
                a = fc->i * fc->i + fc->q * fc->q;
                f = MAX(f, a);
                g += a;
                fc++;
            }
            f = (RKFloat)RKWaveformDigitalAmplitude / sqrtf(f);
            if (fabs(waveform->filterAnchors[i][j].fullScale / f - 1.0f) > 0.01f) {
                RKLog("Warning. g = %.4f   f = %.4e ==? %.4e  (%.3f)\n", g, f, waveform->filterAnchors[i][j].fullScale, waveform->filterAnchors[i][j].fullScale / f);
            }

            // From before: sensitivityGain = 10.0f * log10f(g / (h * 1.0e-6 * waveform->fs));
            a = powf(10.0f, 0.1f * waveform->filterAnchors[i][j].sensitivityGain);
            a = (RKFloat)RKWaveformDigitalAmplitude * sqrtf(a * 1.0e-6f * waveform->fs);
            if (fabs(waveform->filterAnchors[i][j].fullScale / a - 1.0f) > 0.01f) {
                RKLog("Warning. a / f = %.4f  %.4f\n", f / a, waveform->filterAnchors[i][j].fullScale / f);
                a = f;
            }

            fc = waveform->samples[i] + waveform->filterAnchors[i][j].origin;
            ic = waveform->iSamples[i] + waveform->filterAnchors[i][j].origin;
            for (k = 0; k < waveform->filterAnchors[i][j].length; k++) {
                ic->i = (int16_t)(a * fc->i);
                ic->q = (int16_t)(a * fc->q);
                x = MAX(x, (float)(ic->i * ic->i + ic->q * ic->q));
                ic++;
                fc++;
            }
            x = sqrtf(x) / RKWaveformDigitalAmplitude;
            //RKLog(">x = %.4f\n", x);
            if (x < 0.99f || x > 1.01f) {
                RKLog("Warning. Waveform normalization does not seem to work.  x[%d] = %.4f\n", j, x);
            }
        }
    }

    // Adjust the fc number since the waveform has been down-converted
    waveform->fc = 0.0f;

    free(w);
    free(s);
}

#pragma mark - Others

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
    fileHeader.fc = waveform->fc;
    fileHeader.fs = waveform->fs;
    strcpy(fileHeader.name, waveform->name);
    fwrite(&fileHeader, sizeof(RKWaveFileHeader), 1, fid);
    // Go through all groups
    for (k = 0; k < waveform->count; k++) {
        // Group header
        groupHeader.type = waveform->type;
        groupHeader.depth = waveform->depth;
        groupHeader.filterCounts = waveform->filterCounts[k];
        fwrite(&groupHeader, sizeof(RKWaveFileGroup), 1, fid);
        // Filter anchors
        fwrite(waveform->filterAnchors[k], sizeof(RKFilterAnchor), groupHeader.filterCounts, fid);
        // Waveform samples
        fwrite(waveform->samples[k], sizeof(RKComplex), groupHeader.depth, fid);
        fwrite(waveform->iSamples[k], sizeof(RKInt16C), groupHeader.depth, fid);
    }
    fclose(fid);
}

void RKWaveformNormalizeNoiseGain(RKWaveform *waveform) {
    int i, j, k;
    RKFloat gain;
    RKComplex *x;
    for (k = 0; k < waveform->count; k++) {
        gain = 0.0;
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            x = waveform->samples[k] + waveform->filterAnchors[k][j].origin;
            for (i = 0; i < waveform->filterAnchors[k][j].length; i++) {
                gain += (x->i * x->i);
                if (waveform->type & RKWaveformTypeIsComplex) {
                    gain += (x->q * x->q);
                }
            }
            gain = sqrtf(gain);
            x = waveform->samples[k] + waveform->filterAnchors[k][j].origin;
            for (i = 0; i < waveform->filterAnchors[k][j].length; i++) {
                x->i /= gain;
                x->q /= gain;
                x++;
            }
            waveform->filterAnchors[k][j].filterGain = 1.0f;
        }
    }
}

void RKWaveformSummary(RKWaveform *waveform) {
    int j, k;
    char format[RKMaximumStringLength];
    // Go through all waveforms and filters of each waveform to build the proper format width
    for (k = 0; k < waveform->count; k++) {
        int w0 = 0, w1 = 0, w2 = 0, w3 = 0, w4 = 0;
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            w0 = MAX(w0, (int)log10f((float)waveform->filterAnchors[k][j].length));
            w1 = MAX(w1, (int)log10f((float)waveform->filterAnchors[k][j].inputOrigin));
            w2 = MAX(w2, (int)log10f((float)waveform->filterAnchors[k][j].outputOrigin));
            w3 = MAX(w3, (int)log10f((float)waveform->filterAnchors[k][j].maxDataLength));
            w4 = MAX(w4, (int)log10f(fabs(waveform->filterAnchors[k][j].sensitivityGain)));
        }
        // Add a space for each comma
        w0 += (w0 / 3);
        w1 += (w1 / 3);
        w2 += (w2 / 3);
        w3 += (w3 / 3);
        sprintf(format, "> - Filter[%%%dd][%%%dd/%%%dd] @ (l:%%%ds)   X @ (i:%%%ds, o:%%%ds, d:%%%ds)   %%+%d.2f dB   %%+6.3f rad/s\n",
                (int)log10f((float)waveform->count) + 1,
                (int)log10f((float)waveform->filterCounts[k] + 1),
                (int)log10f((float)waveform->filterCounts[k] + 1),
                w0 + 1,
                w1 + 1,
                w2 + 1,
                w3 + 1,
                w4 + 5);
    }
    // Now we show the summary
    RKLog("Waveform '%s' (%s)   depth = %d x %s   fc = %s MHz   fs = %s MHz   pw = %s us\n",
          waveform->name,
          waveform->type & RKWaveformTypeIsComplex ? "C" : "R",
          waveform->count,
          RKIntegerToCommaStyleString(waveform->depth),
          RKFloatToCommaStyleString(1.0e-6 * waveform->fc),
          RKFloatToCommaStyleString(1.0e-6 * waveform->fs),
          RKFloatToCommaStyleString(1.0e6 * waveform->depth / waveform->fs));

    for (k = 0; k < waveform->count; k++) {
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            RKFloat g = 0.0;
            RKComplex *h = waveform->samples[k] + waveform->filterAnchors[k][j].origin;
            for (int i = 0; i < waveform->filterAnchors[k][j].length; i++) {
                g += (h->i * h->i + h->q * h->q);
                h++;
            }
            g = 10.0f * log10f(g);
            if (waveform->filterAnchors[k][j].filterGain - g > +0.1f ||
                waveform->filterAnchors[k][j].filterGain - g < -0.1f) {
                RKLog(">Error. Filter gain is not accurate.  (waveform: %.2f dB vs calculated: %.2f dB)", waveform->filterAnchors[k][j].filterGain, g);
            }
            RKLog(format,
                  k, j, waveform->filterCounts[k],
                  RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].length),
                  RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].inputOrigin),
                  RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].outputOrigin),
                  RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].maxDataLength),
                  waveform->filterAnchors[k][j].sensitivityGain,
                  waveform->filterAnchors[k][j].subCarrierFrequency);
        }
    }
}
