#ifndef BUTTONS_H
#define BUTTONS_H

#include <inttypes.h>

#define BUTTONS_NUM_OF NUM_CHANNELS
#define HOLD_TIME_US (1000 * 500) //half second

typedef struct {
    bool is_pressed;
    bool is_held;
    uint32_t time;
}button_state_t;

void buttons_init();
void button_check(uint8_t channel);

void buttons_handle_hold(uint8_t);
void buttons_handle_press(uint8_t);
void buttons_handle_release(uint8_t, bool);

#endif