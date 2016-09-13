//
//  main.c
//  RadarKitTest
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RadarKit.h>
#include <signal.h>

RKRadar *radar;

//RKGlobalParamters rkGlobalParameters = {{"radar"}, {"messages.log"}, 0, 0};

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    
    radar = RKInit();
    
    printf("radar occupies %d bytes\n", (int)radar->memoryUsage);

    RKPulseCompressionEngineStart(radar);

    while (radar->active) {
        sleep(1);
    }

    RKFree(radar);
    
    return 0;
}
