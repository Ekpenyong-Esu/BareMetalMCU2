/**
 * @file ts_events.h
 * @brief Touch interrupt setup and deferred servicing
 * @details The application owns HAL_GPIO_EXTI_Callback and forwards the edge
 *          of its touch INT pin to TS_EXTI_Callback, which only raises a flag.
 *          The STMPE811 is cleared from thread context by TS_ServiceIRQ
 *          because clearing it needs I2C transfers.
 */

#ifndef TS_EVENTS_H
#define TS_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_types.h"

/**
 * @brief Enable or disable the STMPE811 interrupt outputs
 * @param hts Touchscreen handle
 * @param enable true to report touch detect and FIFO threshold events
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_EnableInterrupt(TS_HandleTypeDef *hts, bool enable);

/**
 * @brief Configure the MCU side of the touch interrupt line (EXTI + NVIC)
 * @param hts Touchscreen handle carrying the INT port and pin
 * @return TS_StatusTypeDef TS_INVALID_PARAM when no INT pin was configured
 */
TS_StatusTypeDef TS_ITConfig(TS_HandleTypeDef *hts);

/**
 * @brief Note a touch edge from the application's EXTI callback
 * @details Interrupt context. Call it when HAL_GPIO_EXTI_Callback reports the
 *          pin given in TS_ConfigTypeDef::intPin; it runs the activity
 *          callback and defers the rest to TS_ServiceIRQ.
 * @param hts Touchscreen handle
 */
void TS_EXTI_Callback(TS_HandleTypeDef *hts);

/**
 * @brief Report whether an EXTI edge is waiting to be serviced
 * @param hts Touchscreen handle
 * @return bool true while a touch interrupt is pending
 */
bool TS_IrqPending(const TS_HandleTypeDef *hts);

/**
 * @brief Service a pending touch interrupt from thread context
 * @details Call from the main loop or the LVGL task; it clears the STMPE811
 *          interrupt and runs the registered callbacks.
 * @param hts Touchscreen handle
 */
void TS_ServiceIRQ(TS_HandleTypeDef *hts);

/**
 * @brief Read and clear the controller interrupt status, running callbacks
 * @param hts Touchscreen handle
 */
void TS_IRQHandler(TS_HandleTypeDef *hts);

#ifdef __cplusplus
}
#endif

#endif /* TS_EVENTS_H */
