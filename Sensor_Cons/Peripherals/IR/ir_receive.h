/**
 * @file ir_receive.h
 * @brief IR receive path: capture control, buffering and frame handover
 */

#ifndef IR_RECEIVE_H
#define IR_RECEIVE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_types.h"

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Start IR receiver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_StartReceive(IR_Handle_t *handle);

/**
 * @brief Stop IR receiver
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef IR_StopReceive(IR_Handle_t *handle);

/**
 * @brief Take the last decoded frame, if one is pending
 * @param handle: Pointer to IR handle structure
 * @param frame: Pointer to frame structure to fill
 * @return HAL_StatusTypeDef: HAL_OK when a frame was copied out
 */
HAL_StatusTypeDef IR_GetFrame(IR_Handle_t *handle, IR_Frame_t *frame);

/**
 * @brief Discard any partially received frame
 * @note  Internal to the driver.
 * @param handle: Pointer to IR handle structure
 * @return void
 */
void IR_ResetReceiveBuffer(IR_Handle_t *handle);

/**
 * @brief Input capture callback (to be called from HAL interrupt)
 * @param handle: Pointer to IR handle structure
 * @param captureValue: Captured timer value
 * @return void
 */
void IR_InputCaptureCallback(IR_Handle_t *handle, uint32_t captureValue);

/**
 * @brief Timer overflow callback (to be called from HAL interrupt)
 * @param handle: Pointer to IR handle structure
 * @return void
 */
void IR_TimerOverflowCallback(IR_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* IR_RECEIVE_H */
