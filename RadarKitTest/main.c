//
//  main.c
//  RadarKitTest
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit.h>
#include <signal.h>

RKRadar *radar;

void *exitAfterAWhile(void *s) {
    sleep(1);
    RKLog("Forced exit.\n");
    exit(EXIT_SUCCESS);
}

static void handleSignals(int signal) {
    RKLog("User interrupt.\n");
    radar->active = false;
    pthread_t t;
    pthread_create(&t, NULL, exitAfterAWhile, NULL);
}

int main(int argc, const char * argv[]) {
    // insert code here...
    radar = RKInit();
    RKSetProgramName("iRadar");
    rkGlobalParameters.stream = stdout;

    RKLog("Radar state machine occupies %s bytes\n", RKIntegerToCommaStyleString(radar->memoryUsage));

    // Catch Ctrl-C and exit gracefully
    signal(SIGINT, handleSignals);
//    signal(SIGKILL, handleSignals);
//    signal(SIGTERM, handleSignals);
//    signal(SIGQUIT, handleSignals);

//    const int N = 4;
//    int i = 0;
//
//    i = 0;                      RKLog("i = %d --> Next N = %d\n", i, RKNextNBuffer0Slot(i, N));
//    i = RKBuffer0SlotCount - 4; RKLog("i = %d --> Next N = %d\n", i, RKNextNBuffer0Slot(i, N));
//    i = RKBuffer0SlotCount - 3; RKLog("i = %d --> Next N = %d\n", i, RKNextNBuffer0Slot(i, N));
//    i = RKBuffer0SlotCount - 2; RKLog("i = %d --> Next N = %d\n", i, RKNextNBuffer0Slot(i, N));
//    i = RKBuffer0SlotCount - 1; RKLog("i = %d --> Next N = %d\n", i, RKNextNBuffer0Slot(i, N));
//    i = RKBuffer0SlotCount - 1; RKLog("i = %d --> Prev N = %d\n", i, RKPreviousNBuffer0Slot(i, N));
//    i = 0;                      RKLog("i = %d --> Prev N = %d\n", i, RKPreviousNBuffer0Slot(i, N));
//    i = 1;                      RKLog("i = %d --> Prev N = %d\n", i, RKPreviousNBuffer0Slot(i, N));
//    i = 2;                      RKLog("i = %d --> Prev N = %d\n", i, RKPreviousNBuffer0Slot(i, N));
//    i = 3;                      RKLog("i = %d --> Prev N = %d\n", i, RKPreviousNBuffer0Slot(i, N));
//    i = 4;                      RKLog("i = %d --> Prev N = %d\n", i, RKPreviousNBuffer0Slot(i, N));
//

    radar->pulseCompressionEngine->coreCount = 5;
    
    RKGoLive(radar);

    for (int i = 0; i < 50; i++) {
        RKInt16Pulse *pulse = RKGetVacantPulse(radar);
        // Fill in the data...
        //
        //
        RKSetPulseReady(pulse);
        usleep(50000);
    }

    RKStop(radar);
    
    RKLog("Freeing radar ...\n");
    RKFree(radar);
    
    return 0;
}
