#include <RadarKit/RKMisc.h>

char *RKNow() {
    static char timestr[32];
    time_t utc;
    /* Here is a nice reference: http://www.cplusplus.com/ref/ctime/time.html */
    time(&utc);
    strftime(timestr, 32, "%Y/%m/%d %T", localtime(&utc));
    return timestr;
}

int RKLog(const char *whatever, ...) {
    va_list args;
    int ret = 0;
    FILE *logFileID = NULL;

    if (rkGlobalParameters.logfile[0] != 0)
        logFileID = fopen(rkGlobalParameters.logfile, "a");
    if (logFileID != NULL) {
        if (whatever[0] != '.') {
            if (whatever[0] == '-' || whatever[0] == '*') {
                fprintf(logFileID, "%s\n", RKNow());
            } else {
                fprintf(logFileID, "%s : [%s] ", RKNow(), rkGlobalParameters.program);
            }
        }
        va_start(args, whatever);
        ret = vfprintf(logFileID, whatever, args);
        if (whatever[strlen(whatever) - 1] != '\n')
            fprintf(logFileID, "\n");
        fclose(logFileID);
    }
    if (rkGlobalParameters.stream != NULL) {
        va_start(args, whatever);
        char msg[2048];
        snprintf(msg, 2048, "%s : [%s] ", RKNow(), rkGlobalParameters.program);
        vsprintf(msg + strlen(msg), whatever, args);
        int has_ok = (strstr(msg, " OK") != NULL);
        int has_not_ok = (strstr(msg, "ERROR") != NULL);
        int has_warning = (strstr(msg, "WARNING") != NULL);
        if (rkGlobalParameters.showColor) {
            if (has_ok) {
                strncat(msg, "\033[1;32m", 2048);
            } else if (has_not_ok) {
                strncat(msg, "\033[1;31m", 2048);
            } else if (has_warning) {
                strncat(msg, "\033[1;33m", 2048);
            }
        }
        if (rkGlobalParameters.showColor && (has_ok || has_not_ok || has_warning)) {
            strncat(msg, "\033[0m", 2048);
        }
        if (whatever[strlen(whatever) - 1] != '\n') {
            strncat(msg, "\n", 2048);
        }
        va_end(args);
        fprintf(rkGlobalParameters.stream, "%s", msg);
        fflush(rkGlobalParameters.stream);
    }
    return ret;
}


/*************************************************
 *
 *  Integer to string with 3-digit grouping
 */
char *RKIntegerToCommaStyleString(long num) {
    int idx, jdx, kdx;
    static int ibuf = 0;
    static char stringBuffer[8][32];

    sprintf(stringBuffer[ibuf], "%ld", num);
    if (num < 1000) {
        kdx = ibuf;
        ibuf = ibuf == 7 ? 0 : ibuf + 1;
        return stringBuffer[kdx];
    } else {
        kdx = (int)(strlen(stringBuffer[ibuf]) - 1) / 3;
        idx = (int)(strlen(stringBuffer[ibuf]) + kdx);
        jdx = 1;
        stringBuffer[ibuf][idx] = '\0';
        while (idx > 0) {
            idx--;
            stringBuffer[ibuf][idx] = stringBuffer[ibuf][idx - kdx];
            if (jdx > 3) {
                jdx = 0;
                stringBuffer[ibuf][idx] = ',';
                kdx--;
            }
            jdx++;
        }
    }
    kdx = ibuf;
    ibuf = ibuf == 7 ? 0 : ibuf + 1;
    return stringBuffer[kdx];
}

void RKSetWantColor(const bool showColor) {
    rkGlobalParameters.showColor = showColor;
}

int RKSetProgramName(const char *name) {
    if (strlen(name) >= RKMaximumStringLength) {
        return 1;
    }
    snprintf(rkGlobalParameters.program, RKMaximumStringLength, "%s", name);
    return 0;
}

int RKSetLogfile(const char *filename) {
    if (strlen(filename) >= RKMaximumStringLength) {
        return 1;
    }
    snprintf(rkGlobalParameters.logfile, RKMaximumStringLength, "%s", filename);
    return 0;
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
