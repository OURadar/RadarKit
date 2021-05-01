//
//  RKWaveform.c
//  RadarKit
//
//  Created by Boonleng Cheong on 2/19/17.
//  Copyright © 2017-2021 Boonleng Cheong. All rights reserved.
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
    if (count > RKMaximumFilterGroups) {
        waveform->count = RKMaximumFilterGroups;
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
    FILE *fid = fopen(filename, "r");
    if (fid == NULL) {
        RKLog("Error. Unable to read wave file %s\n", filename);
        return NULL;
    }
    RKWaveform *waveform = RKWaveformReadFromReference(fid);
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
    sprintf(waveform->name, "p01");
    
    // Single filter
    waveform->filterCounts[0] = 1;
    
    // Everything simple
    waveform->filterAnchors[0][0].name = 0;
    waveform->filterAnchors[0][0].origin = 0;
    waveform->filterAnchors[0][0].length = 1;
    waveform->filterAnchors[0][0].inputOrigin = 0;
    waveform->filterAnchors[0][0].outputOrigin = 0;
    waveform->filterAnchors[0][0].maxDataLength = RKMaximumGateCount;
    waveform->filterAnchors[0][0].subCarrierFrequency = 0.0f;
    
    // Only a one. No need to set components that are 0's
    waveform->samples[0][0].i = 1.0f;
    waveform->iSamples[0][0].i = RKWaveformDigitalAmplitude;

    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
    return waveform;
}

RKWaveform *RKWaveformInitAsSingleTone(const double fs, const double fc, const double pulsewidth) {
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, (uint32_t)round(pulsewidth * fs));
    RKWaveformLinearFrequencyModulation(waveform, fs, fc, 0.0);
    return waveform;
}

RKWaveform *RKWaveformInitAsLinearFrequencyModulation(const double fs, const double fc, const double pulsewidth, const double bandwidth) {
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, (uint32_t)round(pulsewidth * fs));
    RKWaveformLinearFrequencyModulation(waveform, fs, fc, bandwidth);
    return waveform;
}

RKWaveform *RKWaveformInitAsFrequencyHops(const double fs, const double fc, const double pulsewidth, const double bandwidth, const int hops) {
    uint32_t depth = (uint32_t)round(pulsewidth * fs);
    if (fabs(depth / fs - pulsewidth) / pulsewidth > 0.1) {
        RKLog("Info. Waveform depth %.2f us --> %.2f us due to rounding @ fs = %.2f MHz.\n", 1.0e6 * pulsewidth, 1.0e6 * depth / fs, 1.0e-6 * fs);
    }
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(hops == 1 ? 1 : 2 * hops, depth);
    RKWaveformFrequencyHops(waveform, fs, fc, bandwidth);
    return waveform;
}

RKWaveform *RKWaveformInitAsFakeTimeFrequencyMultiplexing(void) {
    int i, j;
    const uint32_t longPulseWidth = 500 * 8;
    const uint32_t shortPulseWidth = 50 * 8;
    const uint32_t transitionWidth = 100 * 8;
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, longPulseWidth + shortPulseWidth);

    waveform->fs = 80.0e6;
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
    waveform->filterAnchors[0][0].maxDataLength = RKMaximumGateCount - longPulseWidth - shortPulseWidth - transitionWidth;
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

RKWaveform *RKWaveformInitAsTimeFrequencyMultiplexing(const double fs, const double fc, const double bandwidth, const double pulsewidth) {
    // Say bandwidth = 4 MHz. Going from 0 to 2 MHz
    RKWaveform *waveform = RKWaveformInitAsLinearFrequencyModulation(fs, fc, pulsewidth, 0.5 * bandwidth);
    RKWaveformApplyWindow(waveform, RKWindowTypeTukey, 0.05);
    // Say bandwidth = 4 MHz. Going atmost from -2 to 0 MHz, fc @ -1 MHz
    //RKWaveform *fill = RKWaveformInitAsFrequencyHops(fs, fc - 0.25 * bandwidth, 2.0e-6, 0.0, 1);
    RKWaveform *fill = RKWaveformInitAsSingleTone(fs, fc - 0.25 * bandwidth, 2.0e-6);
    RKWaveformApplyWindow(fill, RKWindowTypeHamming);
    // Expand waveform by the fill pulse
    RKWaveformAppendWaveform(waveform, fill, 10);
    RKWaveformFree(fill);
    // Modify the name and properties
    sprintf(waveform->name, "tfm");
    waveform->fc = fc;
    return waveform;
}

