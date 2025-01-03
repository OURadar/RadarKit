#include <RadarKit/RKMisc.h>

#pragma mark - Colors

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
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (i == 0xDEADBEEF) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKGetColorOfIndex: Destroying static mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }

    pthread_mutex_lock(&lock);
    char *string = str[k];
    k = k == 3 ? 0 : k + 1;
    pthread_mutex_unlock(&lock);

    snprintf(string, sizeof(str[0]), "\033[38;5;%dm", colors[i % sizeof(colors)]);
    return string;
}

char *RKGetBackgroundColor(void) {
    static int c = 0;
    return RKGetBackgroundColorOfIndex(c++);
}

char *RKGetBackgroundColorOfIndex(const int i) {
    const uint8_t colors[] = {
        210, 197, 202, 130, 136,
         70,  28,  22,  24,  30,
         31,  38,  39,  33,  27,
         99,  57,  90, 162, 241,
        236
    };
    static int k = 3;
    static char str[4][32];
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (i == 0xDEADBEEF) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKGetBackgroundColorOfIndex: Destroying static mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }

    pthread_mutex_lock(&lock);
    k = k == 3 ? 0 : k + 1;
    pthread_mutex_unlock(&lock);

    snprintf(str[k], sizeof(str[0]), "\033[97;48;5;%dm", colors[i % sizeof(colors)]);
    return str[k];
}

char *RKGetBackgroundColorOfCubeIndex(const int c) {
    int i = (c / 100) % 10;
    int j = (c / 10) % 10;
    int k = c % 10;
    static int s = 3;
    static char str[4][32];
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (c == 0xDEADBEEF) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKGetBackgroundColorOfCubeIndex: Destroying static mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }

    pthread_mutex_lock(&lock);
    s = s == 3 ? 0 : s + 1;
    pthread_mutex_unlock(&lock);

    int f = j > 2 ? 0 : 15;
    snprintf(str[s], sizeof(str[0]), "\033[38;5;%d;48;5;%dm", f, 16 + i * 36 + j * 6 + k);
    return str[s];
}

#pragma mark - JSON / Dictionary / Parsing

