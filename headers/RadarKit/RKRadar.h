//
//  RKRadar.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKRadar__
#define __RadarKit_RKRadar__

#include <RadarKit/RKTypes.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

enum RKInitFlag {
    RKInitFlagWantMomentBuffer = 1,
    RKInitFlagWantRawIQBuffer  = 1 << 1,
    RKInitFlatWantEverything   = RKInitFlagWantMomentBuffer | RKInitFlagWantMomentBuffer
};

extern RKRadar *RKInit(void);
extern int RKFree(RKRadar *radar);

//#ifdef __cplusplus
//}
//#endif

#endif /* defined(__RadarKit_RKRadar__) */
