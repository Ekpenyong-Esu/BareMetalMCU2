/**
 * @file dac_output.h
 * @brief DAC conversion control and output value access
 */

#ifndef DAC_OUTPUT_H
#define DAC_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dac_types.h"

/**
 * @brief   Load an output code and start the conversion
 * @param   hdac Handle
 * @param   channel Channel, must match the initialized channel
 * @param   value Code from 0 to DAC_MAX_VALUE_12BIT
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DAC_SetValue(DAC_HandleStruct *hdac, uint32_t channel, uint32_t value);

/**
 * @brief   Start the conversion using the code already loaded
 * @param   hdac Handle
 * @param   channel Channel, must match the initialized channel
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DAC_Start(DAC_HandleStruct *hdac, uint32_t channel);

/**
 * @brief   Stop the conversion, leaving the last value on the output
 * @param   hdac Handle
 * @param   channel Channel, must match the initialized channel
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DAC_Stop(DAC_HandleStruct *hdac, uint32_t channel);

/**
 * @brief   Read the code currently held in the data output register
 * @param   hdac Handle
 * @param   channel Channel, must match the initialized channel
 * @retval  uint32_t Current code, or 0 when the arguments are rejected
 */
uint32_t DAC_GetValue(const DAC_HandleStruct *hdac, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* DAC_OUTPUT_H */
