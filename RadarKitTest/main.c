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

// User parameters in a struct
typedef struct user_params {
    int   coresForPulseCompression;
    int   coresForProductGenerator;
    int   prf;
    int   fs;
    int   g;
    int   verbose;
    int   testSIMD;
    int   testModuloMath;
    int   testPulseCompression;
    int   sleepInterval;
    bool  noColor;
    bool  quietMode;
    bool  developerMode;
    bool  simulate;
} UserParams;

// Global variables
RKRadar *myRadar = NULL;

// Functions
void *exitAfterAWhile(void *s) {
    sleep(10);
    RKLog("Forced exit.\n");
    exit(EXIT_SUCCESS);
}

static void handleSignals(int signal) {
    if (myRadar == NULL) {
        return;
    }
    fprintf(stderr, "\n");
    RKLog("Caught a %s (%d)  radar->state = 0x%x.\n", RKSignalString(signal), signal, myRadar->state);
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
           "  -L (--test-lean-system)\n"
           "         Run with arguments '-v -f 2000 -F 50e6 -c 2,2'.\n"
           "\n"
           "  -s (--simulate)\n"
           "         Sets the program to simulate data stream (default, if none of the tests\n"
           "         is specified).\n"
           "\n"
           "  --test-mod\n"
           "         Sets the program to test modulo macros.\n"
           "\n"
           "  --test-simd\n"
           "         Sets the program to test SIMD instructions.\n"
           "         To test the SIMD performance, use --test-simd=2\n"
           "\n"
           "  --test-pulse-compression\n"
           "         Sets the program to test the pulse compression using a simple case with.\n"
           "         an impulse filter.\n"
           "\n"
           "\n"
           "EXAMPLES:\n"
           "     Here are some examples of typical configurations.\n"
           "\n"
           "  radar\n"
           "         Runs the program with every default, i.e., simulate a data stream with\n"
           "         default PRF (5000 Hz), default core counts (8 compression, 4 products)\n"
           "\n"
           "  radar -f 2000\n"
           "         Runs the program with PRF = 2000 Hz.\n"
           "\n"
           );
}

