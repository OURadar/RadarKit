#include <RadarKit.h>

// Meant for waveform conversion going from those generated before 2021 to the new format with RadarKit v2.5

#pragma pack(push, 1)

typedef union rk_wave_file_header_v1 {
    struct {
        char            name[256];                                             // Waveform name
        uint8_t         count;                                                 // Count of groups / tones
        uint32_t        depth;                                                 // Waveform depth
        double          fc;                                                    // Carrier frequency
        double          fs;                                                    // Sampling frequency
    };
    char bytes[512];
} RKWaveFileGlobalHeaderV1;

typedef union rk_wave_file_group_v1 {
    struct {
        RKWaveformType  type;                                                  // Waveform type of this tone
        uint32_t        depth;                                                 // Waveform depth
        uint32_t        filterCount;                                           // Count of filters
    };
    char bytes[32];
} RKWaveFileGroupHeaderV1;

#pragma pack(pop)

#pragma mark - Functions

static RKResult convert(const char *name, int verbose) {
    int j, k;
    size_t r;
    char filename[RKMaximumPathLength];
    sprintf(filename, "old-waveforms/%s.rkwav", name);
    FILE *fid = fopen(filename, "r");
    if (fid == NULL) {
        RKLog("Error. Unable to open file.\n");
    }
    
    RKWaveFileGlobalHeaderV1 *fileHeader = (RKWaveFileGlobalHeaderV1 *)malloc(sizeof(RKWaveFileGlobalHeaderV1));
    RKWaveFileGroupHeaderV1 *groupHeader = (RKWaveFileGroupHeaderV1 *)malloc(sizeof(RKWaveFileGroupHeaderV1));
    
    r = fread(fileHeader, sizeof(RKWaveFileGlobalHeaderV1), 1, fid);
    if (verbose) {
        RKLog("fileHeader->count = %s   fileHeader->depth = %s\n",
              RKIntegerToCommaStyleString(fileHeader->count),
              RKIntegerToCommaStyleString(fileHeader->depth));
    }
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(fileHeader->count, fileHeader->depth);
    waveform->fc = fileHeader->fc;
    waveform->fs = fileHeader->fs;
    waveform->count = 1;
    strncpy(waveform->name, fileHeader->name, RKNameLength);

    for (k = 0; k < fileHeader->count; k++) {
        fread(groupHeader, sizeof(RKWaveFileGroupHeaderV1), 1, fid);
        if (k == 0) {
            waveform->type = groupHeader->type;
        }
        waveform->filterCounts[k] = groupHeader->filterCount;
        r = fread(waveform->filterAnchors[k], sizeof(RKFilterAnchor), waveform->filterCounts[k], fid);
        if (r == 0) {
            RKLog("Error. Unable to read filter anchors from %p\n", fid);
            return EXIT_FAILURE;
        }
        waveform->filterAnchors[k][0].maxDataLength = RKMaximumGateCount;
        for (j = 0; j < waveform->filterCounts[k]; j++) {
            if (waveform->filterAnchors[k][j].length > waveform->depth) {
                RKLog("Error. This waveform is invalid.  length = %s > depth %s\n",
                      RKIntegerToCommaStyleString(waveform->filterAnchors[k][j].length),
                      RKIntegerToCommaStyleString(waveform->depth));
                return EXIT_FAILURE;
            }
        }
        fread(waveform->samples[k], sizeof(RKComplex), waveform->depth, fid);
        fread(waveform->iSamples[k], sizeof(RKInt16C), waveform->depth, fid);
        if (verbose > 2) {
            for (j = 0; j < waveform->depth; j++) {
                printf("->x[%d] = %7.2f%+7.2f   %7d%+7d\n", j,
                       waveform->samples[k][j].i, waveform->samples[k][j].q,
                       waveform->iSamples[k][j].i, waveform->iSamples[k][j].q);
            }
        }
    }
    long fpos = ftell(fid);
    if (verbose > 1) {
        RKLog("fpos = %s\n", RKIntegerToCommaStyleString(fpos));
    }
    fclose(fid);
    free(fileHeader);
    free(groupHeader);
    if (verbose) {
        RKWaveformSummary(waveform);
    }
    
    sprintf(filename, "waveforms/%s.rkwav", name);
    RKLog("Generating %s...\n", filename);
    RKWaveformWriteFile(waveform, filename);

    RKLog("Reading back %s ...\n", filename);
    RKWaveform *newWaveform = RKWaveformInitFromFile(filename);
    if (verbose) {
        RKWaveformSummary(newWaveform);
    }
    if (newWaveform->count != waveform->count ||
        newWaveform->depth != waveform->depth ||
        newWaveform->filterCounts[0] != waveform->filterCounts[0] ||
        newWaveform->filterAnchors[0][0].outputOrigin != waveform->filterAnchors[0][0].outputOrigin ||
        newWaveform->filterAnchors[0][0].subCarrierFrequency != waveform->filterAnchors[0][0].subCarrierFrequency) {
        RKLog("Error. There are conversion errors.\n");
        if (!verbose) {
            RKWaveformSummary(waveform);
            RKWaveformSummary(newWaveform);
        }
    }
    RKWaveformFree(waveform);
    RKWaveformFree(newWaveform);
    return RKResultSuccess;
}

#pragma mark - Main

//
//
//  M A I N
//
//

int main(int argc, const char * argv[]) {

    RKSetWantScreenOutput(true);

    char *term = getenv("TERM");
    if (term == NULL || (strcasestr(term, "color") == NULL && strcasestr(term, "ansi") == NULL)) {
        RKSetWantColor(false);
    }

    int verbose = 1;
    
    convert("ofm", verbose);

    printf("\n\n");

    convert("ofmd", verbose);
    
    return EXIT_SUCCESS;
}
