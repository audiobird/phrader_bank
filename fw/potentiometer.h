#ifndef POT_H
#define POT_H

#include <inttypes.h>

//init potentiometer and related parts/pins
void pot_init();

//returns the previously obtained pot value
uint16_t pot_get_value(uint8_t chan);

//reads each pot 256 times and averages them out
//averaged values can be obtained using pot_get_value()
void pot_read_and_average_all();

#endif