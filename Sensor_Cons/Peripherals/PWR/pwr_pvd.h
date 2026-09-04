/**
 ******************************************************************************
 * @file    pwr_pvd.h
 * @brief   Programmable Voltage Detector control
 ******************************************************************************
 */

#ifndef __PWR_PVD_H__
#define __PWR_PVD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Enable the Programmable Voltage Detector
 * @details Monitors VDD and triggers EXTI line 16 when it crosses the threshold
 * @param   level PVD threshold level
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnablePVD(PWR_PVDLevelTypeDef level);

/**
 * @brief   Disable the Programmable Voltage Detector
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_DisablePVD(void);

/**
 * @brief   Get PVD output status
 * @retval  bool True if VDD is below the configured threshold
 */
bool PWR_GetPVDStatus(void);

/**
 * @brief   Enable the PVD interrupt in the NVIC
 * @details The EXTI line 16 configuration is done by PWR_EnablePVD
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnablePVDInterrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_PVD_H__ */
