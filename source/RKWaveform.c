//
//  RKWaveform.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/19/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKWaveform.h>

void makeHops(RKInt16C *waveTable, double bandwidth, const int count) {
    waveTable[0].i = 1;
    waveTable[0].q = 0;
}