// ks = start of a keyword, should begin with quote or space
char *RKExtractJSON(char *ks, uint8_t *type, char *key, char *value) {
    char *ke = NULL;
    if (*ks == '\0' || strlen(ks) < 3) {
        fprintf(stderr, "Empty JSON string.\n");
        return NULL;
    }
    while (*ks != '"' && *ks != '\0') {
        ks++;
    }
    ke = ++ks;
    while (*ke != '"' && *ke != '\0') {
        ke++;
    }
    if (*ke == '\0') {
        fprintf(stderr, "Expected a close quote for keyword %s\n", ks == NULL ? "(NULL)" : ks);
        fprintf(stderr, "Original: %s\n", ks);
        if (ke == NULL) {
            return NULL;
        }
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

// LIMITATIONS:
//  - Not a generic function, many assumptions
//  - Static array variable limited to 8 elements
//  - Cannot handle certain conditions within quoted strings
//
char *RKGetValueOfKey(const char *string, const char *key) {
    static char valueStrings[8][256];
    static int k = 7;
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    if (string == NULL && key == NULL) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKGetValueOfKey: Destroying static mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }
    pthread_mutex_lock(&lock);
    char *valueString = valueStrings[k];
    k = k == 7 ? 0 : k + 1;
    pthread_mutex_unlock(&lock);
    char *s, *e;
    size_t len;
    char quotedKey[256];
    sprintf(quotedKey, "\"%s\"", key);
    char *keyPosition = strcasestr(string, quotedKey);
    if (keyPosition == NULL) {
        sprintf(quotedKey, "'%s'", key);
        keyPosition = strcasestr(string, quotedKey);
    }

    if (keyPosition != NULL) {
        // Find start of the value
        s = strchr(keyPosition + strlen(quotedKey), ':');
        if (s != NULL) {
            do {
                s++;
            } while (*s == ' ' || *s == '\r' || *s == '\n');
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
            // Find the end of curly bracket, return the entire dictionary
            e = s + 1;
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
        } else if (*s == '"') {
            // Find the end of double quote, return the entire string
            e = s + 1;
            while (*e != '"') {
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
        } else if (*s == '\'') {
            // Find the end of single quote, return the entire string
            e = s + 1;
            while (*e != '\'') {
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
        while (*e != ',' && *e != '}' && *e != ']') {
            e++;
        }
        len = MIN(255, e - s);
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

void RKReplaceAllValuesOfKey(char *string, const char *key, int value) {
    int k;
    char *s = strstr(string, key);
    char *e;
    size_t l;
    char valueString[32];
    while (s != NULL) {
        s += strlen(key);
        while (*s != '\0' && (*s == '"' || *s == ' ' || *s == ':')) {
            s++;
        }
        if (*s == '\0') {
            break;
        }
        e = s;
        while (*e != '\0' && (*e != '}' && *e != ',' && *e != ']')) {
            e++;
        }
        if (*e == '\0') {
            fprintf(stderr, "RKReplaceAllValuesOfKey() encountered an incomplete JSON string.\n");
            return;
        }
        // Now value should be in between s & e
        k = sprintf(valueString, "%d", value);
        if (k < 0) {
            fprintf(stderr, "RKReplaceAllValuesOfKey() encountered an expected results from sprintf().\n");
            return;
        }
        if (e - s < k) {
            // Need to add another character
            l = strlen(e);
            memmove(e + k - 1, e, l);
            *(e + k + -1 + l) = '\0';
        }
        memcpy(s, valueString, k);
        //strncpy(s, valueString, k);
        //snprintf(s, k, "%s", valueString) < 0 ? abort() : (void)0;
        s = strstr(e, key);
    }
}

void RKReplaceEnumOfKey(char *string, const char *key, int value) {
    int k;
    char *s = strcasestr(string, key);
    char *e;
    size_t l;
    char valueString[32];
    if (s == NULL) {
        return;
    }
    s = strstr(s, "Enum");
    if (s == NULL) {
        return;
    }
    s += 6;
    //printf("Enum of %s found.  %s\n", key, s);
    while (*s != '\0' && (*s == '"' || *s == ' ' || *s == ':')) {
        s++;
    }
    if (*s == '\0') {
        return;
    }
    e = s;
    while (*e != '\0' && (*e != '}' && *e != ',' && *e != ']')) {
        e++;
    }
    if (*e == '\0') {
        fprintf(stderr, "RKReplaceEnumOfKey() encountered an incomplete JSON string.\n");
        return;
    }
    // Now value should be in between s & e
    k = sprintf(valueString, "%d", value);
    if (k < 0) {
        fprintf(stderr, "RKReplaceEnumOfKey() encountered an expected results from sprintf().\n");
    }
    //printf("s = %p   e = %p   valueString = %s   k = %d ==? %d\n", s, e, valueString, (int)(e - s), k);
    if (e - s < k) {
        // Need to add another character
        l = strlen(e);
        memmove(e + k - 1, e, l);
        *(e + k + -1 + l) = '\0';
    }
    memcpy(s, valueString, k);
}

void RKReviseLogicalValues(char *string) {
    char *token;
    token = strcasestr(string, "\"true\"");
    while (token) {
        sprintf(token, "true");
        memmove(token + 4, token + 6, strlen(token + 6));
        memset(token + 4 + strlen(token + 6), '\0', 2 * sizeof(char));
        token = strcasestr(token + 6, "\"true\"");
    }
    token = strcasestr(string, "\"false\"");
    while (token) {
        sprintf(token, "false");
        memmove(token + 5, token + 7, strlen(token + 7));
        memset(token + 5 + strlen(token + 7), '\0', 2 * sizeof(char));
        token = strcasestr(token + 7, "\"false\"");
    }
}

// RKJSON* functions

char *RKJSONSkipWhiteSpaces(const char *haystack) {
    char *c = (char *)haystack;
    while (*c == ' ' || *c == '\r' || *c == '\n') {
        c++;
    }
    return c;
}

char *RKJSONScanPassed(char *destination, const char *source, const char delimiter) {
    char *c = (char *)source;
    char *d = destination;
    bool singleQuote = false;
    bool doubleQuote = false;
    bool slash = false;
    bool space = false;
    int curly = 0;
    int round = 0;
    int square = 0;

    #if defined(DEBUG_GET_NEXT_ELEMENT)
    char str[32];
    int k;
    #endif

    char *end = c + strlen(source);

    c = RKJSONSkipWhiteSpaces(c);

    while (c < end) {
        switch (*c) {
            case '\\':
                slash = true;
                break;
            case '\'':
                singleQuote = !singleQuote;
                break;
            case '"':
                if (!slash) {
                    doubleQuote = !doubleQuote;
                } else {
                    slash = false;
                }
                break;
            case '{':
                curly++;
                break;
            case '}':
                curly--;
                break;
            case '(':
                round++;
                break;
            case ')':
                round--;
                break;
            case '[':
                square++;
                break;
            case ']':
                square--;
                break;
            default:
                break;
        }

        if (doubleQuote) {
            *d++ = *c;
        } else {
            // Allow only one space character right after a color (:) or a comma (,)
            if (*c == ':' || *c == ',') {
                space = true;
            }
            if (*c != '\r' && *c != '\n') {
                if (*c != ' ') {
                    *d++ = *c;
                } else if (*c == ' ' && space == true) {
                    space = false;
                    *d++ = *c;
                }
            }
        }

        #if defined(DEBUG_GET_NEXT_ELEMENT)
        k = sprintf(str, "\033[48;5;238;38;5;15m");
        switch (*c) {
            case '\r':
                k += sprintf(str + k, "\\r");
                break;
            case '\n':
                k += sprintf(str + k, "\\n");
                break;
            default:
                k += sprintf(str + k, "%c", *c);
                break;
        }
        *(d + 1) = '\0';
        sprintf(str + k, "\033[m%s", k == 20 ? " " : "");
        printf("%s '%d \"%d {%d (%d [%d \\%d s%d \033[48;5;238m%s\033[m \n",
            str, singleQuote, doubleQuote, curly, round, square, slash, space, destination);
        #endif

        c++;

        if (singleQuote == false &&
            doubleQuote == false &&
            curly == 0 &&
            round == 0 &&
            square == 0 &&
            (*c == delimiter || *c == '}' || *c == ')' || *c == ']' || *c == '\0')) {
            c++;
            break;
        }
    }
    *d = '\0';
    c = RKJSONSkipWhiteSpaces(c);
    return c;
}

char *RKJSONGetElement(char *element, const char *source) {
    return RKJSONScanPassed(element, source, ',');
}

char *RKJSONKeyValueFromElement(char *key, char *value, const char *source) {
    char *c = (char *)source;
    c = RKJSONScanPassed(key, c, ':');
    c = RKJSONScanPassed(value, c, '\0');
    RKUnquote(key);
    return c;
}

char *RKJSONGetValueOfKey(char *keyValue, char *key, char *value, const char *name, const char *source) {
    char *c = (char *)source;
    if (*c == '{') {
        c++;
    }
    *key = '\0';
    *value = '\0';
    *keyValue = '\0';
    do {
        c = RKJSONGetElement(keyValue, c);
        RKJSONKeyValueFromElement(key, value, keyValue);
    } while (strlen(keyValue) > 0 && strcasecmp(key, name));
    return c;
}

#pragma mark -

////////////////////////////////////////////////
//
//  Integer to string with 3-digit grouping
//
char *RKUIntegerToCommaStyleString(const unsigned long long num) {
    int i, j, k;
    static int ibuf = 0;
    static char stringBuffer[32][32];
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (num == (unsigned long long)0xFEEDFACECAFEBEEF) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKUIntegerToCommaStyleString: Destroying mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }

    pthread_mutex_lock(&lock);
    char *string = stringBuffer[ibuf];
    ibuf = ibuf == 31 ? 0 : ibuf + 1;
    pthread_mutex_unlock(&lock);

    i = sprintf(string, "%llu", num);
    if (i <= 3) {
        return string;
    }

    k = (int)(strlen(string) - 1) / 3;
    i = (int)(strlen(string) + k);
    j = 1;
    string[i] = '\0';
    while (i > 0) {
        i--;
        string[i] = string[i - k];
        if (j > 3) {
            j = 0;
            string[i] = ',';
            k--;
        }
        j++;
    }
    return string;
}

char *RKIntegerToCommaStyleString(const long long num) {
    int i, j, k;
    static int ibuf = 0;
    static char stringBuffer[32][32];
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (num == (long long)0xFEEDFACECAFEBEEF) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKIntegerToCommaStyleString: Destroying mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }

    pthread_mutex_lock(&lock);
    char *string = stringBuffer[ibuf];
    ibuf = ibuf == 31 ? 0 : ibuf + 1;
    pthread_mutex_unlock(&lock);

    i = sprintf(string, "%lld", num);
    if (i <= 3) {
        return string;
    }

    k = (int)(strlen(string) - 1) / 3;
    i = (int)(strlen(string) + k);
    j = 1;
    string[i] = '\0';
    while (i > 0) {
        i--;
        string[i] = string[i - k];
        if (j > 3) {
            j = 0;
            string[i] = ',';
            k--;
        }
        j++;
    }
    return string;
}

char *RKIntegerToHexStyleString(const long long num) {
    static int ibuf = 0;
    static char stringBuffer[32][32];
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (num == (long long)0xFEEDFACECAFEBEEF) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKIntegerToHexStyleString: Destroying mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }

    pthread_mutex_lock(&lock);
    char *string = stringBuffer[ibuf];
    ibuf = ibuf == 31 ? 0 : ibuf + 1;
    pthread_mutex_unlock(&lock);

    sprintf(string, "0x%04llx", num);

    return string;
}

////////////////////////////////////////////////
//
//  Float to string with 3-digit grouping
//
char *RKFloatToCommaStyleStringAndDecimals(const double num, const int decimals) {
    int i, j, k;
    static int ibuf = 0;
    static char stringBuffer[32][32];
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (num == (double)0xFEEDFACECAFEBEEF) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKFloatToCommaStyleString: Destroying mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }

    pthread_mutex_lock(&lock);
    char *string = stringBuffer[ibuf];
    ibuf = ibuf == 31 ? 0 : ibuf + 1;
    pthread_mutex_unlock(&lock);

    if (decimals <= 0) {
        i = sprintf(string, "%.0f", num);
    } else {
        i = sprintf(string, "%.*f", decimals, num);
    }
    if ((num < 0 && i - decimals <= 4) || (num >= 0 && i - decimals <= 3) || !isfinite(num)) {
        return string;
    }

    k = (int)(strlen(string) - decimals - (num < 0.0 ? 3 : 2)) / 3;
    const int frac = decimals + 1;
    memmove(string + i - frac + k, string + i - frac, frac + 1);

    i -= frac - k;
    j = 1;
    while (i > 0) {
        i--;
        string[i] = string[i - k];
        if (j > 3) {
            j = 0;
            string[i] = ',';
            k--;
        }
        j++;
    }
    return string;
}

char *RKFloatToCommaStyleString(const double num) {
    return RKFloatToCommaStyleStringAndDecimals(num, 3);
}

#pragma mark - Time

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

struct timeval RKTimeStringInISOFormatToTimeval(const char *string) {
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    strptime(string, "%Y-%m-%dT%H:%M:%S", &tm);
    // Replace timezone with UTC
    char *dot = strrchr(string, '.');
    double frac = 0.0;
    if (dot) {
        sscanf(dot, ".%lf", &frac);
    }
    struct timeval t = {
        .tv_sec = mktime(&tm),
        .tv_usec = (suseconds_t)(frac * 1.0e6)
    };
    return t;
}

double RKTimeStringISOToTimeDouble(const char *string) {
    struct timeval t = RKTimeStringInISOFormatToTimeval(string);
    return (double)t.tv_sec + (double)t.tv_usec * 1.0e-6;
}

char *RKTimevalToString(const struct timeval time, const int format, const bool isUTC) {
    static int ibuf = 0;
    static char stringBuffer[16][64];
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    if (time.tv_sec == (unsigned long)0xFEEDFACE) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKTimevalToString: Destroying mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return NULL;
    }
    pthread_mutex_lock(&lock);
    char *string = stringBuffer[ibuf];
    ibuf = ibuf == 15 ? 0 : ibuf + 1;
    pthread_mutex_unlock(&lock);

    struct tm *(*time_fn)(const time_t *) = isUTC ? localtime : gmtime;

    int n = 0, o = 0;
    char formatString[8];
    if (format == 1180) {
        strftime(string, 20, "%Y-%m-%dT%H:%M:%S", time_fn(&time.tv_sec));
    } else if (format >= 1080) {
        strftime(string, 20, "%Y/%m/%d %H:%M:%S", time_fn(&time.tv_sec));
        n = CLAMP(format - 1080, 0, 6);
        o = 19;
    } else if (format >= 860) {
        strftime(string, 16, "%Y%m%d-%H%M%S", time_fn(&time.tv_sec));
        n = CLAMP(format - 860, 0, 6);
        o = 15;
    } else if (format >= 800) {
        strftime(string, 9, "%Y%m%d", time_fn(&time.tv_sec));
        n = CLAMP(format - 800, 0, 6);
        o = 8;
    } else if (format >= 80) {
        strftime(string, 9, "%H:%M:%S", time_fn(&time.tv_sec));
        n = CLAMP(format - 80, 0, 6);
        o = 8;
    } else if (format >= 60) {
        strftime(string, 7, "%H%M%S", time_fn(&time.tv_sec));
        n = CLAMP(format - 60, 0, 6);
        o = 6;
    } else {
        strftime(string, 16, "%Y%m%d-%H%M%S", time_fn(&time.tv_sec));
    }
    if (n > 0) {
        snprintf(formatString, 7, ".%%0%du", n);
        snprintf(string + o, 63 - o, formatString, (uint32_t)time.tv_usec / (uint32_t)pow(10, 6 - n));
    }
    return string;
}

char *RKTimeDoubleToString(const double time, const int format, const bool isUTC) {
    const struct timeval t = {
        .tv_sec = (time_t)time,
        .tv_usec = (suseconds_t)((time - (double)((time_t)time)) * 1.0e6)
    };
    return RKTimevalToString(t, format, isUTC);
}

#pragma mark - File / Path

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
    struct stat s;
    char path[strlen(filename) + 1];
    strcpy(path, filename);
    char *c = strrchr(path, '/');
    if (c == NULL) {
        return;
    }
    if (lstat(path, &s) && errno == ENOENT) {
        // Recursively create paths that do not exist
        c = path;
        size_t n = strlen(path);
        while ((c) && (size_t)(c - path) < n) {
            if ((c = strchr(c, '/')) == NULL) {
                break;
            }
            *c = '\0';
            //printf("path = |%s|  n = %zu\n", path, (size_t)(c - path));
            if (strlen(path)) {
                if (lstat(path, &s) && errno == ENOENT) {
                    // printf("mkdir %s\n", path);
                    if (mkdir(path, 0755)) {
                        fprintf(stderr, "Error creating directory '%s'\n", path);
                        fprintf(stderr, "Input filename '%s'\n", filename);
                    }
                }
            }
            *c++ = '/';
        }
    }
    return;
}

