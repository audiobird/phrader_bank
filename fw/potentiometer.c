#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "potentiometer.h"

#define NUM_POTS NUM_CHANNELS

uint16_t pot_val[NUM_POTS];

const uint8_t pot_lut[8] = {5, 7, 3, 1, 2, 4, 0, 6};

uint16_t pot_get_value(uint8_t chan)
{
  return pot_val[chan];
}

bool pot_is_ready(void)
{
  return (adc_hw->cs & ADC_CS_READY_BITS);
}

//could probably use dma here
void pot_read_and_average_all()
{
  for (int chan = 0; chan < NUM_POTS; chan++)
  {
    gpio_put_masked((NUM_POTS - 1), pot_lut[chan]);
    uint32_t accum = 0;
    for (int x = 0; x < 512; x++)
    {
      hw_set_bits(&adc_hw->cs, ADC_CS_START_ONCE_BITS);
      while (!pot_is_ready())
      ;
      accum += (adc_hw->result & 0xfff);
    }
    pot_val[chan] = ~(accum >> 5); //16 bit
  }
}

void pot_init()
{
  adc_init();
  adc_gpio_init(28);
  adc_select_input(2);

  // init sel pins
  gpio_init_mask(0x7);
  gpio_set_dir_out_masked(0x07);

  //sets up mux for first pot..
  gpio_put_masked((NUM_POTS - 1), pot_lut[0]);
} 
