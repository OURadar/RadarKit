#include <RadarKit.h>
#include <getopt.h>

//

// User parameters in a struct
typedef struct user_params {
    char directory[RKMaximumFolderPathLength];
    char filename[RKNameLength];
    int verbose;
    bool output;
    bool compressedOutput;
    float SNRThreshold;
    float SQIThreshold;
} UserParams;

#pragma mark - Functions

static void showHelp() {
    char name[] = __FILE__;
    RKShowName();
    *strrchr(name, '.') = '\0';
    printf("Product Generator\n\n"
           "%s [options]\n\n"
           "OPTIONS:\n"
           "     Unless specifically stated, all options are interpreted in sequence. Some\n"
           "     options can be specified multiples times for repetitions. For example, the\n"
           "     verbosity is increased by repeating the option multiple times.\n"
           "\n"
           "  -c (--compress)\n"
           "         Generates compressed I/Q output file.\n"
           "\n"
           "  -d (--dir) " UNDERLINE("value") "\n"
           "         Specifies the directory to store the output files.\n"
           "\n"
           "  -h (--help)\n"
           "         Shows this help text.\n"
           "\n"
           "  -n (--no-output)\n"
           "         No output will be produced. Just do a dry run.\n"
           "\n"
           "  -v (--verbose)\n"
           "         Increases verbosity level, which can be specified multiple times.\n"
           "\n"
           "EXAMPLES:\n"
           "    Here are some examples of typical configurations.\n"
           "\n"
           "    -vn\n"
           "         Runs the program in verbose, and no-output mode.\n"
           "\n"
           "%s (RadarKit %s)\n\n",
           name,
           name,
           RKVersionString());
}


