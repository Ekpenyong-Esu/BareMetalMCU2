/**
  ******************************************************************************
  * @file    can_core.h
  * @brief   CAN lifecycle, mode switching and handle registry
  * @details Owns initialization and the instance-to-handle registry that lets
  *          the HAL MspInit and ISR callbacks find the right handle. Replaces
  *          the former singleton, so CAN1 and CAN2 can be used together.
  *
  * CAN Core Responsibilities:
  * - CAN_Init/DeInit: Full peripheral setup (clock, GPIO, bit timing, HAL init)
  * - Mode switching: Normal / loopback / silent without full reinit
  * - Handle registry: Maps CAN instance -> handle for HAL callback routing
  * - Status queries: Mailbox fill, FIFO depth, last error, counters
  * - Error bookkeeping: Classification and sticky last-error
  *
  * Multi-CAN Support:
  * - CAN1 and CAN2 can be used simultaneously
  * - Each has its own handle; registry in can_core.c maps instance to handle
  * - HAL callbacks (HAL_CAN_TxMailbox*CompleteCallback, HAL_CAN_RxFifo*MsgPendingCallback,
  *   HAL_CAN_ErrorCallback) use this registry to find the correct handle
  *
  * Initialization Sequence:
  * 1. Validate pins (no default — caller must supply TX/RX on this board)
  * 2. Resolve instance (NULL -> CAN1), register handle before HAL_CAN_Init
  *    so HAL_CAN_MspInit can find it for clock/GPIO setup
  * 3. CAN_ApplyBaudRate() — derives prescaler/BS1/BS2 from live PCLK1
  * 4. HAL_CAN_Init() + HAL_CAN_Start()
  * 5. Mark handle initialized
  */

#ifndef CAN_CORE_H
#define CAN_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/**
 * @brief   Initialize a CAN handle
 *
 * Full initialization sequence: pin validation, instance resolution, handle
 * registration, bit-timing derivation, HAL init and start. The instance
 * defaults to CAN1 if config->instance is NULL.
 *
 * @param   hcan Handle to initialize (must be zeroed or uninitialized)
 * @param   config Configuration to apply; config->instance selects the
 *          peripheral (CAN1/CAN2) and defaults to CAN1 when NULL
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 * @note    Pins have no default on this board — every CAN-capable pin is
 *          taken by LCD/SDRAM/USB, so config->pins must be supplied.
 */
HAL_StatusTypeDef CAN_Init(CAN_Handle_t *hcan, const CAN_Config *config);

/**
 * @brief   Release a CAN handle and unregister it
 *
 * Stops the peripheral, deinitializes HAL CAN (which invokes MspDeInit to
 * release clocks and pins), unregisters from the instance map, and clears
 * callbacks and status.
 *
 * @param   hcan Handle to release
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_DeInit(CAN_Handle_t *hcan);

/**
 * @brief   Switch operating mode without full reinit
 *
 * Stops the peripheral, updates the mode, re-inits and restarts. On
 * failure the previous mode is restored where possible.
 *
 * @param   hcan Handle (must be initialized)
 * @param   mode New operating mode
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_SetMode(CAN_Handle_t *hcan, CAN_OperatingMode mode);

/**
 * @brief   Snapshot live bus status
 *
 * Copies counters and last error, then overlays live mailbox/FIFO fill
 * levels and the current HAL error (sticky — only overrides last_error
 * while the condition is still live).
 *
 * @param   hcan Handle
 * @param   status Output snapshot
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_GetStatus(CAN_Handle_t *hcan, CAN_Status *status);

/**
 * @brief   Clear latched error flags and counters
 *
 * Clears FIFO overrun flags (the only writable error flags — EWG/EPV/BOF
 * are read-only ESR bits owned by the controller) and resets error_count
 * and last_error.
 *
 * @param   hcan Handle
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_ClearErrors(CAN_Handle_t *hcan);

/**
 * @brief   Resolve the driver handle that owns a HAL instance
 * @param   instance CAN1 or CAN2
 * @retval  CAN_Handle_t* Owning handle, or NULL when unregistered
 * @note    Used by HAL_CAN_MspInit/MspDeInit and the ISR callbacks
 */
CAN_Handle_t *CAN_FromInstance(const CAN_TypeDef *instance);

/**
 * @brief   Classify the current HAL error flags for a handle
 * @param   hcan Handle
 * @retval  CAN_ErrorType Most severe live error, or CAN_ERROR_NONE
 * @note    Used by can_events to dispatch error callbacks
 */
CAN_ErrorType CAN_GetErrorType(CAN_Handle_t *hcan);

/**
 * @brief   Record an error against a handle (bumps counter, sets last_error)
 * @param   hcan Handle
 * @param   error Error to record
 */
void CAN_RecordError(CAN_Handle_t *hcan, CAN_ErrorType error);

/**
 * @brief   Bump transmit counter for a handle
 * @param   hcan Handle
 */
void CAN_IncrementTxCount(CAN_Handle_t *hcan);

/**
 * @brief   Bump receive counter for a handle
 * @param   hcan Handle
 */
void CAN_IncrementRxCount(CAN_Handle_t *hcan);

#ifdef __cplusplus
}
#endif

#endif /* CAN_CORE_H */
