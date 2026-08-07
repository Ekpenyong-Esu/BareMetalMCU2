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
 * @brief Vector entry points; wired from Core/Src/stm32f4xx_it.c.
 */
void MIC_DMA_IRQHandler(void);
void MIC_I2S_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* MIC_EVENTS_H */