RKWaveform *RKWaveformInitAsFrequencyHoppingChirp(const double fs, const double fc, const double bandwidth, const double pulsewidth, const int count) {
    uint32_t depth = (uint32_t)round(pulsewidth * fs);
    if (fabs(depth / fs - pulsewidth) / pulsewidth > 0.1) {
        RKLog("Info. Waveform depth %.2f us --> %.2f us due to rounding @ fs = %.2f MHz.\n", 1.0e6 * pulsewidth, 1.0e6 * depth / fs, 1.0e-6 * fs);
    }
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(count == 1 ? 1 : 2 * count, depth);
    RKWaveformFrequencyHoppingChirp(waveform, fs, fc, bandwidth);
    return waveform;
}

#pragma mark - Tile / Concatenate / Repeat

RKResult RKWaveformAppendWaveform(RKWaveform *waveform, const RKWaveform *appendix, const uint32_t transitionSamples) {
    
    if (waveform->fs != appendix->fs) {
        RKLog("Error. Both waveforms must have same fs to concatenate.\n");
        return RKResultFailedToExpandWaveform;
    }
    if (waveform->count > 1) {
        RKLog("Error. This function is not built for waveform group count > 1.\n");
        return RKResultFailedToExpandWaveform;
    }
    if (waveform->filterCounts[0] > 1 || appendix->filterCounts[0] > 1) {
        RKLog("Error. This function is not built for waveform filter count > 1.\n");
        return RKResultFailedToExpandWaveform;
    }

    // New depth after expansion
    const uint32_t depth = waveform->depth + appendix->depth;
    RKLog("RKWaveformAppendWaveform() %u -> %u\n", waveform->depth, depth);
    //waveform->samples[0] = realloc(waveform->samples[0], depth * sizeof(RKComplex));
    //waveform->iSamples[0] = realloc(waveform->iSamples[0], depth * sizeof(RKInt16C));
    RKComplex *samples = waveform->samples[0];
    RKInt16C *iSamples = waveform->iSamples[0];
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&waveform->samples[0], RKSIMDAlignSize, depth * sizeof(RKComplex)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&waveform->iSamples[0], RKSIMDAlignSize, depth * sizeof(RKInt16C)));
    memcpy(waveform->samples[0], samples, waveform->depth * sizeof(RKComplex));
    memcpy(waveform->iSamples[0], iSamples, waveform->depth * sizeof(RKInt16C));
    free(samples);
    free(iSamples);

    // Two filters for demultiplexing
    waveform->filterCounts[0]++;
    
    waveform->type |= RKWaveformTypeTimeFrequencyMultiplexing;
    
    // Assume some kind of multiplexing, we can process waveform #1 starting from length of waveform #1
    waveform->filterAnchors[0][0].inputOrigin = waveform->filterAnchors[0][0].length + transitionSamples;
    waveform->filterAnchors[0][0].outputOrigin = waveform->filterAnchors[0][0].length + transitionSamples;

    waveform->filterAnchors[0][1].name = waveform->filterAnchors[0][0].name + 1;
    waveform->filterAnchors[0][1].origin = waveform->filterAnchors[0][0].length;
    waveform->filterAnchors[0][1].length = appendix->filterAnchors[0][0].length;
    waveform->filterAnchors[0][1].inputOrigin = waveform->filterAnchors[0][0].length;
    waveform->filterAnchors[0][1].outputOrigin = 0;
    waveform->filterAnchors[0][1].maxDataLength = waveform->filterAnchors[0][0].length + transitionSamples;
    waveform->filterAnchors[0][1].subCarrierFrequency = appendix->filterAnchors[0][0].subCarrierFrequency;
    waveform->filterAnchors[0][1].sensitivityGain = appendix->filterAnchors[0][0].sensitivityGain;
    waveform->filterAnchors[0][1].filterGain = appendix->filterAnchors[0][0].filterGain;
    
    memcpy(waveform->iSamples[0] + waveform->depth, appendix->iSamples[0], appendix->depth * sizeof(RKInt16C));
    memcpy(waveform->samples[0] + waveform->depth, appendix->samples[0], appendix->depth * sizeof(RKComplex));

    // Update the new depth
    waveform->depth = depth;

    return RKResultSuccess;
}

