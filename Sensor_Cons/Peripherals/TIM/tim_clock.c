/**
 * @file tim_clock.c
 * @brief Timer clock enable and rate query implementation
 *
 * This module handles the STM32F4-specific RCC register manipulation to
 * enable timer clocks and compute the actual timer input clock frequency.
 *
 * STM32F4 Timer Clock Details:
 * - APB1 timers: TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
 * - APB2 timers: TIM1, TIM8, TIM9, TIM10, TIM11
 * - Timer clock = PCLKx * (PPREx > 1 ? 2 : 1)
 *   This doubling happens because the timer clock divider is after the APB
 *   prescaler in the clock tree.
 */

#include "tim_clock.h"

bool TIM_Clock_Enable(const TIM_TypeDef *instance) {
    /* Enable the appropriate RCC clock enable bit for each timer.
     * Uses HAL macros which expand to register writes. */
    if (instance == TIM1) {
        __HAL_RCC_TIM1_CLK_ENABLE();
        return true;
    }
    if (instance == TIM2) {
        __HAL_RCC_TIM2_CLK_ENABLE();
        return true;
    }
    if (instance == TIM3) {
        __HAL_RCC_TIM3_CLK_ENABLE();
        return true;
    }
    if (instance == TIM4) {
        __HAL_RCC_TIM4_CLK_ENABLE();
        return true;
    }
    if (instance == TIM5) {
        __HAL_RCC_TIM5_CLK_ENABLE();
        return true;
    }
    if (instance == TIM6) {
        __HAL_RCC_TIM6_CLK_ENABLE();
        return true;
    }
    if (instance == TIM7) {
        __HAL_RCC_TIM7_CLK_ENABLE();
        return true;
    }
    if (instance == TIM8) {
        __HAL_RCC_TIM8_CLK_ENABLE();
        return true;
    }
    if (instance == TIM9) {
        __HAL_RCC_TIM9_CLK_ENABLE();
        return true;
    }
    if (instance == TIM10) {
        __HAL_RCC_TIM10_CLK_ENABLE();
        return true;
    }
    if (instance == TIM11) {
        __HAL_RCC_TIM11_CLK_ENABLE();
        return true;
    }
    if (instance == TIM12) {
        __HAL_RCC_TIM12_CLK_ENABLE();
        return true;
    }
    if (instance == TIM13) {
        __HAL_RCC_TIM13_CLK_ENABLE();
        return true;
    }
    if (instance == TIM14) {
        __HAL_RCC_TIM14_CLK_ENABLE();
        return true;
    }

    return false; /* Unknown timer instance */
}

uint32_t TIM_Clock_GetHz(const TIM_TypeDef *instance) {
    /* Determine which APB bus the timer is on */
    bool onApb2 = ((instance == TIM1) || (instance == TIM8) || (instance == TIM9) ||
                   (instance == TIM10) || (instance == TIM11)) != 0;

    /* Get the APB bus clock frequency from HAL */
    uint32_t pclkHz = onApb2 ? HAL_RCC_GetPCLK2Freq() : HAL_RCC_GetPCLK1Freq();

    /* Read the APB prescaler from RCC_CFGR:
     * PPRE1 bits [10:8] for APB1, PPRE2 bits [13:11] for APB2
     * Values: 0xx = HCLK not divided, 100 = /2, 101 = /4, 110 = /8, 111 = /16
     * Bit 2 (value 0x4) indicates any division > 1 */
    uint32_t ppre = onApb2 ? ((RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos)
                           : ((RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos);

    /* When APB prescaler > 1, timer clock = 2 * PCLKx.
     * This is a hardware quirk of the STM32F4 clock tree. */
    return ((ppre & 0x4u) != 0u) ? (pclkHz * 2u) : pclkHz;
}

bool TIM_Clock_HasOutputChannels(const TIM_TypeDef *instance) {
    /* TIM6 and TIM7 are basic timers (16-bit, no output channels).
     * All other timers have at least one output compare channel. */
    return ((instance != NULL) && (instance != TIM6) && (instance != TIM7)) != 0;
}
