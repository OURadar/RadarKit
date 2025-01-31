#include <RadarKit.h>
#include <getopt.h>

//

#define barSize 64

enum MomentMethod {
    MomentMethodPulsePair = 0,
    MomentMethodPulsePairHop,
    MomentMethodPulsePairATSR,
    MomentMethodPMultiLag2,
    MomentMethodPMultiLag3,
    MomentMethodPMultiLag4,
    MomentMethodSpectral
};

// User parameters in a struct
typedef struct user_params {
    char                      directory[RKMaximumFolderPathLength];
    char                      filename[RKNameLength];
    int                       verbose;
    int                       cores;
    int                       compressionMethod;
    char                      momentMethod[8];
    bool                      radialNoise;
    bool                      output;
    bool                      compressedOutput;
    float                     SNRThreshold;
    float                     SQIThreshold;
    uint8_t                   engineVerbose[128];                                 // Letter A = 65, 'z' = 122
} UserParams;

typedef struct user_machine {
    RKConfig                  *configs;
    RKBuffer                  pulses;
    RKBuffer                  rays;
    RKFFTModule               *fftModule;
    RKPulseEngine             *pulseMachine;
    RKMomentEngine            *momentMachine;
    RKSweepEngine             *sweepMachine;
    RKRawDataRecorder         *recorder;
    RKPulseRingFilterEngine   *ringMachine;
    RKUserModule              *userModule;
    RKUserModule              (*userModuleInit)(RKWaveform *);
    void                      (*userModuleFree)(RKUserModule);
    uint32_t                  configIndex;
    uint32_t                  pulseIndex;
    uint32_t                  rayIndex;
    char                      bar[barSize + 1];
} Workspace;

