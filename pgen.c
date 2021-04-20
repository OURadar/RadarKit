#include <RadarKit.h>

// Make some private functions available

int prepareScratch(RKScratch *);
int makeRayFromScratch(RKScratch *, RKRay *);
size_t RKScratchAlloc(RKScratch **space, const uint32_t capacity, const uint8_t lagCount, const uint8_t fftOrder, const bool);
void RKScratchFree(RKScratch *);

#pragma mark -

static void buildInCalibrator(RKScratch *space, RKConfig *config) {
    int i, k, p;
    RKFloat r = 0.0f;
    for (k = 0; k < config->filterCount; k++) {
        //RKLog("calibrator: %d ... %d\n", config->filterAnchors[k].outputOrigin, MIN(config->filterAnchors[k].outputOrigin + config->filterAnchors[k].maxDataLength, space->gateCount));
        for (i = config->filterAnchors[k].outputOrigin; i < MIN(config->filterAnchors[k].outputOrigin + config->filterAnchors[k].maxDataLength, space->gateCount); i++) {
            r = (RKFloat)i * space->gateSizeMeters;
            for (p = 0; p < 2; p++) {
                space->rcor[p][i] = 20.0f * log10f(r) + config->systemZCal[p] + config->ZCal[k][p] - config->filterAnchors[k].sensitivityGain - space->samplingAdjustment;
            }
            space->dcal[i] = config->systemDCal + config->DCal[k];
            space->pcal[i] = RKSingleWrapTo2PI(config->systemPCal + config->PCal[k]);
        }
    }
}

#pragma mark - Main

//
//
//  M A I N
//
//

int main(int argc, const char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Please supply a filename.\n");
        return EXIT_FAILURE;
    }
    
    int i, k, p, r;
    bool m = false;
    size_t rsize = 0, tr;
    uint32_t u32 = 0;
    char *c, timestr[32];

    time_t startTime;
    suseconds_t usec = 0;
    
    char filename[1024];
    struct timeval s, e;

    strcpy(filename, argv[1]);
    
    RKSetWantScreenOutput(true);
    RKSetWantColor(false);

    RKLog("Opening file %s ...", filename);
    
    gettimeofday(&s, NULL);
    RKFileHeader *fileHeader = (RKFileHeader *)malloc(sizeof(RKFileHeader));
    
    FILE *fid = fopen(filename, "r");
    if (fid == NULL) {
        RKLog("Error. Unable to open file %s", filename);
    }
    long fsize;
    if (fseek(fid, 0L, SEEK_END)) {
        RKLog("Error. Unable to tell the file size.\n");
        fsize = 0;
    } else {
        fsize = ftell(fid);
        RKLog("File size = %s B\n", RKUIntegerToCommaStyleString(fsize));
    }
    rewind(fid);
    fread(fileHeader, sizeof(RKFileHeader), 1, fid);
    RKLog("dataType = '%s'\n",
           fileHeader->dataType == RKRawDataTypeFromTransceiver ? "Raw" :
           (fileHeader->dataType == RKRawDataTypeAfterMatchedFilter ? "Compressed" : "Unknown"));