long RKCountFilesInPath(const char *path) {
    struct dirent *entry;
    DIR *did = opendir(path);
    if (did == NULL) {
        fprintf(stderr, "Unable to open directory %s\n", path);
        return 0;
    }
    long count = 0;
    while ((entry = readdir(did)) != NULL) {
        // Skip the special entries "." and ".."
        if (entry->d_type == DT_REG) {
            count++;
        }
    }
    closedir(did);
    return count;
}

char *RKLastPartOfPath(const char *path) {
    char *r = strrchr((char *)path, '/');
    if (r == NULL) {
        return (char *)path;
    }
    return r + 1;
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
    return a0 + 1;
}

char *RKLastNPartsOfPath(const char *path, const int n) {
    char *a = (char *)path + strlen(path);
    int k = 0;
    do {
        if (*--a == '/') {
            k++;
        }
    } while (a > path && k < n);
    return a + 1;
}

char *RKFolderOfFilename(const char *filename) {
    static char folder[1024];
    char *s = strrchr((char *)filename, '/');
    if (s == NULL) {
        strcpy(folder, ".");
    } else if (s == filename) {
        strcpy(folder, "/");
    } else {
        strncpy(folder, filename, MIN((size_t)(s - filename), sizeof(folder) - 1));
        folder[sizeof(folder) - 1] = '\0';
    }
    return folder;
}

