//
//  main.c
//  RadarKitTest
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include "RadarKit.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    
    RKRadar *radar = RKInit();
    
    RKFree(radar);
    
    return 0;
}
