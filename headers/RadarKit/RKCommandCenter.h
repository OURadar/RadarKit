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
    RKUserFlagNull            = 0,
	RKUserFlagStatusHealth    = 1,
    RKUserFlagStatusPulses    = (1 << 1),
    RKUserFlagStatusRays      = (1 << 2),
	RKUserFlagDisplayZ        = (1 << 8),        // Display
	RKUserFlagDisplayV        = (1 << 9),
	RKUserFlagDisplayW        = (1 << 10),
	RKUserFlagDisplayD        = (1 << 11),
	RKUserFlagDisplayP        = (1 << 12),
	RKUserFlagDisplayR        = (1 << 13),
	RKUserFlagDisplayK        = (1 << 14),
	RKUserFlagDisplayS        = (1 << 15),
    RKUserFlagDisplayIQ       = (1 << 24),       // Low rate IQ
    RKUserFlagProductZ        = (1 << 32),       // Products
    RKUserFlagProductV        = (1 << 33),
    RKUserFlagProductW        = (1 << 34),
    RKUserFlagProductD        = (1 << 35),
    RKUserFlagProductP        = (1 << 36),
    RKUserFlagProductR        = (1 << 37),
    RKUserFlagProductK        = (1 << 38),
    RKUserFlagProductS        = (1 << 39),
    RKUserFlagProductIQ       = (1 << 48),      // Full rate IQ
    RKUserFlagControl         = (1 << 24)       // Controls
};    

typedef struct  rk_user {
	char         login[64];
	RKUserFlag   access;             // Authorized access priviledge
	RKUserFlag   streams;
	int          radarId;
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
