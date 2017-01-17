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
            case '3':
                flag |= RKUserFlagStatusPositions;
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
    if (flag & RKUserFlagStatusHealth)    { j += sprintf(string + j, "h"); }
    if (flag & RKUserFlagStatusPulses)    { j += sprintf(string + j, "1"); }
    if (flag & RKUserFlagStatusRays)      { j += sprintf(string + j, "2"); }
    if (flag & RKUserFlagStatusPositions) { j += sprintf(string + j, "3"); }
    if (flag & RKUserFlagDisplayZ)        { j += sprintf(string + j, "z"); }
    if (flag & RKUserFlagProductZ)        { j += sprintf(string + j, "Z"); }
    if (flag & RKUserFlagDisplayV)        { j += sprintf(string + j, "v"); }
    if (flag & RKUserFlagProductV)        { j += sprintf(string + j, "V"); }
    if (flag & RKUserFlagDisplayW)        { j += sprintf(string + j, "w"); }
    if (flag & RKUserFlagProductW)        { j += sprintf(string + j, "W"); }
    if (flag & RKUserFlagDisplayD)        { j += sprintf(string + j, "d"); }
    if (flag & RKUserFlagProductD)        { j += sprintf(string + j, "D"); }
    if (flag & RKUserFlagDisplayP)        { j += sprintf(string + j, "p"); }
    if (flag & RKUserFlagProductP)        { j += sprintf(string + j, "P"); }
    if (flag & RKUserFlagDisplayR)        { j += sprintf(string + j, "r"); }
    if (flag & RKUserFlagProductR)        { j += sprintf(string + j, "R"); }
    if (flag & RKUserFlagDisplayK)        { j += sprintf(string + j, "k"); }
    if (flag & RKUserFlagProductK)        { j += sprintf(string + j, "K"); }
    if (flag & RKUserFlagDisplayIQ)       { j += sprintf(string + j, "i"); }
    if (flag & RKUserFlagProductIQ)       {      sprintf(string + j, "I"); }
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
        j += snprintf(string + j, RKMaximumStringLength - j - 1, " %s", radar->desc.name);
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
                j += snprintf(string + j, RKMaximumStringLength - j - 1, "%d. %s\n", k, radar->desc.name);
            }
            snprintf(string + j, RKMaximumStringLength - j - 1, "Select 1-%d" RKEOL, k);
            RKOperatorSendBeaconAndString(O, string);
            break;
            
        case 'd':
            // DSP related
            switch (O->cmd[1]) {
                case 'f':
                    // 'df' - DSP filter
                    break;
                case 'n':
                    // 'dn' - DSP noise override
                    break;
                case 'N':
                    // 'dN' - DSP noise override in dB
                    break;
                default:
                    break;
            }
            break;
            
        case 'p':
            // Change PRT
            
            break;
            
        case 'r':
            RKLog("%s %s selected radar %s\n", engine->name, O->name, input);
            snprintf(string, RKMaximumStringLength - 1, "Radar %s selected." RKEOL, input);
            RKOperatorSendBeaconAndString(O, string);
            break;

        case 'm':
            RKLog("%s %s display data\n", engine->name, O->name);
            user->streams |= RKUserFlagDisplayZ;
            user->rayIndex = RKPreviousModuloS(user->radar->rayIndex, user->radar->desc.rayBufferDepth);
            break;

        case 's':
            // Stream varrious data
            user->streams = RKStringToFlag(O->cmd + 1);
            sprintf(string, "{\"access\": 0x%lx, \"streams\": 0x%lx}" RKEOL, (unsigned long)user->access, (unsigned long)user->streams);
            // Fast foward some indices
            user->rayStatusIndex = RKPreviousModuloS(user->radar->momentEngine->rayStatusBufferIndex, RKBufferSSlotCount);
            //user->pulseIndex = user->radar->pulseIndex;
            //user->rayIndex = user->radar->rayIndex;
            RKOperatorSendBeaconAndString(O, string);
            break;
            
        case 'w':
            // Change waveform
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

    RKRay *ray;
    uint8_t *data;
    RKRayHeader rayHeader;

    // Check IQ
    // Check MM
    // Check system health

    if (engine->radarCount < 1) {
        return 0;
    }

    if (user->radar == NULL) {
        RKLog("User %s has no associated radar.\n", user->login);
        return 0;
    }

    if (user->streams & user->access && td >= 0.05) {
        if (user->streams & RKUserFlagStatusPulses) {
            snprintf(user->string, RKMaximumStringLength - 1, "%s %s %s" RKEOL,
                     RKPulseCompressionEngineStatusString(user->radar->pulseCompressionEngine),
                     RKPositionEngineStatusString(user->radar->positionEngine),
                     RKMomentEngineStatusString(user->radar->momentEngine));
            RKOperatorSendBeaconAndString(O, user->string);
        }
        if (user->streams & RKUserFlagStatusPositions) {
            snprintf(user->string, RKMaximumStringLength - 1, "%s" RKEOL,
                     RKPositionEnginePositionString(user->radar->positionEngine));
            RKOperatorSendBeaconAndString(O, user->string);
        }
        user->timeLastOut = time;
    }
    
    if (user->streams & user->access & RKUserFlagStatusRays) {
        j = 0;
        k = 0;
        endIndex = RKPreviousModuloS(user->radar->momentEngine->rayStatusBufferIndex, RKBufferSSlotCount);
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
        }
    }

    if (user->streams & user->access & RKUserFlagDisplayZ) {
        endIndex = RKPreviousModuloS(user->radar->rayIndex, user->radar->desc.rayBufferDepth);
        while (user->rayIndex != endIndex) {
            ray = RKGetRay(user->radar->rays, user->rayIndex);
            // Duplicate and send the header with only selected products
            memcpy(&rayHeader, &ray->header, sizeof(RKRayHeader));
            rayHeader.productList = RKProductListDisplayZ;
            data = RKGetUInt8DataFromRay(ray, 0);
            O->delim.type = 'm';
            O->delim.size = (uint32_t)sizeof(RKRayHeader);
            RKOperatorSendPackets(O, &O->delim, sizeof(RKNetDelimiter), &rayHeader, O->delim.size, NULL);
            O->delim.type = 'd';
            O->delim.subtype = 'Z';
            O->delim.size = ray->header.gateCount * sizeof(uint8_t);
            RKOperatorSendPackets(O, &O->delim, sizeof(RKNetDelimiter), data, O->delim.size, NULL);
//            RKLog("%s %s ray %d -> %d  gateCount = %d  size %d\n",
//                  engine->name, O->name, user->rayIndex, endIndex, ray->header.gateCount, O->delim.size);
            user->rayIndex = RKNextModuloS(user->rayIndex, user->radar->desc.rayBufferDepth);
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
    
    RKLog(">%s %s preparing user %d ...\n", engine->name, O->name, O->iid);
    memset(user, 0, sizeof(RKUser));
    user->access = RKUserFlagStatusAll;
    user->access |= RKUserFlagDisplayZVWDPRKS;
    user->access |= RKUserFlagProductZVWDPRKS;
    user->access |= RKUserFlagDisplayIQ | RKUserFlagProductIQ;
    user->access |= RKUserFlagControl;
    user->radar = engine->radars[0];
    snprintf(user->login, 63, "radarop");
    user->serverOperator = O;
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
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
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
    RKServerFree(engine->server);
    free(engine);
    
    return;
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
    if (engine->suspendHandler) {
        RKLog("Wait for command center.\n");
        int s = 0;
        while (++s < 10 && engine->suspendHandler) {
            usleep(100000);
        }
        if (s == 10) {
            RKLog("Should not happen.");
            exit(EXIT_FAILURE);
        }
    }
    engine->suspendHandler = true;
    int i;
    for (i = 0; i < engine->radarCount; i++) {
        if (engine->radars[i] == radar) {
            RKLog("%s Removing '%s' ...\n", engine->name, radar->desc.name);
            while (i < engine->radarCount - 1) {
                engine->radars[i] = engine->radars[i + 1];
            }
            engine->radarCount--;
        }
    }
    for (i = 0; i < engine->server->nclient; i++) {
        if (engine->users[i].radar == radar) {
            RKLog("%s Removing '%s' from user %s %s ...\n", engine->name, radar->desc.name, engine->users[i].serverOperator->name, engine->users[i].login);
            engine->users[i].radar = NULL;
        }
    }
    if (engine->radarCount) {
        int j = 0;
        char string[RKMaximumStringLength];
        for (int k = 0; k < engine->radarCount; k++) {
            RKRadar *radar = engine->radars[k];
            j += snprintf(string + j, RKMaximumStringLength - j - 1, "%d. %s\n", k, radar->desc.name);
        }
        printf("Remaining radars\n================\n%s", string);
    }
    engine->suspendHandler = false;
}

#pragma mark - Interactions

void RKCommandCenterStart(RKCommandCenter *center) {
    RKLog("%s starting ...\n", center->name);
    RKServerStart(center->server);
}

void RKCommandCenterStop(RKCommandCenter *center) {
    if (center->verbose > 1) {
        RKLog("%s stopping ...\n", center->name);
    }
    RKServerStop(center->server);
    RKLog("%s stopped.\n", center->name);
}
