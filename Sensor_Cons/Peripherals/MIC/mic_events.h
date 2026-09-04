/**
 ******************************************************************************
 * @file    mic_events.h
 * @brief   Interrupt-context servicing for the PDM microphone
 ******************************************************************************
 */

#ifndef MIC_EVENTS_H
#define MIC_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mic_types.h"

/**
 * @brief Called on each half of the circular PDM buffer.
 * @note  Overridable; the default implementation runs the PDM->PCM pipeline.
 */
void MIC_HalfTransferCallback(MIC_HandleTypeDef *hmic);

/**
 * @brief Called when the circular PDM buffer wraps.
 */
void MIC_TransferCompleteCallback(MIC_HandleTypeDef *hmic);

/**
 * @brief Records a dropout and notifies the registered error callback.
 * @note  HAL_I2S_ErrorCallback is owned by the AUDIO driver, so this must be
 *        invoked by whoever owns that callback if I2S errors matter here.
 */
void MIC_ErrorCallback(MIC_HandleTypeDef *hmic);

/**
 * @brief Vector entry points. The application's vector table passes the
 *        stream or I2S block that fired, e.g. from DMA1_Stream3_IRQHandler()
 *        call MIC_DMA_IRQHandler(DMA1_Stream3); from SPI2_IRQHandler() call
 *        MIC_I2S_IRQHandler(SPI2). Unknown sources are ignored.
 */
void MIC_DMA_IRQHandler(const DMA_Stream_TypeDef *stream);
void MIC_I2S_IRQHandler(const SPI_TypeDef *instance);

#ifdef __cplusplus
}
#endif

#endif /* MIC_EVENTS_H */
