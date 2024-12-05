// Eduardo Stefanel Paludo - GRR20210581
// Natael Pontarolo Gomes - GRR20211786

#ifndef CHRONO_H
#define CHRONO_H

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <sys/time.h>     /* struct timeval definition           */
#include <unistd.h>       /* declaration of gettimeofday()       */

#include <time.h>

typedef struct {
    struct timespec xadd_time1, xadd_time2;
    long long xtotal_ns;
    long xn_events;

} chronometer_t;

void chrono_reset(chronometer_t* chrono);

void chrono_start(chronometer_t* chrono);

long long  chrono_gettotal(chronometer_t* chrono);

long long  chrono_gettotal(chronometer_t* chrono);

long long  chrono_getcount(chronometer_t* chrono);

void chrono_stop(chronometer_t* chrono);

void chrono_reportTime(chronometer_t* chrono, char* s);

void chrono_report_TimeInLoop(chronometer_t* chrono, char* s, int loop_count);

#endif