void proc(UserParams *arg) {

    int i = 0, j = 0, k = 0, o = 0, p = 0;
    uint32_t u32 = 0;
    char *timestr;
    bool m = false;
    bool more = true;
    int i0, i1 = -1;
    size_t bytes, mem = 0;
    suseconds_t usec = 0;
    long filesize = 0;
    struct timeval s, e;
    unsigned int r = 0;
    char *c;

    RKLog("Opening file %s ...", arg->filename);

    gettimeofday(&s, NULL);

    FILE *fid = fopen(arg->filename, "r");
    if (fid == NULL) {
        RKLog("Error. Unable to open file %s", arg->filename);
        exit(EXIT_FAILURE);
    }
    if (fseek(fid, 0L, SEEK_END)) {
        RKLog("Error. Unable to tell the file size.\n");
    } else {
        filesize = ftell(fid);
        RKLog("File size = %s B\n", RKUIntegerToCommaStyleString(filesize));
    }
    rewind(fid);

    RKFileHeader *fileHeader = RKFileHeaderInitFromFid(fid);
    RKConfig *config = &fileHeader->config;
    RKWaveform *waveform = config->waveform;

    if (arg->verbose && fileHeader->dataType == RKRawDataTypeNull) {
        RKLog("Info. fileHeader->preface = %s\n", fileHeader->preface);
        RKLog("Info. fileHeader->format = %d\n", fileHeader->format);
        RKLog("Info. fileHeader->dataType = %d (%s)\n",
              fileHeader->dataType,
              fileHeader->dataType == RKRawDataTypeFromTransceiver ? "Raw" :
              (fileHeader->dataType == RKRawDataTypeAfterMatchedFilter ? "Compressed" : "Unknown"));
    }
    if (fileHeader->dataType != RKRawDataTypeAfterMatchedFilter && fileHeader->dataType != RKRawDataTypeFromTransceiver) {
        if ((c = strrchr(arg->filename, '.')) != NULL) {
            if (!strcasecmp(".rkc", c)) {
                RKLog("Info. Assuming compressed data based on file extension ...\n");
                fileHeader->dataType = RKRawDataTypeAfterMatchedFilter;
            } else if (!strcasestr(".rkr", c)) {
                RKLog("Info. Assuming raw data based on file extension ...\n");
                fileHeader->dataType = RKRawDataTypeFromTransceiver;
            } else {
                fileHeader->dataType = RKRawDataTypeNull;
            }
        } else {
            fileHeader->dataType = RKRawDataTypeNull;
        }
    }
    if (fileHeader->dataType == RKRawDataTypeNull) {
        RKLog("Warning. Sorry but I cannot handle this file. Ask my father.\n");
        exit(EXIT_FAILURE);
    }

    if (arg->compressedOutput && fileHeader->dataType == RKRawDataTypeAfterMatchedFilter) {
        RKLog("Error. Cannot compress match-filter processed I/Q data.");
        exit(EXIT_FAILURE);
    }

    //

    RKBuffer pulseBuffer;
    RKBuffer rayBuffer;
    RKPulse *pulses[RKMaximumPulsesPerRay];
    RKRay *rays[RKMaximumRaysPerSweep];
    RKProduct *product;
    RKMomentScratch *space;

    // const RKProductList productList = RKProductListFloatATSR;
    const RKProductList productList = RKProductListFloatZVWDPR;

    // Ray capacity always respects pulseCapcity / pulseToRayRatio and SIMDAlignSize
    const uint32_t rayCapacity = ((uint32_t)ceilf((float)fileHeader->desc.pulseCapacity / fileHeader->desc.pulseToRayRatio / (float)RKMemoryAlignSize)) * RKMemoryAlignSize;
    if (fileHeader->dataType == RKRawDataTypeFromTransceiver) {
        u32 = fileHeader->desc.pulseCapacity;
    } else if (fileHeader->dataType == RKRawDataTypeAfterMatchedFilter) {
        u32 = (uint32_t)ceilf((float)rayCapacity * sizeof(int16_t) / RKMemoryAlignSize) * RKMemoryAlignSize / sizeof(int16_t);
    } else {
        RKLog("Error. Unable to handle dataType %d", fileHeader->dataType);
        exit(EXIT_FAILURE);
    }
    const uint32_t pulseCapacity = u32;
    bytes = RKPulseBufferAlloc(&pulseBuffer, pulseCapacity, RKMaximumPulsesPerRay);
    if (bytes == 0 || pulseBuffer == NULL) {
        RKLog("Error. Unable to allocate memory for I/Q pulses.\n");
        exit(EXIT_FAILURE);
    }
    mem += bytes;
    if (arg->verbose > 1) {
        RKLog("Pulse buffer occupies %s B  (%s pulses x %s gates)\n",
              RKUIntegerToCommaStyleString(bytes),
              RKIntegerToCommaStyleString(RKMaximumPulsesPerRay),
              RKIntegerToCommaStyleString(pulseCapacity));
    }
    bytes = RKRayBufferAlloc(&rayBuffer, rayCapacity, RKMaximumRaysPerSweep);
    if (bytes == 0 || rayBuffer == NULL) {
        RKLog("Error. Unable to allocate memory for rays.\n");
        exit(EXIT_FAILURE);
    }
    mem += bytes;
    if (arg->verbose > 1) {
        RKLog(">Ray buffer occupies %s B  (%s rays x %s gates)\n",
              RKUIntegerToCommaStyleString(bytes),
              RKIntegerToCommaStyleString(RKMaximumRaysPerSweep),
              RKIntegerToCommaStyleString(rayCapacity));
    }
    bytes = RKProductBufferAlloc(&product, 1, RKMaximumRaysPerSweep, config->pulseGateCount / fileHeader->desc.pulseToRayRatio);
    if (bytes == 0 || product == NULL) {
        RKLog("Error. Unable to allocate memory for products.\n");
        exit(EXIT_FAILURE);
    }
    if (arg->verbose > 1) {
        RKLog(">Product buffer occupies %s B\n",
              RKUIntegerToCommaStyleString(bytes));
    }
    mem += bytes;
    bytes = RKMomentScratchAlloc(&space, rayCapacity, false, NULL);
    if (bytes == 0 || space == NULL) {
        RKLog("Error. Unable to allocate memory for scratch space.\n");
        exit(EXIT_FAILURE);
    }
    mem += bytes;

    if (arg->verbose) {
        RKFileHeaderSummary(fileHeader);
    }

    char sweepBeginMarker[20] = "S", sweepEndMarker[20] = "E";
    if (rkGlobalParameters.showColor) {
        sprintf(sweepBeginMarker, "%sS%s", RKGetColorOfIndex(3), RKNoColor);
        sprintf(sweepEndMarker, "%sE%s", RKGetColorOfIndex(2), RKNoColor);
    }

    // Override some parameters with user supplied values
    if (isfinite(arg->SNRThreshold)) {
        config->SNRThreshold = arg->SNRThreshold;
    }
    if (isfinite(arg->SQIThreshold)) {
        config->SQIThreshold = arg->SQIThreshold;
    }

    // Propagate RKConfig parameters to scratch space
    space->config = config;
    space->gateCount = rayCapacity;
    space->gateSizeMeters = fileHeader->config.pulseGateSize * fileHeader->desc.pulseToRayRatio;
    // Because the pulse-compression engine uses unity noise gain filters, there is an inherent gain difference at different sampling rate
    // The gain difference is compensated here with a calibration factor if raw-sampling is at 1-MHz (150-m)
    // The number 60 is for conversion of range from meters to kilometers in the range correction term.
    space->samplingAdjustment = 10.0f * log10f(space->gateSizeMeters / (150.0f * fileHeader->desc.pulseToRayRatio)) + 60.0f;
    RKCalibratorSimple(NULL, space);
    // The rest of the constants
    space->noise[0] = config->noise[0];
    space->noise[1] = config->noise[1];
    space->velocityFactor = 0.25f * fileHeader->desc.wavelength / config->prt[0] / M_PI;
    space->widthFactor = fileHeader->desc.wavelength / config->prt[0] / (2.0f * sqrtf(2.0f) * M_PI);
    space->KDPFactor = 1.0f / space->gateSizeMeters;

    if (arg->verbose > 1) {
        float *t;
        t = space->S2Z[0];  RKLog(">S2Z = %.1e, %.1e, %.1e, %.1e, %.1e ...\n", t[0], t[10], t[50], t[100], t[250]);
        t = space->dcal;    RKLog(">dcal = %.1f, %.1f, %.1f, %.1f, %.1f ...\n", t[0], t[10], t[50], t[100], t[250]);
        RKLog(">Memory usage = %s B\n", RKIntegerToCommaStyleString(mem));
    }
    RKMarker marker = fileHeader->config.startMarker;

    if (arg->verbose) {
        RKLog("Processing Parameters:\n");
        RKLog(">space.noise = %.2f, %.2f ADU^2\n", space->noise[0], space->noise[1]);
        RKLog(">space->config->SNRThreshold = %.2f dB%s\n", space->config->SNRThreshold, isfinite(arg->SNRThreshold) ? " (user)" : "");
        RKLog(">space->config->SQIThreshold = %.2f%s\n", space->config->SQIThreshold, isfinite(arg->SQIThreshold) ? " (user)" : "");
        if (arg->verbose > 1) {
            RKLog("Initial marker = %04x / %04x\n", marker, RKMarkerScanTypePPI);
        }
    }
    // Test pulse buffer
    if (arg->verbose > 1) {
        for (p = 0; p < RKMaximumPulsesPerRay; p++) {
            RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, p);
            for (j = 0; j < 2; j++) {
                RKComplex *x = RKGetComplexDataFromPulse(pulse, j);
                RKIQZ z = RKGetSplitComplexDataFromPulse(pulse, j);
                if (x == NULL || z.i == NULL || z.q == NULL) {
                    RKLog("Error. Unexpected behavior from pulse buffer %p %p %p\n", x, z.i, z.q);
                }
                for (i = 0; i < pulseCapacity; i++) {
                    z.i[i] = x[i].i;
                    z.q[i] = x[i].q;
                }
            }
        }
        if (p == RKMaximumPulsesPerRay && i == pulseCapacity) {
            RKLog("Pulse buffer checked.  p = %d / %d   i = %d / %d\n", p, RKMaximumPulsesPerRay, i, pulseCapacity);
        }
    }

    //printf("fpos = %s\n", RKUIntegerToCommaStyleString(ftell(fid)));

    // Initialize a pulse buffer for pulse copression

    const uint32_t nfft = 1 << (int)ceilf(log2f((float)MIN(RKMaximumGateCount, pulseCapacity)));

    RKFFTModule *fftModule = RKFFTModuleInit(nfft, arg->verbose);

    RKCompressionScratch *scratch;
    mem = RKCompressionScratchAlloc(&scratch, pulseCapacity, arg->verbose, "PGEN");
    scratch->fftModule = fftModule;

    RKLog("mem = %s   nfft = %s", RKIntegerToCommaStyleString(mem), RKIntegerToCommaStyleString(nfft));

    RKComplex *filters[RKMaximumWaveformCount][config->waveform->count];

    bytes = nfft * sizeof(RKComplex);
    for (k = 0; k < config->waveform->count; k++) {
        for (j = 0; j < config->waveform->filterCounts[k]; j++) {
            const int origin = config->waveform->filterAnchors[k][j].origin;
            const int length = config->waveform->filterAnchors[k][j].length;
            RKLog(
                "k = %d   j = %d   origin = %d   length = %d   nfft = %s   tail = %s\n",
                k, j, origin, length,
                RKIntegerToCommaStyleString(nfft),
                RKIntegerToCommaStyleString(nfft - length)
            );
            POSIX_MEMALIGN_CHECK(posix_memalign((void **)&filters[k][j], RKMemoryAlignSize, bytes))
            memcpy(filters[k][j], &config->waveform->samples[k][origin], length * sizeof(RKComplex));
            memset(&filters[k][j][length], 0, (nfft - length) * sizeof(RKComplex));

            #if defined(_DEBUG_SHOW_FILTER_COEFFICIENTS_)
            printf("config->waveform->filterAnchors[k=%d][j=%d].length = %d   .filterGain = %.2f dB\n", k, j,
                config->waveform->filterAnchors[k][j].length, config->waveform->filterAnchors[k][j].filterGain);
            for (i = 0; i < config->waveform->filterAnchors[j][0].length; i++) {
                const float a = sqrtf(filters[k][j][i].i * filters[k][j][i].i + filters[k][j][i].q * filters[k][j][i].q);
                printf("config->waveform->samples[%d][%3d] = %+8.5f%+8.5fi  -->  filter[%d][%d][%3d] = %8.5f%+8.5fi (%.4f)\n",
                    k, i, config->waveform->samples[k][i].i, config->waveform->samples[k][i].q,
                    k, j, i, filters[k][j][i].i, filters[k][j][i].q,
                    a);
            }
            for (; i < nfft; i++) {
                const float a = sqrtf(filters[k][j][i].i * filters[k][j][i].i + filters[k][j][i].q * filters[k][j][i].q);
                printf("config->waveform->samples[%d][%d] =  _._____+_._____i  -->  filter[%d][%d][%3d] = %8.5f%+8.5fi (%.4f)\n",
                    k, i,
                    k, j, i, filters[k][j][i].i, filters[k][j][i].q,
                    a);
            }
            printf("\n");
            #endif
        }
    }

    // Compressed Output
    FILE *outfid = NULL;
    if (arg->compressedOutput) {
        RKFileHeader *outputFileHeader = (void *)malloc(sizeof(RKFileHeader));
        memset(outputFileHeader, 0, sizeof(RKFileHeader));
        RKWaveFileGlobalHeader *waveGlobalHeader = (void *)malloc(sizeof(RKWaveFileGlobalHeader));
        memset(waveGlobalHeader, 0, sizeof(RKWaveFileGlobalHeader));

        char outputFilename[256];
        strcpy(outputFilename, arg->filename);
        c = strrchr(outputFilename, '.');
        if (c) {
            sprintf(c, ".rkc");
        } else {
            strcat(outputFilename, ".rkc");
        }
        RKLog("Output: \033[38;5;82m%s\033[m\n", outputFilename);
        outfid = fopen(outputFilename, "w");

        sprintf(outputFileHeader->preface, "RadarKit/IQ");
        outputFileHeader->dataType = RKRawDataTypeAfterMatchedFilter;
        outputFileHeader->format = RKRawDataFormat;
        memcpy(&outputFileHeader->desc, &fileHeader->desc, sizeof(RKRadarDesc));
        memcpy(&outputFileHeader->config, &fileHeader->config, sizeof(RKConfig));
        outputFileHeader->bytes[sizeof(RKFileHeader) - 3] = 'E';
        outputFileHeader->bytes[sizeof(RKFileHeader) - 2] = 'O';
        outputFileHeader->bytes[sizeof(RKFileHeader) - 1] = 'L';
        fwrite(outputFileHeader, sizeof(RKFileHeader), 1, outfid);

        strcpy(waveGlobalHeader->name, waveform->name);
        waveGlobalHeader->count = waveform->count;
        waveGlobalHeader->depth = waveform->depth;
        waveGlobalHeader->type = waveform->type;
        waveGlobalHeader->fc = waveform->fc;
        waveGlobalHeader->fs = waveform->fs;
        for (i = 0; i < waveform->count; i++) {
            waveGlobalHeader->filterCounts[i] = waveform->filterCounts[i];
        }
        fwrite(waveGlobalHeader, sizeof(RKWaveFileGlobalHeader), 1, outfid);
        for (i = 0; i < waveform->count; i++) {
            fwrite(waveform->filterAnchors[i], sizeof(RKFilterAnchor), waveform->filterCounts[i], outfid);
            fwrite(waveform->samples[i], sizeof(RKComplex), waveform->depth, outfid);
            fwrite(waveform->iSamples[i], sizeof(RKInt16C), waveform->depth, outfid);
        }

        free(waveGlobalHeader);
        free(outputFileHeader);
    }

    // Since we are using pulseBuffer as a linear buffer, we can just get the anchors ahead of time
    for (p = 0; p < RKMaximumPulsesPerRay; p++) {
        pulses[p] = RKGetPulseFromBuffer(pulseBuffer, p);
    }
    for (r = 0; r < RKMaximumRaysPerSweep; r++) {
        rays[r] = RKGetRayFromBuffer(rayBuffer, r);
    }

    p = 0;    // total pulses per ray
    r = 0;    // total rays per sweep
    for (k = 0; k < RKRawDataRecorderDefaultMaximumRecorderDepth; k++) {
        RKPulse *pulse = pulses[p++];

        j = RKReadPulseFromFileReference(pulse, fileHeader, fid);
        if (j == RKResultSuccess) {
            if (p == 0 && arg->verbose) {
                RKLog("pulse[0].gateCount = %s\n", RKIntegerToCommaStyleString(pulse->header.gateCount));
                RKLog("pulse[0].downSampledGateCount = %s\n", RKIntegerToCommaStyleString(pulse->header.downSampledGateCount));
            }
            if ((marker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI) {
                i0 = (int)floorf(pulse->header.azimuthDegrees);
            } else if ((marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI) {
                i0 = (int)floorf(pulse->header.elevationDegrees);
            } else {
                i0 = 360 * (int)floorf(pulse->header.elevationDegrees - 0.25f) + (int)floorf(pulse->header.azimuthDegrees);
            }
            if (fileHeader->dataType == RKRawDataTypeFromTransceiver) {
                // Pulse compression
                int blindGateCount = config->waveform->depth;
                const int gid = pulse->header.i % config->waveform->count;
                for (j = 0; j < config->waveform->filterCounts[gid]; j++) {
                    int planIndex = (int)ceilf(log2f((float)MIN(pulse->header.gateCount - config->waveform->filterAnchors[gid][j].inputOrigin,
                                                                config->waveform->filterAnchors[gid][j].maxDataLength)));

                    #if defined(_DEBUG_SHOW_PULSE_HEADER)
                    printf("C:%s  gid = %d   j = %d   planIndex = %d   %08x (%s)\n",
                        RKIntegerToCommaStyleString(pulse->header.gateCount), gid, j, planIndex,
                        pulse->header.s, pulse->header.s & RKPulseStatusCompressed ? "C" : "R");
                    #endif

                    // Compression
                    scratch->pulse = pulse;
                    scratch->filter = filters[gid][j];
                    scratch->filterAnchor = &config->waveform->filterAnchors[gid][j];
                    scratch->planIndex = planIndex;

                    // Call the compressor
                    RKBuiltInCompressor(NULL, scratch);

                    // Copy over the parameters used
                    for (o = 0; o < 2; o++) {
                        pulse->parameters.planIndices[o][j] = planIndex;
                        pulse->parameters.planSizes[o][j] = fftModule->plans[planIndex].size;
                    }
                }
                pulse->parameters.filterCounts[0] = j;
                pulse->parameters.filterCounts[1] = j;
                pulse->header.pulseWidthSampleCount = blindGateCount;
                pulse->header.gateCount += 1 - blindGateCount;
                pulse->header.s |= RKPulseStatusCompressed;

                // Down-sampling
                int stride = MAX(1, fileHeader->desc.pulseToRayRatio);
                if (stride > 1) {
                    pulse->header.downSampledGateCount = (pulse->header.gateCount + stride - 1) / stride;
                    for (o = 0; o < 2; o++) {
                        RKComplex *Y = RKGetComplexDataFromPulse(pulse, o);
                        RKIQZ Z = RKGetSplitComplexDataFromPulse(pulse, o);
                        for (i = 0, j = 0; j < pulse->header.gateCount; i++, j+= stride) {
                            Y[i].i = Y[j].i;
                            Y[i].q = Y[j].q;
                            Z.i[i] = Z.i[j];
                            Z.q[i] = Z.q[j];
                        }
                    }
                } else {
                    pulse->header.downSampledGateCount = pulse->header.gateCount;
                }

                // Compressed Output
                if (arg->compressedOutput) {
                    fwrite(&pulse->header, sizeof(RKPulseHeader), 1, outfid);
                    fwrite(RKGetComplexDataFromPulse(pulse, 0), sizeof(RKComplex), pulse->header.downSampledGateCount, outfid);
                    fwrite(RKGetComplexDataFromPulse(pulse, 1), sizeof(RKComplex), pulse->header.downSampledGateCount, outfid);
                }
            } else {
                pulse->header.s |= RKPulseStatusCompleteForMoments;
            }

            // Mark for process later
            m = false;
            if (i1 != i0 || p == RKMaximumPulsesPerRay) {
                if (arg->verbose > 2) {
                    RKLog("i1 = %d   i0 = %d   p = %s\n", i1, i0, RKIntegerToCommaStyleString(p));
                }
                i1 = i0;
                if (p > 0) {
                    m = true;
                }
            }
            if (arg->verbose > 2) {
                int du = (int)(pulse->header.time.tv_usec - usec);
                if (du < 0) {
                    du += 1000000;
                }
                timestr = RKTimevalToString(pulse->header.time, 1084, true);
                printf("P:%05d/%06ju/%05d %s(%06d)   C%d   E%5.2f, A%6.2f  %s x %.1fm %d/%d %02x %s%s%s\n",
                       k, (unsigned long)pulse->header.i, p, timestr, du,
                       pulse->header.configIndex,
                       pulse->header.elevationDegrees, pulse->header.azimuthDegrees,
                       RKIntegerToCommaStyleString(pulse->header.downSampledGateCount),
                       pulse->header.gateSizeMeters * fileHeader->desc.pulseToRayRatio,
                       i0, pulse->header.positionIndex,
                       pulse->header.marker,
                       m ? "*" : "",
                       pulse->header.marker & RKMarkerSweepBegin ? "S" : "",
                       pulse->header.marker & RKMarkerSweepEnd ? "E" : ""
                       );
            }
        } else {
            more = false;
            m = true;
            if (arg->verbose > 1) {
                RKLog("EOF p = %d\n", p);
            }
        }
        // Process ...
        if (m) {
            if (p >= 2) {
                RKPulse *S = pulses[0];
                RKPulse *E = pulses[p - 1];

                // Beamwidth
                float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees,   E->header.azimuthDegrees);
                float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);

                // Consolidate the pulse marker into ray marker
                marker = RKMarkerNull;
                for (i = 0; i < p; i++) {
                    marker |= pulses[i]->header.marker;
                }

                // Get a ray from the buffer, set the ray headers
                // NOTE: pulse->header.timeDouble still has the original time from RKClock, which has a different epoch
                RKRay *ray = rays[r++];
                ray->header.startTime       = S->header.time;
                ray->header.startTimeDouble = (double)S->header.time.tv_sec + 1.0e-6 * (double)S->header.time.tv_usec;
                ray->header.startAzimuth    = S->header.azimuthDegrees;
                ray->header.startElevation  = S->header.elevationDegrees;
                ray->header.endTime         = E->header.time;
                ray->header.endTimeDouble   = (double)E->header.time.tv_sec + 1.0e-6 * (double)E->header.time.tv_usec;
                ray->header.endAzimuth      = E->header.azimuthDegrees;
                ray->header.endElevation    = E->header.elevationDegrees;
                ray->header.configIndex     = E->header.configIndex;
                ray->header.gateCount       = E->header.downSampledGateCount;
                ray->header.gateSizeMeters  = E->header.gateSizeMeters * fileHeader->desc.pulseToRayRatio;
                ray->header.sweepElevation  = config->sweepElevation;
                ray->header.sweepAzimuth    = config->sweepAzimuth;
                ray->header.pulseCount      = p;
                ray->header.marker          = marker;

                // Process using a selected method
                space->gateCount = pulse->header.downSampledGateCount;
                // RKPulsePairHop(space, pulses, p);
                // RKPulsePairATSR(space, pulses, p);
                RKPulsePair(space, pulses, p);
                makeRayFromScratch(space, ray);

                // Timestamp
                timestr = RKTimevalToString(ray->header.startTime, 1084, true);
                if (arg->verbose == 2) {
                    printf("P:%05d R%3d %s [E%.2f, A%.2f]  %s%6.2f-%6.2f  (%4.2f, p%d)  G%s  M%05X  %s%s\n",
                           k, r-1, timestr,
                           config->sweepElevation, config->sweepAzimuth,
                           (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "E" : "A",
                           (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? S->header.elevationDegrees : S->header.azimuthDegrees,
                           (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? E->header.elevationDegrees : E->header.azimuthDegrees,
                           (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? deltaElevation : deltaAzimuth,
                           p,
                           RKIntegerToCommaStyleString(space->gateCount),
                           ray->header.marker,
                           ray->header.marker & RKMarkerSweepBegin ? sweepBeginMarker : " ",
                           ray->header.marker & RKMarkerSweepEnd ? sweepEndMarker : " ");
                } else if (arg->verbose > 2) {
                    // Show with some data
                    RKComplex *cdata = RKGetComplexDataFromPulse(pulse, 0);
                    float *data = RKGetFloatDataFromRay(ray, RKProductIndexZ);
                    printf("p:%05d R%3d %s [E%.2f, A%.2f]  %s%6.2f-%6.2f  (%4.2f, p%d)  G%s  M%05X  %s%s   %.1f, %.1f, %.1f, %.1f, %.1f   %.1f, %.1f, %.1f, %.1f, %.1f\n",
                           k, r-1, timestr,
                           config->sweepElevation, config->sweepAzimuth,
                           (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "E" : "A",
                           (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? S->header.elevationDegrees : S->header.azimuthDegrees,
                           (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? E->header.elevationDegrees : E->header.azimuthDegrees,
                           (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? deltaElevation : deltaAzimuth,
                           p,
                           RKIntegerToCommaStyleString(space->gateCount),
                           ray->header.marker,
                           ray->header.marker & RKMarkerSweepBegin ? sweepBeginMarker : " ",
                           ray->header.marker & RKMarkerSweepEnd ? sweepEndMarker : " ",
                           data[0], data[10], data[100], data[250], data[500],
                           cdata[0].i, cdata[10].i, cdata[100].i, cdata[250].i, cdata[500].i);
                }
                if (arg->verbose) {
                    printf("E%4.1f[%4.1f] A%5.1f[%5.1f] ", ray->header.startElevation, ray->header.sweepElevation, ray->header.startAzimuth, ray->header.sweepAzimuth);
                    RKShowVecFloatLowPrecision(" Z = ", space->Z[0], 10);
                }
            } else if (p > 1) {
                RKPulse *S = pulses[0];
                RKPulse *E = pulses[p - 1];

                // Beamwidth
                float deltaAzimuth   = RKGetMinorSectorInDegrees(S->header.azimuthDegrees,   E->header.azimuthDegrees);
                float deltaElevation = RKGetMinorSectorInDegrees(S->header.elevationDegrees, E->header.elevationDegrees);

                // Consolidate the pulse marker into ray marker
                marker = RKMarkerNull;
                for (i = 0; i < p; i++) {
                    marker |= pulses[i]->header.marker;
                }

                // Timestamp
                if (arg->verbose > 1) {
                    timestr = RKTimevalToString(S->header.time, 1084, true);
                    RKLog(">%05d r=%3d %s p=%d   [E%.2f, A%.2f]  %s%6.2f-%6.2f  (%4.2f)  G%s  M%05x\n",
                           k, r-1, timestr, p,
                           config->sweepElevation, config->sweepAzimuth,
                           (marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "E" : "A",
                           (marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? S->header.elevationDegrees : S->header.azimuthDegrees,
                           (marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? E->header.elevationDegrees : E->header.azimuthDegrees,
                           (marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? deltaElevation : deltaAzimuth,
                           RKIntegerToCommaStyleString(S->header.downSampledGateCount),
                           marker);
                }
            }
            if (fileHeader->dataType == RKRawDataTypeFromTransceiver) {
                for (j = 0; j < 2; j++) {
                    RKInt16C *x = RKGetInt16CDataFromPulse(pulses[0], j);
                    RKInt16C *src = RKGetInt16CDataFromPulse(pulse, j);
                    memcpy(x, src, pulse->header.gateCount * sizeof(RKInt16C));
                }
            } else {
                for (j = 0; j < 2; j++) {
                    RKComplex *x = RKGetComplexDataFromPulse(pulses[0], j);
                    RKComplex *src = RKGetComplexDataFromPulse(pulse, j);
                    memcpy(x, src, pulse->header.downSampledGateCount * sizeof(RKComplex));
                    RKIQZ z = RKGetSplitComplexDataFromPulse(pulses[0], j);
                    for (i = 0; i < pulse->header.downSampledGateCount; i++) {
                        z.i[i] = x[i].i;
                        z.q[i] = x[i].q;
                    }
                }
            }
            // Use the end pulse as the start pulse of next ray
            memcpy(pulses[0], pulse, sizeof(RKPulseHeader));
            p = 1;
        } // if (m) ...
        if (!more) {
            // Break here so k does not increase one more
            break;
        }
        usec = pulse->header.time.tv_usec;
    }
    if (arg->verbose) {
        RKLog("fpos = %s / %s   pulse count = %s   ray count = %s\n",
              RKUIntegerToCommaStyleString(ftell(fid)), RKUIntegerToCommaStyleString(filesize),
              RKIntegerToCommaStyleString(k),
              RKIntegerToCommaStyleString(r));
    }
    if (ftell(fid) != filesize) {
        RKLog("Warning. There is leftover in the file.");
    } else if (r == 0) {
        RKLog("No rays generated. Perhaps this is a transition file.\n");
    }

    fclose(fid);
    if (arg->compressedOutput && outfid) {
        RKLog("Closing output file ...\n");
        fclose(outfid);
    }

    gettimeofday(&e, NULL);
    double dt = RKTimevalDiff(e, s);
    RKLog("Elapsed time = %.3f s\n", dt);

    // Allocate a sweep object
    if (r > 0) {
        RKSweep *sweep = (RKSweep *)malloc(sizeof(RKSweep));
        if (sweep == NULL) {
            RKLog("Error. Unable to allocate memory.\n");
            exit(EXIT_FAILURE);
        }
        memset(sweep, 0, sizeof(RKSweep));

        // Pick the start, transition and end rays
        k = 0;
        RKRay *S = rays[k];
        RKRay *E = rays[k + r - 1];

        // Consolidate some other information and check consistencies
        RKProductList overallMomentList = 0;
        uint8_t gateCountWarningCount = 0;
        uint8_t gateSizeWarningCount = 0;
        for (i = k + 1; i < k  + r - 1; i++) {
            overallMomentList |= rays[i]->header.productList;
            if (rays[i]->header.gateCount != S->header.gateCount) {
                if (++gateCountWarningCount < 5) {
                    RKLog("Warning. Inconsistent gateCount. ray[%s] has %s vs S has %s\n",
                          RKIntegerToCommaStyleString(i), RKIntegerToCommaStyleString(rays[i]->header.gateCount),
                          RKIntegerToCommaStyleString(S->header.gateCount));
                } else if (gateCountWarningCount == 5) {
                    RKLog("Warning. Inconsistent gateCount more than 5 rays / sweep.\n");
                }
            }
            if (rays[i]->header.gateSizeMeters != S->header.gateSizeMeters) {
                if (++gateSizeWarningCount < 5) {
                    RKLog("Warning. Inconsistent gateSizeMeters. ray[%s] has %s vs S has %s\n",
                          RKIntegerToCommaStyleString(i), RKFloatToCommaStyleString(rays[i]->header.gateSizeMeters),
                          RKFloatToCommaStyleString(S->header.gateSizeMeters));
                } else if (gateSizeWarningCount == 5) {
                    RKLog("Warning. Inconsistent gateSize more than 5 rays / sweep.\n");
                }
            }
        }
        if (arg->verbose > 1) {
            RKLog("productList = %016xh\n", overallMomentList);
        }

        // Populate the contents
        sweep->header.rayCount = r;
        sweep->header.gateCount = S->header.gateCount;
        sweep->header.gateSizeMeters = S->header.gateSizeMeters;
        sweep->header.startTime = S->header.startTimeDouble;
        sweep->header.endTime = E->header.endTimeDouble;
        sweep->header.productList = overallMomentList;
        sweep->header.isPPI = (config->startMarker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI;
        sweep->header.isRHI = (config->startMarker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI;
        sweep->header.external = true;
        memcpy(&sweep->header.desc, &fileHeader->desc, sizeof(RKRadarDesc));
        memcpy(&sweep->header.config, config, sizeof(RKConfig));
        memcpy(sweep->rays, rays + k, r * sizeof(RKRay *));
        // Make a suggested filename as .../[DATA_PATH]/20170119/PX10k-20170119-012345-E1.0 (no symbol and extension)
        k = sprintf(sweep->header.filename, "%s/", arg->directory);
        k += sprintf(sweep->header.filename + k, "%s/", RKTimeDoubleToString(sweep->header.startTime, 800, false));
        k += sprintf(sweep->header.filename + k, "%s-", fileHeader->desc.filePrefix);
        k += sprintf(sweep->header.filename + k, "%s-", RKTimeDoubleToString(sweep->header.startTime, 860, false));
        if (sweep->header.isPPI) {
            k += snprintf(sweep->header.filename + k, 6, "E%.1f", sweep->header.config.sweepElevation);
        } else if (sweep->header.isRHI) {
            k += snprintf(sweep->header.filename + k, 6, "A%.1f", sweep->header.config.sweepAzimuth);
        } else {
            k += snprintf(sweep->header.filename + k, 6, "N%03d", sweep->header.rayCount);
        }
        if (k > RKMaximumFolderPathLength + RKMaximumPrefixLength + 25 + RKMaximumFileExtensionLength) {
            RKLog("Error. Suggested filename %s is longer than expected.\n", sweep->header.filename);
        }
        if (arg->verbose) {
            RKLog("Output %s\n", sweep->header.filename);
        }
        RKPreparePath(sweep->header.filename);

        // Initialize a list based on desired moment list. This variable will become all zeros after the next for-loop
        RKProductList list = sweep->header.productList & productList;

        // Base products
        int productCount = __builtin_popcount(list);
        RKLog("productCount = %d\n", productCount);
        for (p = 0; p < productCount; p++) {
            product->desc = RKGetNextProductDescription(&list);
            RKProductInitFromSweep(product, sweep);
            sprintf(product->header.suggestedFilename, "%s-%s.nc", sweep->header.filename, product->desc.symbol);
            if (arg->output) {
                RKProductFileWriterNC(product, product->header.suggestedFilename);
            }
            RKLog(">%d: %s%s\n", p, product->header.suggestedFilename, arg->output ? "" : " -");
        }

        RKSweepFree(sweep);
    }

    if (fileHeader->config.waveform) {
        RKWaveformFree(fileHeader->config.waveform);
        RKWaveformFree(fileHeader->config.waveformDecimate);
    }
    RKPulseBufferFree(pulseBuffer);
    RKRayBufferFree(rayBuffer);
    RKProductBufferFree(product, 1);
    RKMomentScratchFree(space);
    RKFFTModuleFree(fftModule);

    for (k = 0; k < config->waveform->count; k++) {
        for (j = 0; j < config->waveform->filterCounts[k]; j++) {
            RKLog("k = %d  j = %d  len = %u\n", k, j, config->waveform->filterAnchors[k][j].length);
            free(filters[k][j]);
        }
    }

    RKCompressionScratchFree(scratch);

    free(fileHeader);

    return;
}

#pragma mark - Main

//
//
//  M A I N
//
//

int main(int argc, const char **argv) {

    int k, s;
    char str[1024];
    char name[] = __FILE__;
    char *c = strrchr(name, '/');
    if (c) {
        strncpy(name, c + 1, sizeof(name) - 1);
    }
    c = strrchr(name, '.');
    if (c) {
        *c = '\0';
    }
    RKSetProgramName(name);
    RKSetWantScreenOutput(true);

    char *term = getenv("TERM");
    if (term == NULL || (strcasestr(term, "color") == NULL && strcasestr(term, "ansi") == NULL)) {
        RKSetWantColor(false);
    }
    if (argc < 2) {
        fprintf(stderr, "Please supply a filename.\n");
        return EXIT_FAILURE;
    }

    // A struct to collect user input
    UserParams *arg = (UserParams *)malloc(sizeof(UserParams));
    memset(arg, 0, sizeof(UserParams));
    arg->SNRThreshold = NAN;
    arg->SQIThreshold = NAN;
    arg->output = true;

    // Command line options
    struct option options[] = {
        {"alarm"             , no_argument      , NULL, 'A'},    // ASCII 65 - 90 : A - Z
        {"compress"          , no_argument      , NULL, 'c'},
        {"dir"               , required_argument, NULL, 'd'},
        {"help"              , no_argument      , NULL, 'h'},
        {"no-output"         , no_argument      , NULL, 'n'},
        {"snr"               , required_argument, NULL, 's'},
        {"sqi"               , required_argument, NULL, 'q'},
        {"verbose"           , no_argument      , NULL, 'v'},
        {0, 0, 0, 0}
    };
    s = 0;
    for (k = 0; k < sizeof(options) / sizeof(struct option); k++) {
        struct option *o = &options[k];
        s += snprintf(str + s, sizeof(str) - s, "%c%s", o->val, o->has_arg == required_argument ? ":" : (o->has_arg == optional_argument ? "::" : ""));
    }
    optind = 1;
    int opt, ind = 0;
    while ((opt = getopt_long(argc, (char * const *)argv, str, options, &ind)) != -1) {
        switch (opt) {
            case 'A':
                break;
            case 'c':
                arg->compressedOutput = true;
                break;
            case 'd':
                strcpy(arg->directory, optarg);
                break;
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
            case 'n':
                arg->output = false;
                break;
            case 'q':
                arg->SQIThreshold = atof(optarg);
                break;
            case 's':
                arg->SNRThreshold = atof(optarg);
                break;
            case 'v':
                arg->verbose++;
                break;
            default:
                break;
        }
    }
    optind = MIN(argc - 1, optind);
    strcpy(arg->filename, argv[optind]);
    if (strlen(arg->directory)) {
        c = strrchr(arg->directory, '/');
        if (c) {
            *c = '\0';
        }
    } else {
        sprintf(arg->directory, "data/moment");
    }

    if (arg->verbose > 1) {
        printf("str = %s   ind = %d   optind = %d\n", str, ind, optind);
        printf("argc = %d   argv[%d] = %s\n", argc, optind, argv[optind]);
        printf("arg->verbose = %d\n", arg->verbose);
        printf("arg->directory = %s\n", arg->directory);
        printf("arg->filename = %s\n", arg->filename);
        printf("arg->output = %s\n", arg->output ? "true" : "false");
        printf("arg->compressedOutput = %s\n", arg->compressedOutput ? "true" : "false");
    }

    // Show framework name & version
    if (arg->verbose) {
        RKShowName();
    }

    // Now we process
    proc(arg);

    free(arg);

    exit(EXIT_SUCCESS);
}