Workspace *WorkspaceInit(RKRadarDesc *desc, UserParams *arg) {
    Workspace *mod = (Workspace *)malloc(sizeof(Workspace));
    if (mod == NULL) {
        RKLog("Error. Unable to allocate a user module.\n");
        exit(EXIT_FAILURE);
    }
    memset(mod, 0, sizeof(Workspace));

    // const int verbose = desc->initFlags & RKInitFlagVeryVerbose ? 2 : (
    //                     desc->initFlags & RKInitFlagVerbose ? 1 : 0);
    const int verbose = MAX(0, arg->verbose - 1);
    // const int verbose = arg->verbose;

    // Some basic check
    uint32_t multiple = RKMemoryAlignSize / sizeof(RKFloat) * desc->pulseToRayRatio;
    uint32_t goodPulseCapacity = (uint32_t)ceilf((float)desc->pulseCapacity / multiple) * multiple;

    RKLog("pulseCapacity = %s   goodCapacity = %s   verbose = %d\n",
        RKIntegerToCommaStyleString(desc->pulseCapacity), RKIntegerToCommaStyleString(goodPulseCapacity), verbose);

    size_t mem = sizeof(Workspace);

    mod->fftModule = RKFFTModuleInit(desc->pulseCapacity, verbose);

    desc->configBufferSize = RKConfigBufferAlloc(&mod->configs, desc->configBufferDepth);
    desc->pulseBufferSize = RKPulseBufferAlloc(&mod->pulses, desc->pulseCapacity, desc->pulseBufferDepth);
    desc->rayBufferSize = RKRayBufferAlloc(&mod->rays, desc->pulseCapacity / desc->pulseToRayRatio, desc->rayBufferDepth);

    if (mod->configs == NULL || mod->pulses == NULL || mod->rays == NULL) {
        RKLog("Error. Unable to allocate memory for buffer  %p %p %p\n", mod->configs, mod->pulses, mod->rays);
        exit(EXIT_FAILURE);
    }

    mem += desc->configBufferSize + desc->pulseBufferSize + desc->rayBufferSize;

    // Pulse engine (compression)
    mod->pulseMachine = RKPulseEngineInit();
    RKPulseEngineSetVerbose(mod->pulseMachine, MAX(arg->engineVerbose['p'], verbose));
    RKPulseEngineSetEssentials(mod->pulseMachine, desc, mod->fftModule,
                               mod->configs, &mod->configIndex,
                               mod->pulses, &mod->pulseIndex);
    RKPulseEngineSetCoreCount(mod->pulseMachine, arg->cores);
    mem += mod->pulseMachine->memoryUsage;
    if (desc->initFlags & RKInitFlagStartPulseEngine) {
        RKPulseEngineStart(mod->pulseMachine);
    }

    // Pulse ring filter engine
    mod->ringMachine = RKPulseRingFilterEngineInit();
    RKPulseRingFilterEngineSetVerbose(mod->ringMachine, MAX(arg->engineVerbose['r'], verbose));
    RKPulseRingFilterEngineSetEssentials(mod->ringMachine, desc,
                                         mod->configs, &mod->configIndex,
                                         mod->pulses, &mod->pulseIndex);
    RKPulseRingFilterEngineSetCoreCount(mod->ringMachine, 2);
    mem += mod->ringMachine->memoryUsage;
    if (desc->initFlags & RKInitFlagStartRingFilterEngine) {
        RKPulseRingFilterEngineStart(mod->ringMachine);
    }

    // Moment engine
    mod->momentMachine = RKMomentEngineInit();
    RKMomentEngineSetVerbose(mod->momentMachine, MAX(arg->engineVerbose['m'], verbose));
    RKMomentEngineSetEssentials(mod->momentMachine, desc, mod->fftModule,
                                mod->configs, &mod->configIndex,
                                mod->pulses, &mod->pulseIndex,
                                mod->rays, &mod->rayIndex);
    RKMomentEngineSetCoreCount(mod->momentMachine, 2);
    RKMomentEngineSetMomentProcessorToMultiLag3(mod->momentMachine);
    if (!strcasecmp(arg->momentMethod, "pp")) {
        RKMomentEngineSetMomentProcessorToPulsePair(mod->momentMachine);
    } else if (!strcasecmp(arg->momentMethod, "pph")) {
        RKMomentEngineSetMomentProcessorToPulsePairHop(mod->momentMachine);
    } else if (!strcasecmp(arg->momentMethod, "ppa")) {
        RKMomentEngineSetMomentPRocessorToPulsePairATSR(mod->momentMachine);
    } else if (!strcasecmp(arg->momentMethod, "m2")) {
        RKMomentEngineSetMomentProcessorToMultiLag2(mod->momentMachine);
    } else if (!strcasecmp(arg->momentMethod, "m3")) {
        RKMomentEngineSetMomentProcessorToMultiLag3(mod->momentMachine);
    } else if (!strcasecmp(arg->momentMethod, "m4")) {
        RKMomentEngineSetMomentProcessorToMultiLag4(mod->momentMachine);
    } else if (!strcasecmp(arg->momentMethod, "spec")) {
        RKMomentEngineSetMomentProcessorToSpectral(mod->momentMachine);
    } else {
        printf("Method not recognized, reverted to MultiLag3.\n");
        RKMomentEngineSetMomentProcessorToMultiLag3(mod->momentMachine);
    }

    if (arg->radialNoise) {
        RKMomentEngineSetNoiseEstimator(mod->momentMachine, RKRayNoiseEstimator);
    }
    RKMomentEngineStart(mod->momentMachine);
    mem += mod->momentMachine->memoryUsage;

    // Sweep engine
    mod->sweepMachine = RKSweepEngineInit();
    RKSweepEngineSetVerbose(mod->sweepMachine, MAX(arg->engineVerbose['s'], verbose));
    RKSweepEngineSetEssentials(mod->sweepMachine, desc, NULL,
                               mod->configs, &mod->configIndex,
                               mod->rays, &mod->rayIndex);
    RKSweepEngineSetRecord(mod->sweepMachine, arg->output);
    RKSweepEngineStart(mod->sweepMachine);
    mem += mod->sweepMachine->memoryUsage;

    // Recorder
    mod->recorder = RKRawDataRecorderInit();
    RKRawDataRecorderSetEssentials(mod->recorder, desc, NULL,
                                   mod->configs, &mod->configIndex,
                                   mod->pulses, &mod->pulseIndex);
    RKRawDataRecorderSetRawDataType(mod->recorder, RKRawDataTypeAfterMatchedFilter);
    RKRawDataRecorderSetVerbose(mod->recorder, 2);
    // RKRawDataRecorderStart(mod->recorder);
    mem += mod->recorder->memoryUsage;

    // Duplicate the configuration from file
    RKConfig *config = &mod->configs[0];
    config->waveform = RKWaveformInitAsImpulse();
    config->waveformDecimate = config->waveform;

    RKLog("Memory usage = \033[4m%s B\033[24m\n", RKIntegerToCommaStyleString(mem));

    return mod;
}

