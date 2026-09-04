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
 * @brief Bring up the pins, I2S block and DMA stream named in the config,
 *        then apply its audio settings.
 * @param hmic   Caller-owned driver handle.
 * @param hi2s   Caller-owned I2S handle; the driver fills in Instance and Init.
 * @param hdma   Caller-owned DMA handle; the driver fills in Instance and Init.
 * @param config Wiring plus audio settings; see MIC_GetDefaultConfig().
 * @note  The NVIC lines for the stream and the I2S block are left to the
 *        application, which also routes their vectors to MIC_*_IRQHandler().
 */
MIC_StatusTypeDef MIC_Init(MIC_HandleTypeDef *hmic, I2S_HandleTypeDef *hi2s,
                           DMA_HandleTypeDef *hdma, const MIC_ConfigTypeDef *config);

/**
 * @brief Stop capture and tear down the transport.
 */
MIC_StatusTypeDef MIC_DeInit(MIC_HandleTypeDef *hmic);

/**
 * @brief Apply the audio settings and reprogram the I2S sample rate.
 * @note  The wiring fields of @p config are ignored; they are fixed at MIC_Init().
 */
MIC_StatusTypeDef MIC_Configure(MIC_HandleTypeDef *hmic, const MIC_ConfigTypeDef *config);

/**
 * @brief Fill the audio fields with the driver defaults and clear the wiring,
 *        which the application must then supply.
 */
MIC_StatusTypeDef MIC_GetDefaultConfig(MIC_ConfigTypeDef *config);

MIC_StatusTypeDef MIC_SetVolume(MIC_HandleTypeDef *hmic, uint8_t volume);
MIC_StatusTypeDef MIC_GetVolume(const MIC_HandleTypeDef *hmic, uint8_t *volume);
MIC_StatusTypeDef MIC_SetGain(MIC_HandleTypeDef *hmic, MIC_GainTypeDef gain);
MIC_StatusTypeDef MIC_ConfigureNoiseGate(MIC_HandleTypeDef *hmic, bool enable, float threshold);
uint32_t MIC_GetSampleRate(const MIC_HandleTypeDef *hmic);

MIC_StatusTypeDef MIC_GetAudioLevel(const MIC_HandleTypeDef *hmic, float *rms_level,
                                    float *peak_level);
MIC_StatusTypeDef MIC_GetStatistics(const MIC_HandleTypeDef *hmic, MIC_StatisticsTypeDef *stats);
MIC_StatusTypeDef MIC_ResetStatistics(MIC_HandleTypeDef *hmic);

/**
 * @brief Register record-complete and error notifications.
 */
MIC_StatusTypeDef MIC_RegisterCallbacks(MIC_HandleTypeDef *hmic, void (*record_callback)(void),
                                        void (*error_callback)(void));

/**
 * @brief Handle brought up on a given I2S block, for interrupt-context dispatch.
 * @retval NULL when no initialised handle uses that block.
 */
MIC_HandleTypeDef *MIC_FindByInstance(const SPI_TypeDef *instance);

/**
 * @brief Handle whose receive DMA runs on a given stream.
 * @retval NULL when no initialised handle uses that stream.
 */
MIC_HandleTypeDef *MIC_FindByDMAStream(const DMA_Stream_TypeDef *stream);

#ifdef __cplusplus
}
#endif

#endif /* MIC_CORE_H */
