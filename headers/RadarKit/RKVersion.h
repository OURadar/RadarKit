//
//  RKVersion.h
//  RadarKit
//
//  Created by Boonleng Cheong on 4/24/2021.
//

// These are meant to be defined during compile time, do not use these outside of RadarKit, see char *RKVersionString()
#ifdef BETA_BRANCH
#define _RKVersionBranch "b"
#else
#define _RKVersionBranch ""
#endif

#define _RKVersionString "2.6" _RKVersionBranch
