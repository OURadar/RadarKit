//
//  RKLocalCommandCenter.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 9/16/16.
//  Copyright © 2016 Boon Leng Cheong. All rights reserved.
//

#ifndef __RadarKit_RKLocalCommandCenter__
#define __RadarKit_RKLocalCommandCenter__

#include <RadarKit/RKServer.h>

struct RKLocalCommandCenter {
    RKServer *LCC;
};

void RKLocalCommandCenterCommandHandler(void);

#endif /* __RadarKit_RKLocalCommandCenter__ */

