#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "buttons.h"
#include "internal_timer.h"

button_state_t buttons[BUTTONS_NUM_OF];

void buttons_init()
{
    gpio_init_mask(0x7f << 16);
    gpio_init(26);
    gpio_set_dir_in_masked(0x7f << 16);
    gpio_set_dir(26, false);

    for (int x = 0; x < BUTTONS_NUM_OF; x++)
    if (x < 7)
    gpio_pull_up(x + 16);
    else
    gpio_pull_up(26);
}

uint8_t buttons_get()
{
    uint16_t state = gpio_get_all() >> 16;

    uint8_t out = state & 0x7f;

    state >>= 3;

    out |= state & 0x80;

    return ~out;
}

void button_check(uint8_t channel)
{
    static bool prev_state[BUTTONS_NUM_OF];

    uint32_t time = internal_timer_get_count();

    const uint8_t state = buttons_get();
    const uint8_t mask = (uint8_t)(0x80) >> channel;

    button_state_t* x = &buttons[channel];

    x->is_pressed = state & mask;

    if (x->is_pressed == prev_state[channel]){
        if (x->is_pressed)
        {
            if (time - x->time >= HOLD_TIME_US)
            {
                x->is_held = 1;
                buttons_handle_hold(channel);
            }
        }
        return;
    }
    
    if (x->is_pressed)
    buttons_handle_press(channel);
    else
    {
        buttons_handle_release(channel, x->is_held);
        x->is_held = 0;
    }

    x->time = time;

    prev_state[channel] = x->is_pressed;
}
