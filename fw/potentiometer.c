#include "potentiometer.h"

#include "hardware/irq.h"
#include "hardware/dma.h"
#include "hardware/adc.h"

#define NUM_POTS NUM_CHANNELS

const uint8_t mux_pins[3] = {MUX_PIN_0, MUX_PIN_1, MUX_PIN_2};
const uint8_t pot_table[NUM_POTS] = {5, 7, 3, 1, 2, 4, 0, 6};

#define DMA_BUFFER_SIZE 256

uint16_t dma_buffer_dummy;

int16_t pot_values[NUM_POTS];

void pots_setup_mux(uint8_t channel)
{
  uint8_t mux = pot_table[channel];

  for (int x = 0; x < 3; x++)
  gpio_put(mux_pins[x], mux & (1 << x));
}

void dma_handler() 
{
    static uint8_t cnt = 0;
    dma_hw->ints0 = 1 << 0;
    adc_run(false);
    adc_fifo_drain();

    //read the accumulator
    //20 bits
    int32_t samp = dma_hw->sniff_data;
    //16 bits
    samp /= 16;
    pot_values[cnt] = (~samp) - 32768;
    dma_hw->sniff_data = 0;

    //increment to next channel
    cnt++;
    cnt &= 7;
    pots_setup_mux(cnt);

    dma_channel_start(0);
    adc_run(true);
}

void pots_init()
{
    for (int x = 0; x < 3; x++)
    {
        gpio_init(mux_pins[x]);
        gpio_set_dir(mux_pins[x], true);
        gpio_put(mux_pins[x], false);
    }

    adc_init();
    adc_gpio_init(ANALOG_INPUT_PIN);
    adc_select_input(ANALOG_INPUT_PIN - 26);
    adc_fifo_setup(
        true,    // Write each completed conversion to the sample FIFO
        true,    // Enable DMA data request (DREQ)
        1,       // DREQ (and IRQ) asserted when at least 1 sample present
        false,   
        false     
    );
    adc_set_clkdiv(0);
    sleep_ms(1000);

    dma_channel_config cfg = dma_channel_get_default_config(0);

    // Reading from constant address, writing to constant
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, false);
    
    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);

    dma_channel_configure(0, &cfg,
        &dma_buffer_dummy,   // dst
        &adc_hw->fifo,          // src
        DMA_BUFFER_SIZE,        // transfer count
        false                    // start immediately
    );

    dma_sniffer_enable(0, DMA_SNIFF_CTRL_CALC_VALUE_SUM, 1);

    dma_channel_set_irq0_enabled(0, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_handler();
}

int16_t pots_get_value(uint8_t channel)
{
  return pot_values[channel];
}
