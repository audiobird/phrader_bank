#ifndef INTERNAL_TIMER_H
#define INTERNAL_TIMER_H

#include <inttypes.h>
#include <stdbool.h>

//returns true if more than a millisecond has passed since last time true was returned.
bool main_timer();

//returns us count since startup
uint32_t internal_timer_get_count();

#endif