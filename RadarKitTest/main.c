//
//  main.c
//  RadarKitTest
//
//  Created by Boon Leng Cheong
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit.h>
#include <getopt.h>

#define CLEAR                       "\033[0m"
#define UNDERLINE(x)                "\033[4m" x "\033[24m"
#define ROOT_PATH                   "data"

// User parameters in a struct
typedef struct user_params {
    int            coresForPulseCompression;
    int            coresForProductGenerator;
    int            prf;
    int            sprt;
    int            gateCount;                   // Number of gates (simulate mode)
    int            verbose;
    int            testPulseCompression;
    int            sleepInterval;
    bool           simulate;
    bool           writeFiles;
    char           pedzyHost[256];
    char           tweetaHost[256];
    char           relayHost[256];
    RKRadarDesc    desc;
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

void showHelp() {
    printf("RadarKit Test Program\n\n"
           "rktest [options]\n\n"
           "OPTIONS:\n"
           "     Unless specifically stated, all options are interpreted in sequence. Some\n"
           "     options can be specified multiples times for repetitions. For example, the\n"
           "     debris particle count is set for each type sequentially by repeating the\n"
           "     option multiple times for each debris type.\n"
           "\n"
           "  -c (--core) " UNDERLINE("P,M") " (no space after comma)\n"
           "         Sets the number of threads for pulse compression to " UNDERLINE("P") "\n"
           "         and the number of threads for product generator to " UNDERLINE("M") ".\n"
           "         If not specified, the default core counts are 8 / 4.\n"
           "\n"
           "  -C (--show-clocks)\n"
           "         Shows the clock assignment for positions and pulses. This mode can be\n"
           "         used to check if the timing of position and pulses are set properly."
           "\n"
           "  -d (--no-decor)\n"
           "         Removes decoration of text. No color / underline. This should be set for\n"
           "         terminals that do not support color output through escape sequence.\n"
           "\n"
           "  -f (--prf) " UNDERLINE("value") "\n"
           "         Sets the pulse repetition frequency (PRF) to " UNDERLINE("value") " in Hz.\n"
           "         If not specified, the default PRF is 5000 Hz.\n"
           "\n"
           "  -g (--gate) " UNDERLINE("value") "\n"
           "         Sets the number range gates to " UNDERLINE("value") ".\n"
           "         If not specified, the default PRF is 8192 Hz.\n"
           "\n"
           "  -h (--help)\n"
           "         Shows this help text.\n"
           "\n"
           "  -L (--lean-system)\n"
           "         Run with arguments '-v -f 2000 -F 5e6 -c 2,2'.\n"
           "\n"
           "  -M (--medium-system)\n"
           "         Run with arguments '-v -f 5000 -F 20e6 -c 4,2'.\n"
           "\n"
           "  -p (--pedzy-host)\n"
           "         Sets the host of pedzy pedestal controller.\n"
           "\n"
           "  -t (--tweeta-host)\n"
           "         Sets the host of tweeta health relay.\n"
           "\n"
           "  -s (--simulate)\n"
           "         Sets the program to simulate data stream.\n"
           "\n"
           "  -v (--verbose)\n"
           "         Increases verbosity level, which can be specified multiple times.\n"
           "\n"
           "  -T (--test) " UNDERLINE("value") "\n"
           "          0 - Show types\n"
           "          1 - Show colors\n"
           "          2 - Show modulo math\n"
           "          3 - Parsing comma delimited values\n"
           "          4 - Parsing values in a JSON string\n"
           "          5 - Initializing a File Manager\n"
           "          6 - Reading preference file\n"
           "          7 - Count files in a folder RKCountFilesInPath()\n"
           "          8 - File monitor\n"
           "         11 - Initializing a radar system\n"
           "\n"
           "         20 - SIMD quick test\n"
           "         21 - SIMD test with numbers shown\n"
           "         22 - Show window types\n"
           "         23 - Hilbert transform\n"
           "\n"
           "         30 - Make a frequency hopping sequence\n"
           "         31 - Make a TFM waveform\n"
           "         32 - Write a waveform file\n"
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
           "     Here are some examples of typical configurations.\n"
           "\n"
           "  rktest -vL\n"
           "         Runs the program with default settings for a lean system and verbose.\n"
           "\n"
           "  radar -L -f 2000\n"
           "         Same as above but with PRF = 2000 Hz.\n"
           "\n"
           "  radar -T 50\n"
           "         Runs the program to measure SIMD performance.\n"
           "\n"
           );
}

UserParams processInput(int argc, const char **argv) {
    int k;
    
    // A structure unit that encapsulates command line user parameters
    UserParams user;

    // Zero out everything and set some default parameters
	memset(&user, 0, sizeof(UserParams));
    user.gateCount = 2000;
    user.coresForPulseCompression = 2;
    user.coresForProductGenerator = 2;

    // Build a RKRadar initialization description
    user.desc.initFlags = RKInitFlagAllocEverything;
    user.desc.pulseBufferDepth = 8000;
    user.desc.rayBufferDepth = 1440;
    user.desc.latitude = 35.181251;
    user.desc.longitude = -97.436752;
    user.desc.radarHeight = 2.5f;
    user.desc.wavelength = 0.03f;
    strcpy(user.desc.dataPath, ROOT_PATH);

    static struct option long_options[] = {
        {"alarm"                 , no_argument      , NULL, 'A'}, // ASCII 65 - 90 : A - Z
        {"clock"                 , no_argument      , NULL, 'C'},
        {"demo"                  , no_argument      , NULL, 'D'},
        {"fs"                    , required_argument, NULL, 'F'},
        {"hp-system"             , no_argument      , NULL, 'H'},
        {"lean-system"           , no_argument      , NULL, 'L'},
        {"medium-system"         , no_argument      , NULL, 'M'},
        {"test"                  , required_argument, NULL, 'T'},
        {"azimuth"               , required_argument, NULL, 'a'}, // ASCII 97 - 122 : a - z
        {"bandwidth"             , required_argument, NULL, 'b'},
        {"core"                  , required_argument, NULL, 'c'},
        {"no-decor"              , no_argument      , NULL, 'd'},
        {"prf"                   , required_argument, NULL, 'f'},
        {"gate"                  , required_argument, NULL, 'g'},
        {"help"                  , no_argument      , NULL, 'h'},
        {"pedzy-host"            , required_argument, NULL, 'p'},
        {"quiet"                 , no_argument      , NULL, 'q'},
        {"relay"                 , required_argument, NULL, 'r'},
        {"sim"                   , no_argument      , NULL, 's'},
        {"tweeta-host"           , required_argument, NULL, 't'},
        {"verbose"               , no_argument      , NULL, 'v'},
        {"do-not-write"          , no_argument      , NULL, 'w'},
        {"simulate-sleep"        , required_argument, NULL, 'z'},
        {0, 0, 0, 0}
    };
    
    // Construct short_options from long_options
    char str[1024] = "";
    for (k = 0; k < sizeof(long_options) / sizeof(struct option); k++) {
        struct option *o = &long_options[k];
        snprintf(str + strlen(str), 1024, "%c%s", o->val, o->has_arg == required_argument ? ":" : (o->has_arg == optional_argument ? "::" : ""));
    }
    //printf("str = %s\n", str);
    // Process the input arguments and set the parameters
    int opt, long_index = 0;
    while ((opt = getopt_long(argc, (char * const *)argv, str, long_options, &long_index)) != -1) {
        switch (opt) {
            case 'C':
                user.desc.initFlags |= RKInitFlagShowClockOffset;
                break;
            case 'D':
                user.simulate = true;
                user.gateCount = 2000;
                user.prf = 6;
                user.coresForPulseCompression = 2;
                user.coresForProductGenerator = 2;
                break;
            case 'b':
            case 'H':
                user.simulate = true;
                user.gateCount = 60000;
                user.prf = 5000;
                user.coresForPulseCompression = 10;
                user.coresForProductGenerator = 4;
                break;
            case 'L':
                user.simulate = true;
                user.gateCount = 2000;
                user.prf = 2000;
                user.coresForPulseCompression = 2;
                user.coresForProductGenerator = 2;
                break;
            case 'M':
                user.simulate = true;
                user.gateCount = 16000;
                user.prf = 2000;
                user.coresForPulseCompression = 6;
                user.coresForProductGenerator = 4;
                break;
            case 'T':
                RKSetWantScreenOutput(true);
                k = atoi(optarg);
                switch (k) {
                    case 0:
                        RKShowTypeSizes();
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
                    case 11:
                        myRadar = RKInitLean();
                        RKShowOffsets(myRadar);
                        RKFree(myRadar);
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
                    default:
                        RKLog("Test %d is invalid.\n", k);
                        break;
                }
                exit(EXIT_SUCCESS);
                break;
            case 'U':
                exit(EXIT_FAILURE);
                break;
            case 'P':
                if (optarg) {
                    user.testPulseCompression = atoi(optarg);
                } else {
                    user.testPulseCompression = 1;
                }
                break;
            case 'c':
                sscanf(optarg, "%d,%d", &user.coresForPulseCompression, &user.coresForProductGenerator);
                break;
            case 'd':
                RKSetWantColor(false);
                break;
            case 'f':
                k = sscanf(optarg, "%d,%d", &user.prf, &user.sprt);
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
            case 'p':
                strncpy(user.pedzyHost, optarg, sizeof(user.pedzyHost));
                break;
            case 'q':
                user.verbose = MAX(user.verbose - 1, 0);
                break;
            case 'r':
				user.desc.initFlags = RKInitFlagRelay;
                strncpy(user.relayHost, optarg, sizeof(user.relayHost));
                break;
            case 's':
                user.simulate = true;
                break;
            case 't':
                strncpy(user.tweetaHost, optarg, sizeof(user.tweetaHost));
                break;
            case 'v':
                user.verbose++;
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
                exit(EXIT_FAILURE);
                break;
        }
    }
    if (user.prf > 0 && user.simulate == false) {
        RKLog("Warning. PRF has no effects without simulation.\n");
    }
    if (user.verbose == 1) {
        user.desc.initFlags |= RKInitFlagVerbose;
    } else if (user.verbose == 2) {
        user.desc.initFlags |= RKInitFlagVeryVerbose;
    } else if (user.verbose == 3) {
        user.desc.initFlags |= RKInitFlagVeryVeryVerbose;
    }
    if (user.gateCount >= 4000) {
        user.desc.pulseToRayRatio = ceilf((float)user.gateCount / 2000);
    } else {
        user.desc.pulseToRayRatio = 1;
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

    RKSetProgramName("rktest");
    RKSetWantScreenOutput(true);

    char *term = getenv("TERM");
    if (term == NULL || (strcasestr(term, "color") == NULL && strcasestr(term, "ansi") == NULL)) {
        RKSetWantColor(false);
    }
 
    UserParams user = processInput(argc, argv);

    if (user.verbose > 1) {
        printf("TERM = %s --> %s\n", term, rkGlobalParameters.showColor ? "showColor" : "noColor");
    }

    // In the case when no tests are performed, simulate the time-series
    if (user.simulate == false && !(user.desc.initFlags = RKInitFlagRelay)) {
        RKLog("No options specified. Don't want to do anything?\n");
        exit(EXIT_FAILURE);
    } else if (user.simulate == true && user.desc.initFlags == RKInitFlagRelay) {
        RKLog("Info. Simulate takes precedence over relay.\n");
		user.desc.initFlags = RKInitFlagAllocEverything;
		user.simulate = true;
    }

    // Screen output based on verbosity level
    if (!user.verbose) {
        RKSetWantScreenOutput(false);
    }

    // Initialize a radar object
    myRadar = RKInitWithDesc(user.desc);
    if (myRadar == NULL) {
        RKLog("Error. Could not allocate radar.\n");
        exit(EXIT_FAILURE);
    }

    RKSetVerbose(myRadar, user.verbose);
    //RKSetDataUsageLimit(myRadar, (size_t)20 * (1 << 30));
    RKSetMomentProcessorToMultiLag(myRadar, 3);

    RKAddControl(myRadar, "10us pulse", "t w s10");
    RKAddControl(myRadar, "20us pulse", "t w s20");
    RKAddControl(myRadar, "50us pulse", "t w s50");
    RKAddControl(myRadar, "10us 0.1-MHz tone", "t w t10");
    RKAddControl(myRadar, "20us 0.1-MHz tone", "t w t20");
    RKAddControl(myRadar, "50us 0.1-MHz tone", "t w t50");

    RKAddControl(myRadar, "PPI EL 8 deg @ 90 dps", "p ppi 7 90");
    RKAddControl(myRadar, "PPI EL 7 deg @ 45 dps", "p ppi 7 45");
    RKAddControl(myRadar, "PPI EL 6 deg @ 24 dps", "p ppi 6 24");
    RKAddControl(myRadar, "PPI EL 5 deg @ 12 dps", "p ppi 5 12");
    RKAddControl(myRadar, "PPI EL 4 deg @ 6 dps", "p ppi 4 6");
    RKAddControl(myRadar, "PPI EL 3 deg @ 1 dps", "p ppi 3 1");
    RKAddControl(myRadar, "RHI @ AZ 35 deg @ 25 dps", "p rhi 35 0,40 20");
    RKAddControl(myRadar, "Simulate Malfunction Pedestal", "p bad");

    RKCommandCenter *center = RKCommandCenterInit();
    RKCommandCenterSetVerbose(center, user.verbose);
    RKCommandCenterAddRadar(center, myRadar);

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);
    signal(SIGKILL, handleSignals);

    // Set any parameters here:
	RKSetProcessingCoreCounts(myRadar, user.coresForPulseCompression, user.coresForProductGenerator);
	if (!user.writeFiles) {
        RKSetDoNotWrite(myRadar, true);
    }

    if (user.simulate) {

        // Now we use the frame work.

        // Build a series of options for transceiver, only pass down the relevant parameters
        int i = 0;
        char cmd[64] = "";
        if (user.prf) {
            if (user.sprt > 1) {
                i += sprintf(cmd + i, " f %d,%d", user.prf, user.sprt);
            } else {
                i += sprintf(cmd + i, " f %d", user.prf);
            }
        }
        if (user.gateCount) {
            i += sprintf(cmd + i, " g %d", user.gateCount);
        }
        if (user.sleepInterval) {
            i += sprintf(cmd + i, " z %d", user.sleepInterval);
        }
        RKLog("Transceiver input = '%s' (%d / %s)", cmd + 1, i, RKIntegerToCommaStyleString(RKMaximumStringLength));
        RKSetTransceiver(myRadar,
                         (void *)cmd,
                         RKTestTransceiverInit,
                         RKTestTransceiverExec,
                         RKTestTransceiverFree);

        // Build a series of options for pedestal, only pass down the relevant parameters
        if (strlen(user.pedzyHost)) {
            RKLog("Pedestal input = '%s'", user.pedzyHost);
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
            RKLog("Health relay input = '%s'", user.tweetaHost);
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

        RKWaveform *waveform = NULL;
        const char wavfile[] = "waveforms/ofmd.rkwav";
        if (RKFilenameExists(wavfile)) {
            RKLog("Loading waveform from file '%s'...\n", wavfile);
            waveform = RKWaveformInitFromFile(wavfile);
            RKWaveformSummary(waveform);
            RKLog("Adjusting waveform for RX sampling rate ...\n");
            RKWaveformDownConvert(waveform, 2.0 * M_PI * 50.0 / 160.0);
            RKWaveformDecimate(waveform, 32);
            RKWaveformSummary(waveform);
        } else {
            RKLog("Generating waveform using built-in function ...\n");
            //waveform = RKWaveformInitAsTimeFrequencyMultiplexing(2.0, 1.0, 0.25, 100);
            waveform = RKWaveformInitAsLinearFrequencyModulation(5.0, 0.0, 10.0, 1.5);
            RKLog("Waveform LFM generated.\n");
        }
        RKSetWaveform(myRadar, waveform);
        RKWaveformFree(waveform);
        
        RKSweepEngineSetHandleFilesScript(myRadar->sweepEngine, "scripts/handlefiles.sh", true);

        // Radar going live, then wait indefinitely until something happens
        RKGoLive(myRadar);
        RKCommandCenterStart(center);
        RKWaitWhileActive(myRadar);
        RKStop(myRadar);

    } else if (user.desc.initFlags & RKInitFlagRelay) {

        RKRadarRelaySetHost(myRadar->radarRelay, user.relayHost);
        RKSetDoNotWrite(myRadar, true);

        // Radar going live, then wait indefinitely until something happens
        RKGoLive(myRadar);
        RKCommandCenterStart(center);
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
