//
//  RKFileHeader.c
//  RadarKit
//
//  Created by Boonleng Cheong on 5/13/2023.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RKFileHeader.h>

RKFileHeader *RKFileHeaderInit(void) {
    RKFileHeader *fileHeader = (RKFileHeader *)malloc(sizeof(RKFileHeader));
    if (fileHeader == NULL) {
        RKLog("Error. Unable to allocate a file header object.\n");
        exit(EXIT_FAILURE);
    }
    memset(fileHeader, 0, sizeof(RKFileHeader));
    fileHeader->format = RKRawDataFormat;
    return fileHeader;
}

void RKFileHeaderFree(RKFileHeader *fileHeader) {
    free(fileHeader->config.waveformDecimate);
    free(fileHeader->config.waveform);
    free(fileHeader);
}

RKFileHeader *RKFileHeaderInitFromFid(FILE *fid) {
    int k;
    size_t r;

    fseek(fid, 0, SEEK_END);
    long fsize = ftell(fid);
    if (fsize < sizeof(RKFileHeader)) {
        RKLog("Error. File size = %s B is too small to be a valid RadarKit file.\n", RKIntegerToCommaStyleString(fsize));
        exit(EXIT_FAILURE);
    }
    rewind(fid);

    RKFileHeader *fileHeader = RKFileHeaderInit();

    r = fread(fileHeader, sizeof(RKFileHeader), 1, fid);
    if (r == 0) {
        RKLog("Error. Failed reading file header.\n");
        exit(EXIT_FAILURE);
    }
    r *= sizeof(RKFileHeader);

    if (fileHeader->format <= 4) {
        RKLog("Error. Sorry but I am not programmed to read format %d. Ask my father.\n", fileHeader->format);
        exit(EXIT_FAILURE);
    } else if (fileHeader->format == 5) {
        rewind(fid);
        RKFileHeaderF5 *fileHeaderF5 = (RKFileHeaderF5 *)malloc(sizeof(RKFileHeaderF5));
        r = fread(fileHeaderF5, sizeof(RKFileHeaderF5), 1, fid);
        if (r == 0) {
            RKLog("Error. Failed reading file header.\n");
            exit(EXIT_FAILURE);
        }
        r *= sizeof(RKFileHeaderF5);
        RKLog("fileHeaderV1->dataType = %d (%s)\n",
                fileHeaderF5->dataType,
                fileHeaderF5->dataType == RKRawDataTypeFromTransceiver ? "Raw" :
                (fileHeaderF5->dataType == RKRawDataTypeAfterMatchedFilter ? "Compressed" : "Unknown"));
        fileHeader->dataType = fileHeaderF5->dataType;
        fileHeader->desc = fileHeaderF5->desc;
        fileHeader->config.i = fileHeaderF5->config.i;
        fileHeader->config.sweepElevation = fileHeaderF5->config.sweepElevation;
        fileHeader->config.sweepAzimuth = fileHeaderF5->config.sweepAzimuth;
        fileHeader->config.startMarker = fileHeaderF5->config.startMarker;
        fileHeader->config.prt[0] = fileHeaderF5->config.prt[0];
        fileHeader->config.prt[1] = fileHeaderF5->config.prt[1];
        fileHeader->config.pw[0] = fileHeaderF5->config.pw[0];
        fileHeader->config.pw[1] = fileHeaderF5->config.prt[1];
        fileHeader->config.pulseGateCount = fileHeaderF5->config.pulseGateCount;
        fileHeader->config.pulseGateSize = fileHeaderF5->config.pulseGateSize;
        fileHeader->config.ringFilterGateCount = fileHeaderF5->config.pulseRingFilterGateCount;
        fileHeader->config.waveformId[0] = fileHeaderF5->config.waveformId[0];
        fileHeader->config.noise[0] = fileHeaderF5->config.noise[0];
        fileHeader->config.noise[1] = fileHeaderF5->config.noise[1];
        fileHeader->config.systemZCal[0] = fileHeaderF5->config.systemZCal[0];
        fileHeader->config.systemZCal[1] = fileHeaderF5->config.systemZCal[1];
        fileHeader->config.systemDCal = fileHeaderF5->config.systemDCal;
        fileHeader->config.systemPCal = fileHeaderF5->config.systemPCal;
        for (k = 0; k < MIN(8, RKMaximumFilterCount); k++) {
            fileHeader->config.waveformId[k] = fileHeaderF5->config.waveformId[k];
            fileHeader->config.ZCal[k][0] = fileHeaderF5->config.ZCal[k][0];
            fileHeader->config.ZCal[k][1] = fileHeaderF5->config.ZCal[k][1];
            fileHeader->config.DCal[k] = fileHeaderF5->config.DCal[k];
            fileHeader->config.PCal[k] = fileHeaderF5->config.PCal[k];
        }
        strncpy(fileHeader->config.waveformName, fileHeaderF5->config.waveform, RKNameLength - 5);
        fileHeader->config.waveformName[RKNameLength - 5] = '\0';
        strcat(fileHeader->config.waveformName, "-syn");
        // Initialize a waveformDecimate that matches the original config->filterAnchors
        uint32_t u32 = 0;
        for (k = 0; k < fileHeaderF5->config.filterCount; k++) {
            u32 += fileHeaderF5->config.filterAnchors[k].length;
        }
        // No raw (uncompressed) data prior to this built. Talk to Boonleng if you managed to, and needed to, process one
        RKWaveform *waveform = RKWaveformInitWithCountAndDepth(1, u32);
        strcpy(waveform->name, fileHeaderF5->config.waveform);
        waveform->fs = 0.5f * 3.0e8 / (fileHeader->config.pulseGateSize * fileHeader->desc.pulseToRayRatio);
        waveform->filterCounts[0] = fileHeaderF5->config.filterCount;
        for (k = 0; k < waveform->filterCounts[0]; k++) {
            memcpy(&waveform->filterAnchors[0][k], &fileHeaderF5->config.filterAnchors[k], sizeof(RKFilterAnchor));
        }
        // Fill one some dummy samples to get things going
        RKWaveformOnes(waveform);
        RKWaveformNormalizeNoiseGain(waveform);
        fileHeader->config.waveform = waveform;
        fileHeader->config.waveformDecimate = RKWaveformCopy(waveform);
        free(fileHeaderF5);
    } else if (fileHeader->format <= 7) {
        rewind(fid);
        RKFileHeaderF6 *fileHeaderF6 = (RKFileHeaderF6 *)malloc(sizeof(RKFileHeaderF6));
        if (fread(fileHeaderF6, sizeof(RKFileHeaderF6), 1, fid) == 0) {
            RKLog("Error. Failed reading file header.\n");
            fclose(fid);
            return NULL;
        }
        fileHeader->dataType = fileHeaderF6->dataType;
        fileHeader->desc = fileHeaderF6->desc;
        fileHeader->config.i = fileHeaderF6->config.i;
        fileHeader->config.sweepElevation = fileHeaderF6->config.sweepElevation;
        fileHeader->config.sweepAzimuth = fileHeaderF6->config.sweepAzimuth;
        fileHeader->config.startMarker = fileHeaderF6->config.startMarker;
        fileHeader->config.prt[0] = fileHeaderF6->config.prt[0];
        fileHeader->config.prt[1] = fileHeaderF6->config.prt[1];
        fileHeader->config.pw[0] = fileHeaderF6->config.pw[0];
        fileHeader->config.pw[1] = fileHeaderF6->config.prt[1];
        fileHeader->config.pulseGateCount = fileHeaderF6->config.pulseGateCount;
        fileHeader->config.pulseGateSize = fileHeaderF6->config.pulseGateSize;
        fileHeader->config.transitionGateCount = fileHeaderF6->config.transitionGateCount;
        fileHeader->config.ringFilterGateCount = fileHeaderF6->config.ringFilterGateCount;
        fileHeader->config.noise[0] = fileHeaderF6->config.noise[0];
        fileHeader->config.noise[1] = fileHeaderF6->config.noise[1];
        fileHeader->config.systemZCal[0] = fileHeaderF6->config.systemZCal[0];
        fileHeader->config.systemZCal[1] = fileHeaderF6->config.systemZCal[1];
        fileHeader->config.systemDCal = fileHeaderF6->config.systemDCal;
        fileHeader->config.systemPCal = fileHeaderF6->config.systemPCal;
        fileHeader->config.SNRThreshold = fileHeaderF6->config.SNRThreshold;
        fileHeader->config.SQIThreshold = fileHeaderF6->config.SQIThreshold;
        for (k = 0; k < MIN(8, RKMaximumFilterCount); k++) {
            fileHeader->config.waveformId[k] = fileHeaderF6->config.waveformId[k];
            fileHeader->config.ZCal[k][0] = fileHeaderF6->config.ZCal[k][0];
            fileHeader->config.ZCal[k][1] = fileHeaderF6->config.ZCal[k][1];
            fileHeader->config.DCal[k] = fileHeaderF6->config.DCal[k];
            fileHeader->config.PCal[k] = fileHeaderF6->config.PCal[k];
        }
        strncpy(fileHeader->config.waveformName, fileHeaderF6->config.waveformName, RKNameLength);
        strncpy(fileHeader->config.vcpDefinition, fileHeaderF6->config.vcpDefinition, sizeof(fileHeader->config.vcpDefinition) - 1);
        fileHeader->config.vcpDefinition[sizeof(fileHeader->config.vcpDefinition) - 1] = '\0';
        free(fileHeaderF6);
    } else if (fileHeader->format > RKRawDataFormat) {
        RKLog("Error. Sorry but I am not programmed to read format %d. Ask my father.\n", fileHeader->format);
        exit(EXIT_FAILURE);
    }
    if (fileHeader->format >= 7) {
        RKWaveform *waveform = RKWaveformReadFromReference(fid);
        fileHeader->config.waveform = waveform;
        fileHeader->config.waveformDecimate = RKWaveformCopy(waveform);
        RKWaveformDecimate(fileHeader->config.waveformDecimate, fileHeader->desc.pulseToRayRatio);
        if (fileHeader->config.pw[0] < 1.0e-8f) {
            RKLog("Warning. PW is near zero. Setting it to 1/fs = %.4f us\n", 1.0e6f * waveform->depth / waveform->fs);
            fileHeader->config.pw[0] = waveform->depth / waveform->fs;
        }
        // If signals span about -1.0 to +1.0, ZCal ~ 22.5, for -30,000 to +30,000, ZCal ~ -66
        fileHeader->config.systemZCal[0] = -56.0f;
        fileHeader->config.systemZCal[1] = -56.0f;
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
    RKLog(">fileHeader.preface = '%s'   format = %d\n", fileHeader->preface, fileHeader->format);
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
    RKLog(">config.sweepAzimuth = %.2f deg\n", config->sweepAzimuth);
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
    if (fileHeader->format >= 5) {
        RKWaveformSummary(config->waveform);
    }
    if (fileHeader->format >= 6) {
        RKWaveformSummary(config->waveformDecimate);
    }
}

size_t RKFileHeaderWriteToFid(RKFileHeader *header, FILE *fid) {
    int i;
    size_t r;
    r = fwrite(header, sizeof(RKFileHeader), 1, fid);
    if (r == 0) {
        RKLog("Error. Failed writing file header.\n");
        exit(EXIT_FAILURE);
    }
    RKWaveform *waveform = header->config.waveform;
    RKWaveFileGlobalHeader *waveHeader = (void *)malloc(sizeof(RKWaveFileGlobalHeader));
    memset(waveHeader, 0, sizeof(RKWaveFileGlobalHeader));
    strcpy(waveHeader->name, waveform->name);
    waveHeader->count = waveform->count;
    waveHeader->depth = waveform->depth;
    waveHeader->type = waveform->type;
    waveHeader->fc = waveform->fc;
    waveHeader->fs = waveform->fs;
    for (i = 0; i < waveform->count; i++) {
        waveHeader->filterCounts[i] = waveform->filterCounts[i];
    }
    r += fwrite(waveHeader, sizeof(RKWaveFileGlobalHeader), 1, fid);
    for (i = 0; i < waveform->count; i++) {
        // 32-B wave group header
        r += fwrite(waveform->filterAnchors[i], waveform->filterCounts[i] * sizeof(RKFilterAnchor), 1, fid);
        // Waveform samples (flexible size)
        r += fwrite(waveform->samples[i], waveform->depth * sizeof(RKComplex), 1, fid);
        r += fwrite(waveform->iSamples[i], waveform->depth * sizeof(RKInt16C), 1, fid);
    }
    free(waveHeader);
    return r;
}
