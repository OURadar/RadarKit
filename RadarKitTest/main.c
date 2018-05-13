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
    int            verbose;                     // Verbosity
    int            coresForPulseCompression;    // Number of cores for pulse compression
    int            coresForProductGenerator;    // Number of cores for moment calculations
    float          fs;                          // Raw gate sampling bandwidth
    float          prf;                         // Base PRF (Hz)
    int            sprt;                        // Staggered PRT option (2 for 2:3, 3 for 3:4, etc.)
    int            gateCount;                   // Number of gates (simulate mode)
    int            sleepInterval;
    bool           simulate;
    bool           writeFiles;
    RKName         pedzyHost;
    RKName         tweetaHost;
    RKName         relayHost;
    RKName         streams;
    RKRadarDesc    desc;
    RKName         labels[256];
    RKName         commands[256];
    int            controlCount;
} UserParams;

// Global variables
RKRadar *myRadar = NULL;

// Functions
void *exitAfterAWhile(void *s) {
    sleep(3);
    RKLog("Forced exit.\n");
    exit(EXIT_SUCCESS);
}

static void handleSignals(int signal) {
    if (myRadar == NULL) {
        return;
    }
    fprintf(stderr, "\n");
    RKLog("Caught a %s (%d)  radar->state = 0x%x\n", RKSignalString(signal), signal, myRadar->state);
    RKStop(myRadar);
    pthread_t t;
    pthread_create(&t, NULL, exitAfterAWhile, NULL);
}

static void handlePreferenceFileUpdate(void *in) {
    RKFileMonitor *engine = (RKFileMonitor *)in;
    RKLog("%s The preference file has been updated.\n", engine->name);
}

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
           "  -r (--relay) " UNDERLINE("host") "[symbols]\n"
           "         Runs as a relay and connect to remote " UNDERLINE("host") "\n"
           "         If [symbols] are supplied, they will be requested.\n"
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
           "          2 - Show modulo math\n"
           "          3 - Test parsing comma delimited values\n"
           "          4 - Test parsing values in a JSON string\n"
           "          5 - Test initializing a File Manager\n"
           "          6 - Test reading a preference file\n"
           "          7 - Test counting files using RKCountFilesInPath()\n"
           "          8 - Test the file monitor module\n"
           "          9 - Test the internet monitor module\n"
           "         11 - Test initializing a radar system\n"
           "         12 - Test converting a temperature reading to status\n"
           "         13 - Test getting a country name from position\n"
           "         14 - Test reading a netcdf file\n"
           "\n"
           "         20 - SIMD quick test\n"
           "         21 - SIMD test with numbers shown\n"
           "         22 - Show window types\n"
           "         23 - Hilbert transform\n"
           "\n"
           "         30 - Make a frequency hopping sequence\n"
           "         31 - Make a TFM waveform\n"
           "         32 - Generate a waveform file\n"
           "         33 - Generate an fft-wisdom file\n"
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
           "    -v -s1 -L -f 2000\n"
           "         Same as level-1 system but with PRF = 2,000 Hz.\n"
           "\n"
           "    -T 50\n"
           "         Runs the program to measure SIMD performance.\n"
           "\n"
           );
}

