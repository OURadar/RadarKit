//
//  RKCommandCenter.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKCommandCenter.h>

// Private declarations

int socketInitialHandler(RKOperator *);
int socketCommandHandler(RKOperator *);
int socketStreamHandler(RKOperator *);

// Implementation

RKUserFlag RKStringToFlag(const char * string) {
    int j = 0;
    char *c = (char *)string;
    RKUserFlag flag = RKUserFlagNull;
    while (j++ < strlen(string)) {
        switch (*c) {
            case 'h':
                flag |= RKUserFlagStatusHealth;
                break;
            case '1':
                flag |= RKUserFlagStatusPulses;
                break;
            case '2':
                flag |= RKUserFlagStatusRays;
                break;
            case 'z':
                flag |= RKUserFlagDisplayZ;
                break;
            case 'Z':
                flag |= RKUserFlagProductZ;
                break;
            case 'v':
                flag |= RKUserFlagDisplayV;
                break;
            case 'V':
                flag |= RKUserFlagProductV;
                break;
            case 'w':
                flag |= RKUserFlagDisplayW;
                break;
            case 'W':
                flag |= RKUserFlagProductW;
                break;
            case 'd':
                flag |= RKUserFlagDisplayD;
                break;
            case 'D':
                flag |= RKUserFlagProductD;
                break;
            case 'p':
                flag |= RKUserFlagDisplayP;
                break;
            case 'P':
                flag |= RKUserFlagProductP;
                break;
            case 'r':
                flag |= RKUserFlagDisplayR;
                break;
            case 'R':
                flag |= RKUserFlagProductR;
                break;
            case 'i':
                flag |= RKUserFlagDisplayIQ;
                break;
            case 'I':
                flag |= RKUserFlagProductIQ;
                break;
            default:
                break;
        }
        c++;
    }
    return flag;
}

int RKFlagToString(char *string, RKUserFlag flag) {
    int j = 0;
    if (flag & RKUserFlagStatusHealth) { j += sprintf(string + j, "h"); }
    if (flag & RKUserFlagStatusPulses) { j += sprintf(string + j, "1"); }
    if (flag & RKUserFlagStatusRays)   { j += sprintf(string + j, "2"); }
    if (flag & RKUserFlagDisplayZ)     { j += sprintf(string + j, "z"); }
    if (flag & RKUserFlagProductZ)     { j += sprintf(string + j, "Z"); }
    if (flag & RKUserFlagDisplayV)     { j += sprintf(string + j, "v"); }
    if (flag & RKUserFlagProductV)     { j += sprintf(string + j, "V"); }
    if (flag & RKUserFlagDisplayW)     { j += sprintf(string + j, "w"); }
    if (flag & RKUserFlagProductW)     { j += sprintf(string + j, "W"); }
    if (flag & RKUserFlagDisplayD)     { j += sprintf(string + j, "d"); }
    if (flag & RKUserFlagProductD)     { j += sprintf(string + j, "D"); }
    if (flag & RKUserFlagDisplayP)     { j += sprintf(string + j, "p"); }
    if (flag & RKUserFlagProductP)     { j += sprintf(string + j, "P"); }
    if (flag & RKUserFlagDisplayR)     { j += sprintf(string + j, "r"); }
    if (flag & RKUserFlagProductR)     { j += sprintf(string + j, "R"); }
    if (flag & RKUserFlagDisplayK)     { j += sprintf(string + j, "k"); }
    if (flag & RKUserFlagProductK)     { j += sprintf(string + j, "K"); }
    if (flag & RKUserFlagDisplayIQ)    { j += sprintf(string + j, "i"); }
    if (flag & RKUserFlagProductIQ)    { j += sprintf(string + j, "I"); }
    return 0;
}

int socketCommandHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    
    int j, k;
    char input[RKMaximumStringLength];
    char string[RKMaximumStringLength];

    j = snprintf(string, RKMaximumStringLength - 1, "%s %d radar:", engine->name, engine->radarCount);
    for (k = 0; k < engine->radarCount; k++) {
        RKRadar *radar = engine->radars[k];
        j += snprintf(string + j, RKMaximumStringLength - j - 1, " %s", radar->name);
    }

    //int ival;
    char sval1[64], sval2[64];
    
    // Delimited reading ...
    
    switch (O->cmd[0]) {
        case 'a':
            // Authenticate
            sscanf(O->cmd + 1, "%s %s", sval1, sval2);
            RKLog("Authenticating %s %s ... (%d) (%d)\n", sval1, sval2, sizeof(sval1), sizeof(user->login));
            strncpy(user->login, sval1, sizeof(user->login) - 1);
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
        case 's':
            user->streams = RKStringToFlag(O->cmd + 1);
            sprintf(string, "{\"access\": 0x%lux, \"streams\": 0x%lux}" RKEOL, (unsigned long)user->access, (unsigned long)user->streams);
            // Fast foward some indices
            user->rayStatusIndex = user->radar->momentEngine->rayStatusBufferIndex;
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
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    
    int j, k;
    char *c;
    static struct timeval t0;

    ssize_t r;
    uint32_t endIndex;

    gettimeofday(&t0, NULL);
    double time = (double)t0.tv_sec + 1.0e-6 * (double)t0.tv_usec;
    double td = time - user->timeLastOut;

    // Check IQ
    // Check MM
    // Check system health

    if (user->streams & user->access & RKUserFlagStatusPulses && td >= 0.05) {
        snprintf(user->string, RKMaximumStringLength - 1, "%s %s" RKEOL,
                 RKPulseCompressionEngineStatusString(user->radar->pulseCompressionEngine),
                 RKMomentEngineStatusString(user->radar->momentEngine));
        RKOperatorSendBeaconAndString(O, user->string);
        user->timeLastOut = time;
    }
    
    j = 0;
    k = 0;
    if (user->streams & user->access & RKUserFlagStatusRays) {
        endIndex = user->radar->momentEngine->rayStatusBufferIndex;
        while (user->rayStatusIndex != endIndex) {
            c = user->radar->momentEngine->rayStatusBuffer[user->rayStatusIndex];
            k += snprintf(user->string + k, RKMaximumStringLength - k - 1, "%s\n", c);
            user->rayStatusIndex = RKNextModuloS(user->rayStatusIndex, RKBufferSSlotCount);
            j++;
        }
        if (j) {
            // Take out the last '\n', replace it with somethign else + EOL
            snprintf(user->string + k - 1, RKMaximumStringLength - k - 1, "" RKEOL);
            RKOperatorSendBeaconAndString(O, user->string);
            user->timeLastOut = time;
        }
    }

    // Re-evaluate td = time - user->timeLastOut; send a heart beat if nothing has been sent
    if (time - user->timeLastOut >= 1.0) {
        user->timeLastOut = time;
        r = RKOperatorSendBeacon(O);
        if (r < 0) {
            RKLog("Beacon failed (r = %d).\n", r);
            RKOperatorHangUp(O);
        }
    }
    
    return 0;
}

int socketInitialHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    
    RKLog("%s %s prearing user %d ...\n", engine->name, O->name, O->iid);
    memset(user, 0, sizeof(RKUser));
    user->access = RKUserFlagStatusHealth | RKUserFlagStatusPulses| RKUserFlagStatusRays;
    user->access |= RKUserFlagDisplayZVWDPRKS;
    user->access |= RKUserFlagProductZVWDPRKS;
    user->access |= RKUserFlagDisplayIQ | RKUserFlagProductIQ;
    user->access |= RKUserFlagControl;
    user->radar = engine->radars[0];
    return RKResultNoError;
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
            rkGlobalParameters.showColor ? "\033[1;97;48;5;27m" : "", rkGlobalParameters.showColor ? RKNoColor : "");
    engine->verbose = 3;
    engine->server = RKServerInit();
    RKServerSetName(engine->server, engine->name);
    RKServerSetWelcomeHandler(engine->server, &socketInitialHandler);
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
            RKLog("%s Removing '%s' ...\n", engine->name, radar->name);
            while (i < engine->radarCount - 1) {
                engine->radars[i] = engine->radars[i + 1];
            }
            engine->radarCount--;
        }
    }
    if (engine->radarCount) {
        int j = 0;
        char string[RKMaximumStringLength];
        for (int k = 0; k < engine->radarCount; k++) {
            RKRadar *radar = engine->radars[k];
            j += snprintf(string + j, RKMaximumStringLength - j - 1, "%d. %s\n", k, radar->name);
        }
        printf("Remaining radars\n================\n%s", string);
    }
}

#pragma mark - Interactions

void RKCommandCenterStart(RKCommandCenter *center) {
    if (center->verbose) {
        RKLog("%s starting ...\n", center->name);
    }
    RKServerActivate(center->server);
}
