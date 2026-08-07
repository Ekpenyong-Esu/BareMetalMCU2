/**
  ******************************************************************************
  * @file    sys.h
  * @brief   System module interface
  * @details This file contains all the function prototypes for
  *          system configuration and error handling.
  *          It provides core system functions for the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __SYS_H__
#define __SYS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes the system core components
 * @details Performs initial setup of system components and peripherals
 * @param   None
 * @retval  None
 */
void SYS_Init(void);

/**
 * @brief   Configures the system clock
 * @details Sets up the PLL, prescalers and clock sources for optimal performance
 * @note    This function should be called before any peripheral initialization
 * @param   None
 * @retval  None
 */
void SystemClock_Config(void);

/**
 * @brief   Free-running microsecond counter
 * @details Derived from the Cortex-M4 cycle counter (DWT).
 * @note    The cycle counter wraps every 2^32 cycles, so this value jumps from
 *          about 25565279 back to 0 at 168 MHz. Unsigned subtraction of two
 *          readings is only valid if they do not straddle that point, so use
 *          it for short intervals and fall back to HAL_GetTick() for longer
 *          ones.
 * @param   None
 * @retval  Elapsed microseconds since SYS_Init(), modulo the wrap above
 */
uint32_t SYS_GetMicros(void);

/**
 * @brief   Error handler function
 * @details Called when a critical error occurs during peripheral initialization
 *          or operation. Implements failsafe behavior.
 * @param   None
 * @retval  None
 */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_H__ */
