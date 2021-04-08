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
    
    fread(header, sizeof(RKFileHeader), 1, fid);
    
    printf("header:\n");
    printf("->%s\n", RKVariableInString("desc.name", &header->desc.name, RKValueTypeString));
    printf("->desc.name = '%s'\n", header->desc.name);
    printf("\n");
    printf("->desc.latitude = %.6f\n", header->desc.latitude);
    printf("->desc.longitude = %.6f\n", header->desc.longitude);
    printf("\n");
    printf("->config.waveform = '%s'\n", header->config.waveform);
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
    
    bytes = RKPulseBufferAlloc(&pulseBuffer, header->desc.pulseCapacity, RKMaximumPulsesPerRay);
    if (bytes == 0 || pulseBuffer == NULL) {
        RKLog("Error. Unable to allocate memory for I/Q pulses.\n");
        exit(EXIT_FAILURE);
    }
    RKLog("Pulse buffer occupies %s B  (%s pulses x %s gates)\n",
          RKUIntegerToCommaStyleString(bytes),
          RKIntegerToCommaStyleString(RKMaximumPulsesPerRay),
          RKIntegerToCommaStyleString(header->desc.pulseCapacity));
    int k;
    for (k = 0; k < 10; k++) {
        RKPulse *pulse = RKGetPulseFromBuffer(pulseBuffer, 0);

        fread(&pulse->header, sizeof(RKPulseHeader), 1, fid);
        fread(RKGetComplexDataFromPulse(pulse, 0), pulse->header.downSampledGateCount * sizeof(RKComplex), 1, fid);
        fread(RKGetComplexDataFromPulse(pulse, 1), pulse->header.downSampledGateCount * sizeof(RKComplex), 1, fid);

        printf("pulse (EL %.2f, AZ %.2f) %s x %.2fm\n",
               pulse->header.elevationDegrees, pulse->header.azimuthDegrees,
               RKIntegerToCommaStyleString(pulse->header.downSampledGateCount),
               pulse->header.gateSizeMeters * header->desc.pulseToRayRatio);
    }

    fclose(fid);
    free(header);

    gettimeofday(&e, NULL);
    double dt = RKTimevalDiff(e, s);
    RKLog("Elapsed time = %.3f s\n", dt);

    return EXIT_SUCCESS;
}
