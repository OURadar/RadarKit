//
//  RKRelay.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 4/11/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKRelay.h>

#pragma mark - Helper Functions

#pragma mark - Delegate Workers

static int RKRelayInit(RKClient *client) {
}

#pragma mark - Life Cycle

#pragma mark - Properties

#pragma mark - Interactions

int RKRelayExec(RKMasterController input, const char *command, char *response) {
    RKRelay *me = (RKRelay *)input;
    RKClient *client = me->client;
    if (client->verbose > 1) {
        RKLog("%s received '%s'", client->name, command);
    }
    if (!strcmp(command, "disconnect")) {
        RKClientStop(client);
    } else {
        if (client->state < RKClientStateConnected) {
            if (response != NULL) {
                sprintf(response, "NAK. Health Relay not connected." RKEOL);
            }
            return RKResultIncompleteReceive;
        }
        int s = 0;
        uint32_t responseIndex = me->responseIndex;
        size_t size = snprintf(me->latestCommand, RKMaximumStringLength - 1, "%s" RKEOL, command);
        RKNetworkSendPackets(client->sd, me->latestCommand, size, NULL);
        while (responseIndex == me->responseIndex) {
            usleep(10000);
            if (++s % 100 == 0) {
                RKLog("%s Waited %.2f s for response.\n", client->name, (float)s * 0.01f);
            }
            if ((float)s * 0.01f >= 3.0f) {
                RKLog("%s should time out.\n", client->name);
                break;
            }
        }
        if (responseIndex == me->responseIndex) {
            if (response != NULL) {
                sprintf(response, "NAK. Timeout." RKEOL);
            }
            return RKResultTimeout;
        }
        if (response != NULL) {
            strcpy(response, me->responses[responseIndex]);
        }
    }
    return RKResultSuccess;
}
