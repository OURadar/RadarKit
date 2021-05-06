#include <RadarKit.h>
#include <getopt.h>

//

// User parameters in a struct
typedef struct user_params {
    char directory[RKMaximumFolderPathLength];
    char filename[RKNameLength];
    int verbose;
    bool output;
    float SNRThreshold;
    float SQIThreshold;
} UserParams;

// Make some private functions available

int prepareScratch(RKScratch *);
int makeRayFromScratch(RKScratch *, RKRay *);
size_t RKScratchAlloc(RKScratch **space, const uint32_t capacity, const uint8_t lagCount, const uint8_t fftOrder, const bool);
void RKScratchFree(RKScratch *);

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

static void timeval2str(char *timestr, const struct timeval time) {
    size_t bytes = strftime(timestr, 9, "%T", gmtime(&time.tv_sec));
    snprintf(timestr + bytes, 6, ".%04d", (int)time.tv_usec / 100);
}

void proc(UserParams *arg) {
    
    int i = 0, j = 0, k = 0, p = 0, r = 0;
    uint32_t u32 = 0;
    char timestr[16];
    bool m = false;
    bool more = true;
    int i0, i1 = -1;
    size_t bytes, mem = 0;
    suseconds_t usec = 0;
    long filesize = 0;
    struct timeval s, e;

    RKLog("Opening file %s ...", arg->filename);
    
    gettimeofday(&s, NULL);
    RKFileHeader *fileHeader = (RKFileHeader *)malloc(sizeof(RKFileHeader));
    RKWaveform *waveform = NULL;
    RKConfig *config = &fileHeader->config;

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
    fread(fileHeader, sizeof(RKFileHeader), 1, fid);
    if (fileHeader->buildNo <= 4) {
        RKLog("Error. Sorry but I am not programmed to read buildNo = %d. Ask my father.\n", fileHeader->buildNo);
        exit(EXIT_FAILURE);
    } else if (fileHeader->buildNo == 5) {
        rewind(fid);
        RKFileHeaderV1 *fileHeaderV1 = (RKFileHeaderV1 *)malloc(sizeof(RKFileHeaderV1));
        fread(fileHeaderV1, sizeof(RKFileHeaderV1), 1, fid);
        fileHeader->dataType = fileHeaderV1->dataType;
        fileHeader->desc = fileHeaderV1->desc;
        memcpy(&fileHeader->config, &fileHeaderV1->config, sizeof(RKConfigV1));
        fileHeader->config.waveform = NULL;
        fileHeader->config.waveformDecimate = NULL;
        free(fileHeaderV1);
    } else {
        waveform = RKWaveformReadFromReference(fid);
        fileHeader->config.waveform = waveform;
        fileHeader->config.waveformDecimate = RKWaveformCopy(waveform);
        RKWaveformDecimate(fileHeader->config.waveformDecimate, fileHeader->desc.pulseToRayRatio);
    }

    RKBuffer pulseBuffer;
    RKBuffer rayBuffer;
    RKPulse *pulses[RKMaximumPulsesPerRay];
    RKRay *rays[RKMaximumRaysPerSweep];
    RKProduct *product;
    RKScratch *space;

    const RKBaseMomentList momentList = RKBaseMomentListProductZVWDPR;

    // Ray capacity always respects pulseCapcity / pulseToRayRatio and SIMDAlignSize
    const uint32_t rayCapacity = ((uint32_t)ceilf((float)fileHeader->desc.pulseCapacity / fileHeader->desc.pulseToRayRatio / (float)RKSIMDAlignSize)) * RKSIMDAlignSize;
    if (fileHeader->dataType == RKRawDataTypeFromTransceiver) {
        u32 = fileHeader->desc.pulseCapacity;
    } else if (fileHeader->dataType == RKRawDataTypeAfterMatchedFilter) {
        u32 = (uint32_t)ceilf((float)rayCapacity * sizeof(int16_t) / RKSIMDAlignSize) * RKSIMDAlignSize / sizeof(int16_t);
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
    const uint8_t fftOrder = (uint8_t)ceilf(log2f((float)RKMaximumPulsesPerRay));
    bytes = RKScratchAlloc(&space, rayCapacity, RKMaximumLagCount, fftOrder, false);
    if (bytes == 0 || space == NULL) {
        RKLog("Error. Unable to allocate memory for scratch space.\n");
        exit(EXIT_FAILURE);
    }
    mem += bytes;

    if (arg->verbose) {
        long fpos = ftell(fid);
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, p);
        RKReadPulseFromFileReference(pulse, fileHeader->dataType, fid);
        fseek(fid, fpos, SEEK_SET);
        RKLog(">fileHeader.preface = '%s'   buildNo = %d\n", fileHeader->preface, fileHeader->buildNo);
        RKLog(">fileHeader.dataType = '%s'\n",
               fileHeader->dataType == RKRawDataTypeFromTransceiver ? "Raw" :
               (fileHeader->dataType == RKRawDataTypeAfterMatchedFilter ? "Compressed" : "Unknown"));
        RKLog(">desc.name = '%s'\n", fileHeader->desc.name);
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
              RKIntegerToCommaStyleString(pulse->header.downSampledGateCount));
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
        if (fileHeader->buildNo >= 6) {
            RKWaveformSummary(waveform);
        }
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
    RKCalibratorSimple(space, config);
    // The rest of the constants
    space->noise[0] = config->noise[0];
    space->noise[1] = config->noise[1];
    space->velocityFactor = 0.25f * fileHeader->desc.wavelength / config->prt[0] / M_PI;
    space->widthFactor = fileHeader->desc.wavelength / config->prt[0] / (2.0f * sqrtf(2.0f) * M_PI);
    space->KDPFactor = 1.0f / space->gateSizeMeters;
    
    if (arg->verbose > 1) {
        float *t;
        t = space->rcor[0]; RKLog(">rcor = %.1e, %.1e, %.1e, %.1e, %.1e ...\n", t[0], t[10], t[50], t[100], t[250]);
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

    p = 0;    // total pulses per ray
    r = 0;    // total rays per sweep
    for (k = 0; k < RKRawDataRecorderDefaultMaximumRecorderDepth; k++) {
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, p);
        j = RKReadPulseFromFileReference(pulse, fileHeader->dataType, fid);
        if (j == RKResultSuccess) {
            if (p == 0 && arg->verbose) {
                RKLog("pulse[0].downSampledGateCount = %s\n", RKIntegerToCommaStyleString(pulse->header.downSampledGateCount));
            }
            timeval2str(timestr, pulse->header.time);
            if ((marker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI) {
                i0 = (int)floorf(pulse->header.azimuthDegrees);
            } else if ((marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI) {
                i0 = (int)floorf(pulse->header.elevationDegrees);
            } else {
                i0 = 360 * (int)floorf(pulse->header.elevationDegrees - 0.25f) + (int)floorf(pulse->header.azimuthDegrees);
            }
            pulses[p++] = pulse;
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
                printf("P:%05d/%06" PRIu64 "/%05d %s(%06d)   C%d   E%5.2f, A%6.2f  %s x %.1fm %d/%d %02x %s%s%s\n",
                       k, pulse->header.i, p, timestr, du,
                       pulse->header.configIndex,
                       pulse->header.elevationDegrees, pulse->header.azimuthDegrees,
                       RKIntegerToCommaStyleString(pulse->header.downSampledGateCount),
                       pulse->header.gateSizeMeters * fileHeader->desc.pulseToRayRatio,
                       i0, pulse->header.azimuthBinIndex,
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
                RKRay *ray = RKGetRayFromBuffer(rayBuffer, r);
                ray->header.startTime       = S->header.time;
                ray->header.startTimeDouble = S->header.timeDouble;
                ray->header.startAzimuth    = S->header.azimuthDegrees;
                ray->header.startElevation  = S->header.elevationDegrees;
                ray->header.endTime         = E->header.time;
                ray->header.endTimeDouble   = E->header.timeDouble;
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
                RKPulsePairHop(space, pulses, p);
                makeRayFromScratch(space, ray);

                // Timestamp
                timeval2str(timestr, ray->header.startTime);
                if (arg->verbose == 2) {
                    printf("P:%05d R%3d %s [E%.2f, A%.2f]  %s%6.2f-%6.2f  (%4.2f, p%d)  G%s  M%05X  %s%s\n",
                           k, r, timestr,
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
                    float *data = RKGetFloatDataFromRay(ray, RKBaseMomentIndexZ);
                    printf("p:%05d R%3d %s [E%.2f, A%.2f]  %s%6.2f-%6.2f  (%4.2f, p%d)  G%s  M%05X  %s%s   %.1f, %.1f, %.1f, %.1f, %.1f   %.1f, %.1f, %.1f, %.1f, %.1f\n",
                           k, r, timestr,
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
                
                // Collect the ray
                rays[r++] = ray;
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
                    timeval2str(timestr, S->header.time);
                    RKLog(">%05d r=%3d %s p=%d   [E%.2f, A%.2f]  %s%6.2f-%6.2f  (%4.2f)  G%s  M%05x\n",
                           k, r, timestr, p,
                           config->sweepElevation, config->sweepAzimuth,
                           (marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "E" : "A",
                           (marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? S->header.elevationDegrees : S->header.azimuthDegrees,
                           (marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? E->header.elevationDegrees : E->header.azimuthDegrees,
                           (marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? deltaElevation : deltaAzimuth,
                           RKIntegerToCommaStyleString(S->header.downSampledGateCount),
                           marker);
                }
            }
            // Use the end pulse as the start pulse of next ray
            for (j = 0; j < 2; j++) {
                RKComplex *x = RKGetComplexDataFromPulse(pulses[0], j);
                RKIQZ z = RKGetSplitComplexDataFromPulse(pulses[0], j);
                memcpy(x, RKGetComplexDataFromPulse(pulse, j), pulse->header.downSampledGateCount * sizeof(RKComplex));
                for (i = 0; i < pulse->header.downSampledGateCount; i++) {
                    z.i[i] = x[i].i;
                    z.q[i] = x[i].q;
                }
            }
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
    free(fileHeader);

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
        RKBaseMomentList overallMomentList = 0;
        uint8_t gateCountWarningCount = 0;
        uint8_t gateSizeWarningCount = 0;
        for (i = k + 1; i < k  + r - 1; i++) {
            overallMomentList |= rays[i]->header.baseMomentList;
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
            RKLog("momentList = %016xh\n", overallMomentList);
        }

        // Populate the contents
        sweep->header.rayCount = r;
        sweep->header.gateCount = S->header.gateCount;
        sweep->header.gateSizeMeters = S->header.gateSizeMeters;
        sweep->header.startTime = (time_t)S->header.startTime.tv_sec;
        sweep->header.endTime = (time_t)E->header.endTime.tv_sec;
        sweep->header.baseMomentList = overallMomentList;
        sweep->header.isPPI = (config->startMarker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI;
        sweep->header.isRHI = (config->startMarker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI;
        sweep->header.external = true;
        memcpy(&sweep->header.desc, &fileHeader->desc, sizeof(RKRadarDesc));
        memcpy(&sweep->header.config, config, sizeof(RKConfig));
        memcpy(sweep->rays, rays + k, r * sizeof(RKRay *));
        // Make a suggested filename as .../[DATA_PATH]/20170119/PX10k-20170119-012345-E1.0 (no symbol and extension)
        if (arg->directory[0] == '\0') {
            k = sprintf(sweep->header.filename, "%s%s%s/", fileHeader->desc.dataPath, fileHeader->desc.dataPath[0] == '\0' ? "" : "/", RKDataFolderMoment);
            k += strftime(sweep->header.filename + k, 10, "%Y%m%d/", gmtime(&sweep->header.startTime));
        } else {
            k = sprintf(sweep->header.filename, "%s/", arg->directory);
        }
        k += strftime(sweep->header.filename + k, 16, "%Y%m%d-%H%M%S", gmtime(&sweep->header.startTime));
        if (sweep->header.isPPI) {
            k += snprintf(sweep->header.filename + k, 7, "-E%.1f", sweep->header.config.sweepElevation);
        } else if (sweep->header.isRHI) {
            k += snprintf(sweep->header.filename + k, 7, "-A%.1f", sweep->header.config.sweepAzimuth);
        } else {
            k += snprintf(sweep->header.filename + k, 7, "-N%03d", sweep->header.rayCount);
        }
        if (k > RKMaximumFolderPathLength + RKMaximumPrefixLength + 25 + RKMaximumFileExtensionLength) {
            RKLog("Error. Suggested filename %s is longer than expected.\n", sweep->header.filename);
        }
        //RKLog("Output %s\n", sweep->header.filename);

        // Initialize a list based on desired moment list. This variable will become all zeros after the next for-loop
        RKBaseMomentList list = sweep->header.baseMomentList & momentList;

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
    
    RKPulseBufferFree(pulseBuffer);
    RKRayBufferFree(rayBuffer);
    RKProductBufferFree(product, 1);
    RKScratchFree(space);
    
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
        s += snprintf(str + s, 1023 - s, "%c%s", o->val, o->has_arg == required_argument ? ":" : (o->has_arg == optional_argument ? "::" : ""));
    }
    optind = 1;
    int opt, ind = 0;
    while ((opt = getopt_long(argc, (char * const *)argv, str, options, &ind)) != -1) {
        switch (opt) {
            case 'A':
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
    }

    if (arg->verbose > 1) {
        printf("str = %s   ind = %d   optind = %d\n", str, ind, optind);
        printf("argc = %d   argv[%d] = %s\n", argc, optind, argv[optind]);
        printf("arg->verbose = %d\n", arg->verbose);
        printf("arg->directory = %s\n", arg->directory);
        printf("arg->filename = %s\n", arg->filename);
        printf("arg->output = %s\n", arg->output ? "true" : "false");
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
