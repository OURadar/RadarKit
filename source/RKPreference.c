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

RKPreference *RKPreferenceInitWithFile(const char *filename) {
    RKPreference *preference = (RKPreference *)malloc(sizeof(RKPreference));
    if (preference == NULL) {
        RKLog("Error. Unable to allocate RKPreference.\r");
        exit(EXIT_FAILURE);
    }
    memset(preference, 0, sizeof(RKPreference));
    strcpy(preference->filename, filename);
    preference->memoryUsage = sizeof(RKPreference);
    RKLog("%s\n", RKIntegerToCommaStyleString(preference->memoryUsage));
    return preference;
}

RKPreference *RKPreferenceInit(void) {
    return RKPreferenceInitWithFile("pref.conf");
}

void RKPreferenceFree(RKPreference *object) {
    free(object);
}

#pragma mark - Properties

#pragma mark - Interactions

int RKPreferenceUpdate(RKPreference *preference) {
    FILE *fid = fopen(preference->filename, "r");
    if (fid == NULL) {
        RKLog("Error. Unable to open preference file %s.\n", preference->filename);
        return RKResultPreferenceFileNotFound;
    }

    char *line = (char *)malloc(RKMaximumStringLength);

    char *c, *s;

    int i = 0;
    int k = 0;
    while (k < RKPreferenceObjectCount) {
        c = fgets(line, RKMaximumStringLength, fid);
        if (c == NULL) {
            break;
        }
        if (line[0] == '#' || strlen(line) < 2) {
            //printf("Skip line %d %s\n", i, line);
        } else {
            //printf("Process line %d %s", i, line);
            // Find the first white space, divide the line into keyword and valueString
            s = line;
            while (*s != '\0' && (*s != ' ' && *s != '\t')) {
                s++;
            }
            *s++ = '\0';
            // Now, find the first character
            while (*s != '\0' && (*s == ' ' || *s == '\t')) {
                s++;
            }
            strncpy(preference->objects[k].keyword, line, RKNameLength);
            strncpy(preference->objects[k].valueString, s, RKNameLength);
            RKStripTail(preference->objects[k].valueString);
            preference->objects[k].isValid = true;
            if (preference->objects[k].valueString[0] >= '0' && preference->objects[k].valueString[0] <= '9') {
                preference->objects[k].isNumeric = true;
                preference->objects[k].numericCount = sscanf(preference->objects[k].valueString, "%lf %lf %lf %lf",
                                                             &preference->objects[k].parameters[0],
                                                             &preference->objects[k].parameters[1],
                                                             &preference->objects[k].parameters[2],
                                                             &preference->objects[k].parameters[3]);
                /*
                printf("Keyword:'%s'   parameters:'%s' (%d)  %d  (%d) %.1f %.1f %.1f\n",
                       preference->objects[k].keyword, preference->objects[k].valueString, (int)strlen(preference->objects[k].valueString),
                       preference->objects[k].isNumeric, preference->objects[k].numericCount,
                       preference->objects[k].parameters[0],
                       preference->objects[k].parameters[1],
                       preference->objects[k].parameters[2]);
                 */
            }
            k++;
        }
        i++;
    }

    if (k == RKPreferenceObjectCount) {
        RKLog("Warning. Unable to digest all preference values.\n");
    }

    free(line);

    return RKResultSuccess;
}

RKPreferenceObject *RKPreferenceFindKeyword(RKPreference *preference, const char *keyword) {
    int k = 0;
    while (k < RKPreferenceObjectCount && preference->objects[k].isValid == true) {
        if (!strcasecmp(preference->objects[k].keyword, keyword)) {
            return &preference->objects[k];
        }
        k++;
    }
    return NULL;
}