RKResult RKWaveformApplyWindow(RKWaveform *waveform, const RKWindowType type, ...) {
    int j, k;
    va_list args;
    RKFloat *w;
    RKInt16C *x;
    RKComplex *y;
    double parameter = 0.0, g;

    RKFloat *window = (RKFloat *)malloc(waveform->depth * sizeof(RKFloat));
    if (window == NULL) {
        RKLog("Error. Unable to allocate memory in RKWaveformApplyWindow().\n");
        exit(EXIT_FAILURE);
    }
    memset(window, 0, waveform->depth * sizeof(RKFloat));

    va_start(args, type);
    parameter = va_arg(args, double);
    va_end(args);

    RKWindowMake(window, type, waveform->depth, parameter);

    g = 0.0;
    w = window;
    for (k = 0; k < waveform->depth; k++) {
        g += (*w * *w);
        w++;
    }
    g = sqrt(waveform->depth / g);

    for (j = 0; j < waveform->count; j++) {
        w = window;
        x = waveform->iSamples[j];
        y = waveform->samples[j];
        for (k = 0; k < waveform->depth; k++) {
            x->i = (int16_t)((RKFloat)x->i * *w);
            x->q = (int16_t)((RKFloat)x->q * *w);
            y->i *= (*w * g);
            y->q *= (*w * g);
            w++;
            x++;
            y++;
        }
    }

    free(window);

    //RKWaveformNormalizeNoiseGain(waveform);

    return RKResultSuccess;
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

void RKWaveformSingleTone(RKWaveform *waveform, const double fs, const double fc) {
    RKWaveformFrequencyHops(waveform, fs, fc, 0.0);
}

void RKWaveformLinearFrequencyModulation(RKWaveform *waveform, const double fs, const double fc, const double bandwidth) {
    int i, k;
    double beta, kappa, omega, theta;

    // Other parameters
    waveform->fc = fc;
    waveform->fs = fs;
    waveform->type = RKWaveformTypeIsComplex;
    if (bandwidth > 0.0) {
        waveform->type |= RKWaveformTypeLinearFrequencyModulation;
        k = sprintf(waveform->name, "q%02.0f", 1.0e-6 * bandwidth);
    } else {
        waveform->type |= RKWaveformTypeSingleTone;
        if (fc > 0.0) {
            k = sprintf(waveform->name, "t%02.0f", 1.0e-6 * fc);
        } else {
            k = sprintf(waveform->name, "s");
        }
    }
    const double pulsewidth = waveform->depth / waveform->fs;
    if (pulsewidth < 1.0e-6) {
        sprintf(waveform->name + k, ".%.0f", round(10.0e6 * pulsewidth));
    } else if (fmod(1.0e6 * pulsewidth, 1.0) < 0.1) {
        sprintf(waveform->name + k, "%02.0f", 1.0e6 * pulsewidth);
    } else {
        sprintf(waveform->name + k, "%04.1f", 1.0e6 * pulsewidth);
    }
    waveform->filterCounts[0] = 1;

    // Filter parameters
    waveform->filterAnchors[0][0].name = 0;
    waveform->filterAnchors[0][0].origin = 0;
    waveform->filterAnchors[0][0].length = waveform->depth;
    waveform->filterAnchors[0][0].inputOrigin = 0;
    waveform->filterAnchors[0][0].outputOrigin = 0;
    waveform->filterAnchors[0][0].maxDataLength = RKMaximumGateCount - waveform->depth;
    waveform->filterAnchors[0][0].subCarrierFrequency = 2.0f * M_PI * (fc + 0.5 * bandwidth) / fs;

    RKInt16C *w = waveform->iSamples[0];
    RKComplex *x = waveform->samples[0];
    omega = 2.0 * M_PI * fc / fs;
    kappa = 2.0 * M_PI * bandwidth / pulsewidth / (fs * fs);
    theta = omega * (double)(waveform->depth / 2) + 0.5 * kappa * pow((double)(waveform->depth / 2), 2.0);
    for (i = 0; i < waveform->depth; i++) {
        beta = omega * i + 0.5 * kappa * i * i - theta;
        x->i = cosf(beta);
        x->q = sinf(beta);
        w->i = (int16_t)rintf(RKWaveformDigitalAmplitude * x->i);
        w->q = (int16_t)rintf(RKWaveformDigitalAmplitude * x->q);
        x++;
        w++;
    }
    RKWaveformNormalizeNoiseGain(waveform);
    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
    return;
}

//
// This is actually hop pairs: f0, f0, f1, f1, f2, f2, ... around fc
//
void RKWaveformFrequencyHops(RKWaveform *waveform, const double fs, const double fc, const double bandwidth) {
    int i, j, k;
    double f, beta, omega, theta;
    RKComplex *x;
    RKInt16C *w;
    
    const int count = waveform->count == 1 ? 1 : waveform->count / 2;

    waveform->fs = fs;
    waveform->fc = fc;
    waveform->type = RKWaveformTypeIsComplex | RKWaveformTypeFrequencyHopping;
    // h[bb][cc][wwww]
    // h201100.5 = 20 MHz, 11 hops, 0.5us
    // h200501   = 20 MHz,  5 hops, 1.0us
    // h200501.5 = 20 MHz,  5 hops, 1.5us
    // h200502   = 20 MHz,  5 hops, 2.0us
    k = sprintf(waveform->name, "h%02.0f%02d", 1.0e-6 * bandwidth, count);
    const double pulsewidth = waveform->depth / waveform->fs;
    if (pulsewidth < 1.0e-6) {
        sprintf(waveform->name + k, ".%.0f", round(10.0e6 * pulsewidth));
    } else if (fmod(1.0e6 * pulsewidth, 1.0) < 0.1) {
        sprintf(waveform->name + k, "%02.0f", 1.0e6 * pulsewidth);
    } else {
        sprintf(waveform->name + k, "%04.1f", 1.0e6 * pulsewidth);
    }

    const double delta = waveform->count <= 2 ? 0.0 : bandwidth / (double)((waveform->count / 2) - 1);
    
    // Find the best stride to hop
    int stride = RKBestStrideOfHops(waveform->count / 2, false);
    
    // Some variables:
    // omega = discrete frequency
    // beta = argument for exp() / cos()-sin() pair
    for (j = 0, k = 0; k < waveform->count; k++) {
        f = delta * (double)j - 0.5 * bandwidth + fc;
        omega = 2.0 * M_PI * f / fs;
        theta = omega * (double)(waveform->depth / 2);
        waveform->filterCounts[k] = 1;
        waveform->filterAnchors[k][0].name = j;
        waveform->filterAnchors[k][0].origin = 0;
        waveform->filterAnchors[k][0].length = waveform->depth;
        waveform->filterAnchors[k][0].maxDataLength = RKMaximumGateCount;   // Can be replaced with actual depth later
        waveform->filterAnchors[k][0].subCarrierFrequency = omega;
        //RKLog(">f[%d] = %+5.1f MHz   omega = %.3f   n = %d", k, 1.0e-6 * f, omega, n);
        x = waveform->samples[k];
        w = waveform->iSamples[k];
        for (i = 0; i < waveform->depth; i++) {
            beta = omega * i - theta;
            x->i = cos(beta);
            x->q = sin(beta);
            w->i = (int16_t)rintf(RKWaveformDigitalAmplitude * x->i);
            w->q = (int16_t)rintf(RKWaveformDigitalAmplitude * x->q);
            x++;
            w++;
        }
        // Get ready for the next frequency when we are in odd index
        if (k % 2 == 1) {
            j = RKNextNModuloS(j, stride, count);
        }
    }
    RKWaveformNormalizeNoiseGain(waveform);
    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
}

void RKWaveformFrequencyHoppingChirp(RKWaveform *waveform, const double fs, const double fc, const double bandwidth) {
    int i, j, k;
    double fl, fu, beta, kappa, omega, theta;
    RKComplex *x;
    RKInt16C *w;

    const int count = waveform->count == 1 ? 1 : waveform->count / 2;

    waveform->fc = fc;
    waveform->fs = fs;
    waveform->type = RKWaveformTypeIsComplex | RKWaveformTypeFrequencyHoppingChirp;
    // k[bb][cc][wwww]
    // k201100.5 = 20 MHz, 11 hops, 0.5us
    // k200501   = 20 MHz,  5 hops, 1.0us
    // k200501.5 = 20 MHz,  5 hops, 1.5us
    // k200502   = 20 MHz,  5 hops, 2.0us
    k = sprintf(waveform->name, "k%02.0f%02d", 1.0e-6 * bandwidth, count);
    const double pulsewidth = waveform->depth / fs;
    if (pulsewidth < 1.0e-6) {
        sprintf(waveform->name + k, ".%.1f", round(10.0e6 * pulsewidth));
    } else if (fmod(1.0e6 * pulsewidth, 1.0) < 0.1) {
        sprintf(waveform->name + k, "%02.0f", 1.0e6 * pulsewidth);
    } else {
        sprintf(waveform->name + k, "%04.1f", 1.0e6 * pulsewidth);
    }
    
    const double sub = bandwidth / (double)count;

    // Find the best stride to hop
    int stride = RKBestStrideOfHops(count, false);
    
    // Test with BW 20-MHz, count = 5 ==> SBW = 4-MHz for each hop
    // Frequency span: [-10, -6], [-6, -2], [-2, +2], [+2, +6], [+6, +10]
    // Hop Identifier:     (0)       (1)       (2)       (3)       (4)
    // Hop Anchor = i:    (-2)      (-1)       (0)      (+1)      (+2)
    // Some variables:
    // fl = lower bound frequency of the subband
    // fh = upper bound frequency of the subband
    // omega = discrete frequency
    // kappa = rate of frequency change
    // beta = argument for exp() / cos()-sin() pair
    for (j = 0, k = 0; k < waveform->count; k++) {
        i = j - count / 2;
        fl = ((double)i - 0.5) * sub + fc;
        fu = ((double)i + 0.5) * sub + fc;
        omega = 2.0 * M_PI * fl / fs;
        kappa = 2.0 * M_PI * sub / pulsewidth / (fs * fs);
        theta = omega * (double)(waveform->depth / 2) + 0.5 * kappa * pow((double)(waveform->depth / 2), 2.0);
        waveform->filterCounts[k] = 1;
        waveform->filterAnchors[k][0].name = j;
        waveform->filterAnchors[k][0].origin = 0;
        waveform->filterAnchors[k][0].length = waveform->depth;
        waveform->filterAnchors[k][0].maxDataLength = RKMaximumGateCount;
        waveform->filterAnchors[k][0].subCarrierFrequency = 0.0;
        waveform->filterAnchors[k][0].lowerBoundFrequency = fl;
        waveform->filterAnchors[k][0].upperBoundFrequency = fu;
        x = waveform->samples[k];
        w = waveform->iSamples[k];
        for (i = 0; i < waveform->depth; i++) {
            beta = omega * i + 0.5 * kappa * i * i - theta;
            x->i = cos(beta);
            x->q = sin(beta);
            w->i = (int16_t)rintf(RKWaveformDigitalAmplitude * x->i);
            w->q = (int16_t)rintf(RKWaveformDigitalAmplitude * x->q);
            x++;
            w++;
        }
        // Get ready for the next frequency when we are in odd index
        if (k % 2 == 1) {
            j = RKNextNModuloS(j, stride, count);
        }
    }
    RKWaveformNormalizeNoiseGain(waveform);
    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
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
            if (waveform->filterAnchors[l][k].inputOrigin % stride) {
                waveform->filterAnchors[l][k].inputOrigin += stride;
            }
            waveform->filterAnchors[l][k].inputOrigin /= stride;
            if (waveform->filterAnchors[l][k].outputOrigin % stride) {
                waveform->filterAnchors[l][k].outputOrigin += stride;
            }
            waveform->filterAnchors[l][k].outputOrigin /= stride;
            if (waveform->filterAnchors[l][k].maxDataLength % stride) {
                waveform->filterAnchors[l][k].maxDataLength += stride;
            }
            waveform->filterAnchors[l][k].maxDataLength /= stride;
            waveform->filterAnchors[l][k].subCarrierFrequency *= stride;
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
    if (waveform == NULL) {
        RKLog("RKWaveformSummary() Input cannot be NULL.\n");
        return;
    }
    // Go through all waveforms and filters of each waveform to build the proper format width
    int w0 = 0, w1 = 0, w2 = 0, w3 = 0, w4 = 0, w5 = 0;
    for (k = 0; k < waveform->count; k++) {
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            w0 = MAX(w0, (int)log10f((float)waveform->filterAnchors[k][j].length));
            w1 = MAX(w1, (int)log10f((float)waveform->filterAnchors[k][j].inputOrigin));
            w2 = MAX(w2, (int)log10f((float)waveform->filterAnchors[k][j].outputOrigin));
            w3 = MAX(w3, (int)log10f((float)waveform->filterAnchors[k][j].maxDataLength));
            w4 = MAX(w4, (int)log10f(fabs(waveform->filterAnchors[k][j].sensitivityGain)));
            w5 = MAX(w5, MAX((int)log10f(fabs(1.0e-6 * waveform->filterAnchors[k][j].lowerBoundFrequency)),
                             (int)log10f(fabs(1.0e-6 * waveform->filterAnchors[k][j].upperBoundFrequency))));
        }
    }
    // Add a space for each comma
    w0 += (w0 / 3);
    w1 += (w1 / 3);
    w2 += (w2 / 3);
    w3 += (w3 / 3);
    if (waveform->type & RKWaveformTypeFrequencyHoppingChirp) {
        sprintf(format, "> - Filter[%%%dd][%%%dd/%%%dd] @ (n:%%d l:%%%ds)   X @ (i:%%%ds, o:%%%ds, d:%%%ds)   %%+%d.2f dB   [ %%+%d.1f - %%+%d.1f ] MHz\n",
                waveform->count == 1 ? 1 : (int)log10f((float)waveform->count - 1) + 1,
                waveform->filterCounts[0] == 1 ? 1 : (int)log10f((float)waveform->filterCounts[0]) + 1,
                (int)log10f((float)waveform->filterCounts[0]) + 1,
                w0 + 1,
                w1 + 1,
                w2 + 1,
                w3 + 1,
                w4 + 5,
                w5 + 4,
                w5 + 4);
    } else {
        sprintf(format, "> - Filter[%%%dd][%%%dd/%%%dd] @ (l:%%%ds)   X @ (i:%%%ds, o:%%%ds, d:%%%ds)   %%+%d.2f dB   %%+6.3f rad/sam\n",
                waveform->count == 1 ? 1 : (int)log10f((float)waveform->count - 1) + 1,
                waveform->filterCounts[0] == 1 ? 1 : (int)log10f((float)waveform->filterCounts[0] - 1) + 1,
                (int)log10f((float)waveform->filterCounts[0]) + 1,
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
            if (waveform->type & RKWaveformTypeFrequencyHoppingChirp) {
                RKLog(format,
                      k, j, waveform->filterCounts[k], waveform->filterAnchors[k][j].name,
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].length),
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].inputOrigin),
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].outputOrigin),
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].maxDataLength),
                      waveform->filterAnchors[k][j].sensitivityGain,
                      1.0e-6 * waveform->filterAnchors[k][j].lowerBoundFrequency,
                      1.0e-6 * waveform->filterAnchors[k][j].upperBoundFrequency
                      );
            } else {
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
}

