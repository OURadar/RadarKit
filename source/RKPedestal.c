//
//  RKPedestal.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/3/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPedestal.h>

// Internal functions

void *pedestalWorker(void *in);

// Implementations

#pragma mark -
#pragma mark Helper Functions

#pragma mark -
#pragma mark Threads

void *pedestalWorker(void *in) {
    RKPedestalEngine *engine = (RKPedestalEngine *)in;

    int k = 0;

    // Get the latest pulse
    RKPulse *pulse = RKGetPulse(engine->pulseBuffer, *engine->pulseIndex);
    while (pulse->header.s & RKPulseStatusVacant) {
        usleep(1000);
    }
    double t = pulse->header.timeDouble;

    // Search until a time I need

    // Wait until the latest position arrives
    // find the latest position, tag the pulse with the appropriate position
    // linearly interpolate between the best two readings
    // at some point, implement something sophisticated like Klaman filter

    // Set the pulse to have position

    return (void *)NULL;
}

#pragma mark -
#pragma mark Life Cycle

RKPedestalEngine *RKPedestalEngineInit() {
    RKPedestalEngine *engine = (RKPedestalEngine *)malloc(sizeof(RKPedestalEngine));
    memset(engine, 0, sizeof(RKPedestalEngine));
    return engine;
}

void RKPedestalEngineFree(RKPedestalEngine *engine) {
    free(engine);
}

#pragma mark -
#pragma mark Properties

void RKPedestalEngineSetHardwareInit(RKPedestalEngine *engine, RKPedestal hardwareInit(void *), void *hardwareInitInput) {
    engine->hardwareInit = hardwareInit;
    engine->hardwareInitInput = hardwareInitInput;
}

void RKPedestalEngineSetHardwareExec(RKPedestalEngine *engine, int hardwareExec(RKPedestal, const char *)) {
    engine->hardwareExec = hardwareExec;
}

void RKPedestalEngineSetHardwareRead(RKPedestalEngine *engine, int hardwareRead(RKPedestal, RKPosition *)) {
    engine->hardwareRead = hardwareRead;
}

void RKPedestalEngineSetHardwareFree(RKPedestalEngine *engine, int hardwareFree(RKPedestal)) {
    engine->hardwareFree = hardwareFree;
}

#pragma mark -
#pragma mark Interactions

int RKPedestalEngineStart(RKPedestalEngine *engine) {
    //pthread_create() ...
    return RKResultNoError;
}

int RKPedestalEngineStop(RKPedestalEngine *engine) {
    return RKResultNoError;
}
