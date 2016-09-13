//
//  RKPulseCompression.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulseCompression.h>

// Internal functions
void *pulseCompressionCore(void *in);
int pulseId(RKRadar *radar);

// Implementations
int RKPulseCompressionEngineStart(RKRadar *radar) {

    int i, r;

    // Spin off N workers to process I/Q pulses
    for (i = 0; i < radar->pulseCompressionCoreCount; i++) {
        printf("Starting core %d ...\n", i);
        if ((r = pthread_create(&radar->tidPulseCompression[i], NULL, pulseCompressionCore, radar)) != 0) {
            fprintf(stderr, "Error starting compression core.\n");
            return -1;
        }
    }

    return 0;
}

int pulseId(RKRadar *radar) {
    int i;
    pthread_t id = pthread_self();
    printf("id = %p\n", (void *)id);
    for (i = 0; i < radar->pulseCompressionCoreCount; i++) {
        if (id == radar->tidPulseCompression[i]) {
            return i;
        }
    }
    return -1;
}

void *pulseCompressionCore(void *in) {
    RKRadar *radar = (RKRadar *)in;

    const int k = pulseId(radar);

    if (k < 0) {
        fprintf(stderr, "Unable to find my thread ID.\n");
        return NULL;
    }

    printf("I am core %d\n", k);

    while (radar->active) {
        // Only process pulses that

        sleep(1);
    }
    return NULL;
}
