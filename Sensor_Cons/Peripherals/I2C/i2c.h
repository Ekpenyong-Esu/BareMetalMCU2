/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   Aggregator header for the I2C bus driver
  * @details Includes all sub-modules. Consumers that need the HAL handle
  *          should call I2C_GetHandle() instead of referencing hi2c3 directly.
  ******************************************************************************
  */

#ifndef I2C_H
#define I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_types.h"
#include "i2c_core.h"
#include "i2c_transfer.h"

#ifdef __cplusplus
}
#endif

#endif /* I2C_H */
