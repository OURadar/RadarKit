//
//  rkutil.c
//  RadarKit Utility
//
//  Created by Boonleng Cheong
//  Copyright (c) 2016-2019 Boonleng Cheong. All rights reserved.
//

#include <RadarKit.h>
#include <getopt.h>

#define PREFERENCE_FILE             "pref.conf"

// User parameters in a struct
typedef struct user_params {
    RKRadarDesc              desc;
    RKName                   pedzyHost;
    RKName                   tweetaHost;
    RKName                   remoteHost;
    RKName                   relayHost;
    RKName                   ringFilter;
    RKName                   momentMethod;
    RKName                   goCommand;
    RKName                   stopCommand;
    RKName                   streams;
    uint8_t                  verbose;                                            // Verbosity
    int                      port;                                               // Server port other than the default 10000
    int                      coresForPulseCompression;                           // Number of cores for pulse compression
    int                      coresForPulseRingFilter;                            // Number of cores for pulse ring filter
    int                      coresForMomentProcessor;                            // Number of cores for moment calculations
    float                    fs;                                                 // Raw gate sampling bandwidth
    float                    prf;                                                // Base PRF (Hz)
    int                      sprt;                                               // Staggered PRT option (2 for 2:3, 3 for 3:4, etc.)
    int                      gateCount;                                          // Number of gates (simulate mode)
    int                      sleepInterval;                                      // Intermittent sleep period in transceiver simulator in seconds
    int                      recordLevel;                                        // Data recording (1 - moment + health logs only, 2 - everything)
    bool                     simulate;                                           // Run with transceiver simulator
    bool                     ignoreGPS;                                          // Ignore GPS from health relay
    unsigned int             ringFilterGateCount;                                // Number of range gates to apply ring filter
    unsigned int             transitionGateCount;                                // Number of transition gate count
    float                    systemZCal[2];                                      // System calibration for Z
    float                    systemDCal;                                         // System calibration for D
    float                    systemPCal;                                         // System calibration for P
    float                    noise[2];                                           // System noise level
    float                    SNRThreshold;                                       // SNR threshold for moment processors
    float                    SQIThreshold;                                       // SQI threshold for moment processors
    unsigned int             diskUsageLimitGB;                                   // Disk usage limit
    RKControl                controls[RKMaximumControlCount];                    // Controls for GUI
    RKWaveformCalibration    calibrations[RKMaximumWaveformCalibrationCount];    // Waveform specific calibration factors
    uint8_t                  engineVerbose[128];                                 // Letter A = 65, 'z' = 122
    char                     playbackFolder[RKMaximumFolderPathLength];          // Playback folder
} UserParams;

// Global variables
RKRadar *myRadar = NULL;

// Functions
void *exitAfterAWhile(void *s) {
    sleep(3);
    RKLog("Forced exit.\n");
    exit(EXIT_SUCCESS);
}

#pragma mark - Local Functions