void WorkspaceFree(Workspace *mod) {
    if (mod->userModule && mod->userModuleFree) {
        mod->userModuleFree(mod->userModule);
    }

    RKLog("%s tic = %d\n", mod->recorder->name, mod->recorder->tic);

    RKPulseEngineStop(mod->pulseMachine);
    RKPulseRingFilterEngineStop(mod->ringMachine);
    RKMomentEngineStop(mod->momentMachine);
    RKSweepEngineStop(mod->sweepMachine);
    RKRawDataRecorderStop(mod->recorder);

    RKPulseEngineFree(mod->pulseMachine);
    RKPulseRingFilterEngineFree(mod->ringMachine);
    RKMomentEngineFree(mod->momentMachine);
    RKSweepEngineFree(mod->sweepMachine);
    RKRawDataRecorderFree(mod->recorder);

    RKFFTModuleFree(mod->fftModule);

    RKConfigBufferFree(mod->configs);
    RKPulseBufferFree(mod->pulses);
    RKRayBufferFree(mod->rays);

    free(mod);
}

void WorkspaceSetUserModule(Workspace *mod,
                            RKUserModule (*initRoutine)(RKWaveform *),
                            void (*freeRoutine)(RKUserModule),
                            void (*compressor)(RKUserModule, RKCompressionScratch *),
                            void (*calibrator)(RKUserModule, RKMomentScratch *)) {
    RKConfig *config = &mod->configs[0];
    if (config == NULL) {
        RKLog("Error. RKConfig is invalid.\n");
        exit(EXIT_FAILURE);
    }
    mod->userModuleInit = initRoutine;
    mod->userModuleFree = freeRoutine;
    mod->userModule = mod->userModuleInit(config->waveform);
    RKPulseEngineSetCompressor(mod->pulseMachine, compressor, mod->userModule);
    RKMomentEngineSetCalibrator(mod->momentMachine, calibrator, mod->userModule);
}

static void showHelp(void) {
    char name[] = __FILE__;
    RKShowName();
    *strrchr(name, '.') = '\0';
    printf("Product Generator\n\n"
           "%s [options] FILENAME\n\n"
           "OPTIONS:\n"
           "     Unless specifically indicated, all options are interpreted in the order they\n"
           "     are specified. Some options can be specified multiples times for repetitions.\n"
           "     For example, the verbosity is increased by repeating the option multiple times.\n"
           "\n"
           "  -C (--core)\n"
           "         Specifies the number of CPU cores (default = 4).\n"
           "\n"
           "  -d (--dir) " UNDERLINE("value") "\n"
           "         Specifies the directory to store the output files.\n"
           "\n"
           "  -h (--help)\n"
           "         Shows this help text.\n"
           "\n"
           "  -m (--moment)\n"
           "         Sets moment method.\n"
           "         -m pp  uses Pulse Pair\n"
           "         -m pph uses Pulse Pair Hop (Frequency Hopping)\n"
           "         -m pa  uses Pulse ATSR (Alternate Transmit Simultaneous Receive)\n"
           "         -m m2  uses Multilag 2\n"
           "         -m m3  uses Multilag 3\n"
           "         -m spec  uses Spectral Method\n"
           "\n"
           "  -n (--no-output)\n"
           "         No output will be produced. Just do a dry run.\n"
           "\n"
           "  -r (--radial-noise)\n"
           "         Enables radial noise estimator.\n"
           "\n"
           "  -q (--sqi) " UNDERLINE("value") "\n"
           "         Sets the SQI threshold.\n"
           "\n"
           "  -s (--snr) " UNDERLINE("value") "\n"
           "         Sets the SNR threshold.\n"
           "\n"
           "  --version\n"
           "         Shows the version number.\n"
           "\n"
           "  -v (--verbose)\n"
           "         Increases verbosity level, which can be specified multiple times.\n"
           "\n"
           "  -V (--engine-verbose) " UNDERLINE("value") "\n"
           "         Increases verbosity level of specific engines.\n"
           "          m - Moment engine\n"
           "          p - Pulse engine\n"
           "          r - Ring filter engine\n"
           "          s - Sweep engine\n"
           "\n"
           "%s " __RKVersion__ " / " __VERSION__ "\n\n",
           name,
           name
           );
}

