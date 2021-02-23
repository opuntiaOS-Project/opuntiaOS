#include <drivers/generic/rtc.h>
#include <drivers/generic/timer.h>
#include <log.h>
#include <time/time_manager.h>

// #define TIME_MANAGER_DEBUG

static time_t ticks;
static time_t time_since_boot = 0;
static time_t time_since_epoch = 0;

static uint32_t pref_sum_of_days_in_mounts[] = {
    0,
    31,
    31 + 28, // the code that use that should add +1 autmoatically for a leap year
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31 + 30 + 31,
};

bool timeman_is_leap_year(uint32_t year)
{
    return (year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0);
}

uint32_t timeman_days_in_years_since_epoch(uint32_t year)
{
    uint32_t days = 0;
    for (uint32_t y = 1970; y <= year; y++) {
        days += 365;
        if (timeman_is_leap_year(y)) {
            days++;
        }
    }
    return days;
}

/**
 * soy - start of the year
 */
uint32_t timeman_days_in_months_since_soy(uint8_t month, uint32_t year)
{
    uint32_t days = pref_sum_of_days_in_mounts[month];
    if (timeman_is_leap_year(year) && month >= 2) {
        days++;
    }
    return days;
}

time_t timeman_to_seconds_since_epoch(uint8_t secs, uint8_t mins, uint8_t hrs, uint8_t day, uint8_t month, uint32_t year)
{
    time_t res = timeman_days_in_years_since_epoch(year - 1) * 86400 + timeman_days_in_months_since_soy(month - 1, year) * 86400 + (day - 1) * 86400 + hrs * 3600 + mins * 60 + secs;
    return res;
}

int timeman_setup()
{
    uint8_t secs = 0, mins = 0, hrs = 0, day = 0, month = 0;
    uint32_t year = 1970;

    // FIXME: Rewrite as a proper driver
#ifdef __i386__
    rtc_load_time(&secs, &mins, &hrs, &day, &month, &year);
    time_since_epoch = timeman_to_seconds_since_epoch(secs, mins, hrs, day, month, year);
#elif __arm__
    time_since_epoch = pl031_read_rtc();
#endif

#ifdef TIME_MANAGER_DEBUG
    log("Loaded date: %d", time_since_epoch);
#endif
}

void timeman_timer_tick()
{
    if (++ticks >= TIMER_TICKS_PER_SECOND) {
        time_since_boot++;
        time_since_epoch++;
        ticks = 0;
    }
}

time_t timeman_now()
{
    return time_since_epoch;
}

time_t timeman_seconds_since_boot()
{
    return time_since_boot;
}

time_t timeman_get_ticks_from_last_second()
{
    return ticks;
}