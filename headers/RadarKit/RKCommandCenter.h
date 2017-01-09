//
//  RKCommandCenter.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKCommandCenter__
#define __RadarKit_RKCommandCenter__

#include <RadarKit/RKRadar.h>

#define RKCommandCenterMaxConnections 32

typedef uint64_t RKUserFlag;

enum RKUserFlag {
    RKUserFlagNull               = 0,              //
    RKUserFlagControl            = 1,              // Controls
    RKUserFlagStatusHealth       = (1 << 1),       //
    RKUserFlagStatusPulses       = (1 << 2),       //
    RKUserFlagStatusRays         = (1 << 3),       //
    RKUserFlagStatusAll          = 0x0E,           //
    RKUserFlagDisplayIQ          = (1 << 8),       // Low rate IQ
    RKUserFlagProductIQ          = (1 << 9),       // Full rate IQ
    RKUserFlagDisplayZ           = (1 << 16),      // Display
    RKUserFlagDisplayV           = (1 << 17),      //
    RKUserFlagDisplayW           = (1 << 18),      //
    RKUserFlagDisplayD           = (1 << 19),      //
    RKUserFlagDisplayP           = (1 << 20),      //
    RKUserFlagDisplayR           = (1 << 21),      //
    RKUserFlagDisplayK           = (1 << 22),      //
    RKUserFlagDisplayS           = (1 << 23),      //
    RKUserFlagDisplayZVWDPRKS    = 0x0000FF0000,   //
    RKUserFlagProductZ           = (1 << 32),      // Products
    RKUserFlagProductV           = (1 << 33),      //
    RKUserFlagProductW           = (1 << 34),      //
    RKUserFlagProductD           = (1 << 35),      //
    RKUserFlagProductP           = (1 << 36),      //
    RKUserFlagProductR           = (1 << 37),      //
    RKUserFlagProductK           = (1 << 38),      //
    RKUserFlagProductS           = (1 << 39),      //
    RKUserFlagProductZVWDPRKS    = 0xFF00000000    //
};

typedef struct  rk_user {
    char         login[64];
    RKUserFlag   access;             // Authorized access priviledge
    RKUserFlag   streams;
    double       timeLastOut;
    double       timeLastIn;
    uint32_t     rayStatusIndex;
    uint32_t     pulseStatusIndex;
    char         string[RKMaximumStringLength];
    RKRadar      *radar;
} RKUser;

typedef struct rk_command_center {
    // User set variables
    char         name[64];
    int          verbose;
    RKRadar      *radars[4];
    
    // Program set variables
    RKServer     *server;
    int          radarCount;
    RKUser       users[RKCommandCenterMaxConnections];
} RKCommandCenter;

RKCommandCenter *RKCommandCenterInit(void);
void RKCommandCenterFree(RKCommandCenter *);

void RKCommandCenterSetVerbose(RKCommandCenter *, const int);
void RKCommandCenterAddRadar(RKCommandCenter *, RKRadar *);
void RKCommandCenterRemoveRadar(RKCommandCenter *, RKRadar *);

void RKCommandCenterStart(RKCommandCenter *);

#endif /* __RadarKit_RKCommandCenter__ */
