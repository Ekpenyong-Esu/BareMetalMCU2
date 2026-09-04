/**
 ******************************************************************************
 * @file    mic_hw.h
 * @brief   Transport bring-up for the PDM microphone
 * @note    Pins, I2S block and DMA stream all come from the wiring the
 *          application put in MIC_ConfigTypeDef; nothing here is board-fixed.
 ******************************************************************************
 */

#ifndef MIC_HW_H
#define MIC_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mic_types.h"

/**
 * @brief Whether a configuration names hardware this driver can drive.
 * @note  Only I2S2 and I2S3 exist on the F4, so any other instance is refused.
 */
bool MIC_HW_IsValidWiring(const MIC_ConfigTypeDef *config);

/**
 * @brief Configure the PDM clock and data pins named in the wiring.
 */
MIC_StatusTypeDef MIC_HW_InitGPIO(const MIC_ConfigTypeDef *config);

/**
 * @brief Bring up the I2S peripheral in PDM receive mode.
 */
MIC_StatusTypeDef MIC_HW_InitI2S(MIC_HandleTypeDef *hmic);

/**
 * @brief Bring up the circular receive DMA and link it to the I2S handle.
 */
MIC_StatusTypeDef MIC_HW_InitDMA(MIC_HandleTypeDef *hmic);

/**
 * @brief Tear down DMA, I2S and pins in the reverse order of bring-up.
 */
void MIC_HW_DeInit(MIC_HandleTypeDef *hmic);

/**
 * @brief Re-apply the I2S audio frequency after a configuration change.
 */
MIC_StatusTypeDef MIC_HW_SetSampleRate(MIC_HandleTypeDef *hmic, uint32_t sampleRate);

#ifdef __cplusplus
}
#endif

#endif /* MIC_HW_H */