#pragma mark - File

// ----
//  RKWaveFileGlobalHeader
//  - name
//  - group count
//  - global depth
// ----
//
// ----
//  RKWaveFileGroupHeader (0)
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
//  RKWaveFileGroupHeader (1)
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

size_t RKWaveformWriteToReference(RKWaveform *waveform, FILE *fid) {
    int k;
    size_t bytes = 0;
    RKWaveFileGlobalHeader fileHeader;
    RKWaveFileGroupHeader groupHeader;
    memset(&fileHeader, 0, sizeof(RKWaveFileGlobalHeader));
    memset(&groupHeader, 0, sizeof(RKWaveFileGroupHeader));
    // File header
    fileHeader.count = waveform->count;
    fileHeader.depth = waveform->depth;
    fileHeader.fc = waveform->fc;
    fileHeader.fs = waveform->fs;
    strcpy(fileHeader.name, waveform->name);
    fwrite(&fileHeader, sizeof(RKWaveFileGlobalHeader), 1, fid);
    bytes += sizeof(RKWaveFileGlobalHeader);
    // Go through all groups
    for (k = 0; k < waveform->count; k++) {
        // Group header
        groupHeader.type = waveform->type;
        groupHeader.depth = waveform->depth;
        groupHeader.filterCount = waveform->filterCounts[k];
        fwrite(&groupHeader, sizeof(RKWaveFileGroupHeader), 1, fid);
        bytes += sizeof(RKWaveFileGroupHeader);
        // Filter anchors
        fwrite(waveform->filterAnchors[k], sizeof(RKFilterAnchor), groupHeader.filterCount, fid);
        bytes += groupHeader.filterCount * sizeof(RKFilterAnchor);
        // Waveform samples
        fwrite(waveform->samples[k], sizeof(RKComplex), groupHeader.depth, fid);
        bytes += groupHeader.depth * sizeof(RKComplex);
        fwrite(waveform->iSamples[k], sizeof(RKInt16C), groupHeader.depth, fid);
        bytes += groupHeader.depth * sizeof(RKInt16C);
    }
    return bytes;
}

