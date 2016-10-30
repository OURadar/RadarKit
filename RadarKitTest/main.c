//
//  main.c
//  RadarKitTest
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit.h>

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



int main(int argc, const char * argv[]) {

    RKSetProgramName("iRadar");
    RKSetWantScreenOutput(true);

//    RKSIMDDemo(RKSIMDDemoFlagPerformanceTestAll);
//    RKSIMDDemo(RKSIMDDemoFlagShowNumbers);
//    RKTestModuloMath();

    RKLog("Initializing ...\n");

    radar = RKInit();

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
    signal(SIGQUIT, handleSignals);

    RKLog("Radar state machine occupies %s B (%s GiB)\n", RKIntegerToCommaStyleString(radar->memoryUsage), RKFloatToCommaStyleString(1.0e-9f * radar->memoryUsage));

    // Set any parameters here:
    //RKSetWaveformToImpulse(radar);
    RKSetProcessingCoreCounts(radar, 4, 0);
    
    // Go live
    RKGoLive(radar);

//    pulseCompressionTest(radar, RKTestFlagShowResults);
    RKTestSimulateDataStream(radar, 1000);

    RKLog("Freeing radar ...\n");
    RKFree(radar);

    return 0;
}
