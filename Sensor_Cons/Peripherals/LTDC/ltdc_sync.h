/**
 * @file ltdc_sync.h
 * @brief Shadow register reload (VSYNC) synchronisation
 */

#ifndef LTDC_SYNC_H
#define LTDC_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ltdc_types.h"

#define LTDC_RELOAD_TIMEOUT_MS      5000    /*!< Default wait for a reload event */

/**
 * @brief Request a shadow register reload
 * @param driver Driver record
 * @param reload LTDC_SRCR_VBR (at next VSYNC) or LTDC_SRCR_IMR (immediate)
 * @return HAL_StatusTypeDef HAL status
 */
HAL_StatusTypeDef LTDC_RequestReload(LTDC_Driver_t *driver, uint32_t reload);

/**
 * @brief Wait until the requested reload has been applied
 * @param driver Driver record
 * @param timeout_ms Maximum wait in milliseconds
 * @return HAL_StatusTypeDef HAL_OK, HAL_TIMEOUT or HAL_ERROR
 */
HAL_StatusTypeDef LTDC_WaitForReload(LTDC_Driver_t *driver, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_SYNC_H */