static void showHelp() {
    char name[] = __FILE__;
    *strrchr(name, '.') = '\0';
    printf("RadarKit Utility\n\n"
           "%s [options]\n\n"
           "OPTIONS:\n"
           "     Unless specifically stated, all options are interpreted in sequence. Some\n"
           "     options can be specified multiples times for repetitions. For example, the\n"
           "     verbosity is increased by repeating the option multiple times.\n"
           "\n"
           "  -b (--bandwidth) " UNDERLINE("value") "\n"
           "         Sets the system bandwidth to " UNDERLINE("value") " in Hz.\n"
           "         If not specified, the default bandwidth is 5,000,000 Hz.\n"
           "\n"
           "  -c (--core) " UNDERLINE("P,M") " (no space before or after ,)\n"
           "         Sets the number of threads for pulse compression to " UNDERLINE("P") "\n"
           "         and the number of threads for product generator to " UNDERLINE("M") ".\n"
           "         If not specified, the default core counts are 8 / 4.\n"
           "\n"
           "  -C (--show-clocks)\n"
           "         Shows the clock assignment for positions and pulses. This mode can be\n"
           "         used to check if the timing of position and pulses are set properly.\n"
           "\n"
           "  -d (--decimate) " UNDERLINE("value") "\n"
           "         Sets the decimation factor from pulse to ray buffers to be " UNDERLINE("value") ".\n"
           "         The default is derived autmatically where the gate spacing of rays\n"
           "         would be 60 meters.\n"
           "\n"
           "  -e (--empty-style)\n"
           "         Set styles of text to be empty. No color / underline. This should be set\n"
           "         for terminals that do not support color output through escape sequence.\n"
           "\n"
           "  -f (--prf) " UNDERLINE("value") "\n"
           "         Sets the pulse repetition frequency (PRF) to " UNDERLINE("value") " in Hz.\n"
           "         If not specified, the default PRF is 1,000 Hz.\n"
           "\n"
           "  -f (--prf) " UNDERLINE("value,mode") " (no space before or after ,)\n"
           "         Sets the pulse repetition frequency (PRF) to " UNDERLINE("value") " in Hz,\n"
           "         along with a staggered ratio determined by " UNDERLINE("mode") " where\n"
           "         is either 2 for (2:3), 3 for (3:4) and 4 for (4:5).\n"
           "\n"
           "  -g (--gate) " UNDERLINE("value") "\n"
           "         Sets the number range gate capacity to " UNDERLINE("value") ".\n"
           "         If not specified, the default gate capacity is 8,000 bins.\n"
           "\n"
           "  -h (--help)\n"
           "         Shows this help text.\n"
           "\n"
           "  -S (--system) " UNDERLINE("level") "\n"
           "         Sets the simulation to run one of the following levels:\n"
           "          1 - 5-MHz 2,000 gates\n"
           "          2 - 10-MHz 10,000 gates\n"
           "          3 - 20-MHz 20,000 gates\n"
           "          4 - 50-MHz 50,000 gates\n"
           "          5 - 100-MHz 100,000 gates\n"
           "\n"
           "  -r (--relay) " UNDERLINE("host") " [symbols]\n"
           "         Runs as a relay and connect to remote " UNDERLINE("host") "\n"
           "         If [symbols] are supplied, they will be requested initially.\n"
           "\n"
           "  -p (--pedzy-host)\n"
           "         Sets the host of pedzy pedestal controller.\n"
           "\n"
           "  -t (--tweeta-host)\n"
           "         Sets the host of tweeta health relay.\n"
           "\n"
           "  -s (--simulate) " UNDERLINE("[system]") "\n"
           "         Sets the program to simulate data stream.\n"
           "         The optional system value selectes the system level to simulate (see -S).\n"
           "\n"
           "  -v (--verbose)\n"
           "         Increases verbosity level, which can be specified multiple times.\n"
           "\n"
           "  -V (--engine-verbose) " UNDERLINE("value") "\n"
           "         Increases verbosity level of specific engines.\n"
           "          0 - Clock of position\n"
           "          1 - Clock of transceiver\n"
           "          a - Position engine\n"
           "          m - Moment engine\n"
           "          p - Pulse engine\n"
           "          r - Ring filter engine\n"
           "          s - Sweep engine\n"
           "          w - RadarHub WebSocket Reporter\n"
           "\n"
           "  -T (--test) " UNDERLINE("value") "\n"
           "         Tests a specific component of the RadarKit framework.\n"
           "%s"
           "\n"
           "EXAMPLES:\n"
           "    Here are some examples of typical configurations.\n"
           "\n"
           "    -vs1  (no space after s)\n"
           "         Runs the program in verbose mode, and to simulate a level-1 system.\n"
           "\n"
           "    -vs2\n"
           "         Runs the program in verbose mode, and to simulate a level-2 system.\n"
           "\n"
           "    -s1 -Vss\n"
           "         Runs the program in verbose level = 2 for sweep engine, and to simulate\n"
           "         a level-1 system.\n"
           "\n"
           "    -s1 -Vwww\n"
           "         Runs the program in verbose level = 3 for reporter engine, and to simulate\n"
           "         a level-1 system.\n"
           "\n"
           "    -v -s1 -L -f2000\n"
           "         Same as level-1 system but with PRF = 2,000 Hz.\n"
           "\n"
           "    -T60\n"
           "         Runs the program to measure SIMD performance.\n"
           "\n\n"
           "%s (RadarKit %s)\n\n",
           name,
           RKTestByNumberDescription(9),
           name,
           RKVersionString());
}

static void setSystemLevel(UserParams *user, const int level) {
    switch (level) {
        case 0:
            // Debug
            user->fs = 2000000;
            user->gateCount = 32;
            user->coresForPulseCompression = 2;
            user->coresForPulseRingFilter = 2;
            user->coresForMomentProcessor = 2;
            user->desc.positionBufferDepth = 100;
            user->desc.pulseBufferDepth = 100;
            user->desc.rayBufferDepth = 400;
            user->desc.pulseToRayRatio = 2;
            user->prf = 10;
            sprintf(user->momentMethod, "PulsePair");
            break;
        case 1:
            // Minimum: 5-MHz
            user->fs = 5000000;
            user->gateCount = 2048;
            user->coresForPulseCompression = 2;
            user->coresForPulseRingFilter = 2;
            user->coresForMomentProcessor = 2;
            user->desc.pulseToRayRatio = 2;
            break;
        case 2:
            // Low: 10-MHz
            user->fs = 10000000;
            user->gateCount = 10000;
            user->coresForPulseCompression = 2;
            user->coresForPulseRingFilter = 2;
            user->coresForMomentProcessor = 2;
            user->desc.pulseToRayRatio = 4;
            break;
        case 3:
            // Intermediate: 20-MHz
            user->fs = 20000000;
            user->gateCount = 20000;
            user->coresForPulseCompression = 4;
            user->coresForPulseRingFilter = 2;
            user->coresForMomentProcessor = 2;
            user->desc.pulseToRayRatio = 8;
            break;
        case 4:
            // High: 50-MHz
            user->fs = 50000000;
            user->gateCount = 50000;
            user->coresForPulseCompression = 6;
            user->coresForPulseRingFilter = 2;
            user->coresForMomentProcessor = 4;
            user->desc.pulseToRayRatio = 16;
            break;
        case 5:
            // Full: 100-MHz
            user->fs = 100000000;
            user->gateCount = 100000;
            user->coresForPulseCompression = 12;
            user->coresForPulseRingFilter = 3;
            user->coresForMomentProcessor = 4;
            user->desc.pulseToRayRatio = 16;
            break;
        case 6:
            // Secret: 200-MHz
            user->fs = 200000000;
            user->gateCount = 200000;
            user->coresForPulseCompression = 22;
            user->coresForPulseRingFilter = 3;
            user->coresForMomentProcessor = 4;
            user->desc.pulseToRayRatio = 16;
            user->prf = 600;
            break;
        default:
            // Default
            RKLog("Error. There is no level %d.\n", level);
            exit(EXIT_FAILURE);
            break;
    }
}

