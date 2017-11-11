//
//  RKPulseRingFilter.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 11/11/17.
//  Copyright (c) 2015-2018 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPulseRingFilter.h>

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

#pragma mark - Life Cycle

RKPulseRingFilterEngine *RKPulseRingFilterEngineInit(void) {
    RKPulseRingFilterEngine *engine = (RKPulseRingFilterEngine *)malloc(sizeof(RKPulseRingFilterEngine));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate a pulse ring filter engine.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKPulseRingFilterEngine));
    sprintf(engine->name, "%s<PulseRingFilter>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorPulseRingFilterEngine) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->state = RKEngineStateAllocated;
    engine->useSemaphore = true;
    engine->memoryUsage = sizeof(RKPulseRingFilterEngine);
    pthread_mutex_init(&engine->coreMutex, NULL);
    return engine;
}

void RKPulseRingFilterEngineFree(RKPulseRingFilterEngine *engine) {
    if (engine->state & RKEngineStateActive) {
        RKPulseRingFilterEngineStop(engine);
    }
    pthread_mutex_destroy(&engine->coreMutex);
    free(engine);
}

#pragma mark - Properties

void RKPulseRingFilterEngineSetVerbose(RKPulseRingFilterEngine *engine, const int verb) {
    engine->verbose = verb;
}

#pragma mark - Interactions

