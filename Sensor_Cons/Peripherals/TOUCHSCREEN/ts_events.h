/**
 * @file ts_events.h
 * @brief Touch interrupt setup and deferred servicing
 * @details The EXTI callback only raises a flag; the STMPE811 is cleared from
 *          thread context because clearing it needs I2C transfers.
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
 * @param hts Touchscreen handle
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_ITConfig(TS_HandleTypeDef *hts);

/**
 * @brief Report whether an EXTI edge is waiting to be serviced
 * @return bool true while a touch interrupt is pending
 */
bool TS_IrqPending(void);

/**
 * @brief Service a pending touch interrupt from thread context
 * @details Call from the main loop or the LVGL task; it clears the STMPE811
 *          interrupt and runs the registered callbacks.
 */
void TS_ServiceIRQ(void);

/**
 * @brief Read and clear the controller interrupt status, running callbacks
 * @param hts Touchscreen handle
 */
void TS_IRQHandler(TS_HandleTypeDef *hts);

#ifdef __cplusplus
}
#endif

#endif /* TS_EVENTS_H */
