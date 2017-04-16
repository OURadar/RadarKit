#include <RadarKit/RKMisc.h>

// Strip out \r, \n, white space, \10 (BS), etc.
void stripTrailingUnwanted(char *str) {
    char *c = str + strlen(str) - 1;
    while (c >= str && (*c == '\r' || *c == '\n' || *c == ' ' || *c == 10)) {
        *c-- = '\0';
    }
}

#pragma mark -

char *RKNow(void) {
    static char timestr[32];
    time_t utc;
    //
    // Here is a nice reference: http://www.cplusplus.com/ref/ctime/time.html
    //
    time(&utc);
    strftime(timestr, 32, "%Y/%m/%d %T", localtime(&utc));
    return timestr;
}

char *RKGetColor(void) {
    static int c = 0;
    return RKGetColorOfIndex(c++);
}

char *RKGetColorOfIndex(const int i) {
    //
    // Here is a reference: http://misc.flogisoft.com/bash/tip_colors_and_formatting
    //
    const uint8_t colors[] = {197, 214, 226, 46, 50, 33, 99, 164};
    static int k = 3;
    static char str[4][32];
    k = k == 3 ? 0 : k + 1;
    snprintf(str[k], 31, "\033[1;38;5;%dm", colors[i % sizeof(colors)]);
    return str[k];
}

char *RKGetBackgroundColor(void) {
    static int c = 0;
    return RKGetBackgroundColorOfIndex(c++);
}

char *RKGetBackgroundColorOfIndex(const int i) {
    const uint8_t colors[] = {167, 197, 202, 178, 136, 70, 28, 30, 39, 27, 99, 57, 90, 162, 241, 236};
    static int k = 3;
    static char str[4][32];
    k = k == 3 ? 0 : k + 1;
    snprintf(str[k], 31, "\033[1;97;48;5;%dm", colors[i % sizeof(colors)]);
    return str[k];
}

// BUG: This function is limited to an array of 8 elements
char *RKGetValueOfKey(const char *string, const char *key) {
    static int k = 7;
    static char valueStrings[8][256];
    k = k == 7 ? 0 : k + 1;
    char *s, *e;
    size_t len;
    char *valueString = valueStrings[k];
    char *keyPosition = strcasestr(string, key);
    
    if (keyPosition != NULL) {
        // Find start of the value
        s = strchr(keyPosition + strlen(key), ':');
        if (s != NULL) {
            do {
                s++;
            } while (*s == '"' || *s == '\'' || *s == ' ');
        } else {
            return NULL;
        }
        // Array
        if (*s == '[') {
            // Find the end of bracket, return the entire array
            e = s + 1;
            while (*e != ']') {
                e++;
            }
            len = e - s + 1;
            if (len > 0) {
                strncpy(valueString, s, len);
                valueString[len] = '\0';
            } else {
                valueString[0] = '\0';
            }
            return valueString;
        } else if (*s == '{') {
            // Find the end of curly bracket, return the entire array
            e = s;
            while (*e != '}') {
                e++;
            }
            len = e - s + 1;
            if (len > 0) {
                strncpy(valueString, s, len);
                valueString[len] = '\0';
            } else {
                valueString[0] = '\0';
            }
            return valueString;
        }
        // Find end of the value
        e = s;
        while (*e != '"' && *e != '\'' && *e != ',' && *e != '}' && *e != ']') {
            e++;
        }
        len = e - s;
        if (len > 0) {
            strncpy(valueString, s, len);
            valueString[len] = '\0';
        } else {
            valueString[0] = '\0';
        }
        return valueString;
    }
    return NULL;
}

int RKIndentCopy(char *dst, char *src) {
    int k = 0;
    char *e, *s = src;
    do {
        e = strchr(s, '\n');
        if (e) {
            *e = '\0';
            k += sprintf(dst + k, "    %s\n", s);
            s = e + 1;
        }
    } while (e != NULL);
    k += sprintf(dst + k, "    %s", s);
    return k;
}

#pragma mark -

////////////////////////////////////////////////
//
//  Integer to string with 3-digit grouping
//
char *RKIntegerToCommaStyleString(const long num) {
    int i, j, k;
    static int ibuf = 0;
    static char stringBuffer[8][32];

    const int r = ibuf;

    ibuf = ibuf == 7 ? 0 : ibuf + 1;

    sprintf(stringBuffer[r], "%ld", num);
    if (num < 1000) {
        return stringBuffer[r];
    } else {
        k = (int)(strlen(stringBuffer[r]) - 1) / 3;
        i = (int)(strlen(stringBuffer[r]) + k);
        j = 1;
        stringBuffer[r][i] = '\0';
        while (i > 0) {
            i--;
            stringBuffer[r][i] = stringBuffer[r][i - k];
            if (j > 3) {
                j = 0;
                stringBuffer[r][i] = ',';
                k--;
            }
            j++;
        }
    }
    return stringBuffer[r];
}

