/**
 ******************************************************************************
 * @file    gpio.h
 * @brief   Simple GPIO driver for STM32F4
 * @details GPIO controls the pins on the chip. Each pin can be an input
 *          (reads high or low) or an output (sets high or low). This file
 *          is the main place to set up pins, read them, write them, and
 *          turn on pin interrupts.
 ******************************************************************************
 */

#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported functions --------------------------------------------------------*/

/* --- Configuration -------------------------------------------------------- */

/**
 * @brief  Turn on the clock for a GPIO port so its pins can be used
 * @param  GPIOx: Port to enable (GPIOA to GPIOK)
 * @note   GPIOJ and GPIOK are not bonded out on the LQFP144 package used by
 *         the STM32F429I-DISC1.
 * @retval HAL_OK if ok, HAL_ERROR if port is not valid
 */
HAL_StatusTypeDef GPIO_Driver_ClockEnable(GPIO_TypeDef *GPIOx);

/**
 * @brief  Set up one or more pins on a port
 * @param  GPIOx: Port to use
 * @param  GPIO_Init: Settings for the pins (input/output, pull, speed, alt function)
 * @note   Turns on the port clock for you, so you do not need to do it by hand.
 * @note   If you use an interrupt mode (GPIO_MODE_IT_*), also call
 *         GPIO_Driver_EnableIRQ to turn on the interrupt.
 * @retval HAL_OK if ok, HAL_ERROR if not
 */
HAL_StatusTypeDef GPIO_Driver_Pin_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);

/**
 * @brief  Reset pins back to default state (analog, no pull)
 * @param  GPIOx: Port to use
 * @param  GPIO_Pin: Which pins to reset (pin mask)
 * @retval HAL_OK if ok, HAL_ERROR if not
 */
HAL_StatusTypeDef GPIO_Driver_Pin_DeInit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/* --- Pin state ------------------------------------------------------------ */

/**
 * @brief  Read if a pin is high or low
 * @param  GPIOx: Port to use
 * @param  GPIO_Pin: Which pin to read (one pin only)
 * @retval GPIO_PIN_SET if high, GPIO_PIN_RESET if low
 */
GPIO_PinState GPIO_Driver_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/**
 * @brief  Set pins high or low
 * @param  GPIOx: Port to use
 * @param  GPIO_Pin: Which pins to set (pin mask)
 * @param  PinState: GPIO_PIN_SET for high, GPIO_PIN_RESET for low
 */
void GPIO_Driver_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

/**
 * @brief  Flip pins: high becomes low, low becomes high
 * @param  GPIOx: Port to use
 * @param  GPIO_Pin: Which pins to flip (pin mask)
 */
void GPIO_Driver_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/* --- Port state ----------------------------------------------------------- */

/**
 * @brief  Read all 16 pins of a port at once
 * @param  GPIOx: Port to use
 * @retval Value of all pins, bit n is pin n. Returns 0 if port is NULL
 */
uint16_t GPIO_Driver_ReadPort(GPIO_TypeDef *GPIOx);

/**
 * @brief  Set all 16 pins of a port at once
 * @param  GPIOx: Port to use
 * @param  PortValue: Value to write, bit n is pin n
 * @note   This changes every pin on the port, even pins used by other drivers.
 */
void GPIO_Driver_WritePort(GPIO_TypeDef *GPIOx, uint16_t PortValue);

/* --- Interrupts ----------------------------------------------------------- */

/**
 * @brief  Turn on the interrupt for a pin
 * @param  GPIO_Pin: Which pin (one pin only)
 * @param  PreemptPriority: How urgent this interrupt is
 * @param  SubPriority: Sub level if two interrupts have same urgency
 * @note   You must first set the pin to interrupt mode in GPIO_Driver_Pin_Init.
 *         This just turns on the interrupt in the system.
 * @note   Pins 5 to 9 share one interrupt, and pins 10 to 15 share another.
 *         So the priority you set affects all pins in that group.
 * @retval HAL_OK if ok, HAL_ERROR if pin is not valid
 */
HAL_StatusTypeDef GPIO_Driver_EnableIRQ(uint16_t GPIO_Pin, uint32_t PreemptPriority,
                                        uint32_t SubPriority);

/**
 * @brief  Turn off the interrupt for a pin
 * @param  GPIO_Pin: Which pin (one pin only)
 * @note   For pins 5 to 15, this turns off the shared interrupt for the whole group.
 * @retval HAL_OK if ok, HAL_ERROR if pin is not valid
 */
HAL_StatusTypeDef GPIO_Driver_DisableIRQ(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H */
