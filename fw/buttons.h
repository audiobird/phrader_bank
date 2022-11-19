#ifndef BUTTONS_H
#define BUTTONS_H

#include "phrader.h"

#define BUTTONS_NUM_OF NUM_CHANNELS
#define HOLD_TIME_US (400 * 1000) // 2/5 of a second

//maximum amount of time between taps while counting.
#define TAP_MAX_BETWEEN_DURATION_US (150 * 1000)

//maximum button press length to be considered a tap.
#define TAP_MAX_TIME_DOWN_US        HOLD_TIME_US

typedef struct { 
    bool is_pressed;
    bool is_held;
    uint32_t time_last_pressed;
    uint32_t time_last_released;
    uint32_t time;
    uint8_t tap_count;
}button_state_t;

void buttons_init();
void button_check(uint8_t channel);

void buttons_handle_hold(uint8_t);
void buttons_handle_press(uint8_t);
void buttons_handle_release(uint8_t);
void buttons_handle_double_tap(uint8_t);
void buttons_handle_tap(uint8_t);

#endif