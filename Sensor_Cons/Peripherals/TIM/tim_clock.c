/**
 * @file tim_clock.c
 * @brief Timer clock enable and rate query
 */

#include "tim_clock.h"

bool TIM_Clock_Enable(const TIM_TypeDef *instance)
{
    if (instance == TIM1)  { __HAL_RCC_TIM1_CLK_ENABLE();  return true; }
    if (instance == TIM2)  { __HAL_RCC_TIM2_CLK_ENABLE();  return true; }
    if (instance == TIM3)  { __HAL_RCC_TIM3_CLK_ENABLE();  return true; }
    if (instance == TIM4)  { __HAL_RCC_TIM4_CLK_ENABLE();  return true; }
    if (instance == TIM5)  { __HAL_RCC_TIM5_CLK_ENABLE();  return true; }
    if (instance == TIM6)  { __HAL_RCC_TIM6_CLK_ENABLE();  return true; }
    if (instance == TIM7)  { __HAL_RCC_TIM7_CLK_ENABLE();  return true; }
    if (instance == TIM8)  { __HAL_RCC_TIM8_CLK_ENABLE();  return true; }
    if (instance == TIM9)  { __HAL_RCC_TIM9_CLK_ENABLE();  return true; }
    if (instance == TIM10) { __HAL_RCC_TIM10_CLK_ENABLE(); return true; }
    if (instance == TIM11) { __HAL_RCC_TIM11_CLK_ENABLE(); return true; }
    if (instance == TIM12) { __HAL_RCC_TIM12_CLK_ENABLE(); return true; }
    if (instance == TIM13) { __HAL_RCC_TIM13_CLK_ENABLE(); return true; }
    if (instance == TIM14) { __HAL_RCC_TIM14_CLK_ENABLE(); return true; }

    return false;
}

uint32_t TIM_Clock_GetHz(const TIM_TypeDef *instance)
{
    bool onApb2 = ((instance == TIM1) || (instance == TIM8) || (instance == TIM9) ||
                  (instance == TIM10) || (instance == TIM11)) != 0;

    uint32_t pclkHz = (int)onApb2 ? HAL_RCC_GetPCLK2Freq() : HAL_RCC_GetPCLK1Freq();
    
    uint32_t ppre = (int)onApb2 ? ((RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos)
                           : ((RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos);

    /* Bit 2 of the APB prescaler field marks any divider above 1, which doubles
     * the clock the timers receive relative to their bus. */
    return ((ppre & 0x4u) != 0u) ? (pclkHz * 2u) : pclkHz;
}

bool TIM_Clock_HasOutputChannels(const TIM_TypeDef *instance)
{
    return (instance != NULL) && (instance != TIM6) && (instance != TIM7);
}
