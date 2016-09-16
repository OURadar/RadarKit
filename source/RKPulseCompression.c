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
            fprintf(stderr, "Error. Failed to start a compression core.\n");
            return -1;
        }
        if ((r = sem_init(&radar->semPulseCompression[i], 0, 0)) != 0) {
            fprintf(stderr, "Error. Unable to initialize a semaphore.\n");
            return -2;
        }
    }

    return 0;
}

int pulseId(RKRadar *radar) {
    int i;
    pthread_t id = pthread_self();
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

    //printf("I am core %d\n", k);

    struct timespec ts;

    while (radar->active) {

#if defined(__APPLE__)
        usleep(1000);
#else
        // Only process pulses that
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        sem_timedwait(&radar->semPulseCompression[k], &ts);
#endif
    }
    return NULL;
}
