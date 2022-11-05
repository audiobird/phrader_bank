#include <stdio.h> 
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "dac.h"
#include "potentiometer.h"
#include "ws2812.h"
#include "hardware/gpio.h"
#include "buttons.h"
#include "pico/multicore.h"
#include "internal_timer.h"

#define CHANNEL_MASK (NUM_CHANNELS - 1)

#define BUFFER_LENGTH 8192

typedef struct
{
  uint8_t playback    : 1;
  uint8_t recording   : 1;
  uint8_t record_conf : 1;
  uint8_t attenuvert  : 1;
  uint8_t unipolar    : 1;
  uint8_t neg         : 1;
  uint8_t reserved    : 2;
} flags_t;

typedef struct
{
  uint16_t length;
  uint16_t position;
  uint16_t buffer[BUFFER_LENGTH];
  uint8_t led_mode;
  flags_t flags;
} sample_buffer_t;

sample_buffer_t buffer[NUM_CHANNELS];

void buttons_handle_hold(uint8_t channel)
{
  sample_buffer_t *x = &buffer[channel];

  if (x->flags.playback)
  {
    // stop playback
    x->flags.playback = 0;
    x->led_mode = 0;
  }
  else if (x->flags.recording)
  {
    // if held while recording we can confirm the recording valid!
    x->flags.record_conf = 1;
  }
}

void buttons_handle_press(uint8_t channel)
{
  sample_buffer_t *x = &buffer[channel];

  // on press.
  // if not recording or playing back.
  if (!x->flags.playback && !x->flags.recording)
  {
    // start recording.
    x->length = 0;
    x->position = 0;
    x->flags.recording = 1;
    x->flags.record_conf = 0;
    x->led_mode = 1;
  }
  else if (x->flags.playback)
  {
    // turn on attenuverter.... on release
    // probably do a light thing.
  }
  else
  {
    // should not possible.
  }
}

void buttons_handle_release(uint8_t channel, bool is_held)
{
  sample_buffer_t *x = &buffer[channel];

  // on release.
  if (x->flags.recording)
  {
    // if recording we need to stop!
    // does not matter if button was held or not.
    x->flags.recording = 0;
    x->led_mode = 0;

    if (x->flags.record_conf)
    {
      x->flags.playback = 1;
      x->led_mode = 2;
    }
    else
    {
      //on a not valid recording press we can change the channel's mode
      if(!x->flags.unipolar)
      {
        x->flags.unipolar = 1;
        x->flags.neg = 0;
      }
      else 
      {
        if (x->flags.neg == 0)
        x->flags.neg = 1;
        else
        x->flags.unipolar = 0;
      }
    }
    
    x->flags.attenuvert = 0;
  }
  else if (x->flags.playback)
  {
    // release during phrase playback...
    if (is_held)
    {
      // holding clears phrase
    }
    else
    {
      // releasing turns on attenuverter...
      x->flags.attenuvert = 1;
    }
  }
  else
  {
    // should not possible.
  }
}

void attenuvert(uint16_t *samp, uint16_t mod)
{
  int32_t val = (*samp) - 32768;
  int32_t m = mod - 32768;
  val *= m;
  val /= 32768;
  *samp = val + 32768;
}

uint16_t handle_sample(sample_buffer_t *x, uint16_t samp)
{

  // we can just dump the sample back if we arent recording or playing back.
  if (!x->flags.playback && !x->flags.recording)
    return samp;

  if (x->flags.playback)
  {
    // we need to play back a recording
    // the slider will become an attenuverter.
    uint16_t out = x->buffer[x->position++];

    // reset the counter of necessary.
    if (x->position >= x->length)
      x->position = 0;

    // attenuate based on incoming sample.
    // center = nothing; top = as recorded; bottom = inverted.
    if (x->flags.attenuvert)
      attenuvert(&out, samp);

    // output
    return out;
  }
  else
  {
    // we need to record the sample.
    x->buffer[x->length++] = samp;

    // if we record too much....
    // end or overwrite?
    if (x->length >= BUFFER_LENGTH)
    {
      x->flags.recording = 0;
      x->flags.playback = 1;
    }

    return samp;
  }
}

void main_1()
{ 
  ws2812_init();
  sleep_ms(16);
  leds_startup();

  multicore_fifo_push_blocking(1);

  while (2)
  {
    leds_update_all();
    sleep_ms(8);
  }
}

int main()
{
  // set cpu to 250mhz
  set_sys_clock_khz(250000, false);

  //set vreg to pwm mode
  gpio_init(23);
  gpio_set_dir(23, true);
  gpio_put(23, true);

  dac_init();
  buttons_init();
  pots_init();

  multicore_launch_core1(main_1);

  while(!multicore_fifo_pop_blocking())
  ;

  
  
  uint8_t channel = 0;
  uint32_t prev_time = time_us_32();

  while (1)
  {
    while (!(main_timer()))
      ;

    button_check(channel);

    uint16_t res = pots_get_value(channel);

    sample_buffer_t* x = &buffer[channel];

    if (x->flags.unipolar)
    {
      int32_t temp = res;

      int32_t outmax = 65535; 
      int32_t outmin = 32768;

      if (x->flags.neg)
      {
        outmax = 32767;
        outmin = 0;
      }

      temp *= (outmax - outmin + 1);
      temp /= 65535;
      temp += outmin;
      
      res = temp;

    }

    res = handle_sample(x, res);

    dac_output_sample(res, channel);

    led_set_val(channel, res, x->led_mode);

    channel++;
    channel &= CHANNEL_MASK;
  }
}
