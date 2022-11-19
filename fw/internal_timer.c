#include "internal_timer.h"
#include "pico/stdlib.h"

#define SAMPLE_RATE_US (16 * 1000)
#define LOOP_TIME (SAMPLE_RATE_US / NUM_CHANNELS)

uint32_t prev_time = 0;
uint32_t current_time = 0;


uint32_t internal_timer_get_count()
{
    return current_time;
}


bool main_timer()
{
  current_time = time_us_32();

  if(current_time - prev_time < LOOP_TIME)
  return 0;

  prev_time = current_time;

  return 1;
}