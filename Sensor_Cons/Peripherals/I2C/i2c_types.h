/**
  ******************************************************************************
  * @file    i2c_types.h
  * @brief   Shared vocabulary for the I2C driver
  ******************************************************************************
  */

#ifndef I2C_TYPES_H
#define I2C_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define I2C_TIMEOUT_DEFAULT     1000U
#define I2C_TIMEOUT_SHORT       100U
#define I2C_TIMEOUT_LONG        5000U

#define I2C_ADDR_MIN            0x08U
#define I2C_ADDR_MAX            0x77U

typedef enum {
    I2C_OK = 0,
    I2C_ERROR,
    I2C_BUSY,
    I2C_TIMEOUT,
    I2C_NACK,
    I2C_INVALID_PARAM
} I2C_StatusTypeDef;

typedef struct {
    uint32_t ClockSpeed;
    uint32_t DutyCycle;
    uint32_t AddressingMode;
    uint32_t OwnAddress1;
    uint32_t DualAddressMode;
    uint32_t OwnAddress2;
    uint32_t GeneralCallMode;
    uint32_t NoStretchMode;
} I2C_ConfigTypeDef;

/**
 * @brief One device on the shared bus, owned by the driver that talks to it
 * @note  Carries the address as well as the bus settings, so a slow device and
 *        a fast one can share the bus without either imposing its clock on the
 *        other.
 */
typedef struct {
    uint16_t          address;  /*!< Slave address, already shifted for the HAL */
    I2C_ConfigTypeDef config;   /*!< Bus settings this device needs */
    bool              ready;    /*!< Set once the config has been accepted */
} I2C_Device_t;

#ifdef __cplusplus
}
#endif

#endif /* I2C_TYPES_H */
