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
    
    int i, j, k;
    
    char string[RKMaximumStringLength];
    char input[RKMaximumStringLength];
    
    const int c = O->iid;
    
    RKLog("%s has %d radar\n", engine->name, engine->radarCount);
    
    sscanf(O->cmd + 1, "%s", input);
    
    // Delimited reading ...
    
    switch (O->cmd[0]) {
        case 'a':
            RKOperatorSendBeaconAndString(O, "Hello" RKEOL);
            // Authenticate
            j = 0;
            for (k = 0; k < engine->radarCount; k++) {
                RKRadar *radar = engine->radars[k];
                j += snprintf(string + j, RKMaximumStringLength - j - 1, "%d. %s\n", k, radar->name);
            }
            snprintf(string + j, RKMaximumStringLength - j - 1, "Select 1-%d" RKEOL, k);
            RKOperatorSendBeaconAndString(O, string);
            break;
        case 'r':
            RKLog("%s/%s selected radar %s\n", engine->name, O->name, input);
            snprintf(string, RKMaximumStringLength - 1, "Radar %s selected." RKEOL, input);
            RKOperatorSendBeaconAndString(O, string);
            break;
        default:
            snprintf(string, RKMaximumStringLength, "Unknown command '%s'." RKEOL, O->cmd);
            RKOperatorSendBeaconAndString(O, string);
            break;
    }
    return 0;
}

int socketStreamHandler(RKOperator *O) {
//    RKCommandCenter *engine = O->userResource;
    
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
    RKServerSetName(engine->server, engine->name);
    RKServerSetCommandHandler(engine->server, &socketCommandHandler);
    RKServerSetStreamHandler(engine->server, &socketStreamHandler);
    RKServerSetSharedResource(engine->server, engine);
    return engine;
}

void RKCommandCenterFree(RKCommandCenter *engine) {
    free(engine);
}

#pragma mark - Properties

void RKCommandCenterSetVerbose(RKCommandCenter *engine, const int verbose) {
    RKServer *server = engine->server;
    server->verbose = verbose;
    engine->verbose = verbose;
}

void RKCommandCenterAddRadar(RKCommandCenter *engine, RKRadar *radar) {
    if (engine->radarCount >= 4) {
        RKLog("%s unable to add another radar.\n", engine->name);
    }
    engine->radars[engine->radarCount++] = radar;
}

void RKCommandCenterRemoveRadar(RKCommandCenter *engine, RKRadar *radar) {
    for (int i = 0; i < engine->radarCount; i++) {
        if (engine->radars[i] == radar) {
            RKLog("%s removing %s radar ...\n", engine->name, radar->name);
            while (i < engine->radarCount - 1) {
                engine->radars[i] = engine->radars[i + 1];
            }
        }
    }
    int j = 0;
    char string[RKMaximumStringLength];
    for (int k = 0; k < engine->radarCount; k++) {
        RKRadar *radar = engine->radars[k];
        j += snprintf(string + j, RKMaximumStringLength - j - 1, "%d. %s\n", k, radar->name);
    }
    printf("%s\n", string);
}

#pragma mark - Interactions

void RKCommandCenterStart(RKCommandCenter *center) {
    if (center->verbose) {
        RKLog("%s starting ...\n", center->name);
    }
    RKServerActivate(center->server);
}
