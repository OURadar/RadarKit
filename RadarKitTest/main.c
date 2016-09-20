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

int main(int argc, const char * argv[]) {
    // insert code here...
    radar = RKInit();
    RKSetProgramName("iRadar");
    rkGlobalParameters.stream = stdout;

    RKLog("Hello, World!\n");

    RKLog("Radar state machine occupies %s bytes\n", RKIntegerToCommaStyleString(radar->memoryUsage));

    //RKPulseCompressionEngineStart(radar);

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

    RKGoLive(radar);

    while (radar->active) {
        radar->pulseCompressionEngine->index = RKNextBuffer0Slot(radar->pulseCompressionEngine->index);
        usleep(50000);
        if (radar->pulseCompressionEngine->index > 50) {
            radar->active = false;
            break;
        }
    }

    RKFree(radar);
    
    return 0;
}
