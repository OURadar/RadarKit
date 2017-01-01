#include <RadarKit/RKMisc.h>

// Strip out \r, \n, white space, \10 (BS), etc.
void stripTrailingUnwanted(char *str) {
    char *c = str + strlen(str) - 1;
    while (c >= str && (*c == '\r' || *c == '\n' || *c == ' ' || *c == 10)) {
        *c-- = '\0';
    }
}

#pragma mark -

char *RKNow() {
    static char timestr[32];
    time_t utc;
    /* Here is a nice reference: http://www.cplusplus.com/ref/ctime/time.html */
    time(&utc);
    strftime(timestr, 32, "%Y/%m/%d %T", localtime(&utc));
    return timestr;
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
char *RKFloatToCommaStyleString(const float num) {
    char *intString = RKIntegerToCommaStyleString((long)num);
    snprintf(intString + strlen(intString), 32 - strlen(intString), ".%03.0f", 1000.0f * (num - floorf(num))); 
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
