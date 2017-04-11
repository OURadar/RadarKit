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
    //RKLog("%s\n", RKIntegerToCommaStyleString(preference->memoryUsage));
    RKPreferenceUpdate(preference);
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
    memset(preference->objects, 0, RKPreferenceObjectCount * sizeof(RKPreferenceObject));
    preference->count = 0;

    char *line = (char *)malloc(RKMaximumStringLength);

    char *c, *s, *e;

    int i = 0;
    int k = 0;
    while (k < RKPreferenceObjectCount) {
        c = fgets(line, RKMaximumStringLength, fid);
        if (c == NULL) {
            break;
        }
        if (line[0] == '#' || strlen(line) < 2) {
            #if defined(DEBUG)
            printf("Skip line %d %s\n", i, line);
            #endif
        } else {
            #if defined(DEBUG)
            printf("Process line %d %s", i, line);
            #endif
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
            // Terminate anything pass '#'
            if (s != '\0') {
                e = s + 1;
                while (*e != '\0' && *e != '#') {
                    e++;
                }
                if (*e != '\0') {
                    *e = '\0';
                }
            }
            strncpy(preference->objects[k].keyword, line, RKNameLength);
            strncpy(preference->objects[k].valueString, s, RKMaximumStringLength);
            RKStripTail(preference->objects[k].valueString);
            preference->objects[k].isValid = true;
            if ((preference->objects[k].valueString[0] >= '0' && preference->objects[k].valueString[0] <= '9') ||
                (preference->objects[k].valueString[0] == '.' && preference->objects[k].valueString[1] >= '0' && preference->objects[k].valueString[1] <= '9') ||
                (preference->objects[k].valueString[0] == '-' && preference->objects[k].valueString[1] >= '0' && preference->objects[k].valueString[1] <= '9') ||
                (preference->objects[k].valueString[0] == '+' && preference->objects[k].valueString[1] >= '0' && preference->objects[k].valueString[1] <= '9')) {
                preference->objects[k].isNumeric = true;
                preference->objects[k].numericCount = sscanf(preference->objects[k].valueString, "%lf %lf %lf %lf",
                                                             &preference->objects[k].parameters[0],
                                                             &preference->objects[k].parameters[1],
                                                             &preference->objects[k].parameters[2],
                                                             &preference->objects[k].parameters[3]);
            }
            preference->count++;
            #if defined(DEBUG)
            printf("Keyword:'%s'   parameters:'%s' (%d)  %d  (%d) %.1f %.1f %.1f\n",
                   preference->objects[k].keyword, preference->objects[k].valueString, (int)strlen(preference->objects[k].valueString),
                   preference->objects[k].isNumeric, preference->objects[k].numericCount,
                   preference->objects[k].parameters[0],
                   preference->objects[k].parameters[1],
                   preference->objects[k].parameters[2]);
            #endif
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
    if (!strcasecmp(keyword, preference->previousKeyword)) {
        k = preference->previousIndex + 1;
        if (k >= preference->count) {
            return NULL;
        }
    }
    while (k < RKPreferenceObjectCount && preference->objects[k].isValid == true) {
        if (!strcasecmp(preference->objects[k].keyword, keyword)) {
            strcpy(preference->previousKeyword, keyword);
            preference->previousIndex = k;
            return &preference->objects[k];
        }
        k++;
    }
    return NULL;
}

int RKPreferenceGetKeywordCount(RKPreference *preference, const char *keyword) {
    int k = 0;
    while (k < RKPreferenceObjectCount && preference->objects[k].isValid == true) {
        if (!strcasecmp(preference->objects[k].keyword, keyword)) {
            k++;
        }
    }
    return k;
}
