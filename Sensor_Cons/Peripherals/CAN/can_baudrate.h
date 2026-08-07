/**
  ******************************************************************************
  * @file    can_baudrate.h
  * @brief   Baud-rate timing tables for the CAN peripheral
  ******************************************************************************
  */

#ifndef CAN_BAUDRATE_H
#define CAN_BAUDRATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/**
 * @brief Apply prescaler / SJW / time-segment settings for @p config to @p hcan.
 * @note  Only fills the timing fields; does not call HAL_CAN_Init.
 */
HAL_StatusTypeDef CAN_ApplyBaudRate(CAN_HandleTypeDef *hcan, const CAN_Config *config);

#ifdef __cplusplus
}
#endif

#endif /* CAN_BAUDRATE_H */
