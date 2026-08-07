/**
 * @file ltdc_sync.c
 * @brief Shadow register reload (VSYNC) synchronisation
 */

#include "ltdc_sync.h"
#include "ltdc_core.h"

HAL_StatusTypeDef LTDC_RequestReload(LTDC_Driver_t *driver, uint32_t reload)
{
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    driver->reloadFlag = 0;
    HAL_LTDC_Reload(driver->hltdc, reload);
    return HAL_OK;
}

HAL_StatusTypeDef LTDC_WaitForReload(LTDC_Driver_t *driver, uint32_t timeout_ms)
{
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    uint32_t start = HAL_GetTick();
    while (driver->reloadFlag == 0) {
        if ((HAL_GetTick() - start) > timeout_ms) {
            return HAL_TIMEOUT;
        }
        __WFE();    /* HAL_LTDC_ReloadEventCallback() issues __SEV() to wake us */
    }

    driver->reloadFlag = 0;
    return HAL_OK;
}
