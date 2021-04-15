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
    char *c;
    size_t rsize = 0, tr;
    char timestr[32];
    time_t startTime;
    suseconds_t usec = 0;
    
    char filename[1024];
    struct timeval s, e;

    strcpy(filename, argv[1]);
    
    RKSetWantScreenOutput(true);

    RKLog("Opening file %s ...", filename);
//    int k = 0;
//    gettimeofday(&s, NULL);
//    for (int i = 0; i < 500; i++) {
//        printf("Trial #%04d   Filename = %s\n", i, filename);
//        RKProductCollection *collection = RKProductCollectionInitWithFilename(filename);
//        k += collection->count;
//        RKProductCollectionFree(collection);
//    }
//    gettimeofday(&e, NULL);
//    double dt = RKTimevalDiff(e, s);
//    RKLog("Elapsed time = %.3f s   (%s files / sec)\n", dt, RKFloatToCommaStyleString((double)k / dt));
    
    gettimeofday(&s, NULL);
    RKFileHeader *header = (RKFileHeader *)malloc(sizeof(RKFileHeader));
    
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
    fread(header, sizeof(RKFileHeader), 1, fid);

    printf("header:\n");
    printf("->%s\n", RKVariableInString("desc.name", &header->desc.name, RKValueTypeString));
    printf("->desc.name = '%s'\n", header->desc.name);
    printf("\n");
    printf("->desc.latitude = %.6f\n", header->desc.latitude);
    printf("->desc.longitude = %.6f\n", header->desc.longitude);
    printf("\n");
    printf("->config.waveform = '%s'\n", header->config.waveform);
    printf("->config.prt = %.1f ms / %.1f ms\n", 1.0e3 * header->config.prt[0], 1.0e3 * header->config.prt[1]);
    printf("\n");
    printf("->desc.pulseCapacity = %u\n", header->desc.pulseCapacity);
    printf("->desc.pulseToRayRatio = %u\n", header->desc.pulseToRayRatio);
    printf("->desc.productBufferDepth = %u\n", header->desc.productBufferDepth);
    printf("->desc.wavelength = %.2f m\n", header->desc.wavelength);
    printf("\n");
    printf("->dataType = '%s'\n",
           header->dataType == RKRawDataTypeFromTransceiver ? "Raw" :
           (header->dataType == RKRawDataTypeAfterMatchedFilter ? "Compressed" : "Unknown"));

    RKScratch *space;
    
    RKBuffer pulseBuffer;
    RKBuffer rayBuffer;
    RKProduct *products;
    size_t bytes;
    
    RKPulse *pulses[50];
    RKRay *ray;
    
    int i0;
    int i1 = 0;
    int count = 0;
    
    c = strrchr(filename, '.');
    if (c == NULL) {
        RKLog("Error. No file extension.");
        exit(EXIT_FAILURE);
    }
    
    // Ray capacity always respects pulseCapcity / pulseToRayRatio and SIMDAlignSize
    const uint32_t rayCapacity = ((uint32_t)ceilf((float)header->desc.pulseCapacity / header->desc.pulseToRayRatio * sizeof(RKFloat) / (float)RKSIMDAlignSize)) * RKSIMDAlignSize / sizeof(RKFloat);

    uint32_t u32 = 0;
    if (!strcmp(".rkr", c)) {
        u32 = header->desc.pulseCapacity;
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
    RKLog("Pulse buffer occupies %s B  (%s pulses x %s gates)\n",
          RKUIntegerToCommaStyleString(bytes),
          RKIntegerToCommaStyleString(RKMaximumPulsesPerRay),
          RKIntegerToCommaStyleString(pulseCapacity));
    
    printf("rayCapacity = %s\n", RKIntegerToCommaStyleString(rayCapacity));
    bytes = RKRayBufferAlloc(&rayBuffer, rayCapacity, 400);
    if (bytes == 0 || rayBuffer == NULL) {
        RKLog("Error. Unable to allocate memory for rays.\n");
        exit(EXIT_FAILURE);
    }

    bytes = RKProductBufferAlloc(&products, header->desc.productBufferDepth, RKMaximumRaysPerSweep, 100);
    if (bytes == 0 || products == NULL) {
        RKLog("Error. Unable to allocate memory for products.\n");
        exit(EXIT_FAILURE);
    }

    const int maxPulseCount = 100;
    size_t mem = RKScratchAlloc(&space, rayCapacity, 3, (uint8_t)ceilf(log2f((float)maxPulseCount)), true);

    // Config changed, propagate parameters to scratch space
    RKConfig *config = &header->config;
    space->gateCount = rayCapacity;
    space->gateSizeMeters = header->config.pulseGateSize;
    // Because the pulse-compression engine uses unity noise gain filters, there is an inherent gain difference at different sampling rate
    // The gain difference is compensated here with a calibration factor if raw-sampling is at 1-MHz (150-m)
    // The number 60 is for conversion of range from meters to kilometers in the range correction term.
    space->samplingAdjustment = 10.0f * log10f(space->gateSizeMeters / (150.0f * header->desc.pulseToRayRatio)) + 60.0f;
    buildInCalibrator(space, config);
    // The rest of the constants
    space->noise[0] = config->noise[0];
    space->noise[1] = config->noise[1];
    space->SNRThreshold = config->SNRThreshold;
    space->SQIThreshold = config->SQIThreshold;
    space->velocityFactor = 0.25f * header->desc.wavelength / config->prt[0] / M_PI;
    space->widthFactor = header->desc.wavelength / config->prt[0] / (2.0f * sqrtf(2.0f) * M_PI);
    space->KDPFactor = 1.0f / space->gateSizeMeters;

    printf("space->gateCount = %d\b", space->gateCount);
    printf("space->mask @ %p\b", space->mask);
    
    printf("memory use = %s\n", RKIntegerToCommaStyleString(mem));

    RKMarker marker = header->config.startMarker;
    
    printf("sweep.Elevation = %.2f\n", header->config.sweepElevation);
    printf("marker = %04x / %04x\n", marker, RKMarkerScanTypePPI);

    p = 0;
    r = 0;
    for (k = 0; k < MIN(50, RKRawDataRecorderDefaultMaximumRecorderDepth); k++) {
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, p);
        pulses[p] = pulse;

        rsize = fread(&pulse->header, sizeof(RKPulseHeader), 1, fid);
        if (rsize != 1) {
            break;
        }
        fread(RKGetComplexDataFromPulse(pulse, 0), pulse->header.downSampledGateCount * sizeof(RKComplex), 1, fid);
        fread(RKGetComplexDataFromPulse(pulse, 1), pulse->header.downSampledGateCount * sizeof(RKComplex), 1, fid);

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
            m = true;
        }
        startTime = pulse->header.time.tv_sec;
        
        tr = strftime(timestr, 24, "%T", gmtime(&startTime));
        tr += sprintf(timestr + tr, ".%06d", (int)pulse->header.time.tv_usec);
        
        printf("pulse %s (%06d) (%" PRIu64 ") (EL %.2f, AZ %.2f) %s x %.2fm %d/%d %d %s%s%s\n",
               timestr, pulse->header.time.tv_usec - usec,
               pulse->header.i,
               pulse->header.elevationDegrees, pulse->header.azimuthDegrees,
               RKIntegerToCommaStyleString(pulse->header.downSampledGateCount),
               pulse->header.gateSizeMeters * header->desc.pulseToRayRatio,
               i0, pulse->header.azimuthBinIndex,
               pulse->header.marker,
               m ? "*" : "",
               pulse->header.marker & RKMarkerSweepBegin ? "S" : "",
               pulse->header.marker & RKMarkerSweepEnd ? "E" : ""
               );
        
        // Process ...
        if (m == true && p >= 3) {
            space->gateCount = pulse->header.downSampledGateCount;

            printf("Processing ray ... k = %d   p = %d   mask @ %p   g = %d / %d\n", k, p, space->mask, space->gateCount, space->capacity);

            RKPulsePairHop(space, pulses, p);
            ray = RKGetRayFromBuffer(rayBuffer, r);
            makeRayFromScratch(space, ray);
            
            p = 0;
            r++;
        }

        usec = pulse->header.time.tv_usec;
        p++;
    }
    printf("r = %zu / %s / %s\n",
           rsize,
           RKUIntegerToCommaStyleString(ftell(fid)),
           RKUIntegerToCommaStyleString(fsize));
    if (ftell(fid) != fsize) {
        RKLog("Warning. There is leftover in the file.");
    }

    fclose(fid);
    free(header);

    gettimeofday(&e, NULL);
    double dt = RKTimevalDiff(e, s);
    RKLog("Elapsed time = %.3f s\n", dt);

    return EXIT_SUCCESS;
}
