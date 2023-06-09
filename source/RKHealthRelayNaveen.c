//
//  RKHealthRelayNaveen.c
//  RadarKit
//
//  Created by Boonleng Cheong on 6/9/2023.
//  Copyright © Boonleng Cheong. All rights reserved.
//

#include <RadarKit/RKHealthRelayNaveen.h>

// Internal Functions

static int healthRelayNaveenRead(RKClient *client) {
    return RKResultSuccess;
}

static int healthRelayNaveenGreet(RKClient *client) {
    RKLog("%s Naveen @ %s:%d connected.\n", client->name, client->hostIP, client->port);
    // RKHealthRelayTweeta *me = (RKHealthRelayTweeta *)client->userResource;
    return RKResultSuccess;
}

#pragma mark - Protocol Implementations

// Implementations
RKHealthRelayNaveen RKHealthRelayNaveenInit(RKRadar *radar, void *input) {
    RKHealthRelayNaveen *me = (RKHealthRelayNaveen *)malloc(sizeof(RKHealthRelayNaveen));
     if (me == NULL) {
        RKLog("Error. Unable to allocated RKHealthRelayNaveen.\n");
        return NULL;
    }
    memset(me, 0, sizeof(RKHealthRelayNaveen));
    me->radar = radar;

    // What the server is uses a TCP socket at default port 9557. The payload is aywas a line string terminated by \r\n
    RKClientDesc desc;
    memset(&desc, 0, sizeof(RKClientDesc));
    sprintf(desc.name, "%s<NaveenAutoRelay>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColorOfIndex(RKEngineColorHealthRelayNaveen) : "",
            rkGlobalParameters.showColor ? RKNoColor : "");
    strncpy(desc.hostname, (char *)input, RKNameLength - 1);
    char *colon = strstr(desc.hostname, ":");
    if (colon != NULL) {
        *colon = '\0';
        sscanf(colon + 1, "%d", &desc.port);
    } else {
        desc.port = 9557;
    }
    desc.type = RKNetworkSocketTypeTCP;
    desc.format = RKNetworkMessageFormatNewLine;
    desc.reconnect = true;
    desc.timeoutSeconds = RKNetworkTimeoutSeconds;
    desc.verbose =
    radar->desc.initFlags & RKInitFlagVeryVeryVerbose ? 3 :
    (radar->desc.initFlags & RKInitFlagVeryVerbose ? 2:
     (radar->desc.initFlags & RKInitFlagVerbose ? 1 : 0));

    me->client = RKClientInitWithDesc(desc);
    RKClientSetUserResource(me->client, me);
    RKClientSetGreetHandler(me->client, &healthRelayNaveenGreet);
    RKClientSetReceiveHandler(me->client, &healthRelayNaveenRead);
    RKClientStart(me->client);

   return (RKHealthRelay)me;
}

int RKHealthRelayNaveenExec(RKHealthRelayNaveen input, const char * command, char _Nullable *response) {
    return RKResultSuccess;
}

int RKHealthRelayNaveenFree(RKHealthRelayNaveen input) {
    if (input == NULL) {
        return RKResultNoRadar;
    }
    RKHealthRelayNaveen *me = (RKHealthRelayNaveen *)input;
    free(engine);
    return RKResultSuccess;
}
