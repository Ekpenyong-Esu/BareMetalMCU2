/**
  ******************************************************************************
  * @file    pwr_regulator.h
  * @brief   Internal voltage regulator scaling
  ******************************************************************************
  */

#ifndef __PWR_REGULATOR_H__
#define __PWR_REGULATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Select voltage scale 1 (highest performance, highest consumption)
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableHighPerformance(void);

/**
 * @brief   Select voltage scale 3 (lowest consumption, reduced max frequency)
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableLowPowerMode(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWR_REGULATOR_H__ */
