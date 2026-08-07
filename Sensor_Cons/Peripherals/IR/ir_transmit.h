/**
 * @file ir_transmit.h
 * @brief IR transmit path: protocol encoding and blocking pulse emission
 */

#ifndef IR_TRANSMIT_H
#define IR_TRANSMIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_types.h"

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Transmit IR frame (NEC protocol)
 * @note  Blocks for the duration of the frame, roughly 68 ms.
 * @param handle: Pointer to IR handle structure
 * @param address: Device address
 * @param command: Command code
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitNEC(IR_Handle_t *handle, uint8_t address, uint8_t command);

/**
 * @brief Transmit IR frame (RC5 protocol)
 * @param handle: Pointer to IR handle structure
 * @param address: Device address, 0..IR_RC5_MAX_ADDRESS
 * @param command: Command code, 0..IR_RC5_MAX_COMMAND
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitRC5(IR_Handle_t *handle, uint8_t address, uint8_t command);

/**
 * @brief Transmit IR frame (SIRC protocol)
 * @param handle: Pointer to IR handle structure
 * @param address: Device address, 0..IR_SIRC_MAX_ADDRESS
 * @param command: Command code, 0..IR_SIRC_MAX_COMMAND
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitSIRC(IR_Handle_t *handle, uint8_t address, uint8_t command);

/**
 * @brief Transmit a caller supplied pulse train
 * @param handle: Pointer to IR handle structure
 * @param pulses: Array of pulse structures
 * @param count: Number of pulses, at most IR_TX_BUFFER_SIZE
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_TransmitCustom(IR_Handle_t *handle, const IR_Pulse_t *pulses, uint16_t count);

#ifdef __cplusplus
}
#endif

#endif /* IR_TRANSMIT_H */
