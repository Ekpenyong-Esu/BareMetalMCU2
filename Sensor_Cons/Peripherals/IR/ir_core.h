/**
 * @file ir_core.h
 * @brief IR driver lifecycle, carrier configuration and status
 */

#ifndef IR_CORE_H
#define IR_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_types.h"

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Initialize IR driver
 * @param handle: Pointer to IR handle structure
 * @param htimCarrier: Pointer to carrier timer handle
 * @param htimCapture: Pointer to capture timer handle
 * @param txChannel: PWM channel for carrier
 * @param rxChannel: Input capture channel
 * @param config: Pointer to configuration structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_Init(IR_Handle_t *handle, TIM_HandleTypeDef *htimCarrier,
                          TIM_HandleTypeDef *htimCapture,
                          uint32_t txChannel, uint32_t rxChannel,
                          const IR_Config_t *config);

/**
 * @brief Deinitialize IR driver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_DeInit(IR_Handle_t *handle);

/**
 * @brief Set event callback function
 * @param handle: Pointer to IR handle structure
 * @param callback: Callback function pointer, NULL to detach
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_SetEventCallback(IR_Handle_t *handle,
                                      void (*callback)(IR_Event_t event, IR_Frame_t *frame));

/**
 * @brief Configure carrier frequency
 * @param handle: Pointer to IR handle structure
 * @param frequency: Carrier frequency in Hz, must be non-zero
 * @param dutyCycle: Duty cycle percentage (1-99)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_ConfigureCarrier(IR_Handle_t *handle, uint32_t frequency, uint8_t dutyCycle);

/**
 * @brief Set protocol tolerance
 * @param handle: Pointer to IR handle structure
 * @param tolerance: Tolerance in microseconds
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_SetTolerance(IR_Handle_t *handle, uint16_t tolerance);

/**
 * @brief Get current state
 * @param handle: Pointer to IR handle structure
 * @return IR_State_t: Current state
 */
IR_State_t IR_GetState(IR_Handle_t *handle);

/**
 * @brief Get last error
 * @param handle: Pointer to IR handle structure
 * @return uint32_t: Error code
 */
uint32_t IR_GetError(IR_Handle_t *handle);

/**
 * @brief Clear error and reset state
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_ClearError(IR_Handle_t *handle);

/**
 * @brief Reject a NULL or uninitialised handle
 * @note  Internal to the driver.
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_ValidateHandle(const IR_Handle_t *handle);

/**
 * @brief Deliver an event to the registered callback, if any
 * @note  Internal to the driver.
 * @param handle: Pointer to IR handle structure
 * @param event: Event to report
 * @param frame: Associated frame, may be NULL
 * @return void
 */
void IR_NotifyEvent(IR_Handle_t *handle, IR_Event_t event, IR_Frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* IR_CORE_H */
