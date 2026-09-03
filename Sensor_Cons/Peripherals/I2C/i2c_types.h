/**
  ******************************************************************************
  * @file    i2c_types.h
  * @brief   Shared types for the I2C driver
  * @details I2C is a two-wire bus. Many sensors share the same two wires
  *          to talk to the MCU. This file holds common types and settings
  *          for I2C.
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

/**
 * @brief Result of an I2C action
 */
typedef enum {
    I2C_OK = 0,        /* All good */
    I2C_ERROR,         /* Something went wrong */
    I2C_BUSY,          /* Bus is busy, try later */
    I2C_TIMEOUT,       /* Took too long, no answer */
    I2C_NACK,          /* Device said no (no ack) */
    I2C_INVALID_PARAM  /* Bad setting given */
} I2C_StatusTypeDef;

/**
 * @brief Settings for the I2C bus
 * @details Tells the bus how fast to run and how to address devices.
 */
typedef struct {
    uint32_t ClockSpeed;      /* Bus speed in Hz, e.g. 100000 for 100 kHz */
    uint32_t DutyCycle;       /* High/low time ratio for fast mode */
    uint32_t AddressingMode;  /* 7-bit or 10-bit address */
    uint32_t OwnAddress1;     /* Our own address on the bus */
    uint32_t DualAddressMode; /* Use second address or not */
    uint32_t OwnAddress2;     /* Second own address if used */
    uint32_t GeneralCallMode; /* Listen to broadcast or not */
    uint32_t NoStretchMode;   /* Allow device to pause clock or not */
} I2C_ConfigTypeDef;

/**
 * @brief One sensor or chip on the shared I2C bus
 * @details Each device has its own address and bus speed. This lets a slow
 *          sensor and a fast sensor share the same two wires.
 */
typedef struct {
    uint16_t          address;  /*!< Address of the device on the bus */
    I2C_ConfigTypeDef config;   /*!< Bus speed and settings for this device */
    bool              ready;    /*!< True if the device is set up and ready */
} I2C_Device_t;

#ifdef __cplusplus
}
#endif

#endif /* I2C_TYPES_H */
