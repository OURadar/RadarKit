//
//  RKFoundation.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/17/15.
//
//

#ifndef __RadarKit_RKFoundation__
#define __RadarKit_RKFoundation__

#include <RadarKit/RKTypes.h>
#include <RadarKit/RKMisc.h>

#define RKEOL                       "\r\n"

#define RKNextBuffer0Slot(i)        (i == RKBuffer0SlotCount - 1 ? 0 : i + 1)
#define RKPreviousBuffer0Slot(i)    (i == 0 ? RKBuffer0SlotCount - 1 : i - 1)

#define _POSIX_C_SOURCE 199309L

void stripTrailingUnwanted(char *str);

#endif /* defined(__RadarKit_RKFoundation__) */
