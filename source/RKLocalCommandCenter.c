//
//  RKLocalCommandCenter.c
//  _RadarKit
//
//  Created by Boon Leng Cheong on 9/16/16.
//  Copyright © 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKLocalCommandCenter.h>

int socketCommandHandler(RKOperator *O) {
    switch (O->cmd[0]) {
        case 'a':
            RKOperatorSendString(O, "Hello\n");
            break;
        default:
            break;
    }
    return 0;
}

int socketStreamHandler(RKOperator *O) {
    static struct timeval t0, t1;

    gettimeofday(&t0, NULL);

    // Check IQ
    // Check MM
    // Check system health

    // Heart beat
    char str[64];
    double td = RKTimevalDiff(t1, t0);
    if (td >= 1.0f) {
        t1 = t0;
        //snprintf(str, 63, "beat %.4f", td);
        //RKOperatorSendString(O, str);
        RKOperatorSendBeacon(O);
    }

    return 0;
}
