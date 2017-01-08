//
//  RKCommandCenter.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKCommandCenter.h>

// Private declarations

int socketCommandHandler(RKOperator *);
int socketStreamHandler(RKOperator *);

// Implementation

int socketCommandHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    char string[RKMaximumStringLength];
    
    RKLog("%s / %s has server @ %p with %d radars\n", engine->name, O->name, engine, engine->radarCount);
    
    switch (O->cmd[0]) {
        case 'a':
            RKOperatorSendBeaconAndString(O, "Hello" RKEOL);
            break;
        default:
            snprintf(string, RKMaximumStringLength, "Unknown command '%s'." RKEOL, O->cmd);
            RKOperatorSendBeaconAndString(O, string);
            break;
    }
    return 0;
}

int socketStreamHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    
    static struct timeval t0, t1;

    ssize_t r;

    gettimeofday(&t0, NULL);

    // Check IQ
    // Check MM
    // Check system health

    // Heart beat
    double td = RKTimevalDiff(t0, t1);
    if (td >= 1.0f) {
        t1 = t0;
        r = RKOperatorSendBeacon(O);
        if (r < 0) {
            RKLog("Beacon failed (r = %d).\n", r);
            RKOperatorHangUp(O);
        }
    }

    return 0;
}

#pragma mark - Life Cycle

RKCommandCenter *RKCommandCenterInit(void) {
    RKCommandCenter *engine = (RKCommandCenter *)malloc(sizeof(RKCommandCenter));
    if (engine == NULL) {
        RKLog("Error. Unable to allocate local command center.\n");
        return NULL;
    }
    memset(engine, 0, sizeof(RKCommandCenter));
    sprintf(engine->name, "%s<CommandCenter>%s",
            rkGlobalParameters.showColor ? "\033[1;44m" : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->verbose = 3;
    engine->server = RKServerInit();
    RKServerSetCommandHandler(engine->server, &socketCommandHandler);
    RKServerSetStreamHandler(engine->server, &socketStreamHandler);
    RKServerSetSharedResource(engine->server, engine);
    return engine;
}

void RKCommandCenterFree(RKCommandCenter *engine) {
    free(engine);
}

#pragma mark - Properties

void RKCommandCenterSetVerbose(RKCommandCenter *engine, const int verb) {
    engine->verbose = verb;
}

void RKCommandCenterAddRadar(RKCommandCenter *engine, RKRadar *radar) {
    
}

void RKCommandCenterRemoveRadar(RKCommandCenter *engine, RKRadar *radar) {
    
}

#pragma mark - Interactions

void RKCommandCenterStart(RKCommandCenter *center) {
    if (center->verbose) {
        RKLog("%s starting ...\n", center->name);
    }
    RKServerActivate(center->server);
    RKServer *server = center->server;
    while (server->state < RKServerStateActive) {
        usleep(1000);
    }
}