void proc(UserParams *arg) {

    int k, r, z;
    long filesize = 0;
    float progress = 0.0f;
    struct timeval s, e;

    printf("PGEN " __RKVersion__ " / " __VERSION__ "\n");
    printf("Processing %s ...\n", arg->filename);

    if (arg->verbose) {
        RKSetWantScreenOutput(true);
    } else {
        RKSetWantScreenOutput(false);
    }

    // Show RadarKit framework name & version
    if (arg->verbose) {
        RKShowName();
    }

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

    if (arg->verbose > 1) {
        RKFileHeaderSummary(fileHeader);
    }

    // Revise pulse capacity for RadarKit 6
    uint32_t multiple = RKMemoryAlignSize * fileHeader->desc.pulseToRayRatio;
    uint32_t goodPulseCapacity = (uint32_t)ceilf((float)fileHeader->desc.pulseCapacity / multiple) * multiple;

    // Radar descriptor with matching pulse capacity
    RKRadarDesc desc = {
        .initFlags = RKInitFlagAllocConfigBuffer | RKInitFlagAllocRawIQBuffer | RKInitFlagAllocMomentBuffer,
        .configBufferDepth = 3,
        .pulseCapacity = goodPulseCapacity,
        .pulseBufferDepth = RKMaximumPulsesPerRay + 500,
        .pulseToRayRatio = fileHeader->desc.pulseToRayRatio,
        .rayBufferDepth = 380,
        .wavelength = fileHeader->desc.wavelength,
        .longitude = fileHeader->desc.longitude,
        .latitude = fileHeader->desc.latitude,
        .heading = fileHeader->desc.heading,
    };
    strcpy(desc.filePrefix, fileHeader->desc.filePrefix);
    strcpy(desc.dataPath, arg->directory);

    // Pass down verbose flag to radar descriptor
    if (arg->verbose > 1) {
        desc.initFlags |= RKInitFlagVerbose;
        if (arg->verbose > 2) {
            desc.initFlags |= RKInitFlagVeryVerbose;
        }
    }

    // Decide if pulse engine and ring filter engines are necessary
    if (fileHeader->dataType == RKRawDataTypeFromTransceiver) {
        desc.initFlags |= RKInitFlagStartPulseEngine | RKInitFlagStartRingFilterEngine;
    }

    // Actual workspace initialization
    Workspace *workspace = WorkspaceInit(&desc, arg);

    // There should only be one RKConfig that matters
    RKConfig *config = &workspace->configs[0];
    memcpy(&workspace->configs[0], &fileHeader->config, sizeof(RKConfig));
    config->waveform = fileHeader->config.waveform;
    config->waveformDecimate = fileHeader->config.waveformDecimate;
    if (isfinite(arg->SQIThreshold)) {
        config->SQIThreshold = arg->SQIThreshold;
        RKLog("Info. SQIThreshold -> %.4f\n", config->SQIThreshold);
    }
    if (isfinite(arg->SNRThreshold)) {
        config->SNRThreshold = arg->SNRThreshold;
        RKLog("Info. SNRThreshold -> %.4f dB\n", config->SNRThreshold);
    }

    workspace->bar[barSize] = '\0';

    RKPulseEngineSetFilterByWaveform(workspace->pulseMachine, config->waveform);

    gettimeofday(&s, NULL);

    // RKRawDataRecorderSetRecord(workspace->recorder, true);

    uint32_t rayIndex = workspace->rayIndex;
    for (k = 0; k < RKRawDataRecorderDefaultMaximumRecorderDepth; k++) {
        RKPulse *pulse = RKPulseEngineGetVacantPulse(workspace->pulseMachine, RKPulseStatusUsedForMoments);

        // Pulse engine sleeps before things propagate. Wait if the buffer to about 75% full.
        z = 1;
        while (workspace->pulseMachine->maxWorkerLag > 0.75f || workspace->momentMachine->maxWorkerLag > 0.75f) {
            usleep(100);
            if (z++ % 1000 == 0) {
                RKLog("Waiting for workers ...  z = %d / %.1fs   %.1f   %.1f\n", z, z * 0.0001f,
                    workspace->pulseMachine->maxWorkerLag, workspace->momentMachine->maxWorkerLag);
            }
        }

        r = RKReadPulseFromFileReference(pulse, fileHeader, fid);
        if (r != RKResultSuccess) {
            break;
        }

        // Replace some radar state machine variables
        // pulse->header.i = k;
        pulse->header.configIndex = 0;
        if (rayIndex != workspace->rayIndex ||
            pulse->header.marker & RKMarkerSweepEnd ||
            pulse->header.marker & RKMarkerSweepBegin) {
            rayIndex = workspace->rayIndex;
            if (pulse->header.marker & RKMarkerSweepEnd) {
                progress = 100.0f;
            } else {
                progress = ftell(fid) * 100.0f / filesize;
            }
            if (arg->verbose > 1) {
                RKLog("k = %05d   pulseIndex = %04d   rayIndex = %03d   EL %5.1f  AZ %5.1f  s%04x  m%04x  c%d\n",
                    k, workspace->pulseIndex, workspace->rayIndex,
                    pulse->header.elevationDegrees, pulse->header.azimuthDegrees,
                    pulse->header.s, pulse->header.marker, pulse->header.configIndex);
            } else {
                r = (int)(progress / 100.0f * barSize);
                memset(workspace->bar, '#', r);
                memset(workspace->bar + r, '.', barSize - r);
                printf("\rProgress %s %.1f%% ", workspace->bar, progress);
                fflush(stdout);
            }
        }
        pulse->header.s = RKPulseStatusHasIQData | RKPulseStatusHasPosition;
        if (fileHeader->dataType == RKRawDataTypeAfterMatchedFilter) {
            pulse->header.s |= RKPulseStatusCompleteForMoments;
        }

        #if defined(DEBUG_STATE_MACHINE)
        if (workspace->pulseIndex % 100 == 0) {
            RKPulse *first = RKGetPulseFromBuffer(workspace->pulses, 0);
            RKLog("pulseIndex = %u   .s = %x (%x =? %x)   [0] = %x\n",
                workspace->pulseIndex, pulse->header.s, pulse->header.s & RKPulseStatusReadyForMoments, RKPulseStatusReadyForMoments,
                first->header.s);
        }
        #endif

        if (pulse->header.marker & RKMarkerSweepEnd || ftell(fid) == filesize) {
            printf("\r\033[0J");
            fflush(stdout);
            if (arg->verbose > 1 && ftell(fid) == filesize) {
                RKLog("EOF\n");
            }
            if (pulse->header.marker & RKMarkerSweepEnd) {
                workspace->pulseIndex = RKPreviousModuloS(workspace->pulseIndex, desc.pulseBufferDepth);
                break;
            }
        }
    }
    fclose(fid);

    printf("\rConcluding ... ");
    fflush(stdout);
    printf("\r");

    RKPulseEngineWaitWhileBusy(workspace->pulseMachine);
    RKMomentEngineWaitWhileBusy(workspace->momentMachine);

    RKSweepEngineFlush(workspace->sweepMachine);

    // RKRawDataRecorderSetRecord(workspace->recorder, false);

    // End the stopwatch
    if (arg->verbose == 0) {
        char *summary = RKSweepEngineLatestSummary(workspace->sweepMachine);
        printf("\033[0J\r%s\n", summary);
    } else {
        gettimeofday(&e, NULL);
        double dt = RKTimevalDiff(e, s);
        RKLog("Process time = %.3f s   pulse count = %s\n", dt, RKIntegerToCommaStyleString(k));
    }

    RKReadPulseFromFileReference(NULL, NULL, NULL);

    WorkspaceFree(workspace);

    RKFileHeaderFree(fileHeader);
}