char *RKFileExtension(const char *filename) {
    static char ext[16];
    char *e = strrchr((char *)filename, '.');
    if (e == NULL) {
        ext[0] = '\0';
        return ext;
    }
    if (!strcmp(e, ".gz") || !strcmp(e, ".xz")) {
        *e = '\0';
        char *f = strrchr(filename, '.');
        *e = '.';
        if (f != NULL) {
            e = f;
        }
    }
    strcpy(ext, e);
    return ext;
}

char *RKPathStringByExpandingTilde(const char *path) {
    static char fullpath[1024];
    int k;
    char *c = (char *)path;
    while (*c == ' ') {
        c++;
    }
    if (*c == '~') {
        char *home = getenv("HOME");
        if (home != NULL) {
            k = snprintf(fullpath, sizeof(fullpath), "%s", getenv("HOME"));
            k--;
            if (fullpath[k] == '/') {
                fullpath[k] = '\0';
            } else {
                k++;
            }
            snprintf(fullpath + k, sizeof(fullpath) - k, "/%s", c + 2);
        } else {
            fprintf(stderr, "Error. HOME environmental variable not set.\n");
            snprintf(fullpath, sizeof(fullpath), "%s", c);
        }
    } else {
        snprintf(fullpath, sizeof(fullpath), "%s", c);
    }
    return fullpath;
}

void RKReplaceFileExtension(char *filename, const char *pattern, const char *replacement) {
    char *needle = strstr(filename, pattern);
    if (needle) {
        strcpy(needle, replacement);
    }
}

#pragma mark - Enum to String

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

#pragma mark - String

// Strip out \r, \n, white space, \10 (BS), etc.
int RKStripTail(char *string) {
    int k = 0;
    char *c = string + strlen(string) - 1;
    while (c != string && (*c == '\r' || *c == '\n' || *c == ' ' || *c == 10)) {
        *c-- = '\0';
        k++;
    }
    return k;
}

int RKUnquote(char *string) {
    char q = string[0];
    if (q != '"' && q != '\'') {
        return (int)strlen(string);
    }
    char *e = strrchr(string + 1, q);
    int k = (int)(e - string - 1);
    memmove(string, string + 1, k);
    string[k] = '\0';
    return k;
}

int RKIndentCopy(char *dst, char *src, const int width) {
    int k = 0;
    char *e, *s = src;
    if (width == 0) {
        k = sprintf(dst, "%s", src);
        return k;
    }
    size_t w;
    do {
        e = strchr(s, '\n');
        if (e) {
            w = e - s + 1;
            memset(dst + k, ' ', width);
            memcpy(dst + k + width, s, w);
            k += width + w;
            s = e + 1;
        }
    } while (e != NULL);
    *(dst + k) = '\0';
    char indent[width + 1];
    memset(indent, ' ', width);
    indent[width] = '\0';
    k += sprintf(dst + k, "%s%s", indent, s);
    return k;
}

int RKStringCenterized(char *dst, const char *src, const int width) {
    dst[width] = '\0';
    int c = (int)strlen(src);
    if (c > width - 1) {
        c = width - 1;
    }
    int l = (width - c) / 2;
    memset(dst, ' ', width);
    memcpy(dst + l, src, c);
    return width;
}

char *RKNextNoneWhite(const char *string) {
    char *c = (char *)string;
    // The current character should be non-white but skip it if it is
    while (*c != '\0' && (*c == ' ' || *c == '\t')) {
        c++;
    }
    // Then, forward to the next white space
    while (*c != '\0' && (*c != ' ' && *c != '\t')) {
        c++;
    }
    // Keep going until the white space is over
    while (*c != '\0' && (*c == ' ' || *c == '\t')) {
        c++;
    }
    return c;
}

char *RKLastLine(const char *lines) {
    char *l = (char *)lines;
    char *l2 = l;
    char *r = l;
    while (r != NULL) {
        r = strchr(l, '\n');
        if (r) {
            l = r + 1;
            if (strlen(l) > 1) {
                l2 = l;
            }
        }
    }
    return l2;
}