////////////////////////////////////////////////
//
//  Float to string with 3-digit grouping
//
char *RKFloatToCommaStyleString(const double num) {
    char *intString = RKIntegerToCommaStyleString((long)num);
    if (num < 0.0 && num > -1.0) {
        sprintf(intString, "-0");
    }
    snprintf(intString + strlen(intString), 32 - strlen(intString), ".%03.0f", 1000.0f * fabs(num - trunc(num)));
    return intString;
}

#pragma mark -

// Timeval difference through subtraction
// subtrahend is strubtracted from minuend
// m - s
double RKTimevalDiff(const struct timeval m, const struct timeval s) {
    return (double)m.tv_sec - (double)s.tv_sec + 1.0e-6 * ((double)m.tv_usec - (double)s.tv_usec);
}

// Timespec difference through subtraction
// m - s
double RKTimespecDiff(const struct timespec m, const struct timespec s) {
    return (double)m.tv_sec - (double)s.tv_sec + 1.0e-9 * ((double)m.tv_nsec - (double)s.tv_nsec);
}

void RKUTCTime(struct timespec *t) {
#ifdef __MACH__
    // OS X does not have clock_gettime()
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    t->tv_sec = mts.tv_sec;
    t->tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, t);
#endif
}

bool RKFilenameExists(const char *filename) {
    if (access(filename, R_OK | W_OK) == 0) {
        return true;
    } else {
        struct stat buf;
        int r = stat(filename, &buf);
        if (r == 0 && S_ISREG(buf.st_mode)) {
            return true;
        }
    }
    return false;
}

void RKPreparePath(const char *filename) {
    char path[1024];
    strcpy(path, filename);
    char *c = strrchr(path, '/');
    if (c == NULL) {
        return;
    }
    *c = '\0';
    DIR *dir = opendir(path);
    if (dir == NULL) {
        char cmd[1024];
        sprintf(cmd, "mkdir -p %s", path);
        system(cmd);
    } else {
        closedir(dir);
    }
    return;
}

char *RKSignalString(const int signal) {
    static char string[32];
    switch (signal) {
        case SIGHUP:    sprintf(string, "SIGHUP"); break;
        case SIGINT:    sprintf(string, "SIGINT"); break;
        case SIGQUIT:   sprintf(string, "SIGQUIT"); break;
        case SIGILL:    sprintf(string, "SIGILL"); break;
        case SIGTRAP:   sprintf(string, "SIGTRAP"); break;
        case SIGABRT:   sprintf(string, "SIGABRT = SIGIOT"); break;
#if  (defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE))
        case SIGPOLL:   sprintf(string, "SIGPOLL"); break;
#else	/* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
        case SIGEMT:    sprintf(string, "SIGEMT"); break;
#endif	/* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
        case SIGFPE:    sprintf(string, "SIGFPE"); break;
        case SIGKILL:   sprintf(string, "SIGKILL"); break;
        case SIGBUS:    sprintf(string, "SIGBUS"); break;
        case SIGSEGV:   sprintf(string, "SIGSEGV"); break;
        case SIGSYS:    sprintf(string, "SIGSYS"); break;
        case SIGPIPE:   sprintf(string, "SIGPIPE"); break;
        case SIGALRM:   sprintf(string, "SIGALRM"); break;
        case SIGTERM:   sprintf(string, "SIGTERM"); break;
        case SIGURG:    sprintf(string, "SIGURG"); break;
        case SIGSTOP:   sprintf(string, "SIGSTOP"); break;
        case SIGTSTP:   sprintf(string, "SIGTSTP"); break;
        case SIGCONT:   sprintf(string, "SIGCONT"); break;
        case SIGCHLD:   sprintf(string, "SIGCHLD"); break;
        case SIGTTIN:   sprintf(string, "SIGTTIN"); break;
        case SIGTTOU:   sprintf(string, "SIGTTOU"); break;
#if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
        case SIGIO: sprintf(string, "SIGIO"); break;
#endif
        case SIGXCPU:   sprintf(string, "SIGXCPU"); break;
        case SIGXFSZ:   sprintf(string, "SIGXFSZ"); break;
        case SIGVTALRM: sprintf(string, "SIGVTALRM"); break;
        case SIGPROF:   sprintf(string, "SIGPROF"); break;
#if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
        case SIGWINCH:  sprintf(string, "SIGWINCH"); break;
        case SIGINFO:   sprintf(string, "SIGINFO"); break;
#endif
        case SIGUSR1:   sprintf(string, "SIGUSR1"); break;
        case SIGUSR2:   sprintf(string, "SIGUSR2"); break;
        default: sprintf(string, "SIGUNKNOWN"); break;
    }
    return string;
}

int RKStripTail(char *string) {
    int k = 0;
    char *c = string + strlen(string) - 1;
    while (c != string && (*c == '\r' || *c == '\n' || *c == ' ')) {
        *c-- = '\0';
        k++;
    }
    return k;
}

float RKUMinDiff(const float m, const float s) {
    float d = m - s;
    if (d < -180.0f) {
        d += 360.0f;
    } else if (d >= 180.0f) {
        d -= 360.0f;
    }
    if (d < 0.0f) {
        d = -d;
    }
    return d;
}

