//
//  RKRadar.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#include <RadarKit/RKRadar.h>

RKRadar *RKInit(void) {
    RKRadar *radar;
    if (posix_memalign((void **)&radar, RKSIMDAlignSize, sizeof(RKRadar))) {
        return NULL;
    }
    printf("Radar initialized\n");
    return radar;
}

int RKFree(RKRadar *radar) {
    printf("Freeing Radar\n");
    free(radar);
    return EXIT_SUCCESS;
}
