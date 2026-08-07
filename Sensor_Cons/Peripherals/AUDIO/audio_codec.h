/**
  ******************************************************************************
  * @file    audio_codec.h
  * @brief   CS43L22 codec control for the audio subsystem
  * @details Everything that reaches the codec over I2C lives here: the reset
  *          sequence, the register map and the volume/mute translation. The
  *          transport backends and the core never issue an I2C transaction.
  ******************************************************************************
  */

#ifndef AUDIO_CODEC_H
#define AUDIO_CODEC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_types.h"

/**
 * @brief   Reset, identify and configure the codec for playback
 * @details Releases the codec from reset, verifies the part number, applies the
 *          playback register sequence, restores @p volume and powers the codec
 *          up.
 * @param   volume Volume to program once the codec responds (0..AUDIO_VOLUME_MAX)
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef Audio_CodecInit(uint8_t volume);

/**
 * @brief   Program the master volume of both output channels
 * @param   volume Volume level, clamped to AUDIO_VOLUME_MAX
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef Audio_CodecSetVolume(uint8_t volume);

/**
 * @brief   Mute or unmute the headphone outputs
 * @param   mute True to mute, false to unmute
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef Audio_CodecSetMute(bool mute);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_CODEC_H */
