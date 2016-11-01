//
//  main.c
//  RadarKitTest
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit.h>
#include <getopt.h>

RKRadar *radar = NULL;

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

#define CLEAR                       "\033[0m"
#define UNDERLINE(x)                "\033[4m" x "\033[24m"
#define PROGNAME                    "radartest"

void showHelp() {
    printf("RadarKit Test Program\n\n"
           PROGNAME " [options]\n\n"
           "OPTIONS\n"
           "     Unless specifically stated, all options are interpreted in sequence. Some\n"
           "     options can be specified multiples times for repetitions. For example, the\n"
           "     debris particle count is set for each type sequentially by repeating the\n"
           "     option multiple times for each debris type.\n"
           "\n"
           "  -c (--cpu) " UNDERLINE("P, M") "\n"
           "         Sets the number of threads for pulse compression to " UNDERLINE("P") "\n"
           "         and the number of threads for moment calculation to " UNDERLINE("M") ".\n"
           "         If not specified, the default core counts are 8 / 4.\n"
           "\n"
           "  -f (--prf) " UNDERLINE("value") "\n"
           "         Sets the pulse repetition frequency (PRF) to " UNDERLINE("value") " in Hz.\n"
           "         If not specified, the default PRF is 5000 Hz.\n"
           "\n"
           "  -h (--help)\n"
           "         Shows this help text.\n"
           "\n"
           "  -S (--test-mod)\n"
           "         Sets the program to test modulo macros.\n"
           "\n"
           "  -s (--simulate)\n"
           "         Sets the program to simulate data stream.\n"
           "\n"
           "  -S (--test-simd)\n"
           "         Sets the program to test SIMD instructions.\n"
           "         To test the SIMD performance, use --test-simd=2\n"
           "\n"
           );
}

typedef struct user_params {
    int   threadsPulseCompression;
    int   threadsMoment;
    int   prf;
    int   verbose;
    int   testSIMD;
    int   testModuloMath;
    bool  quietMode;
    bool  simulate;
} UserParams;

UserParams processInput(int argc, char **argv) {
    int k;
    
    // A structure unit that encapsulates command line user parameters
    UserParams user;
    memset(&user, 0, sizeof(UserParams));
    
    static struct option long_options[] = {
        {"alarm"         , no_argument      , 0, 'A'}, // ASCII 65 - 90 : A - Z
        {"test-mod"      , no_argument      , 0, 'M'},
        {"test-simd"     , optional_argument, 0, 'S'},
        {"azimuth"       , required_argument, 0, 'a'}, // ASCII 97 - 122 : a - z
        {"cpu"           , required_argument, 0, 'c'},
        {"prf"           , required_argument, 0, 'f'},
        {"help"          , no_argument      , 0, 'h'},
        {"sim"           , no_argument      , 0, 's'},
        {"verbose"       , no_argument      , 0, 'v'},
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
                sscanf(optarg, "%d,%d", &user.threadsPulseCompression, &user.threadsMoment);
                //printf("core counts = %d / %d\n", user.threadsPulseCompression, user.threadsMoment);
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

    RKSetProgramName("iRadar");
    RKSetWantScreenOutput(true);

    // SIMD Tests
    if (user.testSIMD) {
        RKSIMDDemoFlag flag = RKSIMDDemoFlagNull;
        if (user.verbose) {
            flag |= RKSIMDDemoFlagShowNumbers;
        }
        if (user.testSIMD > 1) {
            flag |= RKSIMDDemoFlagPerformanceTestAll;
        }
        RKSIMDDemo(flag);
    }
  
    // Modulo Macros Tests
    if (user.testModuloMath) {
        RKTestModuloMath();
    }
    
    // In the case when no tests are performed, simulate the time-series
    if (user.simulate == false) {
        if (user.testSIMD == false && user.testModuloMath == false) {
            user.simulate = true;
        } else {
            return EXIT_SUCCESS;
        }
    }

    RKLog("Initializing ...\n");

    radar = RKInit();

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);

    RKLog("Radar state machine occupies %s B (%s GiB)\n", RKIntegerToCommaStyleString(radar->memoryUsage), RKFloatToCommaStyleString(1.0e-9f * radar->memoryUsage));

    // Set any parameters here:
    RKSetProcessingCoreCounts(radar, user.threadsPulseCompression, user.threadsMoment);

    // Go live
    RKGoLive(radar);

//    pulseCompressionTest(radar, RKTestFlagShowResults);

    if (user.simulate) {
        RKTestSimulateDataStream(radar, user.prf);
    }

    RKLog("Freeing radar ...\n");
    RKFree(radar);

    return 0;
}
