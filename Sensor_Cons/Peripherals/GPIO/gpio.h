/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   Reusable GPIO driver for STM32F4
  * @details Single entry point for GPIO: configuring a pin, reading/writing pin
  *          and port state, and unmasking a pin's EXTI line in the NVIC.
  *          Peripherals call this driver instead of the HAL, so a port to
  *          another MCU family only has to replace gpio.c.
  ******************************************************************************
  */

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported functions --------------------------------------------------------*/

/* --- Configuration -------------------------------------------------------- */

/**
 * @brief  Enable the peripheral clock of a GPIO port
 * @param  GPIOx: GPIO port (GPIOA..GPIOI)
 * @retval HAL_OK, or HAL_ERROR if the port is not one of GPIOA..GPIOI
 */
HAL_StatusTypeDef GPIO_Driver_ClockEnable(GPIO_TypeDef *GPIOx);

/**
 * @brief  Configure one or more pins of a port
 * @param  GPIOx: GPIO port
 * @param  GPIO_Init: mode, pull, speed and alternate function
 * @note   Enables the port clock first, so callers never need the per-port
 *         __HAL_RCC_GPIOx_CLK_ENABLE macro.
 * @note   A GPIO_MODE_IT_* mode also configures the EXTI line; call
 *         GPIO_Driver_EnableIRQ afterwards to unmask it in the NVIC.
 * @retval HAL_OK or HAL_ERROR
 */
HAL_StatusTypeDef GPIO_Driver_Pin_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);

/**
 * @brief  Reset one or more pins to their default (analog, floating) state
 * @param  GPIOx: GPIO port
 * @param  GPIO_Pin: pin mask
 * @retval HAL_OK or HAL_ERROR
 */
HAL_StatusTypeDef GPIO_Driver_Pin_DeInit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/* --- Pin state ------------------------------------------------------------ */

/**
 * @brief  Read the level of a single pin
 * @param  GPIOx: GPIO port
 * @param  GPIO_Pin: pin mask (one bit)
 * @retval GPIO_PIN_SET or GPIO_PIN_RESET
 */
GPIO_PinState GPIO_Driver_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/**
 * @brief  Drive one or more output pins
 * @param  GPIOx: GPIO port
 * @param  GPIO_Pin: pin mask
 * @param  PinState: GPIO_PIN_SET or GPIO_PIN_RESET
 */
void GPIO_Driver_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

/**
 * @brief  Invert one or more output pins
 * @param  GPIOx: GPIO port
 * @param  GPIO_Pin: pin mask
 */
void GPIO_Driver_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/* --- Port state ----------------------------------------------------------- */

/**
 * @brief  Read all 16 pins of a port at once
 * @param  GPIOx: GPIO port
 * @retval Port input level, bit n = pin n
 */
uint16_t GPIO_Driver_ReadPort(GPIO_TypeDef *GPIOx);

/**
 * @brief  Drive all 16 pins of a port at once
 * @param  GPIOx: GPIO port
 * @param  PortValue: bit n = pin n
 * @note   Writes every pin of the port, including ones owned by other drivers.
 */
void GPIO_Driver_WritePort(GPIO_TypeDef *GPIOx, uint16_t PortValue);

/* --- Interrupts ----------------------------------------------------------- */

/**
 * @brief  Enable the NVIC channel serving a pin's EXTI line
 * @param  GPIO_Pin: pin mask (one bit)
 * @param  PreemptPriority: NVIC pre-emption priority
 * @param  SubPriority: NVIC sub priority
 * @note   The edge comes from the GPIO_MODE_IT_* mode passed to
 *         GPIO_Driver_Pin_Init; this only unmasks the interrupt.
 * @note   Pins 5..9 and 10..15 each share one NVIC channel, so the priority
 *         applies to every pin in that group.
 * @retval HAL_OK, or HAL_ERROR if GPIO_Pin is not a single valid pin
 */
HAL_StatusTypeDef GPIO_Driver_EnableIRQ(uint16_t GPIO_Pin, uint32_t PreemptPriority, uint32_t SubPriority);

/**
 * @brief  Disable the NVIC channel serving a pin's EXTI line
 * @param  GPIO_Pin: pin mask (one bit)
 * @note   Disables the whole shared channel for pins 5..15.
 * @retval HAL_OK, or HAL_ERROR if GPIO_Pin is not a single valid pin
 */
HAL_StatusTypeDef GPIO_Driver_DisableIRQ(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */
