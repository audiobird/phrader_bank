#ifndef __LED_DRIVER_H__
#define __LED_DRIVER_H__

#include "phrader.h"

//this will be the driver.

enum led_mode{
    LED_MODE_DEF,
    LED_MODE_REC,
    LED_MODE_PLA,
    LED_MODE_INDICATE,
    LED_MODE_ATTEN,
    
};

#define LINEAR_FADE_LUT_SIZE 128

typedef struct rgb_led
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_led_t;

void pixel_driver_init();
void pixel_driver_start_update_isr_timer();

//used to manually update the leds withot relying on the interrupt handler
void pixel_update_all();

void pixel_set(rgb_led_t* x, uint8_t channel);


#endif