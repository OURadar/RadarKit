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

    RKServer *M = RKServerInit();

    RKServerSetCommandHandler(M, &socketCommandHandler);
    
    RKServerActivate(M);


    while (radar->active) {
        sleep(1);
    }

    RKServerStop(M);

    RKFree(radar);
    
    return 0;
}
