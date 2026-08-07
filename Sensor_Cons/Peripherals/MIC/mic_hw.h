/**
  ******************************************************************************
  * @file    mic_hw.h
  * @brief   Board wiring and transport bring-up for the PDM microphone
  ******************************************************************************
  */

#ifndef MIC_HW_H
#define MIC_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mic_types.h"

/**
 * @brief Configure the PDM clock and data pins.
 */
MIC_StatusTypeDef MIC_HW_InitGPIO(void);

/**
 * @brief Bring up the I2S peripheral in PDM receive mode.
 */
MIC_StatusTypeDef MIC_HW_InitI2S(MIC_HandleTypeDef *hmic);

/**
 * @brief Bring up the circular receive DMA and link it to the I2S handle.
 */
MIC_StatusTypeDef MIC_HW_InitDMA(MIC_HandleTypeDef *hmic);

/**
 * @brief Re-apply the I2S audio frequency after a configuration change.
 */
MIC_StatusTypeDef MIC_HW_SetSampleRate(MIC_HandleTypeDef *hmic, uint32_t sampleRate);

#ifdef __cplusplus
}
#endif

#endif /* MIC_HW_H */
