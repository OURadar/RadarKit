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
		RKRadarRelayUpdateStreams(radar->radarRelay, consolidatedStreams);
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
                RKLog("%s %s Ping x %s\n", engine->name, O->name, RKIntegerToCommaStyleString(user->pingCount));
            }
            // There is no need to send a response. The delegate function socketStreamHandler sends a beacon periodically
        } else if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
            k = 0;
            while (!(user->radar->state & RKRadarStateLive)) {
                usleep(100000);
                if (++k % 10 == 0 && engine->verbose > 1) {
                    RKLog("%s sleep 1/%.1f s   radar->state = 0x%04x\n", engine->name, (float)k * 0.1f, user->radar->state);
                }
            }
            user->commandCount++;
            RKLog("%s %s Received command '%s%s%s'\n",
				  engine->name, O->name,
				  rkGlobalParameters.showColor ? RKGreenColor : "",
				  commandString,
				  rkGlobalParameters.showColor ? RKNoColor : "");
            // Process the command
            switch (commandString[0]) {
                case 'a':
                    // Authenticate
                    sscanf(commandString + 1, "%s %s", sval1, sval2);
                    RKLog(">%s %s Authenticating %s %s ... (%d) (%d)\n", engine->name, O->name, sval1, sval2, strlen(sval1), sizeof(user->login));
                    // Check authentication here. For now, control is always authorized
                    //
                    //
                    user->access |= RKStreamControl;
                    // Update some info
                    strncpy(user->login, sval1, sizeof(user->login) - 1);
                    user->controlSetIndex = (uint32_t)-1;
                    break;

                case 'd':
                    // DSP related
                    switch (commandString[commandString[1] == ' ' ? 2 : 1]) {
                        case 'c':
                            RKClearPulseBuffer(user->radar->pulses, user->radar->desc.pulseBufferDepth);
                            RKClearRayBuffer(user->radar->rays, user->radar->desc.rayBufferDepth);
                            sprintf(string, "ACK. Buffers cleared." RKEOL);
                            RKOperatorSendCommandResponse(O, string);
                            break;
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
                            RKSoftRestart(user->radar);
                            RKCommandCenterSkipToCurrent(engine, user->radar);
                            sprintf(string, "ACK. Soft restart executed." RKEOL);
                            RKOperatorSendCommandResponse(O, string);
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
                                    HIGHLIGHT("d") " [DSP_PAMETER] [VALUE] - DSP parameters,\n"
                                    "    where index can be (coming soon):\n"
                                    "        t - treshold to censor using VALUE in dB\n"
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
                                    "        s 2 - Look at the level-II data generation.\n"
                                    "        s 3 - Look at the pedestal raw data.\n"
                                    "\n"
                                    HIGHLIGHT("v") " - Sets a simple VCP (coming soon)\n"
                                    "    e.g.,\n"
                                    "        v 2:2:20 180 - a volume at EL 2° to 20° at 2° steps, AZ slew at 180°/s\n"
                                    "\n"
                                    HIGHLIGHT("b") " - Simulate a push button event from a piece of hardware\n"
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
                    newStream = RKStreamFromString(commandString + 1);
                    //RKLog("%s %s newStream = %llx\n", engine->name, O->name, newStream);
                    k = user->rayIndex;
                    pthread_mutex_lock(&user->mutex);
                    user->streamsInProgress = RKStreamNull;
                    user->streams = newStream;
                    user->rayStatusIndex = RKPreviousModuloS(user->radar->momentEngine->rayStatusBufferIndex, RKBufferSSlotCount);
                    user->rayAnchorsIndex = user->radar->sweepEngine->rayAnchorsIndex;
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
                    user->ascopeMode = RKNextModuloS(user->ascopeMode, 4);
                    sprintf(string, "ACK. AScope mode to %d" RKEOL, user->ascopeMode);
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
                    snprintf(string, RKMaximumStringLength - 1, "NAK. Unknown command '%s'." RKEOL, commandString);
                    RKOperatorSendCommandResponse(O, string);
                    break;
            }
        } else if (user->radar->desc.initFlags & RKInitFlagRelay) {
            switch (commandString[0]) {
                case 'a':
                    RKLog("%s %s Queue command '%s' to relay.\n", engine->name, O->name, commandString);
                    RKRadarRelayExec(user->radar->radarRelay, commandString, string);
                    O->delimTx.type = RKNetworkPacketTypeControls;
                    O->delimTx.size = (uint32_t)strlen(string);
                    RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), string, O->delimTx.size, NULL);

                case 'r':
                    // Change radar
                    sscanf("%s", commandString + 1, sval1);
                    RKLog(">%s %s Selected radar %s\n", engine->name, O->name, sval1);
                    snprintf(string, RKMaximumStringLength - 1, "ACK. %s selected." RKEOL, sval1);
                    RKOperatorSendCommandResponse(O, string);
                    break;
                    
                case 's':
                    // Stream varrious data
                    user->streams = RKStreamFromString(commandString + 1);
                    
                    consolidateStreams(engine);

                    k = user->rayIndex;
                    pthread_mutex_lock(&user->mutex);
                    user->streamsInProgress = RKStreamNull;
                    pthread_mutex_unlock(&user->mutex);
                    RKLog(">%s %s Reset progress.\n", engine->name, O->name);
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
        } else {
            RKLog("%s The radar is neither a DSP system nor a relay.\n", engine->name);
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

    RKPulse *pulse;
    RKPulseHeader pulseHeader;

    RKRay *ray;
    RKRayHeader rayHeader;
    
    RKSweep *sweep;
    RKSweepHeader sweepHeader;
    
    uint8_t *u8Data = NULL;
    float *f32Data = NULL;

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

    pthread_mutex_lock(&user->mutex);

    if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
        // Modes "1", "2", "3" and "4" are for signal processor only - showing the latest summary text view
        if (user->streams & user->access && td >= 0.05) {
            // Stream "1" - Overall status
            if (user->streams & RKStreamStatusPulses) {
                k = snprintf(user->string, RKMaximumStringLength - 1, "%s | %s | %s | %s | %s |" RKEOL,
                             RKPulseCompressionEngineStatusString(user->radar->pulseCompressionEngine),
                             RKPulseRingFilterEngineStatusString(user->radar->pulseRingFilterEngine),
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

        // Send another set of controls if the radar controls have changed.
        if (user->controlSetIndex != user->radar->controlSetIndex && user->access & RKStreamControl) {
            user->controlSetIndex = user->radar->controlSetIndex;
            j = sprintf(user->string, "{\"Radars\":[");
            for (k = 0; k < engine->radarCount; k++) {
                RKRadar *radar = engine->radars[k];
                j += sprintf(user->string + j, "\"%s\", ", radar->desc.name);
            }
            if (k > 0) {
                j += sprintf(user->string + j - 2, "], ") - 2;
            } else {
                j += sprintf(user->string + j, "], ");
            }
            // Should only send the controls if the user has been authenticated
            RKMakeJSONStringFromControls(user->scratch, user->radar->controls, user->radar->controlIndex);
            j += sprintf(user->string + j, "\"Controls\":["
                        "{\"Label\":\"Go\", \"Command\":\"y\"}, "
                        "{\"Label\":\"Stop\", \"Command\":\"z\"}, "
                        "%s"
                        "]}" RKEOL, user->scratch);
            O->delimTx.type = RKNetworkPacketTypeControls;
            O->delimTx.size = j;
            RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), user->string, O->delimTx.size, NULL);
        }
    }

    // For contiguous streaming:
    // If we just started a connection, grab the payload that is either:
    // 1) Up to latest available:
    //      i) For a health, it is radar->healthIndex - 1
    //     ii) For a ray, it is radar->rayIndex - (number of workers)
    //    iii) For a pulse, it is radar->pulseIndex - (number of workers)
    //     iv) For a sweep, it is radar->sweepEngine->rayAnchorsIndex
    // 2) The latest slot it will be stored. It is crucial to ensure that:
    //      i) For a health, it is RKStatusReady
    //     ii) For a ray, it has RKRayStatusReady set
    //    iii) For a pulse, it has RKPulseStatusReadyForMoment set
    //     iv) For a sweep, rayAnchorsIndex has increased
    // 3) Once the first payload is sent, the stream is consider in progress (streamsInProgress)
    // 4) If (2) can't be met within X secs, in progress flag is not set so (2) will be checked
    //    again in the next iteraction.

    // Processor Status
    if (user->streams & user->access & RKStreamStatusProcessorStatus) {
        endIndex = RKPreviousModuloS(user->radar->statusIndex, user->radar->desc.statusBufferDepth);
        if (!(user->streamsInProgress & RKStreamStatusProcessorStatus)) {
            if (engine->verbose) {
                RKLog("%s %s Fast forward RKStatus -> %d (%s).\n", engine->name, O->name, endIndex,
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
                if (user->streams & RKStreamProductSh) {
                    rayHeader.productList |= RKProductListProductSh;
                }
                if (user->streams & RKStreamProductSv) {
                    rayHeader.productList |= RKProductListProductSv;
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
                    } else if (productList & RKProductListProductSh) {
                        productList ^= RKProductListProductSh;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexSh);
                    } else if (productList & RKProductListProductSv) {
                        productList ^= RKProductListProductSv;
                        f32Data = RKGetFloatDataFromRay(ray, RKProductIndexSv);
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
                RKLog("%s %s No product ray / deactivated.  streamsInProgress = 0x%08x\n",
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
                if (user->streams & RKStreamDisplaySh) {
                    rayHeader.productList |= RKProductListDisplaySh;
                }
                if (user->streams & RKStreamDisplaySv) {
                    rayHeader.productList |= RKProductListDisplaySv;
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
                    } else if (displayList & RKProductListDisplaySh) {
                        displayList ^= RKProductListDisplaySh;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexSh);
                    } else if (displayList & RKProductListDisplaySv) {
                        displayList ^= RKProductListDisplaySv;
                        u8Data = RKGetUInt8DataFromRay(ray, RKProductIndexSv);
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
            } // while (user->rayIndex != endIndex) ...
        } else {
            if ((int)ray->header.i > 0) {
                RKLog("%s %s No display ray / deactivated.  streamsInProgress = 0x%08x\n",
                      engine->name, O->name, user->streamsInProgress);
            }
        } // if (ray->header.s & RKRayStatusReady && engine->server->state == RKServerStateActive) ...
    } // else if if (user->streams & user->access & RKStreamDisplayZVWDPRKS) ...

    // Sweep
    if (user->streams & user->access & RKStreamSweepZVWDPRKS) {
        // Sweep streams - no skipping
        if (user->rayAnchorsIndex != user->radar->sweepEngine->rayAnchorsIndex) {
            user->rayAnchorsIndex = user->radar->sweepEngine->rayAnchorsIndex;
            sweep = RKSweepCollect(user->radar->sweepEngine);
            memcpy(&sweepHeader, &sweep->header, sizeof(RKSweepHeader));

            if (engine->verbose > 1) {
                RKLog("%s New sweep available   C%02d   S%lu.  <--  %x / %x\n", engine->name, sweep->rays[0]->header.configIndex, sweep->header.config.i, sweep->header.productList, sweepHeader.productList);
            }

			// Store a copy of the original list of available products
            uint32_t productList = sweepHeader.productList;

            // Mutate sweep so that header indicates the sweep to be transmitted
            i = 0;
            user->scratch[0] = '\0';
            sweepHeader.productList = RKProductListNone;
            if ((user->streams & RKStreamSweepZ) && (productList & RKProductListProductZ)) {
                sweepHeader.productList |= RKProductListProductZ;
                i += sprintf(user->scratch + i, " Z,");
            }
            if ((user->streams & RKStreamSweepV) && (productList & RKProductListProductV)) {
                sweepHeader.productList |= RKProductListProductV;
                i += sprintf(user->scratch + i, " V,");
            }
            if ((user->streams & RKStreamSweepW) && (productList & RKProductListProductW)) {
                sweepHeader.productList |= RKProductListProductW;
                i += sprintf(user->scratch + i, " W,");
            }
            if ((user->streams & RKStreamSweepD) && (productList & RKProductListProductD)) {
                sweepHeader.productList |= RKProductListProductD;
                i += sprintf(user->scratch + i, " D,");
            }
            if ((user->streams & RKStreamSweepP) && (productList & RKProductListProductP)) {
                sweepHeader.productList |= RKProductListProductP;
                i += sprintf(user->scratch + i, " P,");
            }
            if ((user->streams & RKStreamSweepR) && (productList & RKProductListProductR)) {
                sweepHeader.productList |= RKProductListProductR;
                i += sprintf(user->scratch + i, " R,");
            }
            if ((user->streams & RKStreamSweepK) && (productList & RKProductListProductK)) {
                sweepHeader.productList |= RKProductListProductK;
                i += sprintf(user->scratch + i, " K,");
            }
            if ((user->streams & RKStreamSweepSh) && (productList & RKProductListProductSh)) {
                sweepHeader.productList |= RKProductListProductSh;
                i += sprintf(user->scratch + i, " Sh,");
            }
            if ((user->streams & RKStreamSweepSv) && (productList & RKProductListProductSv)) {
                sweepHeader.productList |= RKProductListProductSv;
                i += sprintf(user->scratch + i, " Sv,");
            }
            // Remove the last ','
            if (i > 1) {
                user->scratch[i - 1] = '\0';
            }

            const uint32_t productCount = __builtin_popcount(sweepHeader.productList);

			if (productCount) {
				size_t sentSize = 0;

				//O->delimTx.type = RKNetworkPacketTypeSweep;
				//O->delimTx.size = (uint32_t)(sizeof(RKSweepHeader) + sweepHeader.rayCount * (sizeof(RKRayHeader) + productCount * sweepHeader.gateCount * sizeof(float)));
				//sentSize += RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &sweepHeader, sizeof(RKSweepHeader), NULL);

				O->delimTx.type = RKNetworkPacketTypeSweepHeader;
				O->delimTx.size = (uint32_t)sizeof(RKSweepHeader);
				sentSize += RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &sweepHeader, sizeof(RKSweepHeader), NULL);

				O->delimTx.type = RKNetworkPacketTypeSweepRay;
				O->delimTx.size = (uint32_t)(sizeof(RKRayHeader) + productCount * sweepHeader.gateCount * sizeof(float));

				for (k = 0; k < sweepHeader.rayCount; k++) {
					ray = sweep->rays[k];
					memcpy(&rayHeader, &ray->header, sizeof(RKRayHeader));
					rayHeader.productList = sweepHeader.productList;
					sentSize += RKOperatorSendPackets(O, &O->delimTx, sizeof(RKNetDelimiter), &rayHeader, sizeof(RKRayHeader), NULL);
					productList = sweepHeader.productList;
					if (engine->verbose > 1 && (k < 3 || k == sweepHeader.rayCount - 1)) {
						RKLog(">%s %s k = %d   moments = %s   (%x)\n", engine->name, O->name, k, user->scratch + 1, productList);
					}
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
						} else if (productList & RKProductListProductSh) {
							productList ^= RKProductListProductSh;
							f32Data = RKGetFloatDataFromRay(ray, RKProductIndexSh);
						} else if (productList & RKProductListProductSv) {
							productList ^= RKProductListProductSv;
							f32Data = RKGetFloatDataFromRay(ray, RKProductIndexSv);
						} else {
							f32Data = NULL;
						}
						if (f32Data) {
							sentSize += RKOperatorSendPackets(O, f32Data, sweep->header.gateCount * sizeof(float), NULL);
						}
					}
				}
				if (engine->verbose) {
					// Offset scratch by one to get rid of the very first space
					RKLog("%s %s Sent sweep S%d (0x%08x) (%s)\n", engine->name, O->name, sweepHeader.config.i, sweepHeader.productList, user->scratch + 1);
					if (engine->verbose > 1) {
						RKLog(">%s %s user->streams = 0x%lx / 0x%lx\n", engine->name, O->name, user->streams, RKStreamSweepZVWDPRKS);
						RKLog(">%s %s Sent a sweep of size %s B (%d)\n", engine->name, O->name, RKIntegerToCommaStyleString(sentSize), productCount);
					}
				}
			} // if (productCount) ...
        } // if (user->rayAnchorsIndex != user->radar->sweepEngine->rayAnchorsIndex) ...
    } // if (user->streams & user->access & RKStreamSweepZVWDPRKS) ...

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
        s = 0;
        while (!(pulse->header.s & RKPulseStatusProcessed) && s++ < 100) {
            usleep(1000);
        }

        if (!(user->streamsInProgress & RKPulseStatusProcessed)) {
            user->pulseIndex = endIndex;
            s = 0;
            while (!(pulse->header.s & RKPulseStatusHasIQData) && engine->server->state == RKServerStateActive && s++ < 20) {
                if (s % 10 == 0) {
                    RKLog("%s %s sleep 0/%.1f s  RKPulse\n", engine->name, O->name, s * 0.1f);
                }
                usleep(100000);
            }
        }

        if (pulse->header.s & RKPulseStatusProcessed && engine->server->state == RKServerStateActive) {
            user->streamsInProgress |= RKStreamDisplayIQ;
            memcpy(&pulseHeader, &pulse->header, sizeof(RKPulseHeader));
            c16DataH = RKGetInt16CDataFromPulse(pulse, 0);
            c16DataV = RKGetInt16CDataFromPulse(pulse, 1);
            userDataH = user->samples[0];
            userDataV = user->samples[1];
            RKComplex *yH;
            RKComplex *yV;
            float scale = 1.0f;

            // Default stride: k = 1
            k = 1;
            switch (user->ascopeMode) {
                case 3:
                    // Show the waveform that was used through the forward sampling path
                    pulseHeader.gateCount = 2000;
                    if (!(user->radar->desc.initFlags & RKInitFlagSignalProcessor)) {
                        break;
                    }
                    i = 0;
                    gid = pulse->header.i % user->radar->pulseCompressionEngine->filterGroupCount;
                    for (k = 0; k < MIN(400, user->radar->pulseCompressionEngine->filterAnchors[gid][0].length); k++) {
                        *userDataH++ = *c16DataH++;
                        *userDataV++ = *c16DataV++;
                        i++;
                    }
                    for (; k < MIN(410, user->radar->pulseCompressionEngine->filterAnchors[gid][0].length + 3); k++) {
                        userDataH->i   = 0;
                        userDataH++->q = 0;
                        userDataV->i   = 0;
                        userDataV++->q = 0;
                        i++;
                    }
                    scale = 10000.0f * sqrtf(user->radar->pulseCompressionEngine->filterAnchors[gid][0].length);
                    // Show the filter that was used as matched filter
                    yH = user->radar->pulseCompressionEngine->filters[gid][0];
                    yV = user->radar->pulseCompressionEngine->filters[gid][0];
                    for (k = 0; k < MIN(400, user->radar->pulseCompressionEngine->filterAnchors[gid][0].length); k++) {
                        userDataH->i   = (int16_t)(scale * yH->i);
                        userDataH++->q = (int16_t)(scale * yH++->q);
                        userDataV->i   = (int16_t)(scale * yV->i);
                        userDataV++->q = (int16_t)(scale * yV++->q);
                        i++;
                    }
                    for (; k < MIN(410, user->radar->pulseCompressionEngine->filterAnchors[gid][0].length + 3); k++) {
                        userDataH->i   = 0;
                        userDataH++->q = 0;
                        userDataV->i   = 0;
                        userDataV++->q = 0;
                        i++;
                    }
                    // Compute an appropriate normalization factor so that 16-bit view on the scope is okay
                    scale = 1.0f / sqrtf((float)user->radar->pulseCompressionEngine->filterAnchors[gid][0].length);
                    // The third part of is the processed data
                    yH = RKGetComplexDataFromPulse(pulse, 0);
                    yV = RKGetComplexDataFromPulse(pulse, 1);
                    for (; i < pulseHeader.gateCount; i++) {
                        userDataH->i   = (int16_t)(scale * yH->i);
                        userDataH++->q = (int16_t)(scale * yH++->q);
                        userDataV->i   = (int16_t)(scale * yV->i);
                        userDataV++->q = (int16_t)(scale * yV++->q);
                    }
                    break;

                case 2:
                    // Down-sampled output data
                    k = user->pulseDownSamplingRatio;

                    pulseHeader.gateCount /= k;
                    pulseHeader.gateSizeMeters *= (float)k;

                    scale = 1.0f;
                    yH = RKGetComplexDataFromPulse(pulse, 0);
                    yV = RKGetComplexDataFromPulse(pulse, 1);
                    for (i = 0; i < pulseHeader.gateCount; i++) {
                        userDataH->i   = (int16_t)(scale * yH->i);
                        userDataH++->q = (int16_t)(scale * yH->q);
                        yH += k;
                        userDataV->i   = (int16_t)(scale * yV->i);
                        userDataV++->q = (int16_t)(scale * yV->q);
                        yV += k;
                    }
                    break;

                case 1:
                    // Down-sampled raw input data
                    k = user->pulseDownSamplingRatio;

                default:
                    // Raw input data
                    // Note that at this point, the gateCount in header is describing the RKComplex data (compressed), not the RKIQZ raw data.
                    pulseHeader.gateCount = MIN(pulseHeader.gateCount * user->radar->desc.pulseToRayRatio / k, 2000);
                    pulseHeader.gateSizeMeters *= (float)k / user->radar->desc.pulseToRayRatio;
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
    
    if (engine->radarCount == 0) {
        RKLog("%s No radar yet.\n", engine->name);
        return RKResultNoRadar;
    }
    
    memset(user, 0, sizeof(RKUser));
    user->access = RKStreamStatusAll;
    user->access |= RKStreamDisplayZVWDPRKS;
    user->access |= RKStreamProductZVWDPRKS;
    user->access |= RKStreamSweepZVWDPRKS;
    user->access |= RKStreamDisplayIQ | RKStreamProductIQ;
    user->radar = engine->radars[0];
    if (user->radar->desc.initFlags & RKInitFlagSignalProcessor) {
        user->rayDownSamplingRatio = (uint16_t)MAX(user->radar->desc.pulseCapacity / user->radar->desc.pulseToRayRatio / 1000, 1);
    } else {
        user->rayDownSamplingRatio = 1;
    }
    user->pulseDownSamplingRatio = (uint16_t)MAX(user->radar->desc.pulseCapacity / 1000, 1);
    user->ascopeMode = 0;
    pthread_mutex_init(&user->mutex, NULL);
    RKLog(">%s %s Pul x %d   Ray x %d ...\n", engine->name, O->name, user->pulseDownSamplingRatio, user->rayDownSamplingRatio);

    snprintf(user->login, 63, "radarop");
    user->serverOperator = O;

    return RKResultNoError;
}

int socketTerminateHandler(RKOperator *O) {
    RKCommandCenter *engine = O->userResource;
    RKUser *user = &engine->users[O->iid];
    pthread_mutex_destroy(&user->mutex);
    RKLog(">%s %s Stream reset.\n", engine->name, O->name);
    user->streams = RKStreamNull;
    user->access = RKStreamNull;
    user->radar = NULL;
    consolidateStreams(engine);
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
    sprintf(engine->name, "%s<OperationCenter>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorCommandCenter) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    engine->verbose = 3;
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
    int i, j, k;
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
        j = 0;
        char string[RKMaximumStringLength];
        for (k = 0; k < engine->radarCount; k++) {
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

void RKCommandCenterSkipToCurrent(RKCommandCenter *engine, RKRadar *radar) {
    int i;
    for (i = 0; i < RKCommandCenterMaxConnections; i++) {
        RKUser *user = &engine->users[i];
        if (user->radar == radar && radar->desc.initFlags & RKInitFlagSignalProcessor) {
            user->pulseIndex  = RKPreviousNModuloS(radar->pulseIndex, 2 * radar->pulseCompressionEngine->coreCount, radar->desc.pulseBufferDepth);
            user->rayIndex    = RKPreviousNModuloS(radar->rayIndex, 2 * radar->momentEngine->coreCount, radar->desc.rayBufferDepth);
            user->healthIndex = RKPreviousModuloS(radar->healthIndex, radar->desc.healthBufferDepth);
        }
    }
}
