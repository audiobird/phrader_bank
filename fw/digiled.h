#ifndef _DIGILED_H
#define _DIGILED_H

#include <inttypes.h>

void led_set_val(uint8_t channel, int32_t val, uint8_t mode);
void ws2812_init();
void leds_update_all();
void leds_startup();

#endif