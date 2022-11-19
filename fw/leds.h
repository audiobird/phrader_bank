#ifndef __LEDS_H__
#define __LEDS_H__

#include "phrader.h"

#define NUM_LEDS NUM_CHANNELS

void leds_init();

void leds_task(sample_buffer_t* x, uint8_t channel);

void led_set_val(uint8_t channel, int32_t val, uint8_t mode);

#endif