static void handleSignals(int signal) {
    if (myRadar == NULL) {
        return;
    }
    fprintf(stderr, "\n");
    if (myRadar->desc.initFlags & RKInitFlagVeryVerbose) {
        RKLog("Caught a %s (%d)  radar->state = 0x%x\n", RKSignalString(signal), signal, myRadar->state);
    }
    bool isForeground = tcgetpgrp(STDIN_FILENO) == getpgrp();
    if (isForeground) {
        RKLog("Use control-D to exit when running in the foreground.\n");
    } else {
        pthread_t t;
        pthread_create(&t, NULL, exitAfterAWhile, NULL);
        RKStop(myRadar);
    }
}

#pragma mark - User Parameters

UserParams *systemPreferencesInit(void) {
    // A structure unit that encapsulates command line user parameters, Zero out everything and set some default parameters
    UserParams *user = (UserParams *)malloc(sizeof(UserParams));
    if (user == NULL) {
        RKLog("Error. Unable to continue.\n");
        exit(EXIT_FAILURE);
    }
    memset(user, 0, sizeof(UserParams));

    // Build a RKRadar initialization description
    user->desc.initFlags = RKInitFlagAllocEverything;
    user->desc.pulseBufferDepth = 2500;
    user->desc.rayBufferDepth = 1500;
    user->desc.latitude = 35.181251;
    user->desc.longitude = -97.436752;
    user->desc.radarHeight = 2.5f;
    user->desc.wavelength = 0.0314f;
    user->desc.pulseToRayRatio = 1;
    user->desc.positionLatency = 0.00001;
    user->port = 10000;
    strcpy(user->desc.dataPath, RKDefaultDataPath);

    return user;
}

void systemPreferencesFree(UserParams *user) {
    free(user);
}

static void updateSystemPreferencesFromControlFile(UserParams *user) {
    int k, s;
    char string[1024];

    // Read in preference configuration file
    RKPreference *userPreferences = RKPreferenceInitWithFile(PREFERENCE_FILE);
    if (userPreferences->count == 0) {
        RKPreferenceFree(userPreferences);
        return;
    }

    RKPreferenceObject *object;

    // Only show the inner working if verbosity level > 1 (1 -> 0, 2+ -> 1)
    const int verb = user->verbose > 1 ? 1 : 0;
    if (verb) {
        RKLog("Reading user preferences ...\n");
    }
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Name",                user->desc.name,            RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "FilePrefix",          user->desc.filePrefix,      RKParameterTypeString, RKMaximumPrefixLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "DataPath",            user->desc.dataPath,        RKParameterTypeString, RKMaximumPathLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "PedzyHost",           user->pedzyHost,            RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "TweetaHost",          user->tweetaHost,           RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "RemoteHost",          user->remoteHost,           RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "MomentMethod",        user->momentMethod,         RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "RingFilter",          user->ringFilter,           RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Latitude",            &user->desc.latitude,       RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Longitude",           &user->desc.longitude,      RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Heading",             &user->desc.heading,        RKParameterTypeFloat,  1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "RingFilterGateCount", &user->ringFilterGateCount, RKParameterTypeUInt,   1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "TransitionGateCount", &user->transitionGateCount, RKParameterTypeUInt,   1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SystemZCal",          user->systemZCal,           RKParameterTypeFloat,  2);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SystemDCal",          &user->systemDCal,          RKParameterTypeFloat,  1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SystemPCal",          &user->systemPCal,          RKParameterTypeFloat,  1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Noise",               user->noise,                RKParameterTypeFloat,  2);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SNRThreshold",        &user->SNRThreshold,        RKParameterTypeFloat,  1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SQIThreshold",        &user->SQIThreshold,        RKParameterTypeFloat,  1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "DiskUsageLimitGB",    &user->diskUsageLimitGB,    RKParameterTypeUInt,   1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "GoCommand",           &user->goCommand,           RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "StopCommand",         &user->stopCommand,         RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "IgnoreGPS",           &user->ignoreGPS,           RKParameterTypeBool, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "DefaultPRF",          &user->prf,                 RKParameterTypeFloat, 1);

    // Revise some parameters
    s = (int)strlen(user->desc.dataPath);
    if (user->desc.dataPath[s - 1] == '/') {
        user->desc.dataPath[s - 1] = '\0';
    }
    if (!isfinite(user->SNRThreshold)) {
        user->SNRThreshold = -20.0f;
    }
    if (user->SQIThreshold < 0.0f) {
        user->SQIThreshold = 0.0f;
    }

    // Shortcuts
    k = 0;
    memset(user->controls, 0, RKMaximumControlCount * sizeof(RKControl));
    while ((object = RKPreferenceFindKeyword(userPreferences, "Shortcut")) != NULL && k < RKMaximumControlCount) {
        RKControlFromPreferenceObject(&user->controls[k], object);
        if (verb) {
            RKLog(">Shortcut '%s' '%s'\n", user->controls[k].label, user->controls[k].command);
        }
        k++;
    }
    if (verb) {
        RKLog(">Shortcut count = %d\n", k);
    }

    // Waveform calibrations
    k = 0;
    memset(user->calibrations, 0, RKMaximumWaveformCalibrationCount * sizeof(RKWaveformCalibration));
    while ((object = RKPreferenceFindKeyword(userPreferences, "WaveformCal")) != NULL && k < RKMaximumWaveformCalibrationCount) {
        RWaveformCalibrationFromPreferenceObject(&user->calibrations[k], object);
        if (verb) {
            s = snprintf(string, RKNameLength, "'%s' (%d)", user->calibrations[k].name, user->calibrations[k].count);
            for (int i = 0; i < MIN(RKMaximumFilterCount, user->calibrations[k].count); i++) {
                s += snprintf(string + s, RKNameLength - s, "   %d:(%.2f %.2f %.2f %.2f)", i,
                              user->calibrations[k].ZCal[i][0],
                              user->calibrations[k].ZCal[i][1],
                              user->calibrations[k].DCal[i],
                              user->calibrations[k].PCal[i]);
            }
            RKLog(">WavformCal %s\n", string);
        }
        k++;
    }
    if (verb) {
        RKLog(">WavformCal count = %d\n", k);
    }
    RKPreferenceFree(userPreferences);
}

