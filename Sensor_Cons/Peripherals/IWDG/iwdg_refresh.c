/**
 * @file iwdg_refresh.c
 * @brief Watchdog counter refresh
 */

#include "iwdg_refresh.h"
#include "iwdg_core.h"

IWDG_StatusTypeDef IWDG_Refresh(void)
{
    IWDG_HandleTypeDef *handle = IWDG_GetHandle();

    /* Refreshing before initialization would write through a null Instance. */
    if (handle == NULL) {
        return IWDG_NOT_READY;
    }

    return (HAL_IWDG_Refresh(handle) == HAL_OK) ? IWDG_OK : IWDG_ERROR;
}
