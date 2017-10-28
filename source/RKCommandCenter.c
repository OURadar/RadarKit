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
int socketInitialHandler(RKOperator *);
int socketTerminateHandler(RKOperator *);

#pragma mark - Helper Functions

static void consolidateStreams(RKCommandCenter *engine) {

    int j, k;
    char string[RKNameLength];
    RKStream consolidatedStreams;
    
    // Consolidate streams
    for (j = 0; j < RKCommandCenterMaxRadars; j++) {
        RKRadar *radar = engine->radars[j];
        if (radar == NULL) {
            continue;
        }
        if (radar->desc.initFlags & RKInitFlagSignalProcessor) {
            continue;
        }
        consolidatedStreams = RKStreamNull;
        for (k = 0; k < RKCommandCenterMaxConnections; k++) {
            RKUser *user = &engine->users[k];
            if (radar == user->radar) {
                consolidatedStreams |= user->streams;
            }
        }
        if (engine->relayStreams != consolidatedStreams) {
            engine->relayStreams = consolidatedStreams;
            string[0] = 's';
            RKFlagToString(string + 1, engine->relayStreams);
            RKLog("%s streams -> 0x%08x '%s'\n", engine->name, engine->relayStreams, string);
            RKRadarRelayExec(radar->radarRelay, string, string);
        } else {
            RKLog("%s streams remain 0x%08x\n", engine->name, engine->relayStreams);
        }
    }
}

int socketCommandHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    RKConfig *config = RKGetLatestConfig(user->radar);
    
    int j, k;
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
    
    double fval1, fval2;
    
    // Delimited reading: each command is separated by a ';'
    // e.g., a radarkit nopasswd;s hz;h hv on;
    
    char *commandString = O->cmd;
    char *commandStringEnd = NULL;

    RKStripTail(commandString);

    RKStream newStream;

    while (commandString != NULL) {
        if ((commandStringEnd = strchr(commandString, ';')) != NULL) {
            *commandStringEnd = '\0';
        }
        // Command 'ping' is most frequent, check this first
        if (!strncmp(commandString, "ping", 4)) {
            user->pingCount++;
            if (engine->verbose && user->pingCount % 100 == 0) {
                RKLog("%s %s ping x %s\n", engine->name, O->name, RKIntegerToCommaStyleString(user->pingCount));
            }
            // There is no need to send a response. The delegate function socketStreamHandler sends a beacon periodically
        } else if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
            user->commandCount++;
            RKLog("%s %s Command '%s'\n", engine->name, O->name, commandString);
            // Process the command
            switch (commandString[0]) {
                case 'a':
                    // Authenticate
                    sscanf(commandString + 1, "%s %s", sval1, sval2);
                    RKLog(">%s %s Authenticating %s %s ... (%d) (%d)\n", engine->name, O->name, sval1, sval2, strlen(sval1), sizeof(user->login));
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

                    RKMakeJSONStringFromControls(sval1, user->radar->controls, RKControlCount);
                    j += sprintf(string + j, "\"Controls\":["
                                 "{\"Label\":\"Go\", \"Command\":\"y\"}, "
                                 "{\"Label\":\"Stop\", \"Command\":\"z\"}, "
                                 "%s"
                                 "]}" RKEOL, sval1);
                    O->delimTx.type = RKNetworkPacketTypeControls;
                    O->delimTx.size = j;
                    RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), string, O->delimTx.size, NULL);
                    break;
                    
                case 'd':
                    // DSP related
                    switch (commandString[1]) {
                        case 'f':
                            // 'df' - DSP filter
                            break;
                        case 'n':
                            // 'dn' - DSP noise override
                            k = sscanf(&commandString[2], "%lf %lf", &fval1, &fval2);
                            if (k == 2) {
                                RKAddConfig(user->radar, RKConfigKeyNoise, fval1, fval2, RKConfigKeyNull);
                                sprintf(string, "ACK. Noise set to %.4f, %.4f" RKEOL, fval1, fval2);
                            } else if (k == -1) {
                                sprintf(string, "ACK. Current noise is %.4f %.4f" RKEOL, config->noise[0], config->noise[1]);
                            } else {
                                sprintf(string, "NAK. Must have two paramters  (k = %d)." RKEOL, k);
                            }
                            RKOperatorSendCommandResponse(O, string);
                            break;
                        case 'N':
                            // 'dN' - DSP noise override in dB
                            break;
                        case 'r':
                            // 'dr' - Restart DSP engines
                            RKMomentEngineStop(user->radar->momentEngine);
                            RKPulseCompressionEngineStop(user->radar->pulseCompressionEngine);
                            RKPulseCompressionEngineStart(user->radar->pulseCompressionEngine);
                            RKMomentEngineStart(user->radar->momentEngine);
                            sprintf(string, "ACK. %s and %s restarted." RKEOL,
                                    user->radar->pulseCompressionEngine->name, user->radar->momentEngine->name);
                            break;
                        case 't':
                            // 'dt' - DSP threshold in SNR dB
                            k = sscanf(&commandString[2], "%lf", &fval1);
                            if (k == 1) {
                                RKAddConfig(user->radar, RKConfigKeySNRThreshold, fval1, RKConfigKeyNull);
                                sprintf(string, "ACK. SNR threshold set to %.2f dB" RKEOL, fval1);
                            } else {
                                sprintf(string, "ACK. Current SNR threshold is %.2f dB" RKEOL, config->SNRThreshold);
                            }
                            RKOperatorSendCommandResponse(O, string);
                            break;
                        default:
                            break;
                    }
                    break;
                    
                case 'h':
                    if (strlen(commandString) == 1 || !strncmp(commandString, "help", 4)) {
                        k = sprintf(string,
                                    "Help\n"
                                    "====\n"
                                    "\n"
                                    HIGHLIGHT("a") " [USERNAME] [ENCRYPTED_PASSWORD] - Authenticate\n"
                                    "\n"
                                    HIGHLIGHT("f") " [FILTER_INDEX] - DSP filters,\n"
                                    "    where index can be (coming soon):\n"
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
                            k += RKIndentCopy(string + k, sval1);
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
                            k += RKIndentCopy(string + k, sval1);
                            k += sprintf(string + k, "\n\n");
                        } else {
                            k += sprintf(string + k, "    INFO: Pedestal not set.\n");
                        }
                        
                        k += sprintf(string + k,
                                     HIGHLIGHT("h") " - " UNDERLINE_ITALIC ("Health Relay") " commands, everything that starts with p goes to the health relay\n"
                                     "    module in a concatenated form, e.g., 'p help' -> 'help' to the health relay.\n\n");
                        if (user->radar->healthRelay) {
                            user->radar->healthRelayExec(user->radar->healthRelay, "help", sval1);
                            RKStripTail(sval1);
                            k += RKIndentCopy(string + k, sval1);
                            k += sprintf(string + k, "\n\n");
                        } else {
                            k += sprintf(string + k, "    INFO: Health Relay not set.\n");
                        }
                        
                        sprintf(string + k, "\n== (%s) ==" RKEOL, RKIntegerToCommaStyleString(k));
                        
                        RKOperatorSendDelimitedString(O, string);
                        
                    } else {
                        
                        // Forward to health relay
                        if (strlen(commandString) < 2) {
                            sprintf(string, "NAK. Empty command to pedestal." RKEOL);
                            RKOperatorSendCommandResponse(O, string);
                            break;
                        }
                        k = 1;
                        while (commandString[k] == ' ') {
                            k++;
                        }
                        user->radar->healthRelayExec(user->radar->healthRelay, commandString + k, string);
                        RKOperatorSendCommandResponse(O, string);
                    }
                    break;

                case 'i':
                    O->delimTx.type = RKNetworkPacketTypeRadarDescription;
                    O->delimTx.size = (uint32_t)sizeof(RKRadarDesc);
                    RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &user->radar->desc, sizeof(RKRadarDesc), NULL);
                    break;

                case 'm':
                    RKLog(">%s %s display data\n", engine->name, O->name);
                    user->streams |= RKStreamDisplayZ;
                    user->rayIndex = RKPreviousModuloS(user->radar->rayIndex, user->radar->desc.rayBufferDepth);
                    break;

                case 'p':
                    // Pass everything to pedestal
                    if (strlen(commandString) < 2) {
                        sprintf(string, "NAK. Empty command to pedestal." RKEOL);
                        RKOperatorSendCommandResponse(O, string);
                        break;
                    }
                    k = 0;
                    do {
                        k++;
                    } while (commandString[k] == ' ');
                    user->radar->pedestalExec(user->radar->pedestal, commandString + k, string);
                    RKOperatorSendCommandResponse(O, string);
                    break;
                
                case 'q':
                    sprintf(string, "Bye." RKEOL);
                    RKOperatorSendCommandResponse(O, string);
                    RKOperatorHangUp(O);
                    break;

                case 'r':
                    user->radar->dataRecorder->doNotWrite = !user->radar->dataRecorder->doNotWrite;
                    sprintf(string, "ACK. IQ data recorder set to %s." RKEOL, user->radar->dataRecorder->doNotWrite ? "standby" : "active");
                    RKOperatorSendCommandResponse(O, string);
                    break;

                case 's':
                    // Stream varrious data
                    newStream = RKStringToFlag(commandString + 1);
                    k = user->rayIndex;
                    pthread_mutex_lock(&user->mutex);
                    user->streamsInProgress = RKStreamNull;
                    user->streams = newStream;
                    user->rayStatusIndex = RKPreviousModuloS(user->radar->momentEngine->rayStatusBufferIndex, RKBufferSSlotCount);
                    pthread_mutex_unlock(&user->mutex);
                    sprintf(string, "{\"access\": 0x%lx, \"streams\": 0x%lx, \"indices\":[%d,%d]}" RKEOL,
                            (unsigned long)user->access, (unsigned long)user->streams, k, user->rayIndex);
                    RKOperatorSendCommandResponse(O, string);
                    break;
                    
                case 't':
                    // Pass everything to transceiver
                    if (strlen(commandString) < 2) {
                        sprintf(string, "NAK. Empty command to transceiver." RKEOL);
                        RKOperatorSendCommandResponse(O, string);
                        break;
                    }
                    k = 1;
                    while (commandString[k] == ' ') {
                        k++;
                    }
                    user->radar->transceiverExec(user->radar->transceiver, commandString + k, string);
                    RKOperatorSendCommandResponse(O, string);
                    break;
                    
                case 'x':
                    //engine->developerInspect = RKNextModuloS(engine->developerInspect, 4);
                    user->ascopeMode = RKNextModuloS(engine->developerInspect, 4);
                    sprintf(string, "ACK. Developer inspect set to %d" RKEOL, engine->developerInspect);
                    RKOperatorSendCommandResponse(O, string);
                    break;
                    
                case 'b':  // Button event
                case 'y':  // Start everything
                case 'z':  // Stop everything
                    // Passed to the master controller
                    if (user->radar->masterController == NULL) {
                        sprintf(string, "NAK. Not ready." RKEOL);
                    } else {
                        user->radar->masterControllerExec(user->radar->masterController, commandString, string);
                    }
                    RKOperatorSendCommandResponse(O, string);
                    break;
                    
                default:
                    snprintf(string, RKMaximumStringLength - 1, "Unknown command '%s'." RKEOL, commandString);
                    RKOperatorSendCommandResponse(O, string);
                    break;
            }
        } else {
            switch (commandString[0]) {
                case 'a':
                    RKLog("%s %s Queue command '%s' to relay.\n", engine->name, O->name, commandString);
                    RKRadarRelayExec(user->radar->radarRelay, commandString, string);
                    O->delimTx.type = RKNetworkPacketTypeControls;
                    O->delimTx.size = (uint32_t)strlen(string);
                    RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), string, O->delimTx.size, NULL);

                case 'r':
                    sscanf("%s", commandString + 1, sval1);
                    RKLog(">%s %s selected radar %s\n", engine->name, O->name, sval1);
                    snprintf(string, RKMaximumStringLength - 1, "ACK. %s selected." RKEOL, sval1);
                    RKOperatorSendCommandResponse(O, string);
                    break;
                    
                case 's':
                    // Stream varrious data
                    user->streams = RKStringToFlag(commandString + 1);
                    
                    consolidateStreams(engine);

                    k = user->rayIndex;
                    pthread_mutex_lock(&user->mutex);
                    user->streamsInProgress = RKStreamNull;
                    pthread_mutex_unlock(&user->mutex);
                    RKLog(">%s %s reset progress.\n", engine->name, O->name);
                    sprintf(string, "{\"access\": 0x%lx, \"streams\": 0x%lx, \"indices\":[%d,%d]}" RKEOL,
                            (unsigned long)user->access, (unsigned long)user->streams, k, user->rayIndex);
                    RKOperatorSendCommandResponse(O, string);
                    break;
                    
                default:
                    // Just forward to the right radar
                    RKLog("%s %s Queue command '%s' to relay.\n", engine->name, O->name, commandString);
                    RKRadarRelayExec(user->radar->radarRelay, commandString, string);
                    RKOperatorSendCommandResponse(O, string);
                    break;
            }
        }
        // Get to the next command
        if (commandStringEnd != NULL) {
            commandString = commandStringEnd + 1;
            // Strip out some space after ';'
            while (*commandString == '\r' || *commandString == '\n' || *commandString == ' ') {
                commandString++;
            }
            if (*commandString == '\0') {
                commandString = NULL;
            }
        } else {
            commandString = NULL;
        }
    } // while (commandString != NULL) ...
    
    return 0;
}

int socketStreamHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    
    int i, j, k, s;
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
    float *f32Data = NULL;

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

    if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
        // Modes "1", "2", "3" and "4" are for signal processor only - showing the latest summary text view
        if (user->streams & user->access && td >= 0.05) {
            // Stream "1" - Overall status
            if (user->streams & RKStreamStatusPulses) {
                k = snprintf(user->string, RKMaximumStringLength - 1, "%s | %s | %s | %s |" RKEOL,
                             RKPulseCompressionEngineStatusString(user->radar->pulseCompressionEngine),
                             RKPositionEngineStatusString(user->radar->positionEngine),
                             RKMomentEngineStatusString(user->radar->momentEngine),
                             RKDataRecorderStatusString(user->radar->dataRecorder));
                O->delimTx.type = RKNetworkPacketTypePlainText;
                O->delimTx.size = k + 1;
                RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
                user->timeLastOut = time;
            }
            // Stream "3" - Positions
            if (user->streams & RKStreamStatusPositions) {
                k = snprintf(user->string, RKMaximumStringLength - 1, "%s" RKEOL,
                             RKPositionEnginePositionString(user->radar->positionEngine));
                O->delimTx.type = RKNetworkPacketTypePlainText;
                O->delimTx.size = k + 1;
                RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
                user->timeLastOut = time;
            }
            // Stream "4" - Internal Engines
            if (user->streams & RKStreamStatusEngines) {
                k = snprintf(user->string, RKMaximumStringLength - 1, "Pos:0x%03x/%04d  Pul:0x%03x/%05d  Mom:0x%03x/%04d  Hea:0x%03x/%02d  Swe:0x%03x  Fil:0x%03x" RKEOL,
                             user->radar->positionEngine->state,
                             user->radar->positionIndex,
                             user->radar->pulseCompressionEngine->state,
                             user->radar->pulseIndex,
                             user->radar->momentEngine->state,
                             user->radar->rayIndex,
                             user->radar->healthEngine->state,
                             user->radar->healthIndex,
                             user->radar->sweepEngine->state,
                             user->radar->dataRecorder->state);
                O->delimTx.type = RKNetworkPacketTypePlainText;
                O->delimTx.size = k + 1;
                RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
                user->timeLastOut = time;
            }
        }
        
        // Stream "2" - Level-II / Moment status - no skipping
        if (user->streams & user->access & RKStreamStatusRays) {
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
    }

    pthread_mutex_lock(&user->mutex);

    // For contiguous streaming:
    // If we just started a connection, grab the payload that is either:
    // 1) Up to latest available:
    //      i) For a health, it is radar->healthIndex - 1
    //     ii) For a ray, it is radar->rayIndex - (number of workers)
    //    iii) For a pulse, it is readar->pulseIndex - (number of workers)
    // 2) The latest slot it will be stored. It is crucial to ensure that:
    //      i) For a health, it is RKStatusReady
    //     ii) For a ray, it has RKRayStatusReady set
    //    iii) For a pulse, it has RKPulseStatusReadyForMoment set
    // 3) Once the first payload is sent, the stream is consider in progress (streamsInProgress)
    // 4) If (2) can't be met within 2 secs, in progress flag is not set so (2) will be checked
    //    again in the next iteraction.

    // Processor Status
    if (user->streams & user->access & RKStreamStatusProcessorStatus) {
        endIndex = RKPreviousModuloS(user->radar->statusIndex, user->radar->desc.statusBufferDepth);
        if (!(user->streamsInProgress & RKStreamStatusProcessorStatus)) {
            if (engine->verbose) {
                RKLog("%s Fast forward RKStatus -> %d (%s).\n", engine->name, endIndex,
                      user->radar->status[user->radar->statusIndex].flag == RKStatusFlagVacant ? "vacant" : "ready");
            }
            user->statusIndex = endIndex;
        }
        s = 0;
        while (!(user->radar->status[user->statusIndex].flag == RKStatusFlagReady) && engine->server->state == RKServerStateActive && s++ < 20) {
            if (s % 10 == 0 && engine->verbose > 1) {
                RKLog("%s %s sleep 0/%.1f s  RKStatus\n", engine->name, O->name, s * 0.1f);
            }
            usleep(100000);
        }
        if (user->radar->status[user->statusIndex].flag == RKStatusFlagReady && engine->server->state == RKServerStateActive) {
            user->streamsInProgress |= RKStreamStatusProcessorStatus;
            while (user->statusIndex != endIndex) {
                O->delimTx.type = RKNetworkPacketTypeProcessorStatus;
                O->delimTx.size = (uint32_t)sizeof(RKStatus);
                RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &user->radar->status[user->statusIndex], sizeof(RKStatus), NULL);
                user->statusIndex = RKNextModuloS(user->statusIndex, user->radar->desc.statusBufferDepth);
            }
        } else {
            printf("No Status / Deactivated.\n");
        }
    }
    
    // Health Status
    if (user->streams & user->access & RKStreamStatusHealth) {
        endIndex = RKPreviousModuloS(user->radar->healthIndex, user->radar->desc.healthBufferDepth);
        if (!(user->streamsInProgress & RKStreamStatusHealth)) {
            if (engine->verbose) {
                RKLog("%s Fast forward RKHealth -> %d (%s).\n", engine->name, endIndex,
                      user->radar->healths[user->radar->healthIndex].flag == RKStatusFlagVacant ? "vacant" : "ready");
            }
            user->healthIndex = endIndex;
        }
        s = 0;
        while (!(user->radar->healths[user->healthIndex].flag == RKHealthFlagReady) && engine->server->state == RKServerStateActive && s++ < 20) {
            if (s % 10 == 0 && engine->verbose > 1) {
                RKLog("%s %s sleep 0/%.1f s  RKHealth\n", engine->name, O->name, s * 0.1f);
            }
            usleep(100000);
        }
        if (user->radar->healths[user->healthIndex].flag == RKHealthFlagReady && engine->server->state == RKServerStateActive) {
            user->streamsInProgress |= RKStreamStatusHealth;
            j = 0;
            k = 0;
            while (user->healthIndex != endIndex && k < RKMaximumStringLength - 200) {
                c = user->radar->healths[user->healthIndex].string;
                k += sprintf(user->string + k, "%s\n", c);
                user->healthIndex = RKNextModuloS(user->healthIndex, user->radar->desc.healthBufferDepth);
                j++;
            }
            if (j) {
                // Take out the last '\n', replace it with somethign else + EOL
                snprintf(user->string + k - 1, RKMaximumStringLength - k - 1, "" RKEOL);
                O->delimTx.type = RKNetworkPacketTypeHealth;
                O->delimTx.size = k;
                RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
            }
        } else {
            printf("No Health / Deactivated.\n");
        }
    }

    // Product or display streams - no skipping
    if (user->streams & user->access & RKStreamProductZVWDPRKS) {
        // Product streams - assume no display as display data can be derived later
        if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
            endIndex = RKPreviousNModuloS(user->radar->rayIndex, 2 * user->radar->momentEngine->coreCount, user->radar->desc.rayBufferDepth);
        } else {
            endIndex = RKPreviousModuloS(user->radar->rayIndex, user->radar->desc.rayBufferDepth);
        }
        ray = RKGetRay(user->radar->rays, endIndex);

        if (!(user->streamsInProgress & RKStreamProductZVWDPRKS)) {
            user->rayIndex = endIndex;
            s = 0;
            while (!(ray->header.s & RKRayStatusReady) && engine->server->state == RKServerStateActive && s++ < 20) {
                if (s % 10 == 0 && engine->verbose > 1) {
                    RKLog("%s %s sleep 0/%.1f s  RKRay\n", engine->name, O->name, s * 0.1f);
                }
                usleep(100000);
            }
        }

        if (ray->header.s & RKRayStatusReady && engine->server->state == RKServerStateActive) {
            if (!(user->streamsInProgress & RKStreamProductZVWDPRKS)) {
                user->streamsInProgress |= (user->streams & RKStreamProductZVWDPRKS);
            }
            while (user->rayIndex != endIndex) {
                ray = RKGetRay(user->radar->rays, user->rayIndex);
                // Duplicate and send the header with only selected products
                memcpy(&rayHeader, &ray->header, sizeof(RKRayHeader));
                // Gather the products to be sent
                rayHeader.productList = RKProductListNone;
                if (user->streams & RKStreamProductZ) {
                    rayHeader.productList |= RKProductListProductZ;
                }
                if (user->streams & RKStreamProductV) {
                    rayHeader.productList |= RKProductListProductV;
                }
                if (user->streams & RKStreamProductW) {
                    rayHeader.productList |= RKProductListProductW;
                }
                if (user->streams & RKStreamProductD) {
                    rayHeader.productList |= RKProductListProductD;
                }
                if (user->streams & RKStreamProductP) {
                    rayHeader.productList |= RKProductListProductP;
                }
                if (user->streams & RKStreamProductR) {
                    rayHeader.productList |= RKProductListProductR;
                }
                if (user->streams & RKStreamProductK) {
                    rayHeader.productList |= RKProductListProductK;
                }
                if (user->streams & RKStreamProductS) {
                    rayHeader.productList |= RKProductListProductS;
                }
                uint32_t productList = rayHeader.productList & RKProductListProductZVWDPRK;
                uint32_t productCount = __builtin_popcount(productList);
                //RKLog("ProductCount = %d / %x\n", productCount, productList);
                
                rayHeader.gateCount /= user->rayDownSamplingRatio;
                rayHeader.gateSizeMeters *= (float)user->rayDownSamplingRatio;
                
                O->delimTx.type = RKNetworkPacketTypeRayDisplay;
                O->delimTx.size = (uint32_t)(sizeof(RKRayHeader) + productCount * rayHeader.gateCount * sizeof(float));
                RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &rayHeader, sizeof(RKRayHeader), NULL);
                
                for (j = 0; j < productCount; j++) {
                    if (productList & RKProductListProductZ) {
                        productList ^= RKProductListProductZ;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexZ);
                    } else if (productList & RKProductListProductV) {
                        productList ^= RKProductListProductV;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexV);
                    } else if (productList & RKProductListProductW) {
                        productList ^= RKProductListProductW;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexW);
                    } else if (productList & RKProductListProductD) {
                        productList ^= RKProductListProductD;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexD);
                    } else if (productList & RKProductListProductP) {
                        productList ^= RKProductListProductP;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexP);
                    } else if (productList & RKProductListProductR) {
                        productList ^= RKProductListProductR;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexR);
                    } else if (productList & RKProductListProductK) {
                        productList ^= RKProductListProductK;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexK);
                    } else if (productList & RKProductListProductS) {
                        productList ^= RKProductListProductS;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexS);
                    } else {
                        f32Data = NULL;
                    }
                    
                    if (f32Data) {
                        float *lowRateData = (float *)user->string;
                        for (i = 0, k = 0; i < rayHeader.gateCount; i++, k += user->rayDownSamplingRatio) {
                            lowRateData[i] = f32Data[k];
                        }
                        RKOperatorSendPackets(O, lowRateData, rayHeader.gateCount * sizeof(float), NULL);
                    }
                }
                user->rayIndex = RKNextModuloS(user->rayIndex, user->radar->desc.rayBufferDepth);
            }
        } else {
            if ((int)ray->header.i > 0) {
                RKLog("%s %s No Ray / Deactivated.  streamsInProgress = 0x%08x\n",
                      engine->name, O->name, user->streamsInProgress);
            }
        }
    } else if (user->streams & user->access & RKStreamDisplayZVWDPRKS) {
        // Display streams - no skipping
        if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
            endIndex = RKPreviousNModuloS(user->radar->rayIndex, 2 * user->radar->momentEngine->coreCount, user->radar->desc.rayBufferDepth);
        } else {
            endIndex = RKPreviousModuloS(user->radar->rayIndex, user->radar->desc.rayBufferDepth);
        }
        if (endIndex >= user->radar->desc.rayBufferDepth) {
            RKLog("%s Error. endIndex = %s > %s\n", engine->name, RKIntegerToCommaStyleString(endIndex), RKIntegerToCommaStyleString(user->radar->desc.rayBufferDepth));
            RKLog("%s user->radar->rayIndex = %s / user->radar->desc.rayBufferDepth = %s", engine->name,
                  RKIntegerToCommaStyleString(user->radar->rayIndex),
                  RKIntegerToCommaStyleString(user->radar->desc.rayBufferDepth));
        }
        ray = RKGetRay(user->radar->rays, endIndex);

        if (!(user->streamsInProgress & RKStreamDisplayZVWDPRKS)) {
            user->rayIndex = endIndex;
            s = 0;
            while (!(ray->header.s & RKRayStatusReady) && engine->server->state == RKServerStateActive && s++ < 20) {
                if (s % 10 == 0 && engine->verbose > 1) {
                    RKLog("%s %s sleep 0/%.1f s  RKRay\n", engine->name, O->name, s * 0.1f);
                }
                usleep(100000);
            }
        }

        if (ray->header.s & RKRayStatusReady && engine->server->state == RKServerStateActive) {
            if (!(user->streamsInProgress & RKStreamDisplayZVWDPRKS)) {
                user->streamsInProgress |= (user->streams & RKStreamDisplayZVWDPRKS);
            }
            while (user->rayIndex != endIndex) {
                ray = RKGetRay(user->radar->rays, user->rayIndex);
                // Duplicate and send the header with only selected products
                memcpy(&rayHeader, &ray->header, sizeof(RKRayHeader));
                // Gather the products to be sent
                rayHeader.productList = RKProductListNone;
                if (user->streams & RKStreamDisplayZ) {
                    rayHeader.productList |= RKProductListDisplayZ;
                }
                if (user->streams & RKStreamDisplayV) {
                    rayHeader.productList |= RKProductListDisplayV;
                }
                if (user->streams & RKStreamDisplayW) {
                    rayHeader.productList |= RKProductListDisplayW;
                }
                if (user->streams & RKStreamDisplayD) {
                    rayHeader.productList |= RKProductListDisplayD;
                }
                if (user->streams & RKStreamDisplayP) {
                    rayHeader.productList |= RKProductListDisplayP;
                }
                if (user->streams & RKStreamDisplayR) {
                    rayHeader.productList |= RKProductListDisplayR;
                }
                if (user->streams & RKStreamDisplayK) {
                    rayHeader.productList |= RKProductListDisplayK;
                }
                if (user->streams & RKStreamDisplayS) {
                    rayHeader.productList |= RKProductListDisplayS;
                }
                uint32_t displayList = rayHeader.productList & RKProductListDisplayZVWDPRKS;
                uint32_t displayCount = __builtin_popcount(displayList);
                //RKLog("displayCount = %d / %x\n", productCount, productList);

                rayHeader.gateCount /= user->rayDownSamplingRatio;
                rayHeader.gateSizeMeters *= (float)user->rayDownSamplingRatio;

                O->delimTx.type = RKNetworkPacketTypeRayDisplay;
                O->delimTx.size = (uint32_t)(sizeof(RKRayHeader) + displayCount * rayHeader.gateCount * sizeof(uint8_t));
                RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &rayHeader, sizeof(RKRayHeader), NULL);

                for (j = 0; j < displayCount; j++) {
                    if (displayList & RKProductListDisplayZ) {
                        displayList ^= RKProductListDisplayZ;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexZ);
                    } else if (displayList & RKProductListDisplayV) {
                        displayList ^= RKProductListDisplayV;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexV);
                    } else if (displayList & RKProductListDisplayW) {
                        displayList ^= RKProductListDisplayW;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexW);
                    } else if (displayList & RKProductListDisplayD) {
                        displayList ^= RKProductListDisplayD;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexD);
                    } else if (displayList & RKProductListDisplayP) {
                        displayList ^= RKProductListDisplayP;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexP);
                    } else if (displayList & RKProductListDisplayR) {
                        displayList ^= RKProductListDisplayR;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexR);
                    } else if (displayList & RKProductListDisplayK) {
                        displayList ^= RKProductListDisplayK;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexK);
                    } else if (displayList & RKProductListDisplayS) {
                        displayList ^= RKProductListDisplayS;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexS);
                    } else {
                        u8Data = NULL;
                    }

                    if (u8Data) {
                        uint8_t *lowRateData = (uint8_t *)user->string;
                        for (i = 0, k = 0; i < rayHeader.gateCount; i++, k += user->rayDownSamplingRatio) {
                            lowRateData[i] = u8Data[k];
                        }
                        RKOperatorSendPackets(O, lowRateData, rayHeader.gateCount * sizeof(uint8_t), NULL);
                    }
                }
                user->rayIndex = RKNextModuloS(user->rayIndex, user->radar->desc.rayBufferDepth);
            }
        } else {
            if ((int)ray->header.i > 0) {
                RKLog("%s %s No Ray / Deactivated.  streamsInProgress = 0x%08x\n",
                      engine->name, O->name, user->streamsInProgress);
            }
        }
    }

    // IQ
    if (user->streams & user->access & RKStreamProductIQ) {
        // If I/Q data is sent, there is no need to send another subset of it.
        endIndex = RKPreviousModuloS(user->radar->pulseIndex, user->radar->desc.pulseBufferDepth);
        while (user->pulseIndex != endIndex) {

            user->pulseIndex = RKNextModuloS(user->pulseIndex, user->radar->desc.pulseBufferDepth);
        }
        user->timeLastDisplayIQOut = time;
    } else if (user->streams & user->access & RKStreamDisplayIQ && time - user->timeLastDisplayIQOut >= 0.05) {
        if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
            endIndex = RKPreviousNModuloS(user->radar->pulseIndex, 2 * user->radar->pulseCompressionEngine->coreCount, user->radar->desc.pulseBufferDepth);
        } else {
            endIndex = RKPreviousModuloS(user->radar->pulseIndex, user->radar->desc.pulseBufferDepth);
        }
        pulse = RKGetPulse(user->radar->pulses, endIndex);

        if (!(user->streamsInProgress & RKStreamDisplayIQ)) {
            user->pulseIndex = endIndex;
            s = 0;
            while (!(pulse->header.s & RKPulseStatusHasIQData) && engine->server->state == RKServerStateActive && s++ < 20) {
                if (s % 10 == 0) {
                    RKLog("%s %s sleep 0/%.1f s  RKPulse\n", engine->name, O->name, s * 0.1f);
                }
                usleep(100000);
            }
        }

        if (pulse->header.s & RKPulseStatusHasIQData && engine->server->state == RKServerStateActive) {
            user->streamsInProgress |= RKStreamDisplayIQ;
            memcpy(&pulseHeader, &pulse->header, sizeof(RKPulseHeader));
            c16DataH = RKGetInt16CDataFromPulse(pulse, 0);
            c16DataV = RKGetInt16CDataFromPulse(pulse, 1);
            userDataH = user->samples[0];
            userDataV = user->samples[1];
            RKComplex *yH;
            RKComplex *yV;

            // Default stride: k = 1
            k = 1;
            switch (user->ascopeMode) {
                case 3:
                    // Show the waveform that was used through the forward sampling path
                    pulseHeader.gateCount = 1000;
                    if (!(user->radar->desc.initFlags & RKInitFlagSignalProcessor)) {
                        break;
                    }
                    i = 0;
                    gid = pulse->header.i % user->radar->pulseCompressionEngine->filterGroupCount;
                    for (k = 0; k < MIN(200, user->radar->pulseCompressionEngine->filterAnchors[gid][0].length); k++) {
                        *userDataH++ = *c16DataH++;
                        *userDataV++ = *c16DataV++;
                        i++;
                    }
                    for (; k < MIN(203, user->radar->pulseCompressionEngine->filterAnchors[gid][0].length + 3); k++) {
                        userDataH->i   = 0;
                        userDataH++->q = 0;
                        userDataV->i   = 0;
                        userDataV++->q = 0;
                        i++;
                    }
                    // Show the filter that was used as matched filter
                    yH = user->radar->pulseCompressionEngine->filters[gid][0];
                    yV = user->radar->pulseCompressionEngine->filters[gid][0];
                    for (k = 0; k < MIN(200, user->radar->pulseCompressionEngine->filterAnchors[gid][0].length); k++) {
                        userDataH->i   = (int16_t)(300000.0f * yH->i);
                        userDataH++->q = (int16_t)(300000.0f * yH++->q);

                        userDataV->i   = (int16_t)(300000.0f * yV->i);
                        userDataV++->q = (int16_t)(300000.0f * yV++->q);
                        i++;
                    }

                    // The third part of is the processed data
                    yH = RKGetComplexDataFromPulse(pulse, 0);
                    yV = RKGetComplexDataFromPulse(pulse, 1);
                    for (; i < pulseHeader.gateCount; i++) {
                        userDataH->i   = (int16_t)(yH->i);
                        userDataH++->q = (int16_t)(yH++->q);

                        userDataV->i   = (int16_t)(yV->i);
                        userDataV++->q = (int16_t)(yV++->q);
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
                    pulseHeader.gateCount = MIN(pulseHeader.gateCount, 2000);
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
        } else {
            if ((int)pulse->header.i > 0) {
                RKLog("%s %s No IQ / Deactivated.  streamsInProgress = 0x%08x  header.s = %x\n",
                       engine->name, O->name, user->streamsInProgress, pulse->header.s);
            }
        }
    }

    pthread_mutex_unlock(&user->mutex);

    // Re-evaluate td = time - user->timeLastOut; send a heart beat if nothing has been sent
    if (time - user->timeLastOut >= 1.0) {
        user->timeLastOut = time;
        if (O->beacon.type != RKNetworkPacketTypeBeacon) {
            RKLog("Beacon has been changed %d\n", O->beacon.type);
        }
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
    user->access = RKStreamStatusAll;
    user->access |= RKStreamDisplayZVWDPRKS;
    user->access |= RKStreamProductZVWDPRKS;
    user->access |= RKStreamDisplayIQ | RKStreamProductIQ;
    user->access |= RKStreamControl;
    user->radar = engine->radars[0];
    if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
        user->rayDownSamplingRatio = (uint16_t)MAX(user->radar->desc.pulseCapacity / user->radar->desc.pulseToRayRatio / 1000, 1);
    } else {
        user->rayDownSamplingRatio = 1;
    }
    user->pulseDownSamplingRatio = (uint16_t)MAX(user->radar->desc.pulseCapacity / 1000, 1);
    user->ascopeMode = 1;
    pthread_mutex_init(&user->mutex, NULL);
    RKLog(">%s %s User[%d]   Pul x %d   Ray x %d ...\n", engine->name, O->name, O->iid, user->pulseDownSamplingRatio, user->rayDownSamplingRatio);

    snprintf(user->login, 63, "radarop");
    user->serverOperator = O;

    return RKResultNoError;
}

int socketTerminateHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    pthread_mutex_destroy(&user->mutex);
    RKLog(">%s %s User[%d]   Stream reset.\n", engine->name, O->name, O->iid);
    user->streams = RKStreamNull;
    consolidateStreams(engine);
    return RKResultNoError;
}

#pragma mark - Type Conversions

RKStream RKStringToFlag(const char * string) {
    int j = 0;
    char *c = (char *)string;
    RKStream flag = RKStreamNull;
    while (j++ < strlen(string)) {
        switch (*c) {
            case 'h':
                flag |= RKStreamStatusHealth;
                break;
            case '1':
                flag |= RKStreamStatusPulses;
                break;
            case '2':
                flag |= RKStreamStatusRays;
                break;
            case '3':
                flag |= RKStreamStatusPositions;
                break;
            case '4':
                flag |= RKStreamStatusEngines;
                break;
            case '!':
                flag |= RKStreamStatusProcessorStatus;
                break;
            case 'z':
                flag |= RKStreamDisplayZ;
                break;
            case 'Z':
                flag |= RKStreamProductZ;
                break;
            case 'v':
                flag |= RKStreamDisplayV;
                break;
            case 'V':
                flag |= RKStreamProductV;
                break;
            case 'w':
                flag |= RKStreamDisplayW;
                break;
            case 'W':
                flag |= RKStreamProductW;
                break;
            case 'd':
                flag |= RKStreamDisplayD;
                break;
            case 'D':
                flag |= RKStreamProductD;
                break;
            case 'p':
                flag |= RKStreamDisplayP;
                break;
            case 'P':
                flag |= RKStreamProductP;
                break;
            case 'r':
                flag |= RKStreamDisplayR;
                break;
            case 'R':
                flag |= RKStreamProductR;
                break;
            case 'k':
                flag |= RKStreamDisplayK;
                break;
            case 'K':
                flag |= RKStreamProductK;
                break;
            case 's':
                flag |= RKStreamDisplayS;
                break;
            case 'S':
                flag |= RKStreamProductS;
                break;
            case 'i':
                flag |= RKStreamDisplayIQ;
                break;
            case 'I':
                flag |= RKStreamProductIQ;
                break;
            default:
                break;
        }
        c++;
    }
    return flag;
}