UserParams processInput(int argc, char **argv) {
    int k;
    
    // A structure unit that encapsulates command line user parameters
    UserParams user;
    memset(&user, 0, sizeof(UserParams));
    
    static struct option long_options[] = {
        {"alarm"                 , no_argument      , NULL, 'A'}, // ASCII 65 - 90 : A - Z
        {"no-color"              , no_argument      , NULL, 'C'},
        {"debug-demo"            , no_argument      , NULL, 'D'},
        {"fs"                    , required_argument, NULL, 'F'},
        {"lean-system-test"      , no_argument      , NULL, 'L'},
        {"medium-system-test"    , no_argument      , NULL, 'M'},
        {"test-simd"             , optional_argument, NULL, 'S'},
        {"test-pulse-compression", optional_argument, NULL, 'T'},
        {"azimuth"               , required_argument, NULL, 'a'}, // ASCII 97 - 122 : a - z
        {"core"                  , required_argument, NULL, 'c'},
        {"prf"                   , required_argument, NULL, 'f'},
        {"gate"                  , required_argument, NULL, 'g'},
        {"help"                  , no_argument      , NULL, 'h'},
        {"test-mod"              , no_argument      , NULL, 'm'},
        {"sim"                   , no_argument      , NULL, 's'},
        {"verbose"               , no_argument      , NULL, 'v'},
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
    while ((opt = getopt_long(argc, argv, str, long_options, &long_index)) != -1) {
        switch (opt) {
            case 'A':
                user.quietMode = false;
                break;
            case 'C':
                user.noColor = true;
                break;
            case 'D':
                user.simulate = true;
                user.fs = 5.0e6;
                user.prf = 6;
                user.coresForPulseCompression = 2;
                user.coresForProductGenerator = 2;
                user.developerMode = true;
                break;
            case 'F':
                user.fs = atof(optarg);
                break;
            case 'L':
                user.simulate = true;
                user.fs = 5.0e6;
                user.prf = 2000;
                user.coresForPulseCompression = 2;
                user.coresForProductGenerator = 2;
                break;
            case 'M':
                user.simulate = true;
                user.fs = 20.0e6;
                user.prf = 5000;
                user.coresForPulseCompression = 2;
                user.coresForProductGenerator = 2;
                break;
            case 'S':
                if (optarg) {
                    user.testSIMD = atoi(optarg);
                } else {
                    user.testSIMD = 1;
                }
                break;
            case 'T':
                if (optarg) {
                    user.testPulseCompression = atoi(optarg);
                } else {
                    user.testPulseCompression = 1;
                }
                break;
            case 'c':
                sscanf(optarg, "%d,%d", &user.coresForPulseCompression, &user.coresForProductGenerator);
                break;
            case 'f':
                user.prf = (int)atof(optarg);
                break;
            case 'g':
                user.g = (int)atof(optarg);
                break;
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
                break;
            case 'm':
                if (optarg) {
                    user.testModuloMath = atoi(optarg);
                } else {
                    user.testModuloMath = 1;
                }
                break;
            case 's':
                user.simulate = true;
                break;
            case 'v':
                user.verbose++;
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
    
    return user;
}

//
//
//  M A I N
//
//
int main(int argc, char *argv[]) {

    UserParams user = processInput(argc, argv);

    RKSetProgramName("iRadar");
    RKSetWantScreenOutput(true);
    RKSetWantColor(!user.noColor);

    // SIMD Tests
    bool testAny = false;
    if (user.testSIMD) {
        testAny = true;
        RKTestSIMDFlag flag = RKTestSIMDFlagNull;
        if (user.verbose) {
            flag |= RKTestSIMDFlagShowNumbers;
        }
        if (user.testSIMD > 1) {
            flag |= RKTestSIMDFlagPerformanceTestAll;
        }
        RKTestSIMD(flag);
    }
  
    // Modulo Macros Tests
    if (user.testModuloMath) {
        testAny = true;
        RKTestModuloMath();
    }

    // Pulse Compression Tests
    if (user.testPulseCompression) {
        testAny = true;
    }

    // In the case when no tests are performed, simulate the time-series
    if (user.simulate == false && testAny == false) {
        user.simulate = true;
    }

    if (user.fs <= 10.0e6) {
        myRadar = RKInitLean();
    } else if (user.fs <= 25.0e6) {
        myRadar = RKInitMean();
    } else {
        myRadar = RKInit();
    }
    if (myRadar == NULL) {
        RKLog("Error. Could not allocate radar.\n");
        exit(EXIT_FAILURE);
    }

    RKSetVerbose(myRadar, user.verbose);
    
    if (user.developerMode) {
        RKSetDeveloperMode(myRadar);
    }
    
    RKCommandCenter *center = RKCommandCenterInit();
    RKCommandCenterSetVerbose(center, user.verbose);
    RKCommandCenterAddRadar(center, myRadar);
    RKCommandCenterStart(center);
    
    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);

    // Set any parameters here:
    RKSetProcessingCoreCounts(myRadar, user.coresForPulseCompression, user.coresForProductGenerator);

    if (user.simulate) {

        // Now we use the frame work.

        // Build a series of options for transceiver, only pass down the relevant parameters
        int i = 0;
        char cmd[64] = "";
        if (user.prf) {
            i += sprintf(cmd + i, " f %d", user.prf);
        }
        if (user.g) {
            i += sprintf(cmd + i, " g %d", user.g);
        }
        if (user.fs) {
            i += sprintf(cmd + i, " F %d", user.fs);
        }
        if (user.sleepInterval) {
            i += sprintf(cmd + i, " z %d", user.sleepInterval);
        }
        if (!user.quietMode) {
            RKLog("Transceiver input = '%s' (%d / %d)", cmd + 1, i, RKMaximumStringLength);
        }
        RKSetTransceiver(myRadar, &RKTestSimulateDataStream, cmd);

        // Build a series of options for pedestal
        const char pedzyHost[] = "localhost:9000";
        if (!user.quietMode) {
            RKLog("Pedestal input = '%s'", pedzyHost);
        }
        RKSetPedestal(myRadar, &RKPedestalPedzyInit, (void *)pedzyHost);
        RKSetPedestalExec(myRadar, &RKPedestalPedzyExec);
        RKSetPedestalFree(myRadar, &RKPedestalPedzyFree);
        RKGoLive(myRadar);
        RKWaitWhileActive(myRadar);
        RKStop(myRadar);

    } else if (user.testPulseCompression) {

        RKLog("Testing pulse compression ...");
        RKGoLive(myRadar);
        RKTestPulseCompression(myRadar, RKTestFlagShowResults);

    }
    
    RKCommandCenterRemoveRadar(center, myRadar);
    RKCommandCenterFree(center);

    RKFree(myRadar);

    return 0;
}
