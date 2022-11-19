#include "phrader.h"

#include "internal_timer.h"
#include "buttons.h"
#include "potentiometer.h"
#include "dac.h"
#include "leds.h"

sample_buffer_t buffer[NUM_CHANNELS];

void set_offset(sample_buffer_t* x);
void set_attenuvert(sample_buffer_t* x);
void set_speed(sample_buffer_t* x);
void set_phrase_adjustment_method(sample_buffer_t* x);

void make_fader_unipolar(sample_buffer_t* x);
void attenuvert(sample_buffer_t* x);
void offset(sample_buffer_t* x);

void buttons_handle_hold(uint8_t channel)
{
    sample_buffer_t *x = &buffer[channel];

    x->flags.led_indicate = false;

    if (x->mode == MODE_RECORDING)
    {
      x->mode = MODE_RECORDING_CONF;
    }
    else if (x->mode == MODE_PLAYBACK)
    {
        x->mode = MODE_PHRASE_ADJUSTMENT_SELECTION;
    }
}

void buttons_handle_double_tap(uint8_t channel)
{
  sample_buffer_t *x = &buffer[channel];

  if (x->mode == MODE_PLAYBACK || x->mode == MODE_SET_ATTENUVERT || x->mode == MODE_SET_OFFSET || x->mode == MODE_SET_SPEED)
  {
    x->mode = MODE_DEFAULT;
  }
}

void buttons_handle_press(uint8_t channel)
{
  sample_buffer_t *x = &buffer[channel];

  x->flags.led_indicate = true;

  if (x->mode == MODE_DEFAULT)
  {
    x->mode = MODE_RECORDING;
    x->length = 0;
    x->position = 0;
    x->flags.phrase_adjustment_mode = MODE_SET_ATTENUVERT - MODE_SET_ATTENUVERT;
    x->attenuvert_amnt = 32767;
    x->offset = 0;
    x->speed = SPEEDX1;
  }
}

void buttons_handle_tap(uint8_t channel)
{
  sample_buffer_t *x = &buffer[channel];

  if (x->mode == MODE_DEFAULT)
  {
    x->flags.unipolar ^= 1;
  }
  else if (x->mode == MODE_PLAYBACK)
  {
    x->mode = x->flags.phrase_adjustment_mode + MODE_SET_ATTENUVERT;
  }
  else if (x->mode == MODE_SET_ATTENUVERT || x->mode == MODE_SET_OFFSET || x->mode == MODE_SET_SPEED)
  {
    x->mode = MODE_PLAYBACK;
  }
}

void buttons_handle_release(uint8_t channel)
{
  sample_buffer_t *x = &buffer[channel];

  x->flags.led_indicate = false;

  if (x->mode == MODE_RECORDING)
  {
    x->mode = MODE_DEFAULT;
  }
  else if (x->mode == MODE_RECORDING_CONF || x->mode == MODE_PHRASE_ADJUSTMENT_SELECTION)
  {
    x->mode = MODE_PLAYBACK;
  }
}

void handle_sample(sample_buffer_t *x)
{
  //we are either recording or just default. either way the sample can be recorded for ease of code
  if (x->mode < MODE_PLAYBACK)
  {
    make_fader_unipolar(x);
    x->buffer[x->length++] = x->fader_pos;
    x->output_val = x->fader_pos;

    if (x->length >= BUFFER_LENGTH)
    {
      x->length = BUFFER_LENGTH - 1;

      if (x->mode == MODE_RECORDING_CONF)
      x->mode = MODE_PLAYBACK;
    }
  }
  else if (x->mode >= MODE_PLAYBACK)
  {
    if (x->mode == MODE_SET_OFFSET)
    set_offset(x);
    else if (x->mode == MODE_SET_ATTENUVERT)
    set_attenuvert(x);
    else if (x->mode == MODE_SET_SPEED)
    set_speed(x);
    else if (x->mode == MODE_PHRASE_ADJUSTMENT_SELECTION)
    set_phrase_adjustment_method(x);

    //we are playing back!
    // we need to play back a recording
    // the slider will become an attenuverter.
    x->output_val = x->buffer[x->position / SPEEDX1];
    
    x->position += x->speed;

    // reset the counter of necessary.
    if (x->position >= x->length * SPEEDX1)
    x->position = 0;

    attenuvert(x);
    offset(x);
  }
}

int main()
{
  stdio_init_all();

  // set cpu to 250mhz
  set_sys_clock_khz(250000, false);

  //set vreg to pwm mode
  gpio_init(23);
  gpio_set_dir(23, true);
  gpio_put(23, true);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, true);

  dac_init();
  buttons_init();
  pots_init();

  for (int x = 0; x < FW_VERSION_NUMBER; x++)
  {
    gpio_put(PICO_DEFAULT_LED_PIN, true);
    sleep_ms(250);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    sleep_ms(250);
  }

  leds_init(); 
  
  uint8_t channel = 0;

  while (1)
  {
    if (main_timer())
    {
      button_check(channel);

      sample_buffer_t* x = &buffer[channel];

      x->fader_pos = pots_get_value(channel);
      handle_sample(x);

      dac_output_sample(channel, x->output_val);

      leds_task(x, channel);

      channel++;
      channel &= CHANNEL_MASK;
    }
  }
}

//a simple linear map
void make_fader_unipolar(sample_buffer_t* x)
{
  if (x->flags.unipolar == false)
  return;

  int32_t temp = x->fader_pos;

  temp = ((temp + 32768l) * 32767l) / 65536l;

  x->fader_pos = temp;

}

void attenuvert(sample_buffer_t* x)
{
  int32_t temp = x->output_val;
  temp *= x->attenuvert_amnt;
  temp /= 32768;
  x->output_val = temp;
}

void offset(sample_buffer_t* x)
{
  int32_t temp = x->output_val;
  temp += x->offset;

  if (temp > 32767)
  temp = 32767;
  else if (temp < -32768)
  temp = -32768;

  x->output_val = temp;
}



void set_offset(sample_buffer_t* x)
{
  make_fader_unipolar(x);
  x->offset = x->fader_pos;
}

void set_attenuvert(sample_buffer_t* x)
{
  make_fader_unipolar(x);
  x->attenuvert_amnt = x->fader_pos;
}

void set_speed(sample_buffer_t* x)
{
  //make 3 bit for 8 values
  x->fader_pos >>= 13;

  if (x->fader_pos <= -3)
  x->speed = SPEEDHALF;
  else if (x->fader_pos == -1 || x->fader_pos == 0)
  x->speed = SPEEDX1;
  else if (x->fader_pos >= 2)
  x->speed = SPEEDX2;

}

void set_phrase_adjustment_method(sample_buffer_t* x)
{
  x->fader_pos >>= 13;

  if (x->fader_pos <= -3)
  x->flags.phrase_adjustment_mode = MODE_SET_SPEED - MODE_SET_ATTENUVERT;
  else if (x->fader_pos == -1 || x->fader_pos == 0)
  x->flags.phrase_adjustment_mode = MODE_SET_ATTENUVERT - MODE_SET_ATTENUVERT;
  else if (x->fader_pos >= 2)
  x->flags.phrase_adjustment_mode = MODE_SET_OFFSET - MODE_SET_ATTENUVERT;
}