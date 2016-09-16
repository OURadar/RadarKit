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
