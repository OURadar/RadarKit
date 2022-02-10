//
//  RKReporter.c
//  RadarKit
//
//  Created by Boonleng Cheong on 2/9/22.
//  Copyright (c) 2017-2022 Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKReporter.h>

// Private declarations

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

#pragma mark - Life Cycle

RKReporter *RKReporterInit(void) {
    return NULL;
}

void RKReporterFree(RKReporter *engine) {
    free(engine);
}

#pragma mark - Properties

void RKReporterSetVerbose(RKReporter *engine, const int verbose) {
    engine->verbose = verbose;
}

#pragma mark - Interactions

void RKReporterStart(RKReporter *engine) {

}

void RKReporterStop(RKReporter *engine) {

}
