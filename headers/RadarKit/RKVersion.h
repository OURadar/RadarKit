//
//  RKVersion.h
//  RadarKit
//
//  Created by Boonleng Cheong on 4/24/2021.
//

// These are meant to be defined during compile time, do not use these outside of RadarKit, see char *RKVersionString()
#ifdef BETA_BRANCH
#define _RKVersionBranch_ "b"
#else
#define _RKVersionBranch_ ""
#endif

#define __RKVersion__ "5.9.1" _RKVersionBranch_
