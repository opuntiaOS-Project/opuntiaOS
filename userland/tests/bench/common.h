#pragma once

#include <ctime>
#include <sys/time.h>

#define RUN_BENCH(name, x) for (bench_run = 0, bench_pno = bench_no, gettimeofday(&tv, &tz); bench_run < x; gettimeofday(&ttv, &tz), printf("[BENCH][%s] %d (usec)\n", name, to_usec()), fflush(stdout), bench_run++, gettimeofday(&tv, &tz))

extern int bench_pno;
extern int bench_no;
extern int bench_run;
extern timeval_t tv, ttv;
extern timezone_t tz;

static inline int to_usec()
{
    int sec = ttv.tv_sec - tv.tv_sec;
    int diff = ttv.tv_usec - tv.tv_usec;
    return sec * 1000000 + diff;
}

void bench_pngloader();