int RKFlagToString(char *string, RKStream flag) {
    int j = 0;
    if (flag & RKStreamStatusPulses)          { j += sprintf(string + j, "1"); }
    if (flag & RKStreamStatusRays)            { j += sprintf(string + j, "2"); }
    if (flag & RKStreamStatusPositions)       { j += sprintf(string + j, "3"); }
    if (flag & RKStreamStatusEngines)         { j += sprintf(string + j, "4"); }
    if (flag & RKStreamStatusProcessorStatus) { j += sprintf(string + j, "!"); }
    if (flag & RKStreamStatusHealth)          { j += sprintf(string + j, "h"); }
    if (flag & RKStreamDisplayZ)              { j += sprintf(string + j, "z"); }
    if (flag & RKStreamProductZ)              { j += sprintf(string + j, "Z"); }
    if (flag & RKStreamDisplayV)              { j += sprintf(string + j, "v"); }
    if (flag & RKStreamProductV)              { j += sprintf(string + j, "V"); }
    if (flag & RKStreamDisplayW)              { j += sprintf(string + j, "w"); }
    if (flag & RKStreamProductW)              { j += sprintf(string + j, "W"); }
    if (flag & RKStreamDisplayD)              { j += sprintf(string + j, "d"); }
    if (flag & RKStreamProductD)              { j += sprintf(string + j, "D"); }
    if (flag & RKStreamDisplayP)              { j += sprintf(string + j, "p"); }
    if (flag & RKStreamProductP)              { j += sprintf(string + j, "P"); }
    if (flag & RKStreamDisplayR)              { j += sprintf(string + j, "r"); }
    if (flag & RKStreamProductR)              { j += sprintf(string + j, "R"); }
    if (flag & RKStreamDisplayK)              { j += sprintf(string + j, "k"); }
    if (flag & RKStreamProductK)              { j += sprintf(string + j, "K"); }
    if (flag & RKStreamDisplayS)              { j += sprintf(string + j, "s"); }
    if (flag & RKStreamProductS)              { j += sprintf(string + j, "S"); }
    if (flag & RKStreamDisplayIQ)             { j += sprintf(string + j, "i"); }
    if (flag & RKStreamProductIQ)             { j += sprintf(string + j, "I"); }
    string[j] = '\0';
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
    sprintf(engine->name, "%s<OperationCenter>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorCommandCenter) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->verbose = 3;
    engine->developerInspect = 0;
    engine->memoryUsage = sizeof(RKCommandCenter);
    engine->server = RKServerInit();
    RKServerSetName(engine->server, engine->name);
    RKServerSetWelcomeHandler(engine->server, &socketInitialHandler);
    RKServerSetCommandHandler(engine->server, &socketCommandHandler);
    RKServerSetStreamHandler(engine->server, &socketStreamHandler);
    RKServerSetTerminateHandler(engine->server, &socketTerminateHandler);
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
    for (i = 0; i < engine->server->maxClient; i++) {
        if (engine->server->busy[i] && engine->users[i].serverOperator->state == RKOperatorStateActive && engine->users[i].radar == radar) {
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
    RKLog("%s Started.   mem = %s B   radarCount = %s\n", center->name, RKIntegerToCommaStyleString(center->memoryUsage), RKIntegerToCommaStyleString(center->radarCount));
}

void RKCommandCenterStop(RKCommandCenter *center) {
    if (center->verbose > 1) {
        RKLog("%s Stopping ...\n", center->name);
    }
    RKServerStop(center->server);
    RKLog("%s Stopped.\n", center->name);
}
