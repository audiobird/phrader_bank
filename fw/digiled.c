#include "digiled.h"
#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "digiled.pio.h"
#include "potentiometer.h"

#define NUM_PIXELS NUM_CHANNELS
#define DATA_PIN 3

uint32_t leds[NUM_PIXELS] = {0};

const uint8_t led_tab[128] = 
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

static inline void put_pixel(uint32_t pixel_rgb)
{
    pio_sm_put_blocking(pio1, 0, ~pixel_rgb);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 24) |
           ((uint32_t)(g) << 16) |
           (uint32_t)(b) << 8;
}

static void urgb_u32_to_array(uint8_t* out, uint32_t color)
{
    out[0] = color >> 24;
    out[1] = color >> 16;
    out[2] = color >> 8;
}

void ws2812_init()
{
    uint offset = pio_add_program(pio1, &digiled_program);

    pio_gpio_init(pio1, DATA_PIN);
    pio_sm_set_consecutive_pindirs(pio1, 0, DATA_PIN, 1, true);

    pio_sm_config c = digiled_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, DATA_PIN);
    sm_config_set_out_shift(&c, false, true, 24);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    float div = clock_get_hz(clk_sys) / (8000000);
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(pio1, 0, offset, &c);
    pio_sm_set_enabled(pio1, 0, true);
}


void led_set_val(uint8_t channel, int32_t val, uint8_t mode)
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t is_neg = 0;

    val >>= 8;
    val -= 128;

    if(val == -128)
    val = -127;

    if (val < 0)
    is_neg = 1;

    uint8_t brightness = led_tab[val];

    if (!mode)
    {
        if (!is_neg)
        g = brightness;
        else
        r = led_tab[(val * -1) & 0x7f];
    }
    else if (mode == 1)
    {
        if ((time_us_32() >> 19) & 0x01)
        r = 22;
        else
        r = 0;
    }
    else 
    {
        if (!is_neg)
        g = brightness / 2;
        else
        r = led_tab[(val * -1) & 0x7f] / 2;

        b = g > r ? g : r;
        b+=1;

    }
    
    leds[channel] = urgb_u32(r, g, b);
}

void leds_startup()
{
    uint8_t rgb[NUM_PIXELS][3] = {0};

    for (uint8_t x = 0; x < NUM_PIXELS; x++)
    {
        for (uint8_t y = 0; y < 3; y++)
        {
            for (uint8_t z = 0; z < 128; z++)
            {
                rgb[x][y] = led_tab[z] / 3;
                leds[x] = urgb_u32(rgb[x][0], rgb[x][1], rgb[x][2]);
                leds_update_all();

                uint32_t time = time_us_32();
                while (time_us_32() < time + 500)
                ;
            }
        }
    }
    
    pot_read_and_average_all();

    uint8_t fade_from[NUM_PIXELS][3] = {0};

    for (int x = 0; x < NUM_PIXELS; x++)
    {
        urgb_u32_to_array(fade_from[x], leds[x]);
        led_set_val(x, pot_get_value(x), 0);
        urgb_u32_to_array(rgb[x], leds[x]);
        leds[x] = urgb_u32(fade_from[x][0], fade_from[x][1], fade_from[x][2]);
    }
    

    for (uint8_t x = 0; x < NUM_PIXELS; x++)
    {
        for (uint8_t y = 0; y < 3; y++)
        {
            while (fade_from[x][y] != rgb[x][y])
            {
                if (rgb[x][y] > fade_from[x][y])
                fade_from[x][y]++;
                else if (rgb[x][y] < fade_from[x][y])
                fade_from[x][y]--;
                else
                break;

                leds[x] = urgb_u32(fade_from[x][0], fade_from[x][1], fade_from[x][2]);
                leds_update_all();
                uint32_t time = time_us_32();
                while (time_us_32() < time + 3500)
                ;
            }
        } 
    }
}

void leds_update_all()
{
    for (int x = 0; x < NUM_PIXELS; x++)
    pio_sm_put_blocking(pio1, 0, leds[x]);
}