RKResult RKWaveformWriteFile(RKWaveform *waveform, const char *filename) {
    // Get the path created if it doesn't exist
    RKPreparePath(filename);
    FILE *fid = fopen(filename, "w");
    if (fid == NULL) {
        RKLog("Error. Unable to write wave file %s\n", filename);
        return RKResultFailedToOpenFileForWriting;
    }
    if (strlen(waveform->name) == 0) {
        char *lastPart = strchr(filename, '/');
        if (lastPart == NULL) {
            strcpy(waveform->name, filename);
        } else {
            strcpy(waveform->name, lastPart + 1);
        }
    }
    RKWaveformWriteToReference(waveform, fid);
    fclose(fid);
    return RKResultSuccess;
}

RKWaveform *RKWaveformReadFromReference(FILE *fid) {
    int j, k;
    size_t r;
    RKWaveFileGlobalHeader fileHeader;
    RKWaveFileGroupHeader groupHeader;
    r = fread(&fileHeader, sizeof(RKWaveFileGlobalHeader), 1, fid);
    if (r == 0) {
        RKLog("Error. No file header from %p.\n", fid);
    }
    
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(fileHeader.count, fileHeader.depth);
    //RKLog("fileHeader.groupCount = %d   fileHeader.depth = %d\n", fileHeader.groupCount, fileHeader.depth);

    waveform->fc = fileHeader.fc;
    waveform->fs = fileHeader.fs;
    strncpy(waveform->name, fileHeader.name, RKNameLength);
    
    for (k = 0; k < fileHeader.count; k++) {
        // Read in the waveform of each group
        r = fread(&groupHeader, sizeof(RKWaveFileGroupHeader), 1, fid);
        if (r == 0) {
            RKLog("Error. Failed reading group header from %p.\n", fid);
            return NULL;
        }
        // RKLog("groupHeader.depth = %d\n", groupHeader.depth);
        if (waveform->depth < groupHeader.depth) {
            RKLog("Error. Unable to fit %p into supplied buffer. (%s < %s) @ group %d\n", fid,
                  RKIntegerToCommaStyleString(waveform->depth), RKIntegerToCommaStyleString(groupHeader.depth), k);
            RKWaveformFree(waveform);
            return NULL;
        }
        waveform->type = groupHeader.type;
        waveform->filterCounts[k] = groupHeader.filterCount;
        r = fread(waveform->filterAnchors[k], sizeof(RKFilterAnchor), waveform->filterCounts[k], fid);
        if (r == 0) {
            RKLog("Error. Unable to read filter anchors from %p\n", fid);
            return NULL;
        }
        // Output data from the first filter is allowed up to the maximum
        waveform->filterAnchors[k][0].maxDataLength = RKMaximumGateCount;
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            if (waveform->filterAnchors[k][j].length > waveform->depth) {
                RKLog("Error. This waveform is invalid.  length = %s > depth %s\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].length),
                      RKIntegerToCommaStyleString(waveform->depth));
                RKWaveformFree(waveform);
                fclose(fid);
                return NULL;
            }
            if (waveform->filterAnchors[k][j].origin > RKMaximumGateCount) {
                RKLog("Error. This waveform is invalid.  origin = %s > %s (RKMaximumGateCount)\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].origin),
                      RKIntegerToCommaStyleString(RKMaximumGateCount));
                RKWaveformFree(waveform);
                fclose(fid);
                return NULL;
            }
            if (waveform->filterAnchors[k][j].inputOrigin > RKMaximumGateCount) {
                RKLog("Error. This waveform is invalid.  inputOrigin = %s > %s (RKMaximumGateCount)\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].inputOrigin),
                      RKIntegerToCommaStyleString(RKMaximumGateCount));
                RKWaveformFree(waveform);
                fclose(fid);
                return NULL;
            }
            if (waveform->filterAnchors[k][j].outputOrigin > RKMaximumGateCount) {
                RKLog("Error. This waveform is invalid.  outputOrigin = %s > %s (RKMaximumGateCount)\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].outputOrigin),
                      RKIntegerToCommaStyleString(RKMaximumGateCount));
                RKWaveformFree(waveform);
                fclose(fid);
                return NULL;
            }
        }
        r = fread(waveform->samples[k], sizeof(RKComplex), waveform->depth, fid);
        if (r == 0) {
            RKLog("Error. Failed reading complex samples from %p.\n", fid);
        }
        r = fread(waveform->iSamples[k], sizeof(RKInt16C), waveform->depth, fid);
        if (r == 0) {
            RKLog("Error. Failed reading int16 samples from %p.\n", fid);
        }
    }
    RKWaveformCalculateGain(waveform, RKWaveformGainAll);
    return waveform;
}
