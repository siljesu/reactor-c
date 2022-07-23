#include <time.h>
#include <errno.h>

#include "core/reactor.h"

interval_t _lf_time_epoch_offset;
instant_t convert_timespec_to_ns(struct timespec tp);
struct timespec convert_ns_to_timespec(instant_t t);
void calculate_epoch_offset(void);
void lf_initialize_clock();
int lf_clock_gettime(instant_t* t);
