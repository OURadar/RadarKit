//
//  RKHealth.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 1/28/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_Health__
#define __RadarKit_Health__

#include <RadarKit/RKFoundation.h>
#include <RadarKit/RKClient.h>

typedef struct rk_health_engine RKHealthEngine;

struct rk_health_engine {
    // User defined variables
    char                name[RKNameLength];
};

#endif /* RKHealth_h */
