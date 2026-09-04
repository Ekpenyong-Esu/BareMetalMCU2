/**
 ******************************************************************************
 * @file    audio_codec.h
 * @brief   CS43L22 codec control for the audio subsystem
 * @details Everything that reaches the codec over I2C lives here: the reset
 *          sequence, the register map and the volume/mute translation. The
 *          transport backends and the core never issue an I2C transaction.
 *          The bus, address and reset line come from the handle's config;
 *          the codec's I2C_Device_t lives in the handle.
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
 * @details Registers the codec on config->codecBus, pulses the reset line if
 *          one is wired, verifies the part number, applies the playback
 *          register sequence, restores @p volume and powers the codec up.
 * @param   dev Handle whose config names the bus, address and reset line
 * @param   volume Volume to program once the codec responds (0..AUDIO_VOLUME_MAX)
 * @retval  AUDIO_StatusTypeDef AUDIO_INVALID_PARAM when no bus was given
 */
AUDIO_StatusTypeDef Audio_CodecInit(AUDIO_Handle_t *dev, uint8_t volume);

/**
 * @brief   Program the master volume of both output channels
 * @param   dev Handle whose codec to program
 * @param   volume Volume level, clamped to AUDIO_VOLUME_MAX
 * @retval  AUDIO_StatusTypeDef AUDIO_NOT_READY when the codec was never registered
 */
AUDIO_StatusTypeDef Audio_CodecSetVolume(AUDIO_Handle_t *dev, uint8_t volume);

/**
 * @brief   Mute or unmute the headphone outputs
 * @param   dev Handle whose codec to program
 * @param   mute True to mute, false to unmute
 * @retval  AUDIO_StatusTypeDef AUDIO_NOT_READY when the codec was never registered
 */
AUDIO_StatusTypeDef Audio_CodecSetMute(AUDIO_Handle_t *dev, bool mute);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_CODEC_H */