static void setSystemLevel(UserParams *user, const int level) {
    switch (level) {
        case 0:
            // Debug
            user->fs = 5000000;
            user->gateCount = 30;
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

UserParams processInput(int argc, const char **argv) {
    int k;
    char *c;
    
    // A structure unit that encapsulates command line user parameters
    UserParams user;
    
    // Command line options
    struct option long_options[] = {
        {"alarm"             , no_argument      , NULL, 'A'},    // ASCII 65 - 90 : A - Z
        {"clock"             , no_argument      , NULL, 'C'},
        {"system"            , required_argument, NULL, 'S'},
        {"test"              , required_argument, NULL, 'T'},
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
        {"quiet"             , no_argument      , NULL, 'q'},
        {"relay"             , required_argument, NULL, 'r'},
        {"simulate"          , optional_argument, NULL, 's'},
        {"tweeta-host"       , required_argument, NULL, 't'},
        {"verbose"           , no_argument      , NULL, 'v'},
        {"do-not-write"      , no_argument      , NULL, 'w'},
        {"simulate-sleep"    , required_argument, NULL, 'z'},
        {0, 0, 0, 0}
    };
    
    // Construct short_options from long_options
    char str[1024] = "";
    for (k = 0; k < sizeof(long_options) / sizeof(struct option); k++) {
        struct option *o = &long_options[k];
        snprintf(str + strlen(str), 1023, "%c%s", o->val, o->has_arg == required_argument ? ":" : (o->has_arg == optional_argument ? "::" : ""));
    }
    #if defined(DEBUG)
    printf("str = %s\n", str);
    #endif

    // Zero out everything and set some default parameters
    memset(&user, 0, sizeof(UserParams));
    
    // Build a RKRadar initialization description
    user.desc.initFlags = RKInitFlagAllocEverything;
    user.desc.pulseBufferDepth = 2500;
    user.desc.rayBufferDepth = 1500;
    user.desc.latitude = 35.181251;
    user.desc.longitude = -97.436752;
    user.desc.radarHeight = 2.5f;
    user.desc.wavelength = 0.03f;
    user.desc.pulseToRayRatio = 1;
    strcpy(user.desc.dataPath, ROOT_PATH);
    
    // First pass: just check for verbosity level
    int opt, long_index = 0;
    while ((opt = getopt_long(argc, (char * const *)argv, str, long_options, &long_index)) != -1) {
        switch (opt) {
            case 'v':
                user.verbose++;
                break;
            default:
                break;
        }
    }
    
    // Going from user.verbose to RKInitFlag
    if (user.verbose == 1) {
        user.desc.initFlags |= RKInitFlagVerbose;
    } else if (user.verbose == 2) {
        user.desc.initFlags |= RKInitFlagVeryVerbose;
    } else if (user.verbose == 3) {
        user.desc.initFlags |= RKInitFlagVeryVeryVerbose;
    }
    
    // Read in preference configuration file
    RKPreference *userPreferences = RKPreferenceInitWithFile(PREFERENCE_FILE);
    RKPreferenceObject *object;

    // Only show the inner working if verbosity level > 1 (1 -> 0, 2+ -> 1)
    const int verb = user.verbose > 1 ? 1 : 0;
    if (verb) {
        RKLog("Reading user preferences ...\n");
    }
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Name",       user.desc.name,       RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "FilePrefix", user.desc.filePrefix, RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "DataPath",   user.desc.dataPath,   RKParameterTypeString, RKMaximumPathLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "PedzyHost",  user.pedzyHost,       RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "TweetaHost", user.tweetaHost,      RKParameterTypeString, RKNameLength);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Latitude",   &user.desc.latitude,  RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Longitude",  &user.desc.longitude, RKParameterTypeDouble, 1);
    RKPreferenceGetValueOfKeyword(userPreferences, verb, "Heading",    &user.desc.heading,   RKParameterTypeDouble, 1);   
    k = 0;
    while ((object = RKPreferenceFindKeyword(userPreferences, "Shortcut")) != NULL && k < 256) {
        RKParseQuotedStrings(object->valueString, user.labels[k], user.commands[k], NULL);
        k++;
    }
    user.controlCount = k;

    // Second pass: now we go through the rest of them (all of them except doing nothing for 'v')
    optind = 1;
    long_index = 0;
    while ((opt = getopt_long(argc, (char * const *)argv, str, long_options, &long_index)) != -1) {
        switch (opt) {
            case 'C':
                user.desc.initFlags |= RKInitFlagShowClockOffset;
                break;
            case 'S':
                k = atoi(optarg);
                setSystemLevel(&user, k);
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
                        RKTestShowColors();
                        break;
                    case 2:
                        RKTestModuloMath();
                        break;
                    case 3:
                        RKTestParseCommaDelimitedValues();
                        break;
                    case 4:
                        RKTestJSON();
                        break;
                    case 5:
                        RKTestFileManager();
                        break;
                    case 6:
                        RKTestPreferenceReading();
                        break;
                    case 7:
                        RKTestCountFiles();
                        break;
                    case 8:
                        RKTestFileMonitor();
                        break;
                    case 9:
                        RKTestHostMonitor();
                        break;
                    case 11:
                        myRadar = RKInitLean();
                        RKShowOffsets(myRadar, NULL);
                        RKFree(myRadar);
                        break;
                    case 12:
                        RKTestTemperatureToStatus();
                        break;
                    case 13:
                        RKTestGetCountry();
                        break;
                    case 14:
                        if (argc == optind) {
                            RKLog("No filename given.\n");
                            exit(EXIT_FAILURE);
                        }
                        RKTestReadSweep(argv[optind]);
                        break;
                    case 15:
                        RKTestWaveformProperties();
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
                    case 30:
                        RKTestMakeHops();
                        break;
                    case 31:
                        RKTestWaveformTFM();
                        break;
                    case 32:
                        RKTestWriteWaveform();
                        break;
                    case 33:
                        RKTestWriteFFTWisdom();
                        break;
                    case 40:
                        RKTestPulseCompression((user.verbose ? RKTestFlagVerbose : 0) | RKTestFlagShowResults);
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
                    case 54:
                        RKTestBufferOverview();
                        break;
                    default:
                        RKLog("Test %d is invalid.\n", k);
                        break;
                }
                exit(EXIT_SUCCESS);
                break;
            case 'b':
                user.fs = roundf(atof(optarg));
                break;
            case 'c':
                sscanf(optarg, "%d,%d", &user.coresForPulseCompression, &user.coresForProductGenerator);
                break;
            case 'd':
                user.desc.pulseToRayRatio = atoi(optarg);
                break;
            case 'e':
                RKSetWantColor(false);
                break;
            case 'f':
                k = sscanf(optarg, "%f,%d", &user.prf, &user.sprt);
                if (k < 2) {
                    user.sprt = 0;
                }
                break;
            case 'g':
                user.gateCount = atoi(optarg);
                break;
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
                break;
            case 'i':
                k = sscanf(optarg, "%f,%d", &user.prf, &user.sprt);
                user.prf = 1.0f / user.prf;
                if (k < 2) {
                    user.sprt = 0;
                }
                break;
            case 'm':
                break;
            case 'p':
                strncpy(user.pedzyHost, optarg, sizeof(user.pedzyHost));
                break;
            case 'q':
                user.verbose = MAX(user.verbose - 1, 0);
                break;
            case 'r':
                user.simulate = false;
                user.desc.initFlags = RKInitFlagRelay;
                if (argc > optind && argv[optind][0] != '-') {
                    // The next argument is not an option, interpret as streams
                    if (argv[optind][0] == 's') {
                        // Convert product to sweep: Z V W D P R K --> Y U X C O Q J
                        strcpy(user.streams, &argv[optind][1]);
                        c = user.streams;
                        while (c < user.streams + strlen(user.streams)) {
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
                        if (user.verbose) {
                            RKLog("Stream %s --> %s\n", argv[optind], user.streams);
                        }
                    } else {
                        strcpy(user.streams, argv[optind]);
                        if (user.verbose) {
                            RKLog("Stream %s\n", user.streams);
                        }
                    }
                    optind++;
                }
                strncpy(user.relayHost, optarg, sizeof(user.relayHost));
                break;
            case 's':
                if (strlen(user.relayHost)) {
                    RKLog("Relay and simulation? Perhaps no -s.\n");
                    exit(EXIT_FAILURE);
                }
                user.simulate = true;
                if (optarg) {
                    setSystemLevel(&user, atoi(optarg));
                } else {
                    setSystemLevel(&user, 1);
                }
                break;
            case 't':
                strncpy(user.tweetaHost, optarg, sizeof(user.tweetaHost));
                break;
            case 'v':
                break;
            case 'w':
                user.writeFiles = true;
                break;
            case 'z':
                if (optarg) {
                    user.sleepInterval = atoi(optarg);
                } else {
                    user.sleepInterval = 1000;
                }
                break;
            default:
                if (optarg && strlen(optarg)) {
                    fprintf(stderr, "I don't understand: -%c   optarg = %s\n", opt, optarg);
                } else {
                    fprintf(stderr, "I don't understand: -%c\n", opt);
                }
                exit(EXIT_FAILURE);
                break;
        }
    }
    if (user.simulate == true) {
        if (user.prf == 0) {
            user.prf = 1000;
        }
    } else {
        if (!(user.desc.initFlags == RKInitFlagRelay)) {
            RKLog("No options specified. Don't want to do anything?\n");
            exit(EXIT_FAILURE);
        }
        if (user.prf) {
            RKLog("Warning. PRF has no effects without simulation.\n");
        }
        if (user.gateCount == 0) {
            setSystemLevel(&user, 1);
        }
    }
    if (user.prf > 0) {
        k = user.fs / user.prf;
    } else {
        k = user.fs / 1000.0f;
    }
    if (user.gateCount > k) {
        RKLog("Info. Gate count adjusted: %s -> %s for PRF (%s kHz) and bandwidth (%s MHz)",
              RKIntegerToCommaStyleString(user.gateCount), RKIntegerToCommaStyleString(k),
              RKFloatToCommaStyleString(1.0e-3 * user.prf), RKFloatToCommaStyleString(1.0e-6 * user.fs));
        user.gateCount = k;
    }
    user.desc.pulseCapacity = 10 * ceil(0.1 * user.gateCount);
    
    return user;
}

//
//
//  M A I N
//
//

int main(int argc, const char **argv) {

    int k;

    RKSetProgramName("rktest");
    RKSetWantScreenOutput(true);

    char *term = getenv("TERM");
    if (term == NULL || (strcasestr(term, "color") == NULL && strcasestr(term, "ansi") == NULL)) {
        RKSetWantColor(false);
    }
 
    UserParams user = processInput(argc, argv);
    
    // Screen output based on verbosity level
    if (user.verbose) {
        RKLog("Level II recording: %s\n", user.writeFiles ? "true" : "false");
        if (user.verbose > 1) {
            printf("TERM = %s --> %s\n", term, rkGlobalParameters.showColor ? "showColor" : "noColor");
        }
    } else {
        RKSetWantScreenOutput(false);
    }

    // Initialize a radar object
    myRadar = RKInitWithDesc(user.desc);
    if (myRadar == NULL) {
        RKLog("Error. Could not allocate a radar.\n");
        exit(EXIT_FAILURE);
    }

    RKSetVerbose(myRadar, user.verbose);
    //RKSetDataUsageLimit(myRadar, (size_t)20 * (1 << 30));
    RKSetMomentProcessorToMultiLag(myRadar, 3);
    //RKSetMomentProcessorToPulsePairHop(myRadar);

    for (k = 0; k < user.controlCount; k++) {
        //printf("Adding control '%s' '%s' ...\n", user.labels[k], user.commands[k]);
        RKAddControl(myRadar, user.labels[k], user.commands[k]);
    }

    RKAddConfig(myRadar,
                RKConfigKeySystemZCal, -30.0f, -30.0f,
                RKConfigKeySystemDCal, 0.2f,
                RKConfigKeyZCal2, 20.0f, 20.0f,
                RKConfigKeyNoise, 0.1, 0.1,
                RKConfigKeyNull);
    
    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);
    signal(SIGKILL, handleSignals);
    
    // Set any parameters here:
    RKSetProcessingCoreCounts(myRadar, user.coresForPulseCompression, user.coresForProductGenerator);
    if (!user.writeFiles) {
        RKSetDoNotWrite(myRadar, true);
    }
    
    // Make a command center and add the radar to it
    RKCommandCenter *center = RKCommandCenterInit();
    RKCommandCenterSetVerbose(center, user.verbose);
    RKCommandCenterStart(center);
    RKCommandCenterAddRadar(center, myRadar);

    int i;
    RKName cmd = "";
    
    if (user.simulate) {

        // Now we use the frame work.
        // Build a series of options for transceiver, only pass down the relevant parameters
        i = 0;
        if (user.fs) {
            i += sprintf(cmd + i, " F %.0f", user.fs);
        }
        if (user.prf) {
            if (user.sprt > 1) {
                i += sprintf(cmd + i, " f %.0f,%d", user.prf, user.sprt);
            } else {
                i += sprintf(cmd + i, " f %.0f", user.prf);
            }
        }
        if (user.gateCount) {
            i += sprintf(cmd + i, " g %d", user.gateCount);
        }
        if (user.sleepInterval) {
            i += sprintf(cmd + i, " z %d", user.sleepInterval);
        }
        if (user.verbose > 1) {
            RKLog("Transceiver input = '%s' (%d / %s)", cmd + 1, i, RKIntegerToCommaStyleString(RKMaximumStringLength));
        }
        RKSetTransceiver(myRadar,
                         (void *)cmd,
                         RKTestTransceiverInit,
                         RKTestTransceiverExec,
                         RKTestTransceiverFree);

        // Build a series of options for pedestal, only pass down the relevant parameters
        if (strlen(user.pedzyHost)) {
            if (user.verbose > 1) {
                RKLog("Pedestal input = '%s'", user.pedzyHost);
            }
            RKSetPedestal(myRadar,
                          (void *)user.pedzyHost,
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
        
        if (strlen(user.tweetaHost)) {
            if (user.verbose > 1) {
                RKLog("Health relay input = '%s'", user.tweetaHost);
            }
            RKSetHealthRelay(myRadar,
                             (void *)user.tweetaHost,
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

        myRadar->configs[0].prf[0] = user.prf;

        RKSweepEngineSetHandleFilesScript(myRadar->sweepEngine, "scripts/handlefiles.sh", true);
        //RKUserProductDesc desc;
        //RKSweepEngineRegisterProduct(myRadar->sweepEngine, RKUserProductDesc)

        // Radar going live, then wait indefinitely until something happens
        RKGoLive(myRadar);

        RKFileMonitor *preferenceFileMonitor = RKFileMonitorInit(PREFERENCE_FILE, handlePreferenceFileUpdate);
        
        usleep(1000000);
        RKLog("Starting a new PPI ...\n");
        RKExecuteCommand(myRadar, "p ppi 4 45", NULL);
        RKWaitWhileActive(myRadar);
        RKStop(myRadar);

        RKFileMonitorFree(preferenceFileMonitor);

    } else if (user.desc.initFlags & RKInitFlagRelay) {

        RKRadarRelaySetHost(myRadar->radarRelay, user.relayHost);
        RKSetDoNotWrite(myRadar, true);

        // Assembly a string that describes streams
        if (strlen(user.streams)) {
            RKRadarRelayUpdateStreams(myRadar->radarRelay, RKStreamFromString(user.streams));
        }
        
        // Radar going live, then wait indefinitely until something happens
        RKGoLive(myRadar);
        RKWaitWhileActive(myRadar);
        RKStop(myRadar);
        
    } else {
        
        RKLog("Error. This should not happen.");
        
    }

    
    RKCommandCenterRemoveRadar(center, myRadar);
    RKCommandCenterStop(center);
    RKCommandCenterFree(center);
    
    RKFree(myRadar);
    
    return 0;
}
