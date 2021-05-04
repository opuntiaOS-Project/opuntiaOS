#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sysdep.h>
#include <time.h>

static const char __wday_snames[7][4] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char __wday_lnames[7][11] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};
static const char __mon_snames[12][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char __mon_lnames[12][11] = {
    "January", "February", "March", "April", "May", "June", "July", "Auguest", "September", "October", "November", "December"
};

static inline size_t strftime_append(char* s, size_t remaining, const char* appendee)
{
    size_t copy_size = min(remaining, strlen(appendee));
    memcpy(s, appendee, copy_size);
    return copy_size;
}

size_t strftime(char* s, size_t remaining, const char* format, const tm_t* tm)
{
    char buf[32];
    const char* p = format;
    size_t written = 0;
    remaining--;
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++;
            size_t wr = 0;
            switch (*p) {
            case 'a':
                wr = strftime_append((char*)(s + written), remaining, __wday_snames[tm->tm_wday]);
                written += wr;
                remaining -= wr;
                break;
            case 'A':
                wr = strftime_append((char*)(s + written), remaining, __wday_lnames[tm->tm_wday]);
                written += wr;
                remaining -= wr;
                break;
            case 'R':
                if (tm->tm_min < 10) {
                    snprintf(buf, 32, "%d:0%d", tm->tm_hour, tm->tm_min);
                } else {
                    snprintf(buf, 32, "%d:%d", tm->tm_hour, tm->tm_min);
                }
                wr = strftime_append((char*)(s + written), remaining, buf);
                written += wr;
                remaining -= wr;
                break;
            }
            p++;
        } else {
            if (remaining) {
                s[written] = *p;
                written++;
                remaining--;
            }
            p++;
        }
    }

    s[written] = '\0';
    return written;
}