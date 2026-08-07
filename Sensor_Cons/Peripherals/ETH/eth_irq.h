/**
 * @file eth_irq.h
 * @brief Ethernet interrupt control and event callbacks
 */

#ifndef ETH_IRQ_H
#define ETH_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eth_types.h"

/**
 * @brief   Enable the Ethernet interrupt and switch the driver to interrupt mode
 * @param   handle Driver handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 * @note    If the peripheral is already running it is restarted in interrupt
 *          mode, since the DMA interrupt sources are armed by the start call.
 */
HAL_StatusTypeDef ETH_EnableInterrupts(ETH_Handle_t *handle);

/**
 * @brief   Disable the Ethernet interrupt and return to polled mode
 * @param   handle Driver handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef ETH_DisableInterrupts(ETH_Handle_t *handle);

/**
 * @brief   Report whether interrupt mode is selected
 * @retval  bool true when interrupt mode is active
 */
bool ETH_IsInterruptModeEnabled(void);

/**
 * @brief   Ethernet interrupt service routine
 * @note    Call from ETH_IRQHandler() in stm32f4xx_it.c. The vector symbol is
 *          not defined here so that the vector table cannot be aliased to a
 *          function that expects arguments.
 */
void ETH_Driver_IRQHandler(void);

/**
 * @brief   Transmission complete callback
 * @param   heth HAL handle that raised the event
 */
void ETH_TxCpltCallback(ETH_HandleTypeDef *heth);

/**
 * @brief   Reception complete callback
 * @param   heth HAL handle that raised the event
 */
void ETH_RxCpltCallback(ETH_HandleTypeDef *heth);

/**
 * @brief   Error callback
 * @param   heth HAL handle that raised the event
 */
void ETH_ErrorCallback(ETH_HandleTypeDef *heth);

#ifdef __cplusplus
}
#endif

#endif /* ETH_IRQ_H */