// Strip the tags \033[...[A-Z][a-z]
char *RKStripEscapeSequence(const char *line) {
    static char string[8192];
    int n;
    char *e;
    char *d = string;
    char *s = (char *)line;
    while (*s != '\0') {
        e = strchr(s, '\033');
        if (e == NULL || *(e + 1) != '[') {
            n = (int)strlen(s);
            memcpy(d, s, n);
            d += n;
            break;
        }
        n = (int)(e - s);
        if (n) {
            memcpy(d, s, n);
            d += n;
        }
        s = e + 1;
        while (*s != '\0' && !((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z'))) {
            s++;
        }
        if (*s == '\0') {
            fprintf(stderr, "Error. Escape sequence not balanced.\n");
            break;
        }
        s++;
    }
    *d = '\0';
    return string;
}

#pragma mark - Math

float RKMinDiff(const float m, const float s) {
    float d = m - s;
    if (d < -180.0f) {
        d += 360.0f;
    } else if (d >= 180.0f) {
        d -= 360.0f;
    }
    return d;
}

float RKUMinDiff(const float m, const float s) {
    float d = RKMinDiff(m, s);
    if (d < 0.0f) {
        d = -d;
    }
    return d;
}

float RKModulo360Diff(const float m, const float s) {
    float d = m - s;
    return d < 0.0f ? d + 360.0f : d;
}

bool RKAngularCrossOver(const float a1, const float a2, const float crossover) {
    float d1 = RKMinDiff(a1, crossover);
    if (fabs(d1) <= 0.0001f) {
        return true;
    }
    float d2 = RKMinDiff(a2, crossover);
    if (d1 * d2 < 0.0f && fabsf(d1) < 10.0f) {
        return true;
    }
    return false;
}

#pragma mark - CPU / Performance

#if defined(__APPLE__)

int sched_getaffinity(pid_t pid, size_t cpu_size, cpu_set_t *cpu_set) {
    long core_count = sysconf(_SC_NPROCESSORS_ONLN);
    cpu_set->count = 0;
    for (int i = 0; i < core_count; i++) {
        cpu_set->count |= (1 << i);
    }
    return 0;
}

int pthread_setaffinity_np(pthread_t thread, size_t cpu_size, cpu_set_t *cpu_set) {
    thread_port_t mach_thread;
    int core = 0;

    for (core = 0; core < 8 * cpu_size; core++) {
        if (CPU_ISSET(core, cpu_set)) break;
    }
    printf("binding to core %d\n", core);
    thread_affinity_policy_data_t policy = { core };
    mach_thread = pthread_mach_thread_np(thread);
    thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
    return 0;
}

#endif

long RKGetCPUIndex(const long v) {
	static long c = 1;
	static long count = 0;
	static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    if (v == 0xDEADBEEF) {
        #if defined(DEBUG_MUTEX_DESTROY)
        fprintf(stderr, "RKGetCPUIndex: Destroying mutex lock ...\n");
        #endif
        pthread_mutex_destroy(&lock);
        return 0;
    }
	pthread_mutex_lock(&lock);
	if (count == 0) {
		count = sysconf(_SC_NPROCESSORS_ONLN);
	}
	pthread_mutex_unlock(&lock);
	return count ? c++ % count : v;
}

long RKGetMemoryUsage(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_CHILDREN, &usage)) {
        fprintf(stderr, "Failed in getrusage().   errno = %d\n", errno);
        return -1;
    }
    return usage.ru_maxrss * 1024;
}

