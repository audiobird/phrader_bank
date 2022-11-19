#ifndef __PHRADER_H__
#define __PHRADER_H__

#include <stdio.h> 
#include "pico/stdlib.h"

#ifndef FW_VERSION_NUMBER
#define FW_VERSION_NUMBER 0
#endif

#define CHANNEL_MASK (NUM_CHANNELS - 1)

//for speed adjustment.
typedef enum {
  SPEEDX2   = 4,
  SPEEDX1   = 2,
  SPEEDHALF = 1,
} speed_phase_step_t;

typedef enum {
  MODE_DEFAULT,
  MODE_RECORDING,
  MODE_RECORDING_CONF,
  MODE_PLAYBACK,
  MODE_SET_ATTENUVERT,
  MODE_SET_OFFSET,
  MODE_SET_SPEED,
  MODE_PHRASE_ADJUSTMENT_SELECTION,
} phrader_mode_t;

#define BUFFER_LENGTH     (8192ul)
#define ACCUMU_LENGTH_MAX (BUFFER_LENGTH * PHASE_STEP_X1)

typedef struct
{
  uint8_t unipolar                : 1;
  uint8_t led_indicate            : 1;
  uint8_t phrase_adjustment_mode  : 2;
  uint8_t reserved                : 4;
} flags_t;

typedef struct
{
  uint16_t length;
  uint16_t position;
  int16_t buffer[BUFFER_LENGTH];
  phrader_mode_t mode;
  flags_t flags;

  speed_phase_step_t speed; 
  int16_t attenuvert_amnt;
  int16_t offset;

  int16_t output_val;
  int16_t fader_pos;
} sample_buffer_t;

#endif