//
//
//  M A I N
//
//

int main(int argc, const char **argv) {

    int k, s;
    char *c, str[1024];

    // A struct to collect user input
    UserParams *arg = (UserParams *)malloc(sizeof(UserParams));
    memset(arg, 0, sizeof(UserParams));
    arg->SNRThreshold = NAN;
    arg->SQIThreshold = NAN;
    arg->output = true;
    arg->cores = 4;
    arg->compressionMethod = 3;
    strcpy(arg->momentMethod, "m3");

    // Command line options
    struct option options[] = {
        {"alarm"             , no_argument      , NULL, 'A'},    // ASCII 65 - 90 : A - Z
        {"core"              , required_argument, NULL, 'C'},
        {"engine-verbose"    , required_argument, NULL, 'V'},
        {"compress"          , required_argument, NULL, 'c'},
        {"dir"               , required_argument, NULL, 'd'},
        {"help"              , no_argument      , NULL, 'h'},
        {"method"            , required_argument, NULL, 'm'},
        {"no-output"         , no_argument      , NULL, 'n'},
        {"sqi"               , required_argument, NULL, 'q'},
        {"radial-noise"      , no_argument      , NULL, 'r'},
        {"snr"               , required_argument, NULL, 's'},
        {"version"           , no_argument      , NULL, 'u'},
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
            case 'C':
                arg->cores = atoi(optarg);
                break;
            case 'V':
                c = optarg;
                do {
                    arg->engineVerbose[(int)*c]++;
                } while (*++c != '\0');
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
            case 'm':
                strncpy(arg->momentMethod, optarg, 7);
                break;
            case 'n':
                arg->output = false;
                break;
            case 'q':
                arg->SQIThreshold = atof(optarg);
                break;
            case 'r':
                arg->radialNoise = true;
            case 's':
                arg->SNRThreshold = atof(optarg);
                break;
            case 'u':
                printf("%s / RadarKit %s / %s\n", __FILE__, __RKVersion__, __VERSION__);
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                arg->verbose++;
                break;
            default:
                break;
        }
    }

    if (optind == argc) {
        RKLog("Error. No filename supplied\n");
        exit(EXIT_FAILURE);
    }

    c = getenv("TERM");
    if (c == NULL || (strcasestr(c, "color") == NULL && strcasestr(c, "ansi") == NULL)) {
        RKSetWantColor(false);
    }

    char name[] = __FILE__;
    c = strrchr(name, '/');
    if (c) {
        strncpy(name, c + 1, sizeof(name) - 1);
    }
    c = strrchr(name, '.');
    if (c) {
        *c = '\0';
    }
    RKSetProgramName(name);

    optind = MIN(argc - 1, optind);
    strcpy(arg->filename, argv[optind]);
    if (strlen(arg->directory)) {
        c = strrchr(arg->directory, '/');
        if (c) {
            *c = '\0';
        }
    } else {
        sprintf(arg->directory, "data");
    }

    if (arg->verbose > 1) {
        printf("str = %s   ind = %d   optind = %d\n", str, ind, optind);
        printf("argc = %d   argv[%d] = %s\n", argc, optind, argv[optind]);
        printf("arg->verbose = %d\n", arg->verbose);
        printf("arg->output = %s\n", arg->output ? "true" : "false");
        printf("arg->filename = %s\n", arg->filename);
        printf("arg->directory = %s\n", arg->directory);
        printf("arg->compressedOutput = %s\n", arg->compressedOutput ? "true" : "false");
        printf("arg->method = %s\n", arg->momentMethod);
    }

    // Now we process
    proc(arg);

    free(arg);

    exit(EXIT_SUCCESS);
}