char *RKCountryFromPosition(const double latitude, const double longitude) {
	static char country[64];
	memset(country, 0, sizeof(country));
	char *countries[] = {
		"AD", "42.546245", "1.601554", "Andorra",
		"AE", "23.424076", "53.847818", "United Arab Emirates",
		"AF", "33.93911", "67.709953", "Afghanistan",
		"AG", "17.060816", "-61.796428", "Antigua and Barbuda",
		"AI", "18.220554", "-63.068615", "Anguilla",
		"AL", "41.153332", "20.168331", "Albania",
		"AM", "40.069099", "45.038189", "Armenia",
		"AN", "12.226079", "-69.060087", "Netherlands Antilles",
		"AO", "-11.202692", "17.873887", "Angola",
		"AQ", "-75.250973", "-0.071389", "Antarctica",
		"AR", "-38.416097", "-63.616672", "Argentina",
		"AS", "-14.270972", "-170.132217", "American Samoa",
		"AT", "47.516231", "14.550072", "Austria",
		"AU", "-25.274398", "133.775136", "Australia",
		"AW", "12.52111", "-69.968338", "Aruba",
		"AZ", "40.143105", "47.576927", "Azerbaijan",
		"BA", "43.915886", "17.679076", "Bosnia and Herzegovina",
		"BB", "13.193887", "-59.543198", "Barbados",
		"BD", "23.684994", "90.356331", "Bangladesh",
		"BE", "50.503887", "4.469936", "Belgium",
		"BF", "12.238333", "-1.561593", "Burkina Faso",
		"BG", "42.733883", "25.48583", "Bulgaria",
		"BH", "25.930414", "50.637772", "Bahrain",
		"BI", "-3.373056", "29.918886", "Burundi",
		"BJ", "9.30769", "2.315834", "Benin",
		"BM", "32.321384", "-64.75737", "Bermuda",
		"BN", "4.535277", "114.727669", "Brunei",
		"BO", "-16.290154", "-63.588653", "Bolivia",
		"BR", "-14.235004", "-51.92528", "Brazil",
		"BS", "25.03428", "-77.39628", "Bahamas",
		"BT", "27.514162", "90.433601", "Bhutan",
		"BV", "-54.423199", "3.413194", "Bouvet Island",
		"BW", "-22.328474", "24.684866", "Botswana",
		"BY", "53.709807", "27.953389", "Belarus",
		"BZ", "17.189877", "-88.49765", "Belize",
		"CA", "56.130366", "-106.346771", "Canada",
		"CC", "-12.164165", "96.870956", "Cocos [Keeling] Islands",
		"CD", "-4.038333", "21.758664", "Congo [DRC]",
		"CF", "6.611111", "20.939444", "Central African Republic",
		"CG", "-0.228021", "15.827659", "Congo [Republic]",
		"CH", "46.818188", "8.227512", "Switzerland",
		"CI", "7.539989", "-5.54708", "C™te d'Ivoire",
		"CK", "-21.236736", "-159.777671", "Cook Islands",
		"CL", "-35.675147", "-71.542969", "Chile",
		"CM", "7.369722", "12.354722", "Cameroon",
		"CN", "35.86166", "104.195397", "China",
		"CO", "4.570868", "-74.297333", "Colombia",
		"CR", "9.748917", "-83.753428", "Costa Rica",
		"CU", "21.521757", "-77.781167", "Cuba",
		"CV", "16.002082", "-24.013197", "Cape Verde",
		"CX", "-10.447525", "105.690449", "Christmas Island",
		"CY", "35.126413", "33.429859", "Cyprus",
		"CZ", "49.817492", "15.472962", "Czech Republic",
		"DE", "51.165691", "10.451526", "Germany",
		"DJ", "11.825138", "42.590275", "Djibouti",
		"DK", "56.26392", "9.501785", "Denmark",
		"DM", "15.414999", "-61.370976", "Dominica",
		"DO", "18.735693", "-70.162651", "Dominican Republic",
		"DZ", "28.033886", "1.659626", "Algeria",
		"EC", "-1.831239", "-78.183406", "Ecuador",
		"EE", "58.595272", "25.013607", "Estonia",
		"EG", "26.820553", "30.802498", "Egypt",
		"EH", "24.215527", "-12.885834", "Western Sahara",
		"ER", "15.179384", "39.782334", "Eritrea",
		"ES", "40.463667", "-3.74922", "Spain",
		"ET", "9.145", "40.489673", "Ethiopia",
		"FI", "61.92411", "25.748151", "Finland",
		"FJ", "-16.578193", "179.414413", "Fiji",
		"FK", "-51.796253", "-59.523613", "Falkland Islands",
		"FM", "7.425554", "150.550812", "Micronesia",
		"FO", "61.892635", "-6.911806", "Faroe Islands",
		"FR", "46.227638", "2.213749", "France",
		"GA", "-0.803689", "11.609444", "Gabon",
		"GB", "55.378051", "-3.435973", "United Kingdom",
		"GD", "12.262776", "-61.604171", "Grenada",
		"GE", "42.315407", "43.356892", "Georgia",
		"GF", "3.933889", "-53.125782", "French Guiana",
		"GG", "49.465691", "-2.585278", "Guernsey",
		"GH", "7.946527", "-1.023194", "Ghana",
		"GI", "36.137741", "-5.345374", "Gibraltar",
		"GL", "71.706936", "-42.604303", "Greenland",
		"GM", "13.443182", "-15.310139", "Gambia",
		"GN", "9.945587", "-9.696645", "Guinea",
		"GP", "16.995971", "-62.067641", "Guadeloupe",
		"GQ", "1.650801", "10.267895", "Equatorial Guinea",
		"GR", "39.074208", "21.824312", "Greece",
		"GS", "-54.429579", "-36.587909", "South Georgia and the South Sandwich Islands",
		"GT", "15.783471", "-90.230759", "Guatemala",
		"GU", "13.444304", "144.793731", "Guam",
		"GW", "11.803749", "-15.180413", "Guinea-Bissau",
		"GY", "4.860416", "-58.93018", "Guyana",
		"GZ", "31.354676", "34.308825", "Gaza Strip",
		"HK", "22.396428", "114.109497", "Hong Kong",
		"HM", "-53.08181", "73.504158", "Heard Island and McDonald Islands",
		"HN", "15.199999", "-86.241905", "Honduras",
		"HR", "45.1", "15.2", "Croatia",
		"HT", "18.971187", "-72.285215", "Haiti",
		"HU", "47.162494", "19.503304", "Hungary",
		"ID", "-0.789275", "113.921327", "Indonesia",
		"IE", "53.41291", "-8.24389", "Ireland",
		"IL", "31.046051", "34.851612", "Israel",
		"IM", "54.236107", "-4.548056", "Isle of Man",
		"IN", "20.593684", "78.96288", "India",
		"IO", "-6.343194", "71.876519", "British Indian Ocean Territory",
		"IQ", "33.223191", "43.679291", "Iraq",
		"IR", "32.427908", "53.688046", "Iran",
		"IS", "64.963051", "-19.020835", "Iceland",
		"IT", "41.87194", "12.56738", "Italy",
		"JE", "49.214439", "-2.13125", "Jersey",
		"JM", "18.109581", "-77.297508", "Jamaica",
		"JO", "30.585164", "36.238414", "Jordan",
		"JP", "36.204824", "138.252924", "Japan",
		"KE", "-0.023559", "37.906193", "Kenya",
		"KG", "41.20438", "74.766098", "Kyrgyzstan",
		"KH", "12.565679", "104.990963", "Cambodia",
		"KI", "-3.370417", "-168.734039", "Kiribati",
		"KM", "-11.875001", "43.872219", "Comoros",
		"KN", "17.357822", "-62.782998", "Saint Kitts and Nevis",
		"KP", "40.339852", "127.510093", "North Korea",
		"KR", "35.907757", "127.766922", "South Korea",
		"KW", "29.31166", "47.481766", "Kuwait",
		"KY", "19.513469", "-80.566956", "Cayman Islands",
		"KZ", "48.019573", "66.923684", "Kazakhstan",
		"LA", "19.85627", "102.495496", "Laos",
		"LB", "33.854721", "35.862285", "Lebanon",
		"LC", "13.909444", "-60.978893", "Saint Lucia",
		"LI", "47.166", "9.555373", "Liechtenstein",
		"LK", "7.873054", "80.771797", "Sri Lanka",
		"LR", "6.428055", "-9.429499", "Liberia",
		"LS", "-29.609988", "28.233608", "Lesotho",
		"LT", "55.169438", "23.881275", "Lithuania",
		"LU", "49.815273", "6.129583", "Luxembourg",
		"LV", "56.879635", "24.603189", "Latvia",
		"LY", "26.3351", "17.228331", "Libya",
		"MA", "31.791702", "-7.09262", "Morocco",
		"MC", "43.750298", "7.412841", "Monaco",
		"MD", "47.411631", "28.369885", "Moldova",
		"ME", "42.708678", "19.37439", "Montenegro",
		"MG", "-18.766947", "46.869107", "Madagascar",
		"MH", "7.131474", "171.184478", "Marshall Islands",
		"MK", "41.608635", "21.745275", "Macedonia",
		"ML", "17.570692", "-3.996166", "Mali",
		"MM", "21.913965", "95.956223", "Myanmar",
		"MN", "46.862496", "103.846656", "Mongolia",
		"MO", "22.198745", "113.543873", "Macau",
		"MP", "17.33083", "145.38469", "Northern Mariana Islands",
		"MQ", "14.641528", "-61.024174", "Martinique",
		"MR", "21.00789", "-10.940835", "Mauritania",
		"MS", "16.742498", "-62.187366", "Montserrat",
		"MT", "35.937496", "14.375416", "Malta",
		"MU", "-20.348404", "57.552152", "Mauritius",
		"MV", "3.202778", "73.22068", "Maldives",
		"MW", "-13.254308", "34.301525", "Malawi",
		"MX", "23.634501", "-102.552784", "Mexico",
		"MY", "4.210484", "101.975766", "Malaysia",
		"MZ", "-18.665695", "35.529562", "Mozambique",
		"NA", "-22.95764", "18.49041", "Namibia",
		"NC", "-20.904305", "165.618042", "New Caledonia",
		"NE", "17.607789", "8.081666", "Niger",
		"NF", "-29.040835", "167.954712", "Norfolk Island",
		"NG", "9.081999", "8.675277", "Nigeria",
		"NI", "12.865416", "-85.207229", "Nicaragua",
		"NL", "52.132633", "5.291266", "Netherlands",
		"NO", "60.472024", "8.468946", "Norway",
		"NP", "28.394857", "84.124008", "Nepal",
		"NR", "-0.522778", "166.931503", "Nauru",
		"NU", "-19.054445", "-169.867233", "Niue",
		"NZ", "-40.900557", "174.885971", "New Zealand",
		"OM", "21.512583", "55.923255", "Oman",
		"PA", "8.537981", "-80.782127", "Panama",
		"PE", "-9.189967", "-75.015152", "Peru",
		"PF", "-17.679742", "-149.406843", "French Polynesia",
		"PG", "-6.314993", "143.95555", "Papua New Guinea",
		"PH", "12.879721", "121.774017", "Philippines",
		"PK", "30.375321", "69.345116", "Pakistan",
		"PL", "51.919438", "19.145136", "Poland",
		"PM", "46.941936", "-56.27111", "Saint Pierre and Miquelon",
		"PN", "-24.703615", "-127.439308", "Pitcairn Islands",
		"PR", "18.220833", "-66.590149", "Puerto Rico",
		"PS", "31.952162", "35.233154", "Palestinian Territories",
		"PT", "39.399872", "-8.224454", "Portugal",
		"PW", "7.51498", "134.58252", "Palau",
		"PY", "-23.442503", "-58.443832", "Paraguay",
		"QA", "25.354826", "51.183884", "Qatar",
		"RE", "-21.115141", "55.536384", "RŽunion",
		"RO", "45.943161", "24.96676", "Romania",
		"RS", "44.016521", "21.005859", "Serbia",
		"RU", "61.52401", "105.318756", "Russia",
		"RW", "-1.940278", "29.873888", "Rwanda",
		"SA", "23.885942", "45.079162", "Saudi Arabia",
		"SB", "-9.64571", "160.156194", "Solomon Islands",
		"SC", "-4.679574", "55.491977", "Seychelles",
		"SD", "12.862807", "30.217636", "Sudan",
		"SE", "60.128161", "18.643501", "Sweden",
		"SG", "1.352083", "103.819836", "Singapore",
		"SH", "-24.143474", "-10.030696", "Saint Helena",
		"SI", "46.151241", "14.995463", "Slovenia",
		"SJ", "77.553604", "23.670272", "Svalbard and Jan Mayen",
		"SK", "48.669026", "19.699024", "Slovakia",
		"SL", "8.460555", "-11.779889", "Sierra Leone",
		"SM", "43.94236", "12.457777", "San Marino",
		"SN", "14.497401", "-14.452362", "Senegal",
		"SO", "5.152149", "46.199616", "Somalia",
		"SR", "3.919305", "-56.027783", "Suriname",
		"ST", "0.18636", "6.613081", "Sao Tome and Principe",
		"SV", "13.794185", "-88.89653", "El Salvador",
		"SY", "34.802075", "38.996815", "Syria",
		"SZ", "-26.522503", "31.465866", "Swaziland",
		"TC", "21.694025", "-71.797928", "Turks and Caicos Islands",
		"TD", "15.454166", "18.732207", "Chad",
		"TF", "-49.280366", "69.348557", "French Southern Territories",
		"TG", "8.619543", "0.824782", "Togo",
		"TH", "15.870032", "100.992541", "Thailand",
		"TJ", "38.861034", "71.276093", "Tajikistan",
		"TK", "-8.967363", "-171.855881", "Tokelau",
		"TL", "-8.874217", "125.727539", "Timor-Leste",
		"TM", "38.969719", "59.556278", "Turkmenistan",
		"TN", "33.886917", "9.537499", "Tunisia",
		"TO", "-21.178986", "-175.198242", "Tonga",
		"TR", "38.963745", "35.243322", "Turkey",
		"TT", "10.691803", "-61.222503", "Trinidad and Tobago",
		"TV", "-7.109535", "177.64933", "Tuvalu",
		"TW", "23.69781", "120.960515", "Taiwan",
		"TZ", "-6.369028", "34.888822", "Tanzania",
		"UA", "48.379433", "31.16558", "Ukraine",
		"UG", "1.373333", "32.290275", "Uganda",
		"US", "37.09024", "-95.712891", "United States",
		"UY", "-32.522779", "-55.765835", "Uruguay",
		"UZ", "41.377491", "64.585262", "Uzbekistan",
		"VA", "41.902916", "12.453389", "Vatican City",
		"VC", "12.984305", "-61.287228", "Saint Vincent and the Grenadines",
		"VE", "6.42375", "-66.58973", "Venezuela",
		"VG", "18.420695", "-64.639968", "British Virgin Islands",
		"VI", "18.335765", "-64.896335", "U.S. Virgin Islands",
		"VN", "14.058324", "108.277199", "Vietnam",
		"VU", "-15.376706", "166.959158", "Vanuatu",
		"WF", "-13.768752", "-177.156097", "Wallis and Futuna",
		"WS", "-13.759029", "-172.104629", "Samoa",
		"XK", "42.602636", "20.902977", "Kosovo",
		"YE", "15.552727", "48.516388", "Yemen",
		"YT", "-12.8275", "45.166244", "Mayotte",
		"ZA", "-30.559482", "22.937506", "South Africa",
		"ZM", "-13.133897", "27.849332", "Zambia",
		"ZW", "-19.015438", "29.154857", "Zimbabwe"
	};
    // Special case for the US until we find a better solution
    if (longitude > -125.0 && longitude < -65.0 && latitude > 25.0 && latitude < 50.0) {
        strcpy(country, "United States");
        return country;
    }
	int index = 0;
	double minimum = 999.99;
	for (int k = 0; k < sizeof(countries) / sizeof(countries[0]); k+=4) {
		double testLatitude = atof(countries[k + 1]);
		double testLongitude = atof(countries[k + 2]);
		double metric = (testLatitude - latitude) * (testLatitude - latitude) + (testLongitude - longitude) * (testLongitude - longitude);
		if (minimum > metric) {
			minimum = metric;
			index = k;
		}
	}
	strcpy(country, countries[index + 3]);
	#if defined(DEBUG_HEAVY)
	printf("Best index = %d --> %s (%.3f)\n", index, countries[index + 3], sqrt(minimum));
	#endif
	return country;
}

