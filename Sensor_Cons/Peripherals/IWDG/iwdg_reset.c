/**
 * @file iwdg_reset.c
 * @brief Reset source reporting
 */

#include "iwdg_reset.h"

bool IWDG_WasResetSource(void)
{
    return (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET);
}

void IWDG_ClearResetFlag(void)
{
    __HAL_RCC_CLEAR_RESET_FLAGS();
}
