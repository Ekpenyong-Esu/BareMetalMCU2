/**
 ******************************************************************************
 * @file    can_events.h
 * @brief   CAN callback registration and interrupt management
 * @details Per-handle callback storage and HAL ISR dispatch. Each handle
 *          owns its own tx/rx/error callbacks; the HAL ISR callbacks
 *          resolve the handle via the registry in can_core.c.
 *
 * CAN Events Responsibilities:
 * - CAN_Register*Callback: Store per-handle callbacks (or NULL to clear)
 * - CAN_Enable/DisableInterrupts: Activate/deactivate bxCAN notifications
 * - CAN_Notify*: Dispatch callbacks from transfer and ISR paths
 * - HAL callbacks: HAL_CAN_TxMailbox*CompleteCallback, HAL_CAN_RxFifo*MsgPendingCallback,
 *   HAL_CAN_ErrorCallback — all resolve the handle by instance
 */

#ifndef CAN_EVENTS_H
#define CAN_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/**
 * @brief   Register a transmit-complete callback for a handle
 * @param   hcan Handle
 * @param   callback Function to call on mailbox complete (or NULL to clear)
 */
void CAN_RegisterTxCallback(CAN_Handle_t *hcan, CAN_TxCallback callback);

/**
 * @brief   Register a receive callback for a handle
 * @param   hcan Handle
 * @param   callback Function to call when a frame arrives (or NULL to clear)
 */
void CAN_RegisterRxCallback(CAN_Handle_t *hcan, CAN_RxCallback callback);

/**
 * @brief   Register an error callback for a handle
 * @param   hcan Handle
 * @param   callback Function to call on bus error (or NULL to clear)
 */
void CAN_RegisterErrorCallback(CAN_Handle_t *hcan, CAN_ErrorCallbackFn callback);

/**
 * @brief   Enable bxCAN interrupts for a handle
 * @param   hcan Handle (must be initialized)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_EnableInterrupts(CAN_Handle_t *hcan);

/**
 * @brief   Disable bxCAN interrupts for a handle
 * @param   hcan Handle
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_DisableInterrupts(CAN_Handle_t *hcan);

/**
 * @brief   Dispatch the Rx callback for a handle (called by transfer and ISR)
 * @param   hcan Handle
 * @param   frame Received frame
 */
void CAN_NotifyRx(CAN_Handle_t *hcan, const CAN_Frame *frame);

/**
 * @brief   Dispatch the Tx-complete callback for a handle
 * @param   hcan Handle
 * @param   mailbox Mailbox index (0..2) that completed
 */
void CAN_NotifyTxComplete(CAN_Handle_t *hcan, uint8_t mailbox);

/**
 * @brief   Classify and dispatch the error callback for a handle
 * @param   hcan Handle
 */
void CAN_NotifyError(CAN_Handle_t *hcan);

#ifdef __cplusplus
}
#endif

#endif /* CAN_EVENTS_H */