//    RKLog(">%s\n", RKVariableInString("desc.name", &header->desc.name, RKValueTypeString));
    RKLog(">desc.name = '%s'\n", fileHeader->desc.name);
    RKLog(">desc.latitude, longitude = %.6f, %.6f\n", fileHeader->desc.latitude, fileHeader->desc.longitude);
    RKLog(">desc.pulseCapacity = %s\n", RKIntegerToCommaStyleString(fileHeader->desc.pulseCapacity));
    RKLog(">desc.pulseToRayRatio = %u\n", fileHeader->desc.pulseToRayRatio);
    RKLog(">desc.productBufferDepth = %u\n", fileHeader->desc.productBufferDepth);
    RKLog(">desc.wavelength = %.4f m\n", fileHeader->desc.wavelength);

    RKConfig *config = &fileHeader->config;
    RKLog(">config.waveform = '%s'\n", config->waveform);
    RKLog(">config.prt = %.3f ms (PRF = %s Hz)\n", 1.0e3f * config->prt[0], RKIntegerToCommaStyleString((int)roundf(1.0f / config->prt[0])));
    RKLog(">config.pulseGateCount = %s --> %s\n",
          RKIntegerToCommaStyleString(config->pulseGateCount),
          RKIntegerToCommaStyleString(config->pulseGateCount / fileHeader->desc.pulseToRayRatio));
    RKLog(">config.pulseGateSize = %.3f m --> %.3f m\n",
          config->pulseGateSize,
          config->pulseGateSize * fileHeader->desc.pulseToRayRatio);
    
    RKBuffer pulseBuffer;
    RKBuffer rayBuffer;
    RKPulse *pulses[RKMaximumPulsesPerRay];
    RKRay *rays[RKMaximumRaysPerSweep];
    RKProduct *products;
    RKScratch *space;
    
    int i0;
    int i1 = 0;
    int count = 0;
    size_t mem = 0;
    size_t bytes;

    c = strrchr(filename, '.');
    if (c == NULL) {
        RKLog("Error. No file extension.");
        exit(EXIT_FAILURE);
    }
    
    const RKBaseMomentList momentList = RKBaseMomentListProductZVWDPR;

    // Ray capacity always respects pulseCapcity / pulseToRayRatio and SIMDAlignSize
    const uint32_t rayCapacity = ((uint32_t)ceilf((float)fileHeader->desc.pulseCapacity / fileHeader->desc.pulseToRayRatio * sizeof(RKFloat) / (float)RKSIMDAlignSize)) * RKSIMDAlignSize / sizeof(RKFloat);
    if (!strcmp(".rkr", c)) {
        u32 = fileHeader->desc.pulseCapacity;
    } else if (!strcmp(".rkc", c)) {
        u32 = rayCapacity;
    } else {
        RKLog("Error. Unable to handle extension %s", c);
        exit(EXIT_FAILURE);
    }
    const uint32_t pulseCapacity = u32;
    bytes = RKPulseBufferAlloc(&pulseBuffer, pulseCapacity, RKMaximumPulsesPerRay);
    if (bytes == 0 || pulseBuffer == NULL) {
        RKLog("Error. Unable to allocate memory for I/Q pulses.\n");
        exit(EXIT_FAILURE);
    }
    mem += bytes;
    RKLog("Pulse buffer occupies %s B  (%s pulses x %s gates)\n",
          RKUIntegerToCommaStyleString(bytes),
          RKIntegerToCommaStyleString(RKMaximumPulsesPerRay),
          RKIntegerToCommaStyleString(pulseCapacity));
    bytes = RKRayBufferAlloc(&rayBuffer, rayCapacity, RKMaximumRaysPerSweep);
    if (bytes == 0 || rayBuffer == NULL) {
        RKLog("Error. Unable to allocate memory for rays.\n");
        exit(EXIT_FAILURE);
    }
    mem += bytes;
    RKLog("Ray buffer occupies %s B  (%s rays x %s gates)\n",
          RKUIntegerToCommaStyleString(bytes),
          RKIntegerToCommaStyleString(RKMaximumRaysPerSweep),
          RKIntegerToCommaStyleString(rayCapacity));
    const uint8_t productBufferDepth = __builtin_popcount(momentList);
    bytes = RKProductBufferAlloc(&products, productBufferDepth, RKMaximumRaysPerSweep, config->pulseGateCount / fileHeader->desc.pulseToRayRatio);
    if (bytes == 0 || products == NULL) {
        RKLog("Error. Unable to allocate memory for products.\n");
        exit(EXIT_FAILURE);
    }
    RKLog("Product buffer occupies %s B  (%s products)\n",
          RKUIntegerToCommaStyleString(bytes),
          RKIntegerToCommaStyleString(fileHeader->desc.productBufferDepth));
    mem += bytes;
    const uint8_t fftOrder = (uint8_t)ceilf(log2f((float)RKMaximumPulsesPerRay));
    bytes = RKScratchAlloc(&space, rayCapacity, 3, fftOrder, true);
    if (bytes == 0 || space == NULL) {
        RKLog("Error. Unable to allocate memory for scratch space.\n");
        exit(EXIT_FAILURE);
    }
    mem += bytes;
    
    char sweepBeginMarker[20] = "S", sweepEndMarker[20] = "E";
    if (rkGlobalParameters.showColor) {
        sprintf(sweepBeginMarker, "%sS%s", RKGetColorOfIndex(3), RKNoColor);
        sprintf(sweepEndMarker, "%sE%s", RKGetColorOfIndex(2), RKNoColor);
    }

    // Propagate RKConfig parameters to scratch space
    space->gateSizeMeters = fileHeader->config.pulseGateSize * fileHeader->desc.pulseToRayRatio;
    // Because the pulse-compression engine uses unity noise gain filters, there is an inherent gain difference at different sampling rate
    // The gain difference is compensated here with a calibration factor if raw-sampling is at 1-MHz (150-m)
    // The number 60 is for conversion of range from meters to kilometers in the range correction term.
    space->samplingAdjustment = 10.0f * log10f(space->gateSizeMeters / (150.0f * fileHeader->desc.pulseToRayRatio)) + 60.0f;
    buildInCalibrator(space, config);
    // The rest of the constants
    space->noise[0] = config->noise[0];
    space->noise[1] = config->noise[1];
    space->SNRThreshold = config->SNRThreshold;
    space->SQIThreshold = config->SQIThreshold;
    space->velocityFactor = 0.25f * fileHeader->desc.wavelength / config->prt[0] / M_PI;
    space->widthFactor = fileHeader->desc.wavelength / config->prt[0] / (2.0f * sqrtf(2.0f) * M_PI);
    space->KDPFactor = 1.0f / space->gateSizeMeters;

    RKLog("Memory usage = %s B\n", RKIntegerToCommaStyleString(mem));

    RKMarker marker = fileHeader->config.startMarker;
    
    RKLog("sweep.Elevation = %.2f deg\n", config->sweepElevation);
    RKLog("noise = %.2f, %.2f ADU^2\n", config->noise[0], space->noise[1]);
    RKLog("SNRThreshold = %.2f dB\n", config->SNRThreshold);
    RKLog("SQIThreshold = %.2f\n", config->SQIThreshold);
    RKLog("marker = %04x / %04x\n", marker, RKMarkerScanTypePPI);

    p = 0;
    r = 0;
    int verbose = 1;
    for (k = 0; k < MIN(10000, RKRawDataRecorderDefaultMaximumRecorderDepth); k++) {
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, p);

        rsize = fread(&pulse->header, sizeof(RKPulseHeader), 1, fid);
        if (rsize != 1) {
            break;
        }
        // Read H and V data into channels 0 and 1, respectively
        fread(RKGetComplexDataFromPulse(pulse, 0), pulse->header.downSampledGateCount * sizeof(RKComplex), 1, fid);
        fread(RKGetComplexDataFromPulse(pulse, 1), pulse->header.downSampledGateCount * sizeof(RKComplex), 1, fid);
        pulses[p++] = pulse;

        if ((marker & RKMarkerScanTypeMask) == RKMarkerScanTypePPI) {
            i0 = (int)floorf(pulse->header.azimuthDegrees);
        } else if ((marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI) {
            i0 = (int)floorf(pulse->header.elevationDegrees);
        } else {
            i0 = 360 * (int)floorf(pulse->header.elevationDegrees - 0.25f) + (int)floorf(pulse->header.azimuthDegrees);
        }

        m = false;
        if (i1 != i0 || count == RKMaximumPulsesPerRay) {
            i1 = i0;
            if (k > 0) {
                m = true;
            }
        }
        startTime = pulse->header.time.tv_sec;
        
        tr = strftime(timestr, 24, "%T", gmtime(&startTime));
        tr += sprintf(timestr + tr, ".%06d", (int)pulse->header.time.tv_usec);
        
        if (verbose > 1) {
            printf("pulse %s (%06d) (%" PRIu64 ") (EL %.2f, AZ %.2f) %s x %.2fm %d/%d %02x %s%s%s\n",
                   timestr, pulse->header.time.tv_usec - usec,
                   pulse->header.i,
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
        // Process ...
        if (m) {
            if (p >= 3) {
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
                rays[r++] = ray;

                // Summary of this ray
                printf("%05d ray[%3d] p = %d   [E%.2f, A%.2f]  %s%6.2f-%6.2f  (%4.2f)  G%s  M%05x  %s%s\n",
                       k, r, p,
                       config->sweepElevation, config->sweepAzimuth,
                       (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? "E" : "A",
                       (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? S->header.elevationDegrees : S->header.azimuthDegrees,
                       (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? E->header.elevationDegrees : E->header.azimuthDegrees,
                       (ray->header.marker & RKMarkerScanTypeMask) == RKMarkerScanTypeRHI ? deltaElevation : deltaAzimuth,
                       RKIntegerToCommaStyleString(space->gateCount),
                       ray->header.marker,
                       ray->header.marker & RKMarkerSweepBegin ? sweepBeginMarker : "",
                       ray->header.marker & RKMarkerSweepEnd ? sweepEndMarker : "");
            }
            // Use the end pulse as the start pulse of next ray
            memcpy(RKGetComplexDataFromPulse(pulses[0], 0), RKGetComplexDataFromPulse(pulse, 0), pulse->header.downSampledGateCount * sizeof(RKComplex));
            memcpy(RKGetComplexDataFromPulse(pulses[0], 1), RKGetComplexDataFromPulse(pulse, 1), pulse->header.downSampledGateCount * sizeof(RKComplex));
            memcpy(pulses[0], pulse, sizeof(RKPulse));
            p = 1;
        }

        usec = pulse->header.time.tv_usec;
    }
    RKLog("r = %d / %s / %s\n", r, RKUIntegerToCommaStyleString(ftell(fid)), RKUIntegerToCommaStyleString(fsize));
    if (ftell(fid) != fsize) {
        RKLog("Warning. There is leftover in the file.");
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
//        RKRay *T = rays[1];
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
        if (verbose > 1) {
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
        //k = sprintf(sweep->header.filename, "%s%s%s/", engine->radarDescription->dataPath, engine->radarDescription->dataPath[0] == '\0' ? "" : "/", RKDataFolderMoment);
        //    k += strftime(sweep->header.filename + k, 10, "%Y%m%d", gmtime(&sweep->header.startTime));
        k = sprintf(sweep->header.filename, "/Users/boonleng/Downloads/raxpol/");
        k += sprintf(sweep->header.filename + k, "/%s-", fileHeader->desc.filePrefix);
        k += strftime(sweep->header.filename + k, 16, "%Y%m%d-%H%M%S", gmtime(&sweep->header.startTime));
        if (sweep->header.isPPI) {
            k += sprintf(sweep->header.filename + k, "-E%.1f", sweep->header.config.sweepElevation);
        } else if (sweep->header.isRHI) {
            k += sprintf(sweep->header.filename + k, "-A%.1f", sweep->header.config.sweepAzimuth);
        } else {
            k += sprintf(sweep->header.filename + k, "-N%03d", sweep->header.rayCount);
        }
        if (k > RKMaximumFolderPathLength + RKMaximumPrefixLength + 25 + RKMaximumFileExtensionLength) {
            RKLog("Error. Suggested filename %s is longer than expected.\n", sweep->header.filename);
        }
        RKLog("Output %s\n", sweep->header.filename);
        
        int productCount = __builtin_popcount(sweep->header.baseMomentList & momentList);
        RKLog("productCount = %d\n", productCount);

//        RKName name;
//        RKName unit;
//        RKName symbol;
//        RKName colormap;
//        RKFloat lhma[4] = {0.0f, 0.0f, 0.0f, 0.0f};
//        RKBaseMomentIndex momentIndex = 0;
//        for (p = 0; p < productCount; p++) {
//            // Get the symbol, name, unit, colormap, etc. from the product list
//            RKGetNextProductDescription(symbol, name, unit, colormap, &momentIndex, &momentList);
//            // Build a product description
//            RKProductDesc productDescription;
//            memset(&productDescription, 0, sizeof(RKProductDesc));
//            strcpy(productDescription.name, name);
//            strcpy(productDescription.unit, unit);
//            strcpy(productDescription.symbol, symbol);
//            strcpy(productDescription.colormap, colormap);
//            // Special treatment for RhoHV: A three count piece-wise function
//            if (momentIndex == RKBaseMomentIndexR) {
//                productDescription.pieceCount = 3;
//                productDescription.w[0] = 1000.0f;
//                productDescription.b[0] = -824.0f;
//                productDescription.l[0] = 0.93f;
//                productDescription.w[1] = 300.0f;
//                productDescription.b[1] = -173.0f;
//                productDescription.l[1] = 0.7f;
//                productDescription.w[2] = 52.8571f;
//                productDescription.b[2] = 0.0f;
//                productDescription.l[2] = 0.0f;
//                productDescription.mininimumValue = 0.0f;
//                productDescription.maximumValue = 1.05f;
//            } else {
//                switch (momentIndex) {
//                    case RKBaseMomentIndexZ:
//                        RKZLHMAC
//                        break;
//                    case RKBaseMomentIndexV:
//                        RKV2LHMAC
//                        break;
//                    case RKBaseMomentIndexW:
//                        RKWLHMAC
//                        break;
//                    case RKBaseMomentIndexD:
//                        RKDLHMAC
//                        break;
//                    case RKBaseMomentIndexP:
//                        RKPLHMAC
//                        break;
//                    case RKBaseMomentIndexK:
//                        RKKLHMAC
//                        break;
//                    default:
//                        break;
//                }
//                productDescription.pieceCount = 1;
//                productDescription.mininimumValue = lhma[0];
//                productDescription.maximumValue = lhma[1];
//                productDescription.w[0] = lhma[2];
//                productDescription.b[0] = lhma[3];
//                productDescription.l[0] = 0.0f;
//            }
//        }

        // Base products
//        char filename[256];
//        for (p = 0; p < productCount; p++) {
//            RKProduct *product = &products[p];
//            RKProductInitFromSweep(product, sweep);
//            sprintf(filename, "%s-%s.nc", sweep->header.filename, );
//            RKProductFileWriterNC(product, sweep->header.filename);
//        }
        RKSweepFree(sweep);
    }
    
    RKPulseBufferFree(pulseBuffer);
    RKRayBufferFree(rayBuffer);
    RKProductBufferFree(products, productBufferDepth);
    RKScratchFree(space);
    
    return EXIT_SUCCESS;
}
