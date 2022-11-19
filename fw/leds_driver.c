#include "leds_driver.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

#define NUM_PIXELS NUM_CHANNELS

#if LED_PIN_OLD
#define DATA_PIN 3
#warning "Compiling for depricated LED circuit"
#else
#define DATA_PIN 6
#endif

struct repeating_timer ws2812_update_timer;

uint32_t pixel[NUM_PIXELS] = {0};

const uint8_t linear_fade_lut[128] = 
{
    0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4,
    4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8,
    8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13, 14,
    14, 15, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 22,
    22, 23, 23, 24, 25, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32,
    33, 33, 34, 35, 36, 36, 37, 38, 39, 40, 40, 41, 42, 43, 44, 45
};


inline void pixel_set(rgb_led_t* x, uint8_t channel)
{
    pixel[channel]  = (uint32_t)(linear_fade_lut[x->r]) << 24;
    pixel[channel] |= (uint32_t)(linear_fade_lut[x->g]) << 16;
    pixel[channel] |= (uint32_t)(linear_fade_lut[x->b]) << 8;
}


bool ws2812_update_timer_callback(struct repeating_timer *t) {

    uint32_t* ws2812_bits = t->user_data;

    //well we know this fifo should be totally empty so let's just dump 
    for (int x = 0; x < NUM_PIXELS; x++)
    pio0->txf[0] = ws2812_bits[x];

    return true;
}

void pixel_driver_init()
{
    #if LED_PIN_OLD
    const pio_program_t* prog = &ws2812_old_program;
    #else 
    const pio_program_t* prog = &ws2812_program;
    #endif

    uint offset = pio_add_program(pio0, prog);
    ws2812_program_init(pio0, 0, offset, DATA_PIN, 800000);
    sleep_ms(10);
}

void pixel_driver_start_update_isr_timer(void)
{
    add_repeating_timer_ms(10, ws2812_update_timer_callback, pixel, &ws2812_update_timer);
}

void pixel_update_all()
{
    for (int x = 0; x < NUM_PIXELS; x++)
    pio_sm_put_blocking(pio0, 0, pixel[x]);
}
