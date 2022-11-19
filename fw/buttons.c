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

    if (x->is_pressed != prev_state[channel])
    {
        prev_state[channel] = x->is_pressed;

        //if the button was pressed we should record the time and then figure out if the tap counter timed out.
        if (x->is_pressed)
        {
            x->time_last_pressed = time;

            buttons_handle_press(channel);

            if (time - x->time_last_released >= TAP_MAX_BETWEEN_DURATION_US)
            {
                x->tap_count = 0;
            }
        }
        else
        {
            //the button was released.
            x->time_last_released = time;
            x->is_held = 0;

            buttons_handle_release(channel);

            if (time - x->time_last_pressed >= TAP_MAX_TIME_DOWN_US)
            {
                //this tap was too long.... that's okay. 
                if (x->tap_count)
                {
                    //if we already had a tap counted we should just throw them all away
                    x->tap_count = 0;
                }
            }
            else
            {
                //this was a valid tap. 
                x->tap_count += 1;

                if (x->tap_count == 2)
                {
                    //if we reached two taps we can do something!
                    buttons_handle_double_tap(channel);
                    x->tap_count = 0;
                }
            }
        }
    }
    else
    {
        //okay so our button state hasn't changed... that means we are either holding or not 
        if (x->is_pressed)
        {
            if (time - x->time_last_pressed >= HOLD_TIME_US && !x->is_held)
            {
                x->is_held = 1;
                buttons_handle_hold(channel);
            }
        }
        else
        {
            //now we can check for a tap timeout...
            if (time - x->time_last_released >= TAP_MAX_BETWEEN_DURATION_US && x->tap_count)
            {
                //time out.
                buttons_handle_tap(channel);
                x->tap_count = 0;
            }
        }
    }    
}