static void updateSystemPreferencesFromCommandLine(UserParams *user, int argc, const char **argv, const bool firstPassOnly) {
    int k, s;
    char *c;
    char str[1024];
    bool anyEngineVerbose;

    // Command line options
    struct option long_options[] = {
        {"alarm"             , no_argument      , NULL, 'A'},    // ASCII 65 - 90 : A - Z
        {"clock"             , no_argument      , NULL, 'C'},
        {"dir"               , required_argument, NULL, 'D'},
        {"host"              , required_argument, NULL, 'H'},
        {"port"              , required_argument, NULL, 'P'},
        {"system"            , required_argument, NULL, 'S'},
        {"test"              , required_argument, NULL, 'T'},
        {"engine-verbose"    , required_argument, NULL, 'V'},
        {"show-preference"   , no_argument      , NULL, 'X'},
        {"azimuth"           , required_argument, NULL, 'a'},    // ASCII 97 - 122 : a - z
        {"bandwidth"         , required_argument, NULL, 'b'},
        {"core"              , required_argument, NULL, 'c'},
        {"decimate"          , required_argument, NULL, 'd'},
        {"empty-style"       , no_argument      , NULL, 'e'},
        {"prf"               , required_argument, NULL, 'f'},
        {"gate"              , required_argument, NULL, 'g'},
        {"help"              , no_argument      , NULL, 'h'},
        {"interpulse-period" , required_argument, NULL, 'i'},
        {"pedzy-host"        , required_argument, NULL, 'p'},
        {"relay"             , required_argument, NULL, 'r'},
        {"simulate"          , optional_argument, NULL, 's'},
        {"tweeta-host"       , required_argument, NULL, 't'},
        {"version"           , no_argument      , NULL, 'u'},
        {"verbose"           , no_argument      , NULL, 'v'},
        {"write-data"        , no_argument      , NULL, 'w'},
        {"simulate-sleep"    , required_argument, NULL, 'z'},
        {0, 0, 0, 0}
    };

    // Construct a long-option string
    s = 0;
    for (k = 0; k < sizeof(long_options) / sizeof(struct option); k++) {
        struct option *o = &long_options[k];
        s += snprintf(str + s, 1023 - s, "%c%s", o->val, o->has_arg == required_argument ? ":" : (o->has_arg == optional_argument ? "::" : ""));
    }

    // First pass: just check for verbosity level
    int opt, long_index = 0;
    while ((opt = getopt_long(argc, (char * const *)argv, str, long_options, &long_index)) != -1) {
        switch (opt) {
            case 'v':
                user->verbose++;
                break;
            default:
                break;
        }
    }

    // Going from user->verbose to RKInitFlag
    if (user->verbose == 1) {
        user->desc.initFlags |= RKInitFlagVerbose;
    } else if (user->verbose == 2) {
        user->desc.initFlags |= RKInitFlagVeryVerbose;
    } else if (user->verbose >= 3) {
        user->desc.initFlags |= RKInitFlagVeryVeryVerbose;
    }

    // Early return if we are only interested in setting the verbosity flag
    if (firstPassOnly) {
        return;
    }

    // Second pass: now we go through the rest of them (all of them except doing nothing for 'v')
    // s = 0;
    // for (k = 0; k < sizeof(long_options) / sizeof(struct option); k++) {
    //     struct option *o = &long_options[k];
    //     s += snprintf(str + s, 1023 - s, "%c%s", o->val, o->has_arg == required_argument ? ":" : (o->has_arg == optional_argument ? "::" : ""));
    // }
    #if defined(DEBUG)
    printf("str = %s\n", str);
    #endif
    optind = 1;
    long_index = 0;
    while ((opt = getopt_long(argc, (char * const *)argv, str, long_options, &long_index)) != -1) {
        #if defined(DEBUG)
        fprintf(stderr, "--> %c = %s\n", opt, optarg ? optarg : "NULL");
        #endif
        switch (opt) {
            case 'A':
                break;
            case 'C':
                user->desc.initFlags |= RKInitFlagShowClockOffset;
                break;
            case 'D':
                if (optarg == NULL) {
                    RKLog("Playback folder without folder.\n");
                    exit(EXIT_FAILURE);
                }
                user->simulate = false;
                user->desc.initFlags = RKInitFlagIQPlayback;
                strncpy(user->playbackFolder, RKPathStringByExpandingTilde(optarg), sizeof(user->playbackFolder));
                RKLog("==> %s ==> %s\n", optarg, user->playbackFolder);
                break;
            case 'H':
                strncpy(user->remoteHost, optarg, sizeof(user->remoteHost));
                user->pedzyHost[sizeof(user->pedzyHost) - 1] = '\0';
                break;
            case 'P':
                user->port = atoi(optarg);
                break;
            case 'S':
                k = atoi(optarg);
                setSystemLevel(user, k);
                break;
            case 'T':
                // A bunch of different tests
                k = atoi(optarg);
                RKTestByNumber(k, argc == optind ? NULL : argv[optind]);
                exit(EXIT_SUCCESS);
                break;
            case 'V':
                c = optarg;
                do {
                    user->engineVerbose[(int)*c]++;
                } while (*++c != '\0');
                break;
            case 'X':
                user->verbose = 2;
                RKSetWantScreenOutput(true);
                updateSystemPreferencesFromControlFile(user);
                exit(EXIT_SUCCESS);
                break;
            case 'b':
                user->fs = roundf(atof(optarg));
                break;
            case 'c':
                sscanf(optarg, "%d,%d,%d",
                       &user->coresForPulseCompression,
                       &user->coresForPulseRingFilter,
                       &user->coresForMomentProcessor);
                break;
            case 'd':
                user->desc.pulseToRayRatio = atoi(optarg);
                break;
            case 'e':
                RKSetWantColor(false);
                break;
            case 'f':
                k = sscanf(optarg, "%f,%d", &user->prf, &user->sprt);
                if (k < 2) {
                    user->sprt = 0;
                }
                break;
            case 'g':
                k = (int)strtol(optarg, NULL, 10);
                if (k > 0) {
                    user->gateCount = k;
                }
                break;
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
                break;
            case 'i':
                k = sscanf(optarg, "%f,%d", &user->prf, &user->sprt);
                user->prf = 1.0f / user->prf;
                if (k < 2) {
                    user->sprt = 0;
                }
                break;
            case 'p':
                strncpy(user->pedzyHost, optarg, sizeof(user->pedzyHost));
                user->pedzyHost[sizeof(user->pedzyHost) - 1] = '\0';
                break;
            case 'r':
                user->simulate = false;
                user->desc.initFlags = RKInitFlagRelay;
                if (argc > optind && argv[optind][0] != '-') {
                    // The next argument is not an option, interpret as streams
                    if (argv[optind][0] == 's') {
                        // Convert product to sweep: Z V W D P R K --> Y U X C O Q J
                        strcpy(user->streams, &argv[optind][1]);
                        c = user->streams;
                        while (c < user->streams + strlen(user->streams)) {
                            switch (*c) {
                                case 'z':
                                case 'Z':
                                    *c = 'Y';
                                    break;
                                case 'v':
                                case 'V':
                                    *c = 'U';
                                    break;
                                case 'w':
                                case 'W':
                                    *c = 'X';
                                    break;
                                case 'd':
                                case 'D':
                                    *c = 'C';
                                    break;
                                case 'p':
                                case 'P':
                                    *c = 'O';
                                    break;
                                case 'r':
                                case 'R':
                                    *c = 'Q';
                                    break;
                                case 'k':
                                case 'K':
                                    *c = 'J';
                                    break;
                                default:
                                    break;
                            }
                            c++;
                        }
                        if (user->verbose) {
                            RKLog("Stream %s --> %s\n", argv[optind], user->streams);
                        }
                    } else {
                        strcpy(user->streams, argv[optind]);
                        if (user->verbose) {
                            RKLog("Stream %s\n", user->streams);
                        }
                    }
                    optind++;
                }
                strncpy(user->relayHost, optarg, sizeof(user->relayHost));
                user->relayHost[sizeof(user->relayHost) - 1] = '\0';
                break;
            case 's':
                if (strlen(user->relayHost)) {
                    RKLog("Relay and simulation? Perhaps no -s.\n");
                    exit(EXIT_FAILURE);
                }
                user->simulate = true;
                if (optarg) {
                    k = (int)strtol(optarg, NULL, 10);
                    setSystemLevel(user, k);
                } else {
                    setSystemLevel(user, 1);
                }
                break;
            case 't':
                strncpy(user->tweetaHost, optarg, sizeof(user->tweetaHost));
                user->tweetaHost[sizeof(user->tweetaHost) - 1] = '\0';
                break;
            case 'u':
                printf("Version %s\n", RKVersionString());
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                break;
            case 'w':
                user->recordLevel++;
                printf("user->recordLevel = %d\n", user->recordLevel);
                break;
            case 'z':
                if (optarg) {
                    user->sleepInterval = atoi(optarg);
                } else {
                    user->sleepInterval = 1000;
                }
                break;
            case ':':
                fprintf(stderr, "Missing option argument.\n");
            case '?':
            default:
                if (optarg && strlen(optarg)) {
                    fprintf(stderr, "I don't understand: -%c   optarg = %s\n", opt, optarg);
                }
                exit(EXIT_FAILURE);
                break;
        }
    }
    if (user->verbose == 0) {
        anyEngineVerbose = false;
        for (k = 'a'; k < 'z'; k++) {
            anyEngineVerbose |= user->engineVerbose[k];
        }
        if (anyEngineVerbose) {
            user->verbose++;
        }
    }
    if (user->simulate == true) {
        if (user->prf == 0) {
            user->prf = 1000.0f;
        }
    } else {
        if (!(user->desc.initFlags == RKInitFlagRelay) && !(user->desc.initFlags == RKInitFlagIQPlayback)) {
            fprintf(stderr, "No options specified. Don't want to do anything?\n");
            exit(EXIT_FAILURE);
        }
        if (user->prf) {
            RKLog("Warning. PRF has no effects without simulation.\n");
        }
        if (user->gateCount == 0) {
            setSystemLevel(user, 1);
        }
    }
    if (user->prf > 0) {
        k = user->fs / user->prf;
    } else {
        k = user->fs / 1000.0f;
    }
    if (user->gateCount > k) {
        RKLog("Info. Gate count adjusted: %s -> %s for PRF (%s kHz) and bandwidth (%s MHz)",
              RKIntegerToCommaStyleString(user->gateCount), RKIntegerToCommaStyleString(k),
              RKFloatToCommaStyleString(1.0e-3 * user->prf), RKFloatToCommaStyleString(1.0e-6 * user->fs));
        user->gateCount = k;
    }
    user->desc.pulseCapacity = user->gateCount;
}