// ks = start of a keyword, should begin with quote or space
char *RKExtractJSON(char *ks, uint8_t *type, char *key, char *value) {
    char *ke;
    while (*ks != '"' && *ks != '\0') {
        ks++;
    }
    ke = ++ks;
    while (*ke != '"' && *ke != '\0') {
        ke++;
    }
    if (*ke == '\0') {
        fprintf(stderr, "Expected a close quote for keyword %s\n", ks);
    }
    // Now a keyword is in betwee ks & ke
    strncpy(key, ks, ke - ks);
    key[ke - ks] = '\0';
    char *os, *oe;
    os = ke + 1;
    while (*os != ':' && *os != '\0') {
        os++;
    }
    // Space character(s) in between ':' and '{'
    os++;
    while (*os == ' ') {
        os++;
    }
    if (*os == '{') {
        // This is an object, recursively call myself.
        *type = RKJSONObjectTypeObject;
        oe = os + 1;
        while (*oe != '}') {
            oe++;
        }
        oe++;
        strncpy(value, os, oe - os);
        value[oe - os] = '\0';
        #if defined(DEBUG_HEAVY)
        fprintf(stderr, "Key '%s' Object '%s'\n", key, value);
        #endif
    } else if (*os == '[') {
        // This is an array
        *type = RKJSONObjectTypeArray;
        oe = os + 1;
        while (*oe != ']') {
            oe++;
        }
        oe++;
        strncpy(value, os, oe - os);
        value[oe - os] = '\0';
        #if defined(DEBUG_HEAVY)
        fprintf(stderr, "Key '%s' Array '%s'\n", key, value);
        #endif
    } else if (*os == '"') {
        // This is a string
        *type = RKJSONObjectTypeString;
        oe = os + 1;
        while (*oe != '"') {
            oe++;
        }
        oe++;
        strncpy(value, os, oe - os);
        value[oe - os] = '\0';
    } else {
        // This is just a plain value
        *type = RKJSONObjectTypePlain;
        oe = os + 1;
        while (*oe != ',' && *oe != '}') {
            oe++;
        }
        strncpy(value, os, oe - os);
        value[oe - os] = '\0';
        #if defined(DEBUG_HEAVY)
        fprintf(stderr, "Key '%s' Value '%s'\n", key, value);
        #endif
    }
    while (*oe == ' ') {
        oe++;
    }
    return oe;
}

void RKGoThroughKeywords(const char *string) {

    char *str = (char *)malloc(strlen(string));
    char *key = (char *)malloc(1024);
    char *obj = (char *)malloc(1024);
    char *subKey = (char *)malloc(1024);
    char *subObj = (char *)malloc(1024);
    uint8_t type;
    uint8_t subType;

    strcpy(str, string);

    char *ks;
    char *sks;
    if (*str != '{') {
        fprintf(stderr, "Expected '{'.\n");
    }

    ks = str + 1;
    while (*ks != '\0' && *ks != '}') {
        ks = RKExtractJSON(ks, &type, key, obj);
        if (type != RKJSONObjectTypeObject) {
            continue;
        }
        sks = obj + 1;
        while (*sks != '\0' && *sks != '}') {
            sks = RKExtractJSON(sks, &subType, subKey, subObj);
            if (strcmp("Enum", subKey)) {
                continue;
            }
            fprintf(stderr, "%s --> '%s' --> %d\n", key, subObj, atoi(subObj));
        }
    }

    free(subKey);
    free(subObj);
    free(str);
    free(key);
    free(obj);
}

void RKReplaceKeyValue(char *string, const char *key, int value) {
    int k;
    char *s = strstr(string, key);
    char *e;
    char valueString[32];
    while (s != NULL) {
        s += strlen(key);
        while (*s != '\0' && (*s == '"' || *s == ' ' || *s == ':')) {
            s++;
        }
        e = s;
        while (*e != '\0' && (*e != '}' && *e != ',' && *e != ']')) {
            e++;
        }
        // Now value should be in between s & e
        k = sprintf(valueString, "%d", value);
        if (e - s < k) {
            // Need to add another character
            memmove(e + k - 1, e, strlen(e));
        }
        strncpy(s, valueString, k);
        s = strstr(e, key);
    }
}

long RKCountFilesInPath(const char *path) {
    //struct dirent *dir;
    DIR *did = opendir(path);
    if (did == NULL) {
        fprintf(stderr, "Unable to open directory %s\n", path);
        return 0;
    }
    long offset = telldir(did);
    printf("offset = %ld\n", offset);
    seekdir(did, offset + 3);
    return telldir(did);
}

char *RKLastTwoPartsOfPath(const char *path) {
    char *a0 = strchr((char *)path, '/');
    char *a1 = strchr(a0 + 1, '/');
    char *a2 = strchr(a1 + 1, '/');
    if (a0 == NULL) {
        return a0;
    } else if (a1 != NULL) {
        if (a2 == NULL) {
            return a1;
        } else if (a2 != NULL) {
            do {
                a0 = a1;
                a1 = a2;
                a2 = strchr(a2 + 1, '/');
            } while (a2 != NULL);
        }
    }
    return a0;
}