char *RKGetNextKeyValue(char *json, char *key, char *value) {
    char *c = json;
    char *e = c + strlen(c);
    char *ks, *ke, *vs, *ve;
    // Forward until the next non-space character
    while (*c == ' ' && c < e) {
        c++;
    }
    if (c == e) {
        *key = '\0';
        return NULL;
    }
    // Find the key
    ks = c;
    switch (*ks) {
        case '"':
            ks++;
            ke = strchr(ks, '"');
            break;
        case '\'':
            ks++;
            ke = strchr(ks, '\'');
            break;
        default:
            ke = strchr(ks, ':');
            ke--;
            break;
    }
    if (key) {
        memcpy(key, ks, ke - ks);
        key[ke - ks] = '\0';
    }
    // Find the delimiter ':'
    c = strchr(ke, ':');
    if (c == NULL) {
        fprintf(stderr, "Incomplete string?\n");
        return NULL;
    }
    c++;
    // Forward until the next non-space character
    while (*c == ' ' && c < e) {
        c++;
    }
    if (c == e) {
        *value = '\0';
        return NULL;
    }
    // If this is bracketed piece, find the start and end
    vs = c++;
    switch (*vs) {
        case '"':
            ve = strchr(c, '"');
            break;
        case '[':
            ve = strchr(c, ']');
            break;
        case '{':
            ve = strchr(c, '}');
            break;
        case '\'':
            ve = strchr(c, '\'');
            break;
        default:
            ve = strchr(c, ',');
            if (ve == NULL) {
                ve = e;
            }
            ve--;
            break;
    }
    if (value) {
        memcpy(value, vs, ve - vs + 1);
        value[ve - vs + 1] = '\0';
    }
    if (ve == e - 1) {
        c = NULL;
    } else {
        c = ve + 1;
        while (c < e && (*c == ',' || *c == ' ')) {
            c++;
        }
    }
    return c;
}

