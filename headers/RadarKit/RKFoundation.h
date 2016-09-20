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

#define RKNextNBuffer0Slot(i, N)        (i >= RKBuffer0SlotCount - N ? i - RKBuffer0SlotCount + N : i + N)
#define RKPreviousNBuffer0Slot(i, N)    (i < N ? RKBuffer0SlotCount - N + i : i - N)

void stripTrailingUnwanted(char *str);

#endif /* defined(__RadarKit_RKFoundation__) */
