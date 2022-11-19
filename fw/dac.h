#ifndef DAC_H
#define DAC_H

#include "phrader.h"

#define PWM_MAX_OUTPUT_TARGET (2.5)
#define PWM_MAX_OUTPUT_VOLTAGE (3.3)
#define PWM_VOLTAGE_TARGET_RATIO (PWM_MAX_OUTPUT_VOLTAGE / PWM_MAX_OUTPUT_TARGET)

#define OUTPUT_RESOLUTION_TARGET (9)
#define MAX_OUTPUT_VOLTAGE_VALUE (1 << OUTPUT_RESOLUTION_TARGET)

#define DAC_SHIFT (16 - OUTPUT_RESOLUTION_TARGET)

#define PWM_PER ((uint16_t)(MAX_OUTPUT_VOLTAGE_VALUE * PWM_VOLTAGE_TARGET_RATIO))


void dac_init();

//sends out a sample.
void dac_output_sample(uint8_t channel, int32_t data);

#endif