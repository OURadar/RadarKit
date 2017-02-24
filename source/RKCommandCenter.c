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
            case '4':
                flag |= RKUserFlagStatusEngines;
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
    if (flag & RKUserFlagStatusHealthOld)   { j += sprintf(string + j, "H"); }
    if (flag & RKUserFlagStatusPulses)      { j += sprintf(string + j, "1"); }
    if (flag & RKUserFlagStatusRays)        { j += sprintf(string + j, "2"); }
    if (flag & RKUserFlagStatusPositions)   { j += sprintf(string + j, "3"); }
    if (flag & RKUserFlagStatusEngines)     { j += sprintf(string + j, "4"); }
    if (flag & RKUserFlagStatusHealth)      { j += sprintf(string + j, "h"); }
    if (flag & RKUserFlagDisplayZ)          { j += sprintf(string + j, "z"); }
    if (flag & RKUserFlagProductZ)          { j += sprintf(string + j, "Z"); }
    if (flag & RKUserFlagDisplayV)          { j += sprintf(string + j, "v"); }
    if (flag & RKUserFlagProductV)          { j += sprintf(string + j, "V"); }
    if (flag & RKUserFlagDisplayW)          { j += sprintf(string + j, "w"); }
    if (flag & RKUserFlagProductW)          { j += sprintf(string + j, "W"); }
    if (flag & RKUserFlagDisplayD)          { j += sprintf(string + j, "d"); }
    if (flag & RKUserFlagProductD)          { j += sprintf(string + j, "D"); }
    if (flag & RKUserFlagDisplayP)          { j += sprintf(string + j, "p"); }
    if (flag & RKUserFlagProductP)          { j += sprintf(string + j, "P"); }
    if (flag & RKUserFlagDisplayR)          { j += sprintf(string + j, "r"); }
    if (flag & RKUserFlagProductR)          { j += sprintf(string + j, "R"); }
    if (flag & RKUserFlagDisplayK)          { j += sprintf(string + j, "k"); }
    if (flag & RKUserFlagProductK)          { j += sprintf(string + j, "K"); }
    if (flag & RKUserFlagDisplayS)          { j += sprintf(string + j, "s"); }
    if (flag & RKUserFlagProductS)          { j += sprintf(string + j, "S"); }
    if (flag & RKUserFlagDisplayIQ)         { j += sprintf(string + j, "i"); }
    if (flag & RKUserFlagProductIQ)         {      sprintf(string + j, "I"); }
    return 0;
}

int indentCopy(char *dst, char *src) {
    int k = 0;
    char *e, *s = src;
    do {
        e = strchr(s, '\n');
        if (e) {
            *e = '\0';
            k += sprintf(dst + k, "    %s\n", s);
            s = e + 1;
        }
    } while (e != NULL);
    k += sprintf(dst + k, "    %s", s);
    return k;
}

int socketCommandHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    
    int j, k, s;
    char string[RKMaximumStringLength * 2];

    j = snprintf(string, RKMaximumStringLength - 1, "%s %d radar:", engine->name, engine->radarCount);
    for (k = 0; k < engine->radarCount; k++) {
        RKRadar *radar = engine->radars[k];
        j += snprintf(string + j, RKMaximumStringLength - j - 1, " %s", radar->desc.name);
    }

    //int ival;
    //float fval1, fval2;
    char sval1[RKMaximumStringLength];
    char sval2[RKMaximumStringLength];
    memset(sval1, 0, sizeof(sval1));
    memset(sval2, 0, sizeof(sval2));
    
    // Delimited reading ...
    
    switch (O->cmd[0]) {
        case 'a':
            // Authenticate
            sscanf(O->cmd + 1, "%s %s", sval1, sval2);
            RKLog("Authenticating %s %s ... (%d) (%d)\n", sval1, sval2, strlen(sval1), sizeof(user->login));
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
            s = sprintf(sval1, "vol p 2 140 180");
            for (k = 4; k < 20; k += 2) {
                s += sprintf(sval1 + s, "/p %d 140 180", k);
            }
            s += sprintf(sval1 + s, "/p 20 140,120 180");
            j += sprintf(string + j, "\"Controls\":["
                         "{\"Label\":\"Go\", \"Command\":\"y\"}, "
                         "{\"Label\":\"Stop\", \"Command\":\"z\"}, "
                         "{\"Label\":\"10-tilt Rapid Scan @ 180 dps\", \"Command\":\"%s\"}"
                         "]}" RKEOL, sval1);
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
            if (strlen(O->cmd) == 1 || !strncmp(O->cmd, "help", 4)) {
                k = sprintf(string,
                            "Help\n"
                            "====\n"
                            "\n"
                            HIGHLIGHT("a") " [USERNAME] [ENCRYPTED_PASSWORD] - Authenticate\n"
                            "\n"
                            HIGHLIGHT("f") " [FILTER_INDEX] - DSP filters,\n"
                            "    where index can be:\n"
                            "        0 - No ground clutter filter\n"
                            "        1 - Ground clutter filter @ +/- 0.5 m/s\n"
                            "        2 - Ground clutter filter @ +/- 1.0 m/s\n"
                            "        3 - Ground clutter filter @ +/- 2.0 m/s\n"
                            "\n"
                            HIGHLIGHT("s") " [VALUE] - Get various data streams\n"
                            "    where [VALUE] can be one of a combinations of:\n"
                            "        1 - Overall all view of the system buffer\n"
                            "        2 - Product generation, ray by ray update\n"
                            "        3 - Position data from the pedestal while I/Q is active\n"
                            "        4 - Various engine states\n"
                            "        z - Display stream of Z reflectivity\n"
                            "        v - Display stream of V velocity\n"
                            "        w - Display stream of W width\n"
                            "        d - Display stream of D differential reflectivity\n"
                            "        p - Display stream of P PhiDP differential phase\n"
                            "        r - Display stream of R RhoHV cross-correlation coefficient\n"
                            "        k - Display stream of K KDP specific phase\n"
                            "        s - Display stream of S signal power in dBm\n"
                            "    e.g.,\n"
                            "        s zvwd - streams Z, V, W and D.\n"
                            "        s 1 - Look at the overall system status.\n"
                            "\n"
                            HIGHLIGHT("v") " - Sets a simple VCP (coming soon)\n"
                            "    e.g.,\n"
                            "        v 2:2:20 180 - a volume at EL 2° to 20° at 2° steps, AZ slew at 180°/s\n"
                            "\n"
                            HIGHLIGHT("y") " - Everything goes, default waveform and VCP\n"
                            "\n"
                            HIGHLIGHT("z") " - Everything stops\n"
                            "\n");

                k += sprintf(string + k,
                             HIGHLIGHT("t") " - " UNDERLINE_ITALIC("Transceiver") " commands, everything that starts with t goes to the transceiver\n"
                             "    module in a concatenated form, e.g., 't help' -> 'help' to the transceiver.\n\n");
                if (user->radar->transceiver) {
                    user->radar->transceiverExec(user->radar->transceiver, "help", sval1);
                    RKStripTail(sval1);
                    k += indentCopy(string + k, sval1);
                    k += sprintf(string + k, "\n\n");
                } else {
                    k += sprintf(string + k, "    INFO: Transceiver not set.\n");
                }

                k += sprintf(string + k,
                             HIGHLIGHT("p") " - " UNDERLINE_ITALIC ("Pedestal") " commands, everything that starts with p goes to the pedestal module\n"
                             "    in a concatenated form, e.g., 'p help' -> 'help' to the pedestal.\n\n");
                if (user->radar->pedestal) {
                    user->radar->pedestalExec(user->radar->pedestal, "help", sval1);
                    RKStripTail(sval1);
                    k += indentCopy(string + k, sval1);
                    k += sprintf(string + k, "\n\n");
                } else {
                    k += sprintf(string + k, "    INFO: Pedestal not set.\n");
                }

                k += sprintf(string + k,
                             HIGHLIGHT("p") " - " UNDERLINE_ITALIC ("Health Relay") " commands, everything that starts with p goes to the health relay\n"
                             "    module in a concatenated form, e.g., 'p help' -> 'help' to the health relay.\n\n");
                if (user->radar->healthRelay) {
                    user->radar->healthRelayExec(user->radar->healthRelay, "help", sval1);
                    RKStripTail(sval1);
                    k += indentCopy(string + k, sval1);
                    k += sprintf(string + k, "\n\n");
                } else {
                    k += sprintf(string + k, "    INFO: Health Relay not set.\n");
                }

                k += sprintf(string + k, "\n== (%s) ==" RKEOL, RKIntegerToCommaStyleString(k));

                RKOperatorSendDelimitedString(O, string);
                break;

            } else {

                // Forward to health relay
                k = 1;
                while (O->cmd[k] == ' ') {
                    k++;
                }
                user->radar->healthRelayExec(user->radar->healthRelay, O->cmd + k, string);

            }
            
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

        case 'p':
            // Pass everything to pedestal
            k = 0;
            do {
                k++;
            } while (O->cmd[k] == ' ');
            user->radar->pedestalExec(user->radar->pedestal, O->cmd + k, string);
            RKOperatorSendDelimitedString(O, string);
            break;

        case 't':
        case 'w':
            // Pass everything to transceiver
            k = 1;
            while (O->cmd[k] == ' ') {
                k++;
            }
            user->radar->transceiverExec(user->radar->transceiver, O->cmd + k, string);
            RKOperatorSendDelimitedString(O, string);
            break;
            
        case 'v':
            // Simple volume
            user->radar->pedestalExec(user->radar->pedestal, O->cmd, string);
            break;

        case 'x':
            engine->developerInspect = RKNextModuloS(engine->developerInspect, 4);
            sprintf(string, "ACK. Developer inspect set to %d" RKEOL, engine->developerInspect);
            RKOperatorSendDelimitedString(O, string);
            break;
            
        case 'y':
            // Go - get default command from preference object
            s = sprintf(sval1, "vol p 2 140 180");
            for (k = 4; k < 20; k += 2) {
                s += sprintf(sval1 + s, "/p %d 140 180", k);
            }
            s += sprintf(sval1 + s, "/p 20 140,120 180" RKEOL);
            user->radar->pedestalExec(user->radar->pedestal, sval1, string);
            RKOperatorSendDelimitedString(O, string);
            user->radar->transceiverExec(user->radar->transceiver, "y", string);
            RKOperatorSendDelimitedString(O, string);
            break;
            
        case 'z':
            // Stop everything
            
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

    int gid;
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
    RKInt16C *userDataH = NULL;
    RKInt16C *userDataV = NULL;

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
        if (user->streams & RKUserFlagStatusEngines) {
            k = snprintf(user->string, RKMaximumStringLength - 1, "Pos:0x%02x/%04d  Pul:0x%02x/%05d  Mom:0x%02x/%04d  Hea:0x%02x/%02d  Swe:0x%02x  Fil:0x%02x" RKEOL,
                         user->radar->positionEngine->state,
                         user->radar->positionIndex,
                         user->radar->pulseCompressionEngine->state,
                         user->radar->pulseIndex,
                         user->radar->momentEngine->state,
                         user->radar->rayIndex,
                         user->radar->healthEngine->state,
                         user->radar->healthIndex,
                         user->radar->sweepEngine->state,
                         user->radar->fileEngine->state);
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
        endIndex = RKPreviousNModuloS(user->radar->healthIndex, 1, user->radar->desc.healthBufferDepth);
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
        // If I/Q data is sent, there is no need to send another subset of it.
        endIndex = RKPreviousModuloS(user->radar->pulseIndex, user->radar->desc.pulseBufferDepth);
        while (user->pulseIndex != endIndex) {

            user->pulseIndex = RKNextModuloS(user->pulseIndex, user->radar->desc.pulseBufferDepth);
        }
    } else if (user->streams & user->access & RKUserFlagDisplayIQ && time - user->timeLastDisplayIQOut >= 0.05) {
        endIndex = RKPreviousNModuloS(user->radar->pulseIndex, 2 * user->radar->pulseCompressionEngine->coreCount, user->radar->desc.pulseBufferDepth);
        pulse = RKGetPulse(user->radar->pulses, endIndex);
        memcpy(&pulseHeader, &pulse->header, sizeof(RKPulseHeader));

        c16DataH = RKGetInt16CDataFromPulse(pulse, 0);
        c16DataV = RKGetInt16CDataFromPulse(pulse, 1);

        userDataH = user->samples[0];
        userDataV = user->samples[1];

        RKComplex *yH;
        RKComplex *yV;

        // Default stride: k = 1
        k = 1;
        gid = pulse->header.i % user->radar->pulseCompressionEngine->filterGroupCount;
        switch (engine->developerInspect) {
            case 3:
                // Show the filter that was used
                pulseHeader.gateCount = 1000;
                i = 0;
                for (k = 0; k < MIN(200, user->radar->pulseCompressionEngine->anchors[gid][0].length + 10); k++) {
                    *userDataH++ = *c16DataH++;
                    *userDataV++ = *c16DataV++;
                    i++;
                }

                yH = user->radar->pulseCompressionEngine->filters[gid][0];
                yV = user->radar->pulseCompressionEngine->filters[gid][0];
                for (k = 0; k < MIN(200, user->radar->pulseCompressionEngine->anchors[gid][0].length); k++) {
                    userDataH->i   = (int16_t)(10000.0f * yH->i);
                    userDataH++->q = (int16_t)(10000.0f * yH++->q);

                    userDataV->i   = (int16_t)(10000.0f * yV->i);
                    userDataV++->q = (int16_t)(10000.0f * yV++->q);
                    i++;
                }

                // The second part of is the processed data
                yH = RKGetComplexDataFromPulse(pulse, 0);
                yV = RKGetComplexDataFromPulse(pulse, 1);
                for (; i < pulseHeader.gateCount; i++) {
                    userDataH->i   = (int16_t)(0.04f * yH->i);
                    userDataH++->q = (int16_t)(0.04f * yH++->q);

                    userDataV->i   = (int16_t)(0.04f * yV->i);
                    userDataV++->q = (int16_t)(0.04f * yV++->q);
                }
                break;

            case 2:
                k = user->pulseDownSamplingRatio;

                pulseHeader.gateCount /= k;
                pulseHeader.gateSizeMeters *= (float)k;

                yH = RKGetComplexDataFromPulse(pulse, 0);
                yV = RKGetComplexDataFromPulse(pulse, 1);
                for (i = 0; i < pulseHeader.gateCount; i++) {
                    userDataH->i   = (int16_t)(0.04f * yH->i);
                    userDataH++->q = (int16_t)(0.04f * yH->q);
                    yH += k;

                    userDataV->i   = (int16_t)(0.04f * yV->i);
                    userDataV++->q = (int16_t)(0.04f * yV->q);
                    yV += k;
                }
                break;
                
            case 1:
                k = user->pulseDownSamplingRatio;

            default:
                pulseHeader.gateCount /= k;
                pulseHeader.gateCount = MIN(pulseHeader.gateCount, 1000);
                pulseHeader.gateSizeMeters *= (float)k;
                for (i = 0; i < pulseHeader.gateCount; i++) {
                    *userDataH++ = *c16DataH;
                    *userDataV++ = *c16DataV;

                    c16DataH += k;
                    c16DataV += k;
                }
                break;
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
    RKLog(">%s %s User[%d]   Pul x %d   Ray x %d ...\n", engine->name, O->name, O->iid, user->pulseDownSamplingRatio, user->rayDownSamplingRatio);

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
    engine->developerInspect = 3;
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
        RKLog("%s Stopping ...\n", center->name);
    }
    RKServerStop(center->server);
    RKLog("%s Stopped.\n", center->name);
}
