#include <RadarKit.h>

int main(int argc, const char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Please supply a filename.\n");
        return EXIT_FAILURE;
    }
    
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
    printf("\n");
    printf("->dataType = '%s'\n",
           header->dataType == RKRawDataTypeFromTransceiver ? "Raw" :
           (header->dataType == RKRawDataTypeAfterMatchedFilter ? "Compressed" : "Unknown"));

    RKBuffer pulseBuffer;
    RKBuffer rayBuffer;
    RKProduct *products;
    size_t bytes;
    
    int i0;
    int i1 = 0;
    int count = 0;
    
    bytes = RKPulseBufferAlloc(&pulseBuffer, header->desc.pulseCapacity, RKMaximumPulsesPerRay);
    if (bytes == 0 || pulseBuffer == NULL) {
        RKLog("Error. Unable to allocate memory for I/Q pulses.\n");
        exit(EXIT_FAILURE);
    }
    RKLog("Pulse buffer occupies %s B  (%s pulses x %s gates)\n",
          RKUIntegerToCommaStyleString(bytes),
          RKIntegerToCommaStyleString(RKMaximumPulsesPerRay),
          RKIntegerToCommaStyleString(header->desc.pulseCapacity));

    RKMarker marker = header->config.startMarker;
    
    printf("sweep.Elevation = %.2f\n", header->config.sweepElevation);
    printf("marker = %04x / %04x\n", marker, RKMarkerScanTypePPI);

    int k;
    int m = 0;
    size_t r = 0, tr;
    char timestr[32];
    time_t startTime;
    suseconds_t usec = 0;
    
    for (k = 0; k < MIN(10, RKRawDataRecorderDefaultMaximumRecorderDepth); k++) {
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, k);

        r = fread(&pulse->header, sizeof(RKPulseHeader), 1, fid);
        if (r != 1) {
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

        m = 0;
        if (i1 != i0 || count == RKMaximumPulsesPerRay) {
            i1 = i0;
            m = 1;
            
            // Process ...
            //
            
            k = 0;
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

        usec = pulse->header.time.tv_usec;
    }
    printf("r = %zu / %s / %s\n",
           r,
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
