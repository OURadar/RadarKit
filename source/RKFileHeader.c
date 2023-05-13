//
//  RKFileHeader.c
//  RadarKit
//
//  Created by Boonleng Cheong on 5/13/2023.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RKFileHeader.h>

RKFileHeader *RKFileHeaderAlloc(void) {
    RKFileHeader *fileHeader = (RKFileHeader *)malloc(sizeof(RKFileHeader));
    if (fileHeader == NULL) {
        RKLog("Error. Unable to allocate a file header object.\n");
        exit(EXIT_FAILURE);
    }
    return fileHeader;
}

void RKFileHeaderFree(RKFileHeader *fileHeader) {
    free(fileHeader->config.waveformDecimate);
    free(fileHeader->config.waveform);
    free(fileHeader);
}

RKFileHeader *RKFileHeaderRead(FILE *fid) {
    int k;
    size_t r;

    RKFileHeader *fileHeader = RKFileHeaderAlloc();

    r = fread(fileHeader, sizeof(RKFileHeader), 1, fid);
    if (r == 0) {
        RKLog("Error. Failed reading file header.\n");
        exit(EXIT_FAILURE);
    }
    r *= sizeof(RKFileHeader);

    if (fileHeader->version <= 4) {
        RKLog("Error. Sorry but I am not programmed to read version %d. Ask my father.\n", fileHeader->version);
        exit(EXIT_FAILURE);
    } else if (fileHeader->version == 5) {
        rewind(fid);
        RKFileHeaderV1 *fileHeaderV1 = (RKFileHeaderV1 *)malloc(sizeof(RKFileHeaderV1));
        r = fread(fileHeaderV1, sizeof(RKFileHeaderV1), 1, fid);
        if (r == 0) {
            RKLog("Error. Failed reading file header.\n");
            exit(EXIT_FAILURE);
        }
        r *= sizeof(RKFileHeaderV1);
        RKLog("fileHeaderV1->dataType = %d (%s)\n",
                fileHeaderV1->dataType,
                fileHeaderV1->dataType == RKRawDataTypeFromTransceiver ? "Raw" :
                (fileHeaderV1->dataType == RKRawDataTypeAfterMatchedFilter ? "Compressed" : "Unknown"));
        fileHeader->dataType = fileHeaderV1->dataType;
        fileHeader->desc = fileHeaderV1->desc;
        fileHeader->config.i = fileHeaderV1->config.i;
        fileHeader->config.sweepElevation = fileHeaderV1->config.sweepElevation;
        fileHeader->config.sweepAzimuth = fileHeaderV1->config.sweepAzimuth;
        fileHeader->config.startMarker = fileHeaderV1->config.startMarker;
        fileHeader->config.prt[0] = fileHeaderV1->config.prt[0];
        fileHeader->config.prt[1] = fileHeaderV1->config.prt[1];
        fileHeader->config.pw[0] = fileHeaderV1->config.pw[0];
        fileHeader->config.pw[1] = fileHeaderV1->config.prt[1];
        fileHeader->config.pulseGateCount = fileHeaderV1->config.pulseGateCount;
        fileHeader->config.pulseGateSize = fileHeaderV1->config.pulseGateSize;
        fileHeader->config.ringFilterGateCount = fileHeaderV1->config.pulseRingFilterGateCount;
        fileHeader->config.waveformId[0] = fileHeaderV1->config.waveformId[0];
        memcpy(fileHeader->config.noise, fileHeaderV1->config.noise, (6 + 22 * 2 + 22 + 22 + 2) * sizeof(RKFloat));
        strcpy(fileHeader->config.waveformName, fileHeaderV1->config.waveform);
        strcat(fileHeader->config.waveformName, "-syn");
        // Initialize a waveformDecimate that matches the original config->filterAnchors
        uint32_t u32 = 0;
        for (k = 0; k < fileHeaderV1->config.filterCount; k++) {
            u32 += fileHeaderV1->config.filterAnchors[k].length;
        }
        // No raw (uncompressed) data prior to this built. Talk to Boonleng if you managed to, and needed to, process one
        RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, u32);
        strcpy(waveform->name, fileHeaderV1->config.waveform);
        waveform->fs = 0.5f * 3.0e8 / (fileHeader->config.pulseGateSize * fileHeader->desc.pulseToRayRatio);
        waveform->filterCounts[0] = fileHeaderV1->config.filterCount;
        for (k = 0; k < waveform->filterCounts[0]; k++) {
            memcpy(&waveform->filterAnchors[0][k], &fileHeaderV1->config.filterAnchors[k], sizeof(RKFilterAnchor));
        }
        // Fill one some dummy samples to get things going
        RKWaveformOnes(waveform);
        RKWaveformNormalizeNoiseGain(waveform);
        fileHeader->config.waveform = waveform;
        fileHeader->config.waveformDecimate = RKWaveformCopy(waveform);
        free(fileHeaderV1);
    } else {
        RKWaveform *waveform = RKWaveformReadFromReference(fid);
        fileHeader->config.waveform = waveform;
        fileHeader->config.waveformDecimate = RKWaveformCopy(waveform);
        RKWaveformDecimate(fileHeader->config.waveformDecimate, fileHeader->desc.pulseToRayRatio);
        long fp = ftell(fid);
        r += sizeof(RKWaveFileGlobalHeader);
        for ( k = 0; k < waveform->count; k++) {
            r += waveform->filterCounts[k] * sizeof(RKFilterAnchor);
        }
        r += waveform->depth * (sizeof(RKComplex) + sizeof(RKInt16C));
        if (fp != r) {
            RKLog("Error. I detected a bug.  fp = %s != %s\n", RKIntegerToCommaStyleString(fp), RKIntegerToCommaStyleString(r));
        }
    }
    return fileHeader;
}

