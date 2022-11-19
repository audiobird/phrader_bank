#ifndef POT_H
#define POT_H

#include "phrader.h"

#define MUX_PIN_0 0
#define MUX_PIN_1 1
#define MUX_PIN_2 2

#define ANALOG_INPUT_PIN 28


//init potentiometer and related parts/pins
void pots_init();

//returns the previously obtained pot value
int16_t pots_get_value(uint8_t channel);

//reads each pot 256 times and averages them out
//averaged values can be obtained using pot_get_value()

#endif