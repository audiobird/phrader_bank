#include "leds.h"
#include "leds_driver.h"
#include "potentiometer.h"
#include <string.h>

void leds_startup();

//adjusts sample to fit into the 7 bit lut used by the driver
//returns true if incoming sample was positive
bool scale_samp_to_fade_lut(int16_t* samp);

rgb_led_t led[NUM_CHANNELS];

void leds_init()
{
    pixel_driver_init();
    leds_startup();
    pixel_driver_start_update_isr_timer();
}

void leds_task(sample_buffer_t* x, uint8_t channel)
{
    static uint16_t cnt = 0;

    rgb_led_t l;
    l.r = l.b = l.g = 0;

    cnt++;

    bool is_pos;

    if (x->flags.led_indicate)
    {
        l.r = 128 / 3;
        l.g = 128 / 3;
        l.b = 128 / 3;
        pixel_set(&l, channel);
        return;
    }

    switch (x->mode)
    {
        case MODE_DEFAULT:
        {
            is_pos = scale_samp_to_fade_lut(&x->output_val);

            if (is_pos)
            l.g = x->output_val;
            else
            l.r = x->output_val;

            break;
        }
        case MODE_RECORDING_CONF:
        {
            l.r = 127 * ((time_us_32() >> 19) & 0x01);
            break;
        }
        case MODE_PLAYBACK:
        case MODE_SET_ATTENUVERT:
        case MODE_SET_OFFSET:
        {
            is_pos = scale_samp_to_fade_lut(&x->output_val);

            if (is_pos)
            l.g = x->output_val / 2;
            else
            l.r = x->output_val / 2;

            l.b = l.g > l.r ? l.g : l.r;
            l.b += 5;

            if (x->mode != MODE_PLAYBACK)
            {
                if ((cnt & 0x3ff) < 32)
                {
                    l.b = 0;

                    if (x->mode == MODE_SET_ATTENUVERT)
                    {
                        l.g = 0;
                        l.r = 64;
                    }
                    else
                    {
                        l.r = 0;
                        l.g = 64;
                    } 
                }
            }

            break;
        }
        case MODE_SET_SPEED:
        {   
            int8_t shift_amnt = 0;

            if (x->fader_pos == -2 || x->fader_pos == 1)
            break;

            if (x->fader_pos < -2)
            shift_amnt = 1;
            else if (x->fader_pos > 1)
            shift_amnt = -1;

            l.b = 127 * ((time_us_32() >> (19 + shift_amnt)) & 0x01);

            break;
        }
        case MODE_PHRASE_ADJUSTMENT_SELECTION:
        {
            if (x->fader_pos == -2 || x->fader_pos == 1)
            break;

            if(x->fader_pos < -2)
            l.b = 127;
            else if (x->fader_pos > 1)
            l.g = 127;
            else
            l.r = 127; //attenuvert is red mode.

            break;
        }
    }
    pixel_set(&l, channel);
}

void leds_startup()
{
    for (uint8_t x = 0; x < NUM_LEDS; x++)
    {
        for (uint8_t z = 0; z < LINEAR_FADE_LUT_SIZE; z++)
        {
            uint8_t brightness = z / 3;
            led[x].r = brightness;
            led[x].g = brightness;
            led[x].b = brightness;

            pixel_set(&led[x], x);
            pixel_update_all();

            uint32_t time = time_us_32();
            while (time_us_32() < time + 1500)
            ;
        }
    }
    
    int16_t pot_pos[8];
    bool is_pos[8];

    for (int x = 0; x < NUM_LEDS; x++)
    {
        pot_pos[x] = pots_get_value(x);
        is_pos[x] = scale_samp_to_fade_lut(&pot_pos[x]);
    }
    
}

bool scale_samp_to_fade_lut(int16_t* samp)
{
    *samp >>= 8;

    if (*samp < 0)
    {
        *samp *= -1;

        if (*samp == 128)
        *samp = 127;

        return 0;
    }
    return 1;
}