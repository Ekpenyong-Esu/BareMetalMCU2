/**
 ******************************************************************************
 * @file    pwr_core.h
 * @brief   PWR initialisation and configuration
 ******************************************************************************
 */

#ifndef __PWR_CORE_H__
#define __PWR_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Initialize PWR module with configuration
 * @details Enables the PWR clock, then applies backup access, wakeup pin and
 *          PVD settings from @p config
 * @param   config Pointer to configuration structure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_Init(const PWR_ConfigTypeDef *config);

/**
 * @brief   Initialize PWR module with default settings
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_InitDefault(void);

/**
 * @brief   Get default configuration
 * @param   config Pointer to configuration structure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_GetDefaultConfig(PWR_ConfigTypeDef *config);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_CORE_H__ */
