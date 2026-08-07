/**
  ******************************************************************************
  * @file    audio_i2s.h
  * @brief   I2S transport backend for the audio subsystem
  * @details Exposes nothing but its operations table; every function that
  *          implements the table is private to audio_i2s.c.
  ******************************************************************************
  */

#ifndef AUDIO_I2S_H
#define AUDIO_I2S_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_types.h"

/** Operations table for AUDIO_INTERFACE_I2S. */
extern const AudioIfOps_t AudioI2sOps;

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_I2S_H */
