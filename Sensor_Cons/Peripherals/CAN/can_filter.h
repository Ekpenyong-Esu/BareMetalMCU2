/**
  ******************************************************************************
  * @file    can_filter.h
  * @brief   CAN filter bank configuration
  ******************************************************************************
  */

#ifndef CAN_FILTER_H
#define CAN_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

HAL_StatusTypeDef CAN_ConfigFilter(const CAN_FilterConfig *filter_config);

#ifdef __cplusplus
}
#endif

#endif /* CAN_FILTER_H */
