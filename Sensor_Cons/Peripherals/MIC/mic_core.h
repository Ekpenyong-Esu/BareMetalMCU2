/**
  ******************************************************************************
  * @file    mic_core.h
  * @brief   Microphone lifecycle, configuration and handle registry
  ******************************************************************************
  */

#ifndef MIC_CORE_H
#define MIC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mic_types.h"

/**
 * @brief Bring up GPIO, I2S and DMA, then apply the default configuration.
 */
MIC_StatusTypeDef MIC_Init(MIC_HandleTypeDef *hmic, I2S_HandleTypeDef *hi2s, DMA_HandleTypeDef *hdma);

/**
 * @brief Stop capture and tear down the transport.
 */
MIC_StatusTypeDef MIC_DeInit(MIC_HandleTypeDef *hmic);

/**
 * @brief Apply a configuration and reprogram the I2S sample rate.
 */
MIC_StatusTypeDef MIC_Configure(MIC_HandleTypeDef *hmic, const MIC_ConfigTypeDef *config);

/**
 * @brief Fill a configuration structure with the driver defaults.
 */
MIC_StatusTypeDef MIC_GetDefaultConfig(MIC_ConfigTypeDef *config);

/* Sets digital output volume (post-capture gain). */
MIC_StatusTypeDef MIC_SetVolume(MIC_HandleTypeDef *hmic, uint8_t volume);
MIC_StatusTypeDef MIC_GetVolume(const MIC_HandleTypeDef *hmic, uint8_t *volume);
/* Sets the analog/preamp gain stage. */
MIC_StatusTypeDef MIC_SetGain(MIC_HandleTypeDef *hmic, MIC_GainTypeDef gain);
/* Enables/disables muting samples below threshold to suppress background hiss. */
MIC_StatusTypeDef MIC_ConfigureNoiseGate(MIC_HandleTypeDef *hmic, bool enable, float threshold);
/* Current I2S sample rate in Hz. */
uint32_t MIC_GetSampleRate(const MIC_HandleTypeDef *hmic);

/* Reads the current RMS and peak signal levels. */
MIC_StatusTypeDef MIC_GetAudioLevel(const MIC_HandleTypeDef *hmic, float *rms_level, float *peak_level);
/* Reads accumulated capture statistics (e.g. overrun/error counts). */
MIC_StatusTypeDef MIC_GetStatistics(const MIC_HandleTypeDef *hmic, MIC_StatisticsTypeDef *stats);
/* Zeroes the accumulated capture statistics. */
MIC_StatusTypeDef MIC_ResetStatistics(MIC_HandleTypeDef *hmic);

/**
 * @brief Register record-complete and error notifications.
 */
MIC_StatusTypeDef MIC_RegisterCallbacks(MIC_HandleTypeDef *hmic,
                                        void (*record_callback)(void),
                                        void (*error_callback)(void));

/**
 * @brief Handle registered by MIC_Init(), for interrupt-context dispatch.
 * @retval NULL when the driver is not initialised.
 */
MIC_HandleTypeDef *MIC_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* MIC_CORE_H */
