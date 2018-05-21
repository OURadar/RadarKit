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
	int m = 0;
    while (k < RKPreferenceObjectCount) {
        c = fgets(line, RKMaximumStringLength, fid);
        if (c == NULL) {
            break;
        }
        RKStripTail(line);
        if (line[0] == '#' || strlen(line) < 2) {
            #if defined(DEBUG)
            printf("Skip line %d %s\n", i, line);
            #endif
        } else {
            #if defined(DEBUG)
            printf("Process line %d %s\n", i, line);
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
            if (*s != '\0') {
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
                                                             &preference->objects[k].doubleValues[0],
                                                             &preference->objects[k].doubleValues[1],
                                                             &preference->objects[k].doubleValues[2],
                                                             &preference->objects[k].doubleValues[3]);
			} else if (!strncasecmp(preference->objects[k].valueString, "false", 5) ||
					   !strncasecmp(preference->objects[k].valueString, "true", 4) ||
					   !strncasecmp(preference->objects[k].valueString, "yes", 3) ||
					   !strncasecmp(preference->objects[k].valueString, "no", 2)) {
				sscanf(preference->objects[k].valueString, "%s %s %s %s",
					   preference->objects[k].subStrings[0],
					   preference->objects[k].subStrings[1],
					   preference->objects[k].subStrings[2],
					   preference->objects[k].subStrings[3]);
				for (m = 0; m < 4; m++) {
					preference->objects[k].boolValues[m] = (!strncasecmp(preference->objects[k].subStrings[m], "true", 4) || !strncasecmp(preference->objects[k].subStrings[m], "yes", 4));
				}
			}
            preference->count++;
            #if defined(DEBUG)
            printf("Keyword:'%s'   parameters:'%s' (%d)  %d  (%d) %.1f %.1f %.1f %.1f\n",
                   preference->objects[k].keyword, preference->objects[k].valueString, (int)strlen(preference->objects[k].valueString),
                   preference->objects[k].isNumeric, preference->objects[k].numericCount,
                   preference->objects[k].doubleValues[0],
                   preference->objects[k].doubleValues[1],
                   preference->objects[k].doubleValues[2],
				   preference->objects[k].doubleValues[3]);
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

void RKPreferenceGetValueOfKeyword(RKPreference *preference, const int verb, const char *keyword, void *target, const int type, const int count) {
    int j, n;
    char *c;
    RKPreferenceObject *object = RKPreferenceFindKeyword(preference, keyword);
    if (object == NULL) {
        return;
    }
    RKName string;
    int k = snprintf(string, RKNameLength, "Preference.%s", keyword);
    if (type == RKParameterTypeWaveformCalibration) {
        //printf("value = '%s'\n", object->valueString);
        RKWaveformCalibration *calibration = (RKWaveformCalibration *)target;
        n = sscanf(object->valueString, "%s %" SCNu8, calibration->name, &calibration->count);
        k += snprintf(string + k, RKNameLength - k, "'%s' (%d)", calibration->name, calibration->count);

        if (calibration->count > RKMaxFilterCount) {
            RKLog("Warning. Filter count exceeds the limit.  %d > %d\n", calibration->count, RKMaxFilterCount);
            calibration->count = RKMaxFilterCount;
        }
        // Find the first calibration value (Zh0)
        c = RKNextNoneWhite(object->valueString);
        c = RKNextNoneWhite(c);
        for (j = 0; j < calibration->count; j++) {
            n = sscanf(c, "%f %f %f %f", &calibration->ZCal[j][0], &calibration->ZCal[j][1], &calibration->DCal[j], &calibration->PCal[j]);
            k += snprintf(string + k, RKNameLength - k, "   %d:(%.2f %.2f %.2f %.2f)", j,
                          calibration->ZCal[j][0],
                          calibration->ZCal[j][1],
                          calibration->DCal[j],
                          calibration->PCal[j]);
            c = RKNextNoneWhite(c);
            c = RKNextNoneWhite(c);
            c = RKNextNoneWhite(c);
            c = RKNextNoneWhite(c);
        }
    } else {
        for (int i = 0; i < count; i++) {
            switch (type) {
                case RKParameterTypeInt:
                    ((int *)target)[i] =  (int)MIN(MAX(object->doubleValues[i], -1.0e9), 1.0e9);
                    k += snprintf(string + k, RKNameLength - k, " %u", ((int *)target)[i]);
                    break;
                case RKParameterTypeUInt:
                    ((unsigned int *)target)[i] =  (unsigned int)MIN(MAX(object->doubleValues[i], 0.0), 1.0e9);
                    k += snprintf(string + k, RKNameLength - k, " %u", ((unsigned int *)target)[i]);
                    break;
                case RKParameterTypeBool:
                    ((bool *)target)[i] =  object->boolValues[i];
                    k += snprintf(string + k, RKNameLength - k, " %s", ((bool *)target)[i] ? "True" : "False");
                    break;
                case RKParameterTypeFloat:
                    ((float *)target)[i] =  MIN(MAX(object->doubleValues[i], -1.0e9), 1.0e9);
                    k += snprintf(string + k, RKNameLength - k, " %.4f", ((float *)target)[i]);
                    break;
                case RKParameterTypeDouble:
                    ((double *)target)[i] =  MIN(MAX(object->doubleValues[i], -1.0e9), 1.0e9);
                    k += snprintf(string + k, RKNameLength - k, " %.7f", ((double *)target)[i]);
                    break;
                case RKParameterTypeString:
                    // For string type, count is used as the maximum length, i = count ensure there is only 1 iteration
                    strncpy((char *)target, object->valueString, count);
                    k += snprintf(string + k, RKNameLength - k, " %s", (char *)target);
                    i = count;
                    break;
                default:
                    break;
            }
        }
    }
    if (verb) {
        RKLog(">%s\n", string);
    }
}

