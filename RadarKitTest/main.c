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
#define PROGNAME                    "rktest"

// User parameters in a struct
typedef struct user_params {
    int   coresForPulseCompression;
    int   coresForProductGenerator;
    int   prf;
    int   fs;
    int   verbose;
    int   testSIMD;
    int   testModuloMath;
    int   testPulseCompression;
    bool  noColor;
    bool  quietMode;
    bool  simulate;
} UserParams;

// Global variables
RKRadar *radar = NULL;

// Functions
void *exitAfterAWhile(void *s) {
    sleep(1);
    RKLog("Forced exit.\n");
    exit(EXIT_SUCCESS);
}

static void handleSignals(int signal) {
    if (radar == NULL) {
        return;
    }
    fprintf(stderr, "\n");
    RKLog("Caught a %s (%d)  radar->state = 0x%x.\n", RKSignalString(signal), signal, radar->state);
    RKStop(radar);
    pthread_t t;
    pthread_create(&t, NULL, exitAfterAWhile, NULL);
}

void showHelp() {
    printf("RadarKit Test Program\n\n"
           PROGNAME " [options]\n\n"
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
        {"test-mod"              , no_argument      , NULL, 'M'},
        {"test-simd"             , optional_argument, NULL, 'S'},
        {"test-pulse-compression", optional_argument, NULL, 'T'},
        {"azimuth"               , required_argument, NULL, 'a'}, // ASCII 97 - 122 : a - z
        {"core"                  , required_argument, NULL, 'c'},
        {"prf"                   , required_argument, NULL, 'f'},
        {"help"                  , no_argument      , NULL, 'h'},
        {"sim"                   , no_argument      , NULL, 's'},
        {"verbose"               , no_argument      , NULL, 'v'},
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
            case 'c':
                sscanf(optarg, "%d,%d", &user.coresForPulseCompression, &user.coresForProductGenerator);
                break;
            case 'f':
                user.prf = atoi(optarg);
                break;
            case 'h':
                showHelp();
                exit(EXIT_SUCCESS);
                break;
            case 's':
                user.simulate = true;
                break;
            case 'A':
                user.quietMode = false;
                break;
            case 'C':
                user.noColor = true;
                break;
            case 'D':
                user.fs = 5.0e6;
                user.prf = 6;
                user.coresForPulseCompression = 2;
                user.coresForProductGenerator = 2;
                user.verbose = 3;
                break;
            case 'F':
                user.fs = atof(optarg);
                break;
            case 'L':
                user.fs = 5.0e6;
                user.prf = 2000;
                user.coresForPulseCompression = 2;
                user.coresForProductGenerator = 2;
                user.verbose = 1;
                break;
            case 'M':
                if (optarg) {
                    user.testModuloMath = atoi(optarg);
                } else {
                    user.testModuloMath = 1;
                }
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
            case 'v':
                user.verbose++;
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

    RKSetProgramName("RadarKitTest");
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

    if (user.fs <= 5.0e6) {
        radar = RKInitLean();
    } else if (user.fs <= 20.0e6) {
        radar = RKInitMean();
    } else {
        radar = RKInit();
    }
    if (radar == NULL) {
        exit(EXIT_FAILURE);
    }

    RKSetVerbose(radar, user.verbose);
    
    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);

    // Set any parameters here:
    RKSetProcessingCoreCounts(radar, user.coresForPulseCompression, user.coresForProductGenerator);

    if (user.simulate) {

        // Build a series of options
        char cmd[64] = "";
        int i = 0;
        if (user.prf) {
            i += sprintf(cmd + i, " f %d", user.prf);
        }
        if (user.fs) {
            i += sprintf(cmd + i, " F %d", user.fs);
        }
        if (!user.quietMode) {
            RKLog("Test input = '%s'", cmd + 1);
        }
        // Now we use the frame work.
        RKSetTransceiver(radar, &RKTestSimulateDataStream, cmd);
        RKGoLive(radar);
        RKWaitWhileActive(radar);

    } else if (user.testPulseCompression) {

        RKLog("Testing pulse compression ...");
        RKGoLive(radar);
        RKTestPulseCompression(radar, RKTestFlagShowResults);

    }

    RKFree(radar);

    return 0;
}
