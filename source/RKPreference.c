//
//  RKPreference.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 2/19/17.
//  Copyright © 2017 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKPreference.h>

#pragma mark - Internal Functions

#pragma mark - Implementation

#pragma mark - Life Cycle

RKPreference *RKPreferenceInit(void) {
    RKPreference *object = (RKPreference *)malloc(sizeof(RKPreference));
    if (object == NULL) {
        RKLog("Error. Unable to allocate RKPreference.\r");
        exit(EXIT_FAILURE);
    }
    memset(object, 0, sizeof(RKPreference));
    return object;
}

void RKPreferenceFree(RKPreference *object) {
    free(object);
}

#pragma mark - Properties

#pragma mark - Interactions

