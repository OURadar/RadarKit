//
//  RKPedestal.c
//  RadarKit
//
//  RKPedestal provides a wrapper to interact with RadarKit, tag each raw time-series pulse with a set
//  of proper position that contains azimuth and elevation. It manages the run-loop that continuously
//  monitor the incoming position read, where you would supply the actual read function to interpret
//  the binary stream, decode the stream into an RKPosition slot, which is supplied. Each call of the
//  function expects a proper return of RKPosition. If you expect mutiple read for a complete position
//  description, issue your own internal run-loop to satisfy this requirement. This protocol must be
//  strictly followed. It also serves as a bridge to forward the necessary control command, which will
//  be text form. They will be described in the pedestal control language. Finally, a resource clean
//  up routine is called when the program terminates.
//
//  The main protocols are:
//
//   - Initialize a pointer to a structure to communicate with the hardware (void *),
//     you provide the input through RKPedestalEngineSetHardwareInitInput(). Typecast it to void *.
//     It must be in the form of
//
//         RKPedestal routine(void *);
//
//   - A continuous run loop that continuously ingest position data,
//     you provide a reader delegate routine through RKPedestalEngineSetHardwareRead(). It must be
//     in the form of
//
//         int routine(RKPedestal, RKPosition *);
//
//   - Manages and forwards the current control command in the command queue,
//     you provide a execution delegate routine through RKPedestalEngineSetHardwareExec(). Imust be
//     in the form of
//
//         int routine(RKPedestal, const char *);
//
//   - Close the hardware interaction when it is appropriate
//     you provide a resource deallocation delegate routine through RKPedestalEngineSetHardwareFree().
//     It must be in the form of
//
//         int routine(RKPedestal);
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

    // Get the latest pulse
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

void RKPedestalEngineSetHardwareInit() {

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