void RKFileHeaderSummary(RKFileHeader *fileHeader) {
    RKConfig *config = &fileHeader->config;
    RKLog(">fileHeader.preface = '%s'   version = %d\n", fileHeader->preface, fileHeader->version);
    RKLog(">fileHeader.dataType = '%s'\n",
            fileHeader->dataType == RKRawDataTypeFromTransceiver ? "Raw" :
            (fileHeader->dataType == RKRawDataTypeAfterMatchedFilter ? "Compressed" : "Unknown"));
    RKLog(">desc.name = '%s'\n", fileHeader->desc.name);
    RKLog(">desc.filePrefix = %s\n", fileHeader->desc.filePrefix);
    RKLog(">desc.latitude, longitude = %.6f, %.6f\n", fileHeader->desc.latitude, fileHeader->desc.longitude);
    RKLog(">desc.pulseCapacity = %s\n", RKIntegerToCommaStyleString(fileHeader->desc.pulseCapacity));
    RKLog(">desc.pulseToRayRatio = %u\n", fileHeader->desc.pulseToRayRatio);
    RKLog(">desc.productBufferDepth = %u\n", fileHeader->desc.productBufferDepth);
    RKLog(">desc.wavelength = %.4f m\n", fileHeader->desc.wavelength);
    RKLog(">config.sweepElevation = %.2f deg\n", config->sweepElevation);
    RKLog(">config.prt = %.3f ms (PRF = %s Hz)\n",
            1.0e3f * config->prt[0],
            RKIntegerToCommaStyleString((int)roundf(1.0f / config->prt[0])));
    RKLog(">config.pw = %.2f us (dr = %s m)\n",
            1.0e6 * config->pw[0],
            RKFloatToCommaStyleString(0.5 * 3.0e8 * config->pw[0]));
    RKLog(">config.pulseGateCount = %s -- (/ %d) --> %s\n",
            RKIntegerToCommaStyleString(config->pulseGateCount),
            fileHeader->desc.pulseToRayRatio,
            RKIntegerToCommaStyleString(config->pulseGateCount / fileHeader->desc.pulseToRayRatio));
    RKLog(">config.pulseGateSize = %.3f m -- (x %d) --> %.3f m\n",
            config->pulseGateSize,
            fileHeader->desc.pulseToRayRatio,
            config->pulseGateSize * fileHeader->desc.pulseToRayRatio);
    RKLog(">config.noise = %.2f, %.2f ADU^2\n", config->noise[0], config->noise[1]);
    RKLog(">config.systemZCal = %.2f, %.2f dB\n", config->systemZCal[0], config->systemZCal[1]);
    RKLog(">config.systemDCal = %.2f dB\n", config->systemDCal);
    RKLog(">config.systemPCal = %.2f deg\n", config->systemPCal);
    RKLog(">config.SNRThreshold = %.2f dB\n", config->SNRThreshold);
    RKLog(">config.SQIThreshold = %.2f\n", config->SQIThreshold);
    RKLog(">config.waveformName = '%s'\n", config->waveformName);
    if (fileHeader->version >= 5) {
        RKWaveformSummary(config->waveform);
    }
    if (fileHeader->version >= 6) {
        RKWaveformSummary(config->waveformDecimate);
    }
}