int RKMergeColumns(char *text, const char *left, const char *right, const int indent) {
    const int u = 26; const int v = 38;
    char *ls = (char *)left, *le = NULL;
    char *rs = (char *)right, *re = NULL;
    int w = 0, m = 0, n;
    char format[indent + 4];
    char *plain;
    memset(format, ' ', indent);
    sprintf(format + indent, "%%s");
    //printf("%s--\n", right);
    while (ls != NULL || rs != NULL) {
        if (ls != NULL && (le = strchr(ls, '\n')) != NULL) {
            *le = '\0';
            plain = RKStripEscapeSequence(ls);
            n = (int)strlen(plain);
            //printf("%s (%d) -> %s (%d)\n", ls, (int)(le - ls), plain, n);
            m += sprintf(text + m, format, ls);
            memset(text + m, ' ', u - n);
            m += (u - n);
            ls = le + 1;
            if (*ls == '\0') {
                ls = NULL;
            }
        } else {
            if (rs != NULL) {
                memset(text + m, ' ', u + indent);
                m += u + indent;
            }
        }
        if (rs != NULL && (re = strchr(rs, '\n')) != NULL) {
            *re = '\0';
            plain = RKStripEscapeSequence(rs);
            n = (int)strlen(plain);
            w = MAX(w, n);
            //printf("%s (%d) -> %s (%d) |%c|\n", rs, (int)(re - rs), plain, n, *(re + 1));
            m += sprintf(text + m, "%s", rs);
            memset(text + m, ' ', v - n);
            m += (v - n);
            rs = re + 1;
            if (*rs == '\0') {
                rs = NULL;
            }
        } else {
            memset(text + m, ' ', v);
            m += v;
        }
        m += sprintf(text + m, "\n");
    }
    w += u + indent;
    return w;
}

char *RKBinaryString(char *dst, void *src, const size_t count) {
    uint8_t *c = (uint8_t *)src;
    *dst++ = 'b';
    *dst++ = '\'';
    for (int i = 0; i < count; i++) {
        if (*c >= 32 && *c < 127) {
            if (*c == '\\' || *c == '\'') {
                *dst++ = '\\';
            }
            *dst++ = *c++;
        } else {
            dst += sprintf(dst, "\\x%02x", *c++);
        }
    }
    *dst++ = '\'';
    *dst = '\0';
    return dst;
}

char *RKStringLower(char *string) {
    char *c = string;
    char *e = c + strlen(c);
    do {
        *c = *c > 0x40 && *c < 0x5b ? *c | 0x60 : *c;
    } while (c++ < e);
    return string;
}

char *RKBytesInHex(char *dst, void *src, const size_t count) {
    int i;
    uint8_t *c = (uint8_t *)src;
    *dst++ = '[';
    for (i = 0; i < count; i++) {
        dst += sprintf(dst, ".%02x", *c++);
    }
    *dst++ = ']';
    *dst = '\0';
    return dst;
}

void RKHeadTailBytesInHex(char *dst, void *src, const size_t count) {
    char *dummy = RKBytesInHex(dst, src, 7);
    RKBytesInHex(dummy + sprintf(dummy, " ... "), src + count - 3, 3);
}

void RKHeadTailBinaryString(char *dst, void *src, const size_t count) {
    char *tail = RKBinaryString(dst, src, 25);
    RKBinaryString(tail + sprintf(tail, " ... "), src + count - 5, 5);
}

void RKRadarHubPayloadString(char *dst, void *src, const size_t count) {
    int i;
    uint8_t *c = (uint8_t *)src;
    int r = 0;
    size_t bound = MIN(25, count);
    for (i = 0; i < bound; i++, c++) {
        if (*c >= 32 && *c < 127) {
            r++;
        }
    }
    //printf("r = %d   bound / 2 = %d\n", r, (int)bound / 2);
    if (r > bound * 2 / 3) {
        RKHeadTailBinaryString(dst, src, count);
    } else {
        c = (uint8_t *)src;
        r = sprintf(dst, "b'\\x%02x'", *c++);
        RKHeadTailBytesInHex(dst + r, c, count - 1);
    }
}

void RKSetArrayToFloat32(void *array, const float value, const size_t count) {
    float *f = (float *)array;
    for (int i = 0; i < count; i++) {
        *f++ = value;
    }
}
