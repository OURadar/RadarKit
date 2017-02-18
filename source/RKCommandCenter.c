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
            case 'H':
                flag |= RKUserFlagStatusHealthOld;
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
            case 'k':
                flag |= RKUserFlagDisplayK;
                break;
            case 'K':
                flag |= RKUserFlagProductK;
                break;
            case 's':
                flag |= RKUserFlagDisplayS;
                break;
            case 'S':
                flag |= RKUserFlagProductS;
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
    if (flag & RKUserFlagStatusHealthOld) { j += sprintf(string + j, "H"); }
    if (flag & RKUserFlagStatusPulses)    { j += sprintf(string + j, "1"); }
    if (flag & RKUserFlagStatusRays)      { j += sprintf(string + j, "2"); }
    if (flag & RKUserFlagStatusPositions) { j += sprintf(string + j, "3"); }
    if (flag & RKUserFlagStatusHealth)    { j += sprintf(string + j, "h"); }
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
    if (flag & RKUserFlagDisplayS)        { j += sprintf(string + j, "s"); }
    if (flag & RKUserFlagProductS)        { j += sprintf(string + j, "S"); }
    if (flag & RKUserFlagDisplayIQ)       { j += sprintf(string + j, "i"); }
    if (flag & RKUserFlagProductIQ)       {      sprintf(string + j, "I"); }
    return 0;
}

int socketCommandHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    
    int j, k;
    char string[RKMaximumStringLength];

    j = snprintf(string, RKMaximumStringLength - 1, "%s %d radar:", engine->name, engine->radarCount);
    for (k = 0; k < engine->radarCount; k++) {
        RKRadar *radar = engine->radars[k];
        j += snprintf(string + j, RKMaximumStringLength - j - 1, " %s", radar->desc.name);
    }

    //int ival;
    char sval1[RKNameLength], sval2[RKNameLength];
    float fval1, fval2;
    
    // Delimited reading ...
    
    switch (O->cmd[0]) {
        case 'a':
            // Authenticate
            sscanf(O->cmd + 1, "%s %s", sval1, sval2);
            RKLog("Authenticating %s %s ... (%d) (%d)\n", sval1, sval2, sizeof(sval1), sizeof(user->login));
            strncpy(user->login, sval1, sizeof(user->login) - 1);
            j = sprintf(string, "{\"Radars\":[");
            for (k = 0; k < engine->radarCount; k++) {
                RKRadar *radar = engine->radars[k];
                j += sprintf(string + j, "\"%s\", ", radar->desc.name);
            }
            if (k > 0) {
                j += sprintf(string + j - 2, "], ") - 2;
            } else {
                j += sprintf(string + j, "], ");
            }
            j += sprintf(string + j, "\"Controls\":["
                         "{\"Label\":\"Go\", \"Command\":\"y\"}, "
                         "{\"Label\":\"Stop\", \"Command\":\"z\"}"
                         "]}" RKEOL);
            RKOperatorSendDelimitedString(O, string);
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
            
        case 'h':
            sprintf(string,
                    "a [username] [password] - Authenticate\n"
                    "prt [value] - PRT in seconds\n"
                    "prf [value] - PRF in Hz\n"
                    "df [filter index] - DSP Filters: 0 = \n"
                    );
            RKOperatorSendDelimitedString(O, string);
            break;
            
        case 'p':
            // Change PRT
            if (!strncmp("prt", O->cmd, 3)) {
                //user->radar->transceiverExec(user->radar->transceiver, O->cmd, string);
                k = sscanf(O->cmd + 3, "%f %f", &fval1, &fval2);
                if (k == 2) {
                    RKLog("%s %s Changing PRT to %.4f + %.4f ms ...\n", engine->name, O->name, fval1, fval2);
                } else {
                    RKLog("%s %s Changing PRT to %.4f s ...\n", engine->name, O->name, fval1);
                }
            } else if (!strncmp("prf", O->cmd, 3)) {
                k = sscanf(O->cmd + 3, "%f %f", &fval1, &fval2);
                fval1 = roundf(fval1);
                if (k == 2) {
                    fval2 = roundf(fval2);
                    RKLog("%s %s Changing PRF to %s + %s Hz ...\n", engine->name, O->name, RKIntegerToCommaStyleString((long)fval1), RKIntegerToCommaStyleString((long)fval2));
                } else {
                    RKLog("%s %s Changing PRF to %s Hz ...\n", engine->name, O->name, RKIntegerToCommaStyleString((long)fval1));
                }
            }
            break;
            
        case 'r':
            sscanf("%s", O->cmd + 1, sval1);
            RKLog("%s %s selected radar %s\n", engine->name, O->name, sval1);
            snprintf(string, RKMaximumStringLength - 1, "ACK. %s selected." RKEOL, sval1);
            RKOperatorSendDelimitedString(O, string);
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
            user->rayIndex = RKPreviousModuloS(user->radar->rayIndex, user->radar->desc.rayBufferDepth);
            user->pulseIndex = RKPreviousModuloS(user->radar->pulseIndex, user->radar->desc.pulseBufferDepth);
            user->rayStatusIndex = RKPreviousModuloS(user->radar->momentEngine->rayStatusBufferIndex, RKBufferSSlotCount);
            user->healthIndex = RKPreviousModuloS(user->radar->healthIndex, user->radar->desc.healthBufferDepth);
            RKOperatorSendDelimitedString(O, string);
            break;

        case 't':
        case 'w':
            // Temporary pass everything to transceiver
            user->radar->transceiverExec(user->radar->transceiver, O->cmd, user->radar->transceiverResponse);
            break;
            
        default:
            snprintf(string, RKMaximumStringLength, "Unknown command '%s'." RKEOL, O->cmd);
            RKOperatorSendDelimitedString(O, string);
            break;
    }
    return 0;
}

int socketStreamHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    
    int i, j, k;
    char *c;
    static struct timeval t0;

    ssize_t size;
    uint32_t endIndex;

    gettimeofday(&t0, NULL);
    double time = (double)t0.tv_sec + 1.0e-6 * (double)t0.tv_usec;
    double td = time - user->timeLastOut;

    RKRay *ray;
    RKRayHeader rayHeader;
    uint8_t *u8Data = NULL;

    RKPulse *pulse;
    RKPulseHeader pulseHeader;
    RKInt16C *c16DataH = NULL;
    RKInt16C *c16DataV = NULL;

    if (engine->radarCount < 1) {
        return 0;
    }

    if (user->radar == NULL) {
        RKLog("User %s has no associated radar.\n", user->login);
        return 0;
    }

    if (user->streams & user->access && td >= 0.05) {
        if (user->streams & RKUserFlagStatusPulses) {
            k = snprintf(user->string, RKMaximumStringLength - 1, "%s | %s | %s | %s |" RKEOL,
                         RKPulseCompressionEngineStatusString(user->radar->pulseCompressionEngine),
                         RKPositionEngineStatusString(user->radar->positionEngine),
                         RKMomentEngineStatusString(user->radar->momentEngine),
                         RKFileEngineStatusString(user->radar->fileEngine));
            O->delimTx.type = RKNetworkPacketTypePlainText;
            O->delimTx.size = k + 1;
            RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
            user->timeLastOut = time;
        }
        if (user->streams & RKUserFlagStatusPositions) {
            k = snprintf(user->string, RKMaximumStringLength - 1, "%s" RKEOL,
                         RKPositionEnginePositionString(user->radar->positionEngine));
            O->delimTx.type = RKNetworkPacketTypePlainText;
            O->delimTx.size = k + 1;
            RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
            user->timeLastOut = time;
        }
    }


    if (user->streams & user->access & RKUserFlagStatusHealthOld && time - user->timeLastHealthOut >= 1.0) {
        k = snprintf(user->string, RKMaximumStringLength - 1,
                     "Ready@led=3;Status 1@led=3;Status 2@led=3;Pedestal@led=3;Transceiver@led=3;Clock@led=2;DSP@led=2;Recorder@led=2;SSPA H@num=1,-inf dBm;SSPA V@num=1,-inf dBm;FPGA@num=4,55 degC;Temp 1@num=3,43 degC;Temp 2@num=3,41 degC;Temp 3@num=3,36 degC;Temp 4@num=3,22.81 degC;PRF@num=3,%d Hz" RKEOL,
                     user->radar->configs[user->radar->configIndex].prf[0]);
        O->delimTx.type = 's';
        O->delimTx.size = k + 1;
        RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
        user->timeLastHealthOut = time;
    }
    
    if (user->streams & user->access & RKUserFlagStatusHealth) {
        j = 0;
        k = 0;
        endIndex = RKPreviousModuloS(user->radar->healthIndex, user->radar->desc.healthBufferDepth);
        while (user->healthIndex != endIndex && k < RKMaximumStringLength - 200) {
            c = user->radar->healthEngine->healthBuffer[user->healthIndex].string;
            k += sprintf(user->string + k, "%s\n", c);
            user->healthIndex = RKNextModuloS(user->healthIndex, user->radar->desc.healthBufferDepth);
            j++;
        }
        if (j) {
            // Take out the last '\n', replace it with somethign else + EOL
            snprintf(user->string + k - 1, RKMaximumStringLength - k - 1, "" RKEOL);
            O->delimTx.type = RKNetworkPacketTypePlainText;
            O->delimTx.size = k + 1;
            RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
        }
        user->timeLastOut = time;
    }

    if (user->streams & user->access & RKUserFlagStatusRays) {
        j = 0;
        k = 0;
        endIndex = RKPreviousModuloS(user->radar->momentEngine->rayStatusBufferIndex, RKBufferSSlotCount);
        while (user->rayStatusIndex != endIndex && k < RKMaximumStringLength - 200) {
            c = user->radar->momentEngine->rayStatusBuffer[user->rayStatusIndex];
            k += sprintf(user->string + k, "%s\n", c);
            user->rayStatusIndex = RKNextModuloS(user->rayStatusIndex, RKBufferSSlotCount);
            j++;
        }
        if (j) {
            // Take out the last '\n', replace it with somethign else + EOL
            snprintf(user->string + k - 1, RKMaximumStringLength - k - 1, "" RKEOL);
            O->delimTx.type = RKNetworkPacketTypePlainText;
            O->delimTx.size = k + 1;
            RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
        }
        user->timeLastOut = time;
    }

    if (user->streams & user->access & RKUserFlagDisplayZVWDPRKS) {
        endIndex = RKPreviousModuloS(user->radar->rayIndex, user->radar->desc.rayBufferDepth);
        while (user->rayIndex != endIndex) {
            ray = RKGetRay(user->radar->rays, user->rayIndex);
            // Duplicate and send the header with only selected products
            memcpy(&rayHeader, &ray->header, sizeof(RKRayHeader));
            rayHeader.productList = RKProductListNone;
            if (user->streams & RKUserFlagDisplayZ) {
                rayHeader.productList |= RKProductListDisplayZ;
            }
            if (user->streams & RKUserFlagDisplayV) {
                rayHeader.productList |= RKProductListDisplayV;
            }
            if (user->streams & RKUserFlagDisplayW) {
                rayHeader.productList |= RKProductListDisplayW;
            }
            if (user->streams & RKUserFlagDisplayD) {
                rayHeader.productList |= RKProductListDisplayD;
            }
            if (user->streams & RKUserFlagDisplayP) {
                rayHeader.productList |= RKProductListDisplayP;
            }
            if (user->streams & RKUserFlagDisplayR) {
                rayHeader.productList |= RKProductListDisplayR;
            }
            if (user->streams & RKUserFlagDisplayK) {
                rayHeader.productList |= RKProductListDisplayK;
            }
            if (user->streams & RKUserFlagDisplayS) {
                rayHeader.productList |= RKProductListDisplayS;
            }
            uint32_t productList = rayHeader.productList;
            uint32_t productCount = __builtin_popcount(productList);
            //RKLog("ProductCount = %d / %x\n", productCount, productList);

            rayHeader.gateCount /= user->rayDownSamplingRatio;
            rayHeader.gateSizeMeters *= (float)user->rayDownSamplingRatio;

            O->delimTx.type = 'm';
            O->delimTx.size = (uint32_t)(sizeof(RKRayHeader) + productCount * rayHeader.gateCount * sizeof(uint8_t));
            RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &rayHeader, sizeof(RKRayHeader), NULL);

            for (j = 0; j < productCount; j++) {
                if (productList & RKProductListDisplayZ) {
                    productList ^= RKProductListDisplayZ;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexZ);
                } else if (productList & RKProductListDisplayV) {
                    productList ^= RKProductListDisplayV;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexV);
                } else if (productList & RKProductListDisplayW) {
                    productList ^= RKProductListDisplayW;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexW);
                } else if (productList & RKProductListDisplayD) {
                    productList ^= RKProductListDisplayD;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexD);
                } else if (productList & RKProductListDisplayP) {
                    productList ^= RKProductListDisplayP;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexP);
                } else if (productList & RKProductListDisplayR) {
                    productList ^= RKProductListDisplayR;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexR);
                } else if (productList & RKProductListDisplayK) {
                    productList ^= RKProductListDisplayK;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexK);
                } else if (productList & RKProductListDisplayS) {
                    productList ^= RKProductListDisplayS;
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexS);
                } else {
                    u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexZ);
                }

                uint8_t *lowRateData = (uint8_t *)user->string;
                for (i = 0, k = 0; i < rayHeader.gateCount; i++, k += user->rayDownSamplingRatio) {
                    lowRateData[i] = u8Data[k];
                }
                RKOperatorSendPackets(O, lowRateData, rayHeader.gateCount * sizeof(uint8_t), NULL);
            }
            user->rayIndex = RKNextModuloS(user->rayIndex, user->radar->desc.rayBufferDepth);
        }
    }

    if (user->streams & user->access & RKUserFlagProductIQ) {
        // If I/Q data is sent, there is no need to send a subset of it.
        endIndex = RKPreviousModuloS(user->radar->pulseIndex, user->radar->desc.pulseBufferDepth);
        while (user->pulseIndex != endIndex) {

            user->pulseIndex = RKNextModuloS(user->pulseIndex, user->radar->desc.pulseBufferDepth);
        }
    } else if (user->streams & user->access & RKUserFlagDisplayIQ && time - user->timeLastDisplayIQOut >= 0.05) {
        endIndex = RKPreviousModuloS(user->radar->pulseIndex, user->radar->desc.pulseBufferDepth);
        pulse = RKGetPulse(user->radar->pulses, endIndex);
        memcpy(&pulseHeader, &pulse->header, sizeof(RKPulseHeader));

        pulseHeader.gateCount /= user->pulseDownSamplingRatio;
        pulseHeader.gateSizeMeters *= (float)user->pulseDownSamplingRatio;

        c16DataH = RKGetInt16CDataFromPulse(pulse, 0);
        c16DataV = RKGetInt16CDataFromPulse(pulse, 1);

        for (i = 0, k = 0; i < pulseHeader.gateCount; i++, k += user->pulseDownSamplingRatio) {
            user->samples[0][i] = c16DataH[k];
            user->samples[1][i] = c16DataV[k];
        }

        size = pulseHeader.gateCount * sizeof(RKInt16C);

        O->delimTx.type = RKNetworkPacketTypePulseData;
        O->delimTx.size = (uint32_t)(sizeof(RKPulseHeader) + 2 * size);
        RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &pulseHeader, sizeof(RKPulseHeader), user->samples[0], size, user->samples[1], size, NULL);

        user->timeLastDisplayIQOut = time;
    }

    // Re-evaluate td = time - user->timeLastOut; send a heart beat if nothing has been sent
    if (time - user->timeLastOut >= 1.0) {
        user->timeLastOut = time;
        size = RKOperatorSendBeacon(O);
        if (size < 0) {
            RKLog("Beacon failed (r = %d).\n", size);
            RKOperatorHangUp(O);
        }
    }
    
    return 0;
}

int socketInitialHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    
    memset(user, 0, sizeof(RKUser));
    user->access = RKUserFlagStatusAll;
    user->access |= RKUserFlagDisplayZVWDPRKS;
    user->access |= RKUserFlagProductZVWDPRKS;
    user->access |= RKUserFlagDisplayIQ | RKUserFlagProductIQ;
    user->access |= RKUserFlagControl;
    user->radar = engine->radars[0];
    user->rayDownSamplingRatio = (uint16_t)(user->radar->desc.pulseCapacity / user->radar->desc.pulseToRayRatio / 500);
    user->pulseDownSamplingRatio = (uint16_t)(user->radar->desc.pulseCapacity / 1000);
    RKLog(">%s %s User %d x%d x%d ...\n", engine->name, O->name, O->iid, user->pulseDownSamplingRatio, user->rayDownSamplingRatio);

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
    RKLog("%s Starting ...\n", center->name);
    RKServerStart(center->server);
}

void RKCommandCenterStop(RKCommandCenter *center) {
    if (center->verbose > 1) {
        RKLog("%s stopping ...\n", center->name);
    }
    RKServerStop(center->server);
    RKLog("%s stopped.\n", center->name);
}
