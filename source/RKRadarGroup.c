//
//  RKRadarGroup.c
//  RadarKit
//
//  This is a command center that handles multiple radars
//
//  Created by Boon Leng Cheong on 1/5/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKRadarGroup.h>

typedef struct rk_radar_group {
    RKRadar    *radars;
    RKServer   *server;
} RKRadarGroup;
