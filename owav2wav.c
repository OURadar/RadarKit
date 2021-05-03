#include <RadarKit.h>

#pragma mark - Main

//
//
//  M A I N
//
//

int main(int argc, const char * argv[]) {

    int j, k;
    size_t r;
    
    RKSetWantScreenOutput(true);

    char *term = getenv("TERM");
    if (term == NULL || (strcasestr(term, "color") == NULL && strcasestr(term, "ansi") == NULL)) {
        RKSetWantColor(false);
    }

    FILE *fid = fopen("old-waveforms/ofmd.rkwav", "r");
    if (fid == NULL) {
        RKLog("Error. Unable to open file.\n");
    }
    
    RKWaveFileGlobalHeaderV1 *fileHeader = (RKWaveFileGlobalHeaderV1 *)malloc(sizeof(RKWaveFileGlobalHeaderV1));
    RKWaveFileGroupHeader *groupHeader = (RKWaveFileGroupHeader *)malloc(sizeof(RKWaveFileGroupHeader));
    
    r = fread(fileHeader, sizeof(RKWaveFileGlobalHeaderV1), 1, fid);
    
    RKWaveform *waveform = RKWaveformInitWithCountAndDepth(fileHeader->count, fileHeader->depth);
    waveform->fc = fileHeader->fc;
    waveform->fs = fileHeader->fs;
    strncpy(waveform->name, fileHeader->name, RKNameLength);

    for (k = 0; k < fileHeader->count; k++) {
        fread(groupHeader, sizeof(RKWaveFileGroupHeader), 1, fid);
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
        for (j = 0; j < waveform->depth; j++) {
            printf("->x[%d] = %7.2f%+7.2f   %7d%+7d\n", j,
                   waveform->samples[k][j].i, waveform->samples[k][j].q,
                   waveform->iSamples[k][j].i, waveform->iSamples[k][j].q);
        }
    }
    long fpos = ftell(fid);
    printf("fpos = %s\n", RKIntegerToCommaStyleString(fpos));
    
    fclose(fid);

    RKWaveformSummary(waveform);
    
    free(groupHeader);
    free(fileHeader);

    char filename[] = "waveforms/ofmd.rkwav";
    RKWaveformWriteFile(waveform, filename);
    RKWaveformFree(waveform);

    fid = fopen(filename, "r");
    RKWaveform *newWaveform = RKWaveformReadFromReference(fid);
    fclose(fid);
    
    RKWaveformSummary(newWaveform);
    
    return EXIT_SUCCESS;
}
