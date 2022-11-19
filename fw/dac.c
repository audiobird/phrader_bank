#include "dac.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

#define NUM_DACS NUM_CHANNELS

void dac_output_sample(uint8_t channel, int32_t data)
{
    uint8_t is_b = 0;

    if (channel & 0x01)
    is_b = 1;

    channel >>= 1;

    pwm_set_chan_level(channel + 4, is_b, (data + 32768) >> DAC_SHIFT);
}

void dac_init()
{
    for (int x = 0; x < NUM_DACS; x++)
    {
        gpio_set_function(x + 8, GPIO_FUNC_PWM);
        if (x > 3)
        {
            pwm_set_wrap(x, PWM_PER);
            pwm_set_enabled(x, true);
        }
    }
}
