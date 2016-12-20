#include <time.h>
#include "monotime.h"

double monotime (void)
{
    struct timespec ts; 
    double t;
    clock_gettime (CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1E-9;
}

double monotime_since (double t)
{
    return monotime () - t;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
