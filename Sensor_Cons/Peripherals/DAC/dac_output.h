/**
 * @file dac_output.h
 * @brief DAC conversion control and output value access
 *
 * DAC Output Control:
 * - DAC_SetValue(): Loads code into DHR register and starts conversion (software trigger)
 *   For timer triggers, this loads the value; the timer TRGO starts conversion
 * - DAC_Start(): Starts conversion with already-loaded value (for timer triggers)
 * - DAC_Stop(): Stops conversion, output holds last value
 * - DAC_GetValue(): Reads the current DOR (Data Output Register)
 *
 * Trigger Modes:
 * - DAC_TRIGGER_NONE / SOFTWARE: Conversion starts immediately on DAC_SetValue()
 * - Timer TRGO: Conversion starts on timer update event (TRGO)
 *   Use DAC_SetValue() to load the next value; timer triggers the conversion
 *
 * Alignment Handling:
 * - 12-bit right (DAC_ALIGN_12B_R): value 0..4095 in DHR12R1
 * - 12-bit left (DAC_ALIGN_12B_L): value 0..4095 in DHR12L1 (shifted left 4)
 * - 8-bit right (DAC_ALIGN_8B_R): value 0..255 in DHR8R1
 * The HAL handles alignment; caller provides right-aligned value.
 */

#ifndef DAC_OUTPUT_H
#define DAC_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dac_types.h"

/**
 * @brief   Load an output code and start the conversion (software trigger)
 *
 * Writes the value to the appropriate Data Holding Register (DHR) based on
 * the configured alignment, then triggers a software conversion if the
 * trigger is SOFTWARE/NONE. For timer triggers, this only loads the value;
 * the timer TRGO will start the actual conversion.
 *
 * Validates handle initialization and channel match (must be CH1).
 * Clamps value to the maximum for the configured alignment.
 *
 * @param   hdac Handle (must be initialized)
 * @param   channel Channel, must match the initialized channel (DAC_CHANNEL_1)
 * @param   value Code from 0 to DAC_MAX_VALUE_12BIT (or 255 for 8-bit alignment)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid args or HAL failure
 */
HAL_StatusTypeDef DAC_SetValue(DAC_HandleStruct *hdac, uint32_t channel, uint32_t value);

/**
 * @brief   Start the conversion using the code already loaded
 *
 * For timer-triggered mode: triggers a software conversion (SWTRIG) to
 * immediately convert the currently loaded value. For software trigger
 * mode, this is a no-op (DAC_SetValue already started it).
 *
 * @param   hdac Handle (must be initialized)
 * @param   channel Channel, must match the initialized channel
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid args
 */
HAL_StatusTypeDef DAC_Start(DAC_HandleStruct *hdac, uint32_t channel);

/**
 * @brief   Stop the conversion, leaving the last value on the output
 *
 * Disables the DAC channel. The output pin holds the last converted value
 * (if output buffer enabled) or goes high-Z (if buffer disabled).
 *
 * @param   hdac Handle (must be initialized)
 * @param   channel Channel, must match the initialized channel
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid args
 */
HAL_StatusTypeDef DAC_Stop(DAC_HandleStruct *hdac, uint32_t channel);

/**
 * @brief   Read the code currently held in the data output register
 *
 * Reads the DOR (Data Output Register) which holds the last converted value.
 * This is the actual analog output code, not the holding register.
 *
 * @param   hdac Handle (must be initialized)
 * @param   channel Channel, must match the initialized channel
 * @retval  uint32_t Current code (0..4095), or 0 when the arguments are rejected
 */
uint32_t DAC_GetValue(const DAC_HandleStruct *hdac, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* DAC_OUTPUT_H */