#pragma mark - Radar Parameters

static void updateRadarParameters(UserParams *systemPreferences) {

    int k;

    // Some parameters before the radar is live
    if (!myRadar->active) {
        RKSetProcessingCoreCounts(myRadar,
                                  systemPreferences->coresForPulseCompression,
                                  systemPreferences->coresForPulseRingFilter,
                                  systemPreferences->coresForMomentProcessor);
        RKSetRecordingLevel(myRadar, systemPreferences->recordLevel);
        RKSweepEngineSetFilesHandlingScript(myRadar->sweepEngine, "scripts/handlefiles.sh", RKScriptPropertyProduceTarXz);
        if (systemPreferences->diskUsageLimitGB) {
            RKLog("Setting disk usage limit to %s GB ...\n", RKIntegerToCommaStyleString(systemPreferences->diskUsageLimitGB));
            RKFileManagerSetDiskUsageLimit(myRadar->fileManager, (size_t)systemPreferences->diskUsageLimitGB * (1 << 30));
        }
    }

    // General attributes
    if (strcmp(myRadar->desc.name, systemPreferences->desc.name)) {
        strcpy(myRadar->desc.name, systemPreferences->desc.name);
        RKLog("Radar name changed to '%s'\n", myRadar->desc.name);
    }
    if (strcmp(myRadar->desc.filePrefix, systemPreferences->desc.filePrefix)) {
        strcpy(myRadar->desc.filePrefix, systemPreferences->desc.filePrefix);
        RKLog("Product file prefix changed to '%s'\n", myRadar->desc.filePrefix);
    }

    // GPS override
    if (systemPreferences->ignoreGPS) {
        myRadar->desc.initFlags |= RKInitFlagIgnoreGPS;
        myRadar->desc.latitude = systemPreferences->desc.latitude;
        myRadar->desc.longitude = systemPreferences->desc.longitude;
        myRadar->desc.heading = systemPreferences->desc.heading;
    } else {
        myRadar->desc.initFlags &= ~RKInitFlagIgnoreGPS;
    }

    // Moment methods
    if (!strncasecmp(systemPreferences->momentMethod, "multilag", 8)) {
        int lagChoice = atoi(systemPreferences->momentMethod + 8);
        RKSetMomentProcessorToMultiLag(myRadar, lagChoice);
    } else if (!strncasecmp(systemPreferences->momentMethod, "pulsepairhop", 12)) {
        RKSetMomentProcessorToPulsePairHop(myRadar);
    } else if (!strncasecmp(systemPreferences->momentMethod, "SpectralMoment", 14)) {
        RKSetMomentProcessorToSpectralMoment(myRadar);
    } else {
        RKSetMomentProcessorToPulsePair(myRadar);
    }

    // Always refresh the controls
    RKClearControls(myRadar);
    k = 0;
    while (systemPreferences->controls[k].label[0] != '\0' && k < RKMaximumControlCount) {
        RKAddControl(myRadar, &systemPreferences->controls[k]);
        k++;
    }
    RKConcludeControls(myRadar);

    // Always refresh waveform calibrations
    RKClearWaveformCalibrations(myRadar);
    k = 0;
    while (systemPreferences->calibrations[k].name[0] != '\0' && k < RKMaximumWaveformCalibrationCount) {
        RKAddWaveformCalibration(myRadar, &systemPreferences->calibrations[k]);
        k++;
    }
    RKConcludeWaveformCalibrations(myRadar);

    // Pulse ring filter
    if (!strcasecmp(systemPreferences->ringFilter, "e1") || !strcasecmp(systemPreferences->ringFilter, "elliptical1")) {
        RKSetPulseRingFilterByType(myRadar, RKFilterTypeElliptical1, 0);
    } else if (!strcasecmp(systemPreferences->ringFilter, "e2") || !strcasecmp(systemPreferences->ringFilter, "elliptical2")) {
        RKSetPulseRingFilterByType(myRadar, RKFilterTypeElliptical2, 0);
    } else if (!strcasecmp(systemPreferences->ringFilter, "e3") || !strcasecmp(systemPreferences->ringFilter, "elliptical3")) {
        RKSetPulseRingFilterByType(myRadar, RKFilterTypeElliptical3, 0);
    } else if (!strcasecmp(systemPreferences->ringFilter, "e4") || !strcasecmp(systemPreferences->ringFilter, "elliptical4")) {
        RKSetPulseRingFilterByType(myRadar, RKFilterTypeElliptical4, 0);
    }

    // Refresh all system calibration
    RKAddConfig(myRadar,
                RKConfigKeyPRF, systemPreferences->prf,
                RKConfigKeySystemNoise, systemPreferences->noise[0], systemPreferences->noise[1],
                RKConfigKeySystemZCal, systemPreferences->systemZCal[0], systemPreferences->systemZCal[1],
                RKConfigKeySystemDCal, systemPreferences->systemDCal,
                RKConfigKeySystemPCal, systemPreferences->systemPCal,
                RKConfigKeySNRThreshold, systemPreferences->SNRThreshold,
                RKConfigKeySQIThreshold, systemPreferences->SQIThreshold,
                RKConfigKeyTransitionGateCount, systemPreferences->transitionGateCount,
                RKConfigKeyRingFilterGateCount, systemPreferences->ringFilterGateCount,
                RKConfigKeyNull);

    // Force waveform reload to propagate the new waveform calibration values
//    RKLog("waveform %p\n", myRadar->waveform);
//    RKSetWaveform(myRadar, myRadar->waveform);
}

static void handlePreferenceFileUpdate(void *in) {
    RKFileMonitor *engine = (RKFileMonitor *)in;
    UserParams *user = (UserParams *)engine->userResource;

    // Update user parameters from preference file then update the radar
    updateSystemPreferencesFromControlFile(user);
    updateRadarParameters(user);
}

#pragma mark - Main

//
//
//  M A I N
//
//

int main(int argc, const char **argv) {

    int k;
    RKCommand cmd = "";
    char name[] = __FILE__;
    *strrchr(name, '.') = '\0';

    RKSetProgramName(name);

    char *term = getenv("TERM");
    if (term == NULL || (strcasestr(term, "color") == NULL && strcasestr(term, "ansi") == NULL)) {
        RKSetWantColor(false);
    }

    // Initial a struct of user parameters
    UserParams *systemPreferences = systemPreferencesInit();

    // Check for verbosity with first pass, then update user parameters from a preference file, then override by command line arguments
    updateSystemPreferencesFromCommandLine(systemPreferences, argc, argv, true);
    updateSystemPreferencesFromControlFile(systemPreferences);
    updateSystemPreferencesFromCommandLine(systemPreferences, argc, argv, false);

    // Show framework name & version
    RKShowName();

    // Screen output based on verbosity level
    if (systemPreferences->verbose) {
        RKSetWantScreenOutput(true);
        if (systemPreferences->verbose > 1) {
            printf("TERM = %s --> %s\n", term,
                   rkGlobalParameters.showColor ?
                   RKRedColor "s" RKOrangeColor "h" RKYellowColor "o" RKLimeColor "w" RKGreenColor "C" RKTealColor "o" RKBlueColor "l" RKPurpleColor "o" RKRedColor "r" RKNoColor :
                   "noColor");
        }
    } else {
        RKSetWantScreenOutput(false);
    }
    //printf("rootDataFolder = %s\n", rkGlobalParameters.rootDataFolder);

    // Initialize a radar object
    myRadar = RKInitWithDesc(systemPreferences->desc);
    if (myRadar == NULL) {
        RKLog("Error. Could not allocate a radar.\n");
        exit(EXIT_FAILURE);
    }

    // Verbosity
    RKSetVerbosity(myRadar, systemPreferences->verbose);
    RKSetVerbosityUsingArray(myRadar, systemPreferences->engineVerbose);

    // Update parameters for RadarKit
    updateRadarParameters(systemPreferences);

    // Catch Ctrl-C and some signals alternative handling
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);
    signal(SIGKILL, handleSignals);

    // Make a command center and add the radar to it
    RKCommandCenter *center = RKCommandCenterInit();
    RKCommandCenterSetVerbose(center, systemPreferences->verbose);
    RKCommandCenterSetPort(center, systemPreferences->port);
    RKCommandCenterStart(center);
    RKCommandCenterAddRadar(center, myRadar);

    // Make a reporter and have it call a RadarHub
    RKReporter *reporter = RKReporterInitWithHost(systemPreferences->remoteHost);
    if (reporter == NULL) {
        RKLog("Error. Unable to initiate reporter\n");
    } else {
        int v = MAX(systemPreferences->verbose, systemPreferences->engineVerbose['w']);
        RKReporterSetVerbose(reporter, v);
        RKReporterSetRadar(reporter, myRadar);
        RKReporterStart(reporter);
    }

    // Now we use the framework.
    if (systemPreferences->simulate) {

        // Build a series of options for transceiver, only pass down the relevant parameters
        k = 0;
        if (systemPreferences->fs) {
            k += sprintf(cmd + k, " F %.0f", systemPreferences->fs);
        }
        if (systemPreferences->prf) {
            if (systemPreferences->sprt > 1) {
                k += sprintf(cmd + k, " f %.0f,%d", systemPreferences->prf, systemPreferences->sprt);
            } else {
                k += sprintf(cmd + k, " f %.0f", systemPreferences->prf);
            }
        }
        if (systemPreferences->gateCount) {
            k += sprintf(cmd + k, " g %d", systemPreferences->gateCount);
        }
        if (systemPreferences->sleepInterval) {
            k += sprintf(cmd + k, " z %d", systemPreferences->sleepInterval);
        }
        if (systemPreferences->verbose > 1) {
            RKLog("Transceiver input = '%s' (%d / %s)", cmd + 1, k, RKIntegerToCommaStyleString(RKMaximumStringLength));
        }
        RKSetTransceiver(myRadar,
                         (void *)cmd,
                         RKTestTransceiverInit,
                         RKTestTransceiverExec,
                         RKTestTransceiverFree);

        // Build a series of options for pedestal, only pass down the relevant parameters
        if (strlen(systemPreferences->pedzyHost)) {
            if (systemPreferences->verbose > 1) {
                RKLog("Pedestal input = '%s'", systemPreferences->pedzyHost);
            }
            RKSetPedestal(myRadar,
                          (void *)systemPreferences->pedzyHost,
                          RKPedestalPedzyInit,
                          RKPedestalPedzyExec,
                          RKPedestalPedzyFree);
        } else {
            RKSetPedestal(myRadar,
                          NULL,
                          RKTestPedestalInit,
                          RKTestPedestalExec,
                          RKTestPedestalFree);
        }

        if (strlen(systemPreferences->tweetaHost)) {
            if (systemPreferences->verbose > 1) {
                RKLog("Health relay input = '%s'", systemPreferences->tweetaHost);
            }
            RKSetHealthRelay(myRadar,
                             (void *)systemPreferences->tweetaHost,
                             RKHealthRelayTweetaInit,
                             RKHealthRelayTweetaExec,
                             RKHealthRelayTweetaFree);
        } else {
            RKSetHealthRelay(myRadar,
                             NULL,
                             RKTestHealthRelayInit,
                             RKTestHealthRelayExec,
                             RKTestHealthRelayFree);
        }

        // Radar going live
        RKGoLive(myRadar);

        // Simulation mode
        RKLog("Setting a waveform ...\n");
        //RKExecuteCommand(myRadar, "t w x", NULL);
        //RKExecuteCommand(myRadar, "t w s01", NULL);
        //RKExecuteCommand(myRadar, "t w ofm", NULL);
        //RKExecuteCommand(myRadar, "t w q02", NULL);
        //RKExecuteCommand(myRadar, "t w q10", NULL);
        //RKExecuteCommand(myRadar, "t w h040502.5", NULL);
        //RKExecuteCommand(myRadar, "t w h2007.5", NULL);
        //RKExecuteCommand(myRadar, "t w h2005", NULL);
        RKExecuteCommand(myRadar, "t w h20052.5", NULL);
        //RKExecuteCommand(myRadar, "t w h0507", NULL);
        //RKSetWaveformToImpulse(myRadar);

        RKLog("Starting a new PPI ... PRF = %s Hz\n", RKIntegerToCommaStyleString(systemPreferences->prf));
        if (systemPreferences->prf <= 20.0f) {
            RKExecuteCommand(myRadar, "p ppi 3 2.0", NULL);
        } else if (systemPreferences->prf <= 100.0f) {
            RKExecuteCommand(myRadar, "p ppi 3 5", NULL);
        } else {
            RKExecuteCommand(myRadar, "p ppi 3 60", NULL);
        }

        RKFileMonitor *preferenceFileMonitor = RKFileMonitorInit(PREFERENCE_FILE, handlePreferenceFileUpdate, systemPreferences);

        // Wait indefinitely until something happens through a user command through the command center
        RKWaitWhileActive(myRadar);

        RKFileMonitorFree(preferenceFileMonitor);

    } else if (systemPreferences->desc.initFlags == RKInitFlagRelay) {

        RKRadarRelaySetHost(myRadar->radarRelay, systemPreferences->relayHost);
        RKSetRecordingLevel(myRadar, 0);

        // Assembly a string that describes streams
        if (strlen(systemPreferences->streams)) {
            RKRadarRelayUpdateStreams(myRadar->radarRelay, RKStreamFromString(systemPreferences->streams));
        }

        // Radar going live, then wait indefinitely until something happens
        RKGoLive(myRadar);
        RKWaitWhileActive(myRadar);

    } else if (systemPreferences->desc.initFlags == RKInitFlagIQPlayback) {

        // Build a series of options for transceiver, only pass down the relevant parameters
        k = 0;
        if (strlen(systemPreferences->playbackFolder)) {
            k += snprintf(cmd + k, RKMaximumCommandLength - k, " D %s", systemPreferences->playbackFolder);
            cmd[RKMaximumCommandLength - 1] = '\0';
        }
        if (k == 0 && cmd[0] != '\0') {
            RKLog("I could crash here at k = %d && cmd[0] = %c\n", k, cmd[0]);
        }
        RKSetTransceiver(myRadar,
                         (void *)cmd,
                         RKTestTransceiverInit,
                         RKTestTransceiverExec,
                         RKTestTransceiverFree);

        // Radar going live
        RKGoLive(myRadar);

        // Wait indefinitely until something happens through a user command through the command center
        RKWaitWhileActive(myRadar);

    } else {

        RKSetWantScreenOutput(true);
        RKLog("Error. This should not happen.");

    }

    RKCommandCenterRemoveRadar(center, myRadar);
    RKCommandCenterStop(center);
    RKCommandCenterFree(center);

    if (reporter) {
        RKReporterStop(reporter);
        RKReporterFree(reporter);
    }

    RKFree(myRadar);

    systemPreferencesFree(systemPreferences);

    return 0;
}
