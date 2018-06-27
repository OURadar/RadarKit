//
//  main.c
//  RadarKitTest
//
//  Created by Boon Leng Cheong
//  Copyright (c) 2016-2018 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit.h>
#include <getopt.h>

#define ROOT_PATH                   "data"
#define PREFERENCE_FILE             "pref.conf"

// User parameters in a struct
typedef struct user_params {
    RKRadarDesc              desc;
    RKName                   pedzyHost;
    RKName                   tweetaHost;
    RKName                   relayHost;
    RKName                   momentMethod;
    RKName                   streams;
    uint8_t                  verbose;                                            // Verbosity
    int                      coresForPulseCompression;                           // Number of cores for pulse compression
    int                      coresForProductGenerator;                           // Number of cores for moment calculations
    float                    fs;                                                 // Raw gate sampling bandwidth
    float                    prf;                                                // Base PRF (Hz)
    int                      sprt;                                               // Staggered PRT option (2 for 2:3, 3 for 3:4, etc.)
    int                      gateCount;                                          // Number of gates (simulate mode)
    int                      sleepInterval;                                      // Intermittent sleep period in transceiver simulator in seconds
    int                      recordLevel;                                        // Data recording (1 - moment + health logs only, 2 - everything)
    bool                     simulate;                                           // Run with transceiver simulator
    double                   systemZCal[2];                                      // System calibration for Z
    double                   systemDCal;                                         // System calibration for D
    double                   systemPCal;                                         // System calibration for P
    double                   noise[2];                                           // System noise level
    double                   SNRThreshold;                                       // SNR threshold for moment processors
    RKControl                controls[RKMaximumControlCount];                    // Controls for GUI
    RKWaveformCalibration    calibrations[RKMaximumWaveformCalibrationCount];    // Waveform specific calibration factors
    uint8_t                  engineVerbose[128];                                 // Letter A = 65, 'z' = 122
} UserParams;

// Global variables
RKRadar *myRadar = NULL;

// Functions
void *exitAfterAWhile(void *s) {
    sleep(2);
    RKLog("Forced exit.\n");
    exit(EXIT_SUCCESS);
}

#pragma mark - Local Functions

static void showHelp() {
    printf("RadarKit Test Program\n\n"
           "rktest [options]\n\n"
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
           "  -T (--test) " UNDERLINE("value") "\n"
           "          0 - Show types\n"
           "          1 - Show colors\n"
           "          2 - Test pretty strings\n"
           "          3 - Test modulo math macros\n"
           "          4 - Test parsing comma delimited values\n"
           "          5 - Test parsing values in a JSON string\n"
           "          6 - Test initializing a file maanger - RKFileManagerInit()\n"
           "          7 - Test reading a preference file - RKPreferenceInit()\n"
           "          8 - Test counting files using RKCountFilesInPath()\n"
           "          9 - Test the file monitor module - RKFileMonitor()\n"
           "         10 - Test the internet monitor module - RKHostMonitorInit()\n"
           "         11 - Test initializing a radar system - RKRadarInit()\n"
           "         12 - Test converting a temperature reading to status\n"
           "         13 - Test getting a country name from position\n"
           "         14 - Test generating text for buffer overview\n"
           "         15 - Test reading a netcdf file using RKSweepRead()\n"
           "         16 - Test reading a netcdf file using RKProductRead()\n"
           "\n"
           "         20 - SIMD quick test\n"
           "         21 - SIMD test with numbers shown\n"
           "         22 - Show window types\n"
           "         23 - Hilbert transform\n"
           "         24 - Optimize FFT performance and generate an fft-wisdom file\n"
           "\n"
           "         30 - Make a frequency hopping sequence\n"
           "         31 - Make a TFM waveform\n"
           "         32 - Generate a waveform file\n"
           "         33 - Test showing waveform properties\n"
           "\n"
           "         40 - Pulse compression using simple cases\n"
           "         41 - Calculating one ray using the Pulse Pair method\n"
           "         42 - Calculating one ray using the Pulse Pair Hop method\n"
           "         43 - Calculating one ray using the Multi-Lag method with L = 2\n"
           "         44 - Calculating one ray using the Multt-Lag method with L = 3\n"
           "         45 - Calculating one ray using the Multi-Lag method with L = 4\n"
           "\n"
           "         50 - Measure the speed of SIMD calculations\n"
           "         51 - Measure the speed of pulse compression\n"
           "         52 - Measure the speed of various moment methods\n"
           "         53 - Measure the speed of cached write\n"
           "\n"
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
           "    -v -s1 -L -f2000\n"
           "         Same as level-1 system but with PRF = 2,000 Hz.\n"
           "\n"
           "    -T 50\n"
           "         Runs the program to measure SIMD performance.\n"
           "\n\n"
           "rktest (RadarKit " RKVersionString ")\n\n"
           );
}

