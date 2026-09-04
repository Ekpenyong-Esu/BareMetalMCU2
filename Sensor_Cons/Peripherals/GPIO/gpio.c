/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   Reusable GPIO driver for STM32F4
 * @details STM32F4 implementation of the gpio.h interface. Everything that is
 *          MCU-family specific (the RCC clock macros and the EXTI-to-IRQn
 *          mapping) is contained in this file.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef GPIO_GetPinNumber(uint16_t GPIO_Pin, uint8_t *PinNumber);
static HAL_StatusTypeDef GPIO_GetIRQn(uint16_t GPIO_Pin, IRQn_Type *IRQn);

/* Exported functions --------------------------------------------------------*/

HAL_StatusTypeDef GPIO_Driver_ClockEnable(GPIO_TypeDef *GPIOx) {
    if (GPIOx == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (GPIOx == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (GPIOx == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (GPIOx == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if (GPIOx == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
    else if (GPIOx == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
    else if (GPIOx == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
    else if (GPIOx == GPIOH) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
    else if (GPIOx == GPIOI) {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
    /* Present in the STM32F429 register map, but only bonded out on packages
       larger than the LQFP144 used by the STM32F429I-DISC1. */
    else if (GPIOx == GPIOJ) {
        __HAL_RCC_GPIOJ_CLK_ENABLE();
    }
    else if (GPIOx == GPIOK) {
        __HAL_RCC_GPIOK_CLK_ENABLE();
    }
    else {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef GPIO_Driver_Pin_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init) {
    if (GPIOx == NULL || GPIO_Init == NULL) {
        return HAL_ERROR;
    }

    if (GPIO_Driver_ClockEnable(GPIOx) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_GPIO_Init(GPIOx, GPIO_Init);

    return HAL_OK;
}

HAL_StatusTypeDef GPIO_Driver_Pin_DeInit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    if (GPIOx == NULL || GPIO_Pin == 0U) {
        return HAL_ERROR;
    }

    HAL_GPIO_DeInit(GPIOx, GPIO_Pin);

    return HAL_OK;
}

GPIO_PinState GPIO_Driver_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    if (GPIOx == NULL) {
        return GPIO_PIN_RESET;
    }

    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}

void GPIO_Driver_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState) {
    if (GPIOx == NULL) {
        return;
    }

    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
}

void GPIO_Driver_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    if (GPIOx == NULL) {
        return;
    }

    HAL_GPIO_TogglePin(GPIOx, GPIO_Pin);
}

uint16_t GPIO_Driver_ReadPort(GPIO_TypeDef *GPIOx) {
    if (GPIOx == NULL) {
        return 0U;
    }

    return (uint16_t)GPIOx->IDR;
}

void GPIO_Driver_WritePort(GPIO_TypeDef *GPIOx, uint16_t PortValue) {
    if (GPIOx == NULL) {
        return;
    }

    GPIOx->ODR = PortValue;
}

HAL_StatusTypeDef GPIO_Driver_EnableIRQ(uint16_t GPIO_Pin, uint32_t PreemptPriority,
                                        uint32_t SubPriority) {
    IRQn_Type irqn = NonMaskableInt_IRQn; /* any value: the lookup below sets it */

    if (GPIO_GetIRQn(GPIO_Pin, &irqn) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_NVIC_SetPriority(irqn, PreemptPriority, SubPriority);
    HAL_NVIC_EnableIRQ(irqn);

    return HAL_OK;
}

HAL_StatusTypeDef GPIO_Driver_DisableIRQ(uint16_t GPIO_Pin) {
    IRQn_Type irqn = NonMaskableInt_IRQn; /* any value: the lookup below sets it */

    if (GPIO_GetIRQn(GPIO_Pin, &irqn) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_NVIC_DisableIRQ(irqn);

    return HAL_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Convert a one-hot pin mask to its pin number
 * @param  GPIO_Pin: pin mask, exactly one bit of 0..15 must be set
 * @param  PinNumber: receives 0..15
 * @retval HAL_OK or HAL_ERROR
 */
static HAL_StatusTypeDef GPIO_GetPinNumber(uint16_t GPIO_Pin, uint8_t *PinNumber) {
    /* Reject 0 and multi-pin masks: a single EXTI line must be identified. */
    if (GPIO_Pin == 0U || (GPIO_Pin & (GPIO_Pin - 1U)) != 0U) {
        return HAL_ERROR;
    }

    *PinNumber = (uint8_t)__builtin_ctz(GPIO_Pin);

    return HAL_OK;
}

/**
 * @brief  Find the NVIC channel serving a pin's EXTI line
 * @param  GPIO_Pin: pin mask, exactly one bit set
 * @param  IRQn: receives the interrupt number
 * @retval HAL_OK or HAL_ERROR
 */
static HAL_StatusTypeDef GPIO_GetIRQn(uint16_t GPIO_Pin, IRQn_Type *IRQn) {
    uint8_t pinNumber = 0;

    if (GPIO_GetPinNumber(GPIO_Pin, &pinNumber) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Lines 0..4 have their own vector, 5..9 and 10..15 are grouped. */
    switch (pinNumber) {
        case 0:
            *IRQn = EXTI0_IRQn;
            break;
        case 1:
            *IRQn = EXTI1_IRQn;
            break;
        case 2:
            *IRQn = EXTI2_IRQn;
            break;
        case 3:
            *IRQn = EXTI3_IRQn;
            break;
        case 4:
            *IRQn = EXTI4_IRQn;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            *IRQn = EXTI9_5_IRQn;
            break;
        default:
            *IRQn = EXTI15_10_IRQn;
            break;
    }

    return HAL_OK;
}

/* EXTI interrupt handlers live in Core/Src/stm32f4xx_it.c, not in this driver. */
