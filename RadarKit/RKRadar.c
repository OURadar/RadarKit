//
//  RKRadar.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include "RKRadar.h"

RKRadar *RKInit(void) {
    RKRadar *radar;
    if (posix_memalign((void **)&radar, RKSIMDAlignSize, sizeof(RKRadar))) {
        return NULL;
    }
    return radar;
}

void RKFree(RKRadar *radar) {
    free(radar);
}