static void setSystemLevel(UserParams *user, const int level) {
    switch (level) {
        case 0:
            // Debug
            user->fs = 5000000;
            user->gateCount = 60;
            user->coresForPulseCompression = 2;
            user->coresForProductGenerator = 2;
            user->desc.pulseBufferDepth = 20;
            user->desc.rayBufferDepth = 20;
            user->desc.pulseToRayRatio = 2;
            user->prf = 10;
            break;
        case 1:
            // Minimum: 5-MHz
            user->fs = 5000000;
            user->gateCount = 4000;
            user->coresForPulseCompression = 2;
            user->coresForProductGenerator = 2;
            user->desc.pulseToRayRatio = 2;
            break;
        case 2:
            // Low: 10-MHz
            user->fs = 10000000;
            user->gateCount = 10000;
            user->coresForPulseCompression = 2;
            user->coresForProductGenerator = 2;
            user->desc.pulseToRayRatio = 4;
            break;
        case 3:
            // Intermediate: 20-MHz
            user->fs = 20000000;
            user->gateCount = 20000;
            user->coresForPulseCompression = 4;
            user->coresForProductGenerator = 2;
            user->desc.pulseToRayRatio = 8;
            break;
        case 4:
            // High: 50-MHz
            user->fs = 50000000;
            user->gateCount = 50000;
            user->coresForPulseCompression = 4;
            user->coresForProductGenerator = 4;
            user->desc.pulseToRayRatio = 16;
            break;
        case 5:
            // Full: 100-MHz
            user->fs = 100000000;
            user->gateCount = 100000;
            user->coresForPulseCompression = 8;
            user->coresForProductGenerator = 4;
            user->desc.pulseToRayRatio = 32;
            break;
        case 6:
            // Secret: 200-MHz
            user->fs = 200000000;
            user->gateCount = 200000;
            user->coresForPulseCompression = 10;
            user->coresForProductGenerator = 4;
            user->desc.pulseToRayRatio = 64;
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
    RKLog("Caught a %s (%d)  radar->state = 0x%x\n", RKSignalString(signal), signal, myRadar->state);
    pthread_t t;
    pthread_create(&t, NULL, exitAfterAWhile, NULL);
    RKStop(myRadar);
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
    user->desc.wavelength = 0.03f;
    user->desc.pulseToRayRatio = 1;
    strcpy(user->desc.dataPath, ROOT_PATH);
    
    return user;
}

void userParametersFree(UserParams *user) {
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
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Name",         user->desc.name,       RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "FilePrefix",   user->desc.filePrefix, RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "DataPath",     user->desc.dataPath,   RKParameterTypeString, RKMaximumPathLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "PedzyHost",    user->pedzyHost,       RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "TweetaHost",   user->tweetaHost,      RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "MomentMethod", user->momentMethod,    RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Latitude",     &user->desc.latitude,  RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Longitude",    &user->desc.longitude, RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Heading",      &user->desc.heading,   RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SystemZCal",   user->systemZCal,      RKParameterTypeDouble, 2);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SystemDCal",   &user->systemDCal,     RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SystemPCal",   &user->systemPCal,     RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Noise",        user->noise,           RKParameterTypeDouble, 2);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "SNRThreshold", &user->SNRThreshold,   RKParameterTypeDouble, 1);
    
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
            for (int i = 0; i < MIN(RKMaxFilterCount, user->calibrations[k].count); i++) {
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

    // Command line options
    struct option long_options[] = {
        {"alarm"             , no_argument      , NULL, 'A'},    // ASCII 65 - 90 : A - Z
        {"clock"             , no_argument      , NULL, 'C'},
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
    
    // First pass: just check for verbosity level
    s = 0;
    for (k = 0; k < sizeof(long_options) / sizeof(struct option); k++) {
        struct option *o = &long_options[k];
        s += snprintf(str + s, 1023 - s, "%c%s", o->val, o->val == 'v' ? "" : "::");
    }
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
    s = 0;
    for (k = 0; k < sizeof(long_options) / sizeof(struct option); k++) {
        struct option *o = &long_options[k];
        s += snprintf(str + s, 1023 - s, "%c%s", o->val, o->has_arg == required_argument ? ":" : (o->has_arg == optional_argument ? "::" : ""));
    }
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
            case 'S':
                k = atoi(optarg);
                setSystemLevel(user, k);
                break;
            case 'T':
                // A bunch of different tests
                RKSetWantScreenOutput(true);
                k = atoi(optarg);
                switch (k) {
                    case 0:
                        RKShowTypeSizes();
                        printf("\n");
                        RKNetworkShowPacketTypeNumbers();
                        break;
                    case 1:
                        RKTestTerminalColors();
                        break;
                    case 2:
                        RKTestPrettyStrings();
                        break;
                    case 3:
                        RKTestModuloMath();
                        break;
                    case 4:
                        RKTestParseCommaDelimitedValues();
                        break;
                    case 5:
                        RKTestParseJSONString();
                        break;
                    case 6:
                        RKTestFileManager();
                        break;
                    case 7:
                        RKTestPreferenceReading();
                        break;
                    case 8:
                        RKTestCountFiles();
                        break;
                    case 9:
                        RKTestFileMonitor();
                        break;
                    case 10:
                        RKTestHostMonitor();
                        break;
                    case 11:
                        RKTestInitializingRadar();
                        break;
                    case 12:
                        RKTestTemperatureToStatus();
                        break;
                    case 13:
                        RKTestGetCountry();
                        break;
                    case 14:
                        RKTestBufferOverviewText();
                        break;
                    case 15:
                        if (argc == optind) {
                            RKLog("No filename given.\n");
                            exit(EXIT_FAILURE);
                        }
                        RKTestSweepRead(argv[optind]);
                        break;
                    case 16:
                        if (argc == optind) {
                            RKLog("No filename given.\n");
                            exit(EXIT_FAILURE);
                        }
                        RKTestProductRead(argv[optind]);
                        break;
                    case 20:
                        RKTestSIMD(RKTestSIMDFlagNull);
                        break;
                    case 21:
                        RKTestSIMD(RKTestSIMDFlagShowNumbers);
                        break;
                    case 22:
                        RKTestWindow();
                        break;
                    case 23:
                        RKTestHilbertTransform();
                        break;
                    case 24:
                        RKTestWriteFFTWisdom();
                        break;
                    case 30:
                        RKTestMakeHops();
                        break;
                    case 31:
                        RKTestWaveformTFM();
                        break;
                    case 32:
                        RKTestWaveformWrite();
                        break;
                    case 33:
                        RKTestWaveformProperties();
                        break;
                    case 40:
                        RKTestPulseCompression((user->verbose ? RKTestFlagVerbose : 0) | RKTestFlagShowResults);
                        break;
                    case 41:
                        RKTestOneRay(RKPulsePair, 0);
                        break;
                    case 42:
                        RKTestOneRay(RKPulsePairHop, 0);
                        break;
                    case 43:
                        RKTestOneRay(RKMultiLag, 2);
                        break;
                    case 44:
                        RKTestOneRay(RKMultiLag, 3);
                        break;
                    case 45:
                        RKTestOneRay(RKMultiLag, 4);
                        break;
                    case 50:
                        RKTestSIMD(RKTestSIMDFlagPerformanceTestAll);
                        break;
                    case 51:
                        RKTestPulseCompressionSpeed();
                        break;
                    case 52:
                        RKTestMomentProcessorSpeed();
                        break;
                    case 53:
                        RKTestCacheWrite();
                        break;
                    default:
                        RKLog("Test %d is invalid.\n", k);
                        break;
                }
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
                updateSystemPreferencesFromControlFile(user);
                exit(EXIT_SUCCESS);
                break;
            case 'b':
                user->fs = roundf(atof(optarg));
                break;
            case 'c':
                sscanf(optarg, "%d,%d", &user->coresForPulseCompression, &user->coresForProductGenerator);
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
                break;
            case 'u':
                printf("Version " RKVersionString "\n");
                exit(EXIT_SUCCESS);
                break;
            case 'v':
                break;
            case 'w':
                user->recordLevel++;
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
    if (user->simulate == true) {
        if (user->prf == 0) {
            user->prf = 1000;
        }
    } else {
        if (!(user->desc.initFlags == RKInitFlagRelay)) {
            RKLog("No options specified. Don't want to do anything?\n");
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
    user->desc.pulseCapacity = 10 * ceil(0.1 * user->gateCount);
}

#pragma mark - Radar Parameters

static void updateRadarParameters(UserParams *systemPreferences) {
    
    int k;
    
    // Some parameters before the radar is live
    if (!myRadar->active) {
        RKSetProcessingCoreCounts(myRadar, systemPreferences->coresForPulseCompression, systemPreferences->coresForProductGenerator);
        RKSetRecordingLevel(myRadar, systemPreferences->recordLevel);
        //RKSetDataUsageLimit(myRadar, (size_t)20 * (1 << 30));
        RKSweepEngineSetHandleFilesScript(myRadar->sweepEngine, "scripts/handlefiles.sh", true);
    }

    // Moment methods
    if (!strncasecmp(systemPreferences->momentMethod, "multilag", 8)) {
        int lagChoice = atoi(systemPreferences->momentMethod + 8);
        RKSetMomentProcessorToMultiLag(myRadar, lagChoice);
    } else {
        RKSetMomentProcessorToPulsePairHop(myRadar);
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
    
    // Refresh all system calibration
    RKAddConfig(myRadar,
                RKConfigKeySystemNoise, systemPreferences->noise[0], systemPreferences->noise[1],
                RKConfigKeySystemZCal, systemPreferences->systemZCal[0], systemPreferences->systemZCal[1],
                RKConfigKeySystemDCal, systemPreferences->systemDCal,
                RKConfigKeySystemPCal, systemPreferences->systemPCal,
                RKConfigKeySNRThreshold, systemPreferences->SNRThreshold,
                RKConfigKeyNull);

    // Force waveform reload to propagate the new waveform calibration values
    RKSetWaveform(myRadar, myRadar->waveform);
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

    RKSetProgramName("rktest");
    RKSetWantScreenOutput(true);

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

    // Screen output based on verbosity level
    if (systemPreferences->verbose) {
        if (systemPreferences->verbose > 1) {
            printf("TERM = %s --> %s\n", term,
                   rkGlobalParameters.showColor ?
                   RKRedColor "s" RKOrangeColor "h" RKYellowColor "o" RKLimeColor "w" RKGreenColor "C" RKTealColor "o" RKBlueColor "l" RKPurpleColor "o" RKRedColor "r" RKNoColor :
                   "noColor");
        }
    } else {
        RKSetWantScreenOutput(false);
    }

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
    
    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);
    signal(SIGKILL, handleSignals);

    // Make a command center and add the radar to it
    RKCommandCenter *center = RKCommandCenterInit();
    RKCommandCenterSetVerbose(center, systemPreferences->verbose);
    RKCommandCenterStart(center);
    RKCommandCenterAddRadar(center, myRadar);

    if (systemPreferences->simulate) {

        // Now we use the frame work.
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

        RKSweepEngineSetHandleFilesScript(myRadar->sweepEngine, "scripts/handlefiles.sh", true);

        // Radar going live, then wait indefinitely until something happens
        RKGoLive(myRadar);

        RKFileMonitor *preferenceFileMonitor = RKFileMonitorInit(PREFERENCE_FILE, handlePreferenceFileUpdate, systemPreferences);
        
        usleep(100000);

        RKLog("Setting a waveform ...\n");
        RKExecuteCommand(myRadar, "t w ofm", NULL);
        //RKExecuteCommand(myRadar, "t w q02", NULL);
        //RKExecuteCommand(myRadar, "t w q10", NULL);
        //RKExecuteCommand(myRadar, "t w s01", NULL);
        //RKExecuteCommand(myRadar, "t w barker03", NULL);
        //RKExecuteCommand(myRadar, "t w h2007.5", NULL);
        //RKExecuteCommand(myRadar, "t w h0507", NULL);
        //RKSetWaveformToImpulse(myRadar);

        RKLog("Starting a new PPI ...\n");
        RKExecuteCommand(myRadar, "p ppi 4 45", NULL);
        RKWaitWhileActive(myRadar);
    
        RKFileMonitorFree(preferenceFileMonitor);

    } else if (systemPreferences->desc.initFlags & RKInitFlagRelay) {

        RKRadarRelaySetHost(myRadar->radarRelay, systemPreferences->relayHost);
        RKSetRecordingLevel(myRadar, 0);

        // Assembly a string that describes streams
        if (strlen(systemPreferences->streams)) {
            RKRadarRelayUpdateStreams(myRadar->radarRelay, RKStreamFromString(systemPreferences->streams));
        }
        
        // Radar going live, then wait indefinitely until something happens
        RKGoLive(myRadar);
        RKWaitWhileActive(myRadar);
        
    } else {
        
        RKLog("Error. This should not happen.");
        
    }

    RKCommandCenterRemoveRadar(center, myRadar);
    RKCommandCenterStop(center);
    RKCommandCenterFree(center);
    
    RKFree(myRadar);

    userParametersFree(systemPreferences);

    return 0;
}
