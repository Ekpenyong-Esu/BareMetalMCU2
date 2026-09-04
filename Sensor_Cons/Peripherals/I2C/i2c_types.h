/**
 ******************************************************************************
 * @file    i2c_types.h
 * @brief   Shared types for the I2C driver
 * @details I2C is a two-wire bus. Many sensors share the same two wires
 *          to talk to the MCU.
 *
 *          The application owns one I2C_Bus_t per peripheral it wants to
 *          use (I2C1..I2C3) and says which pins carry it. Device drivers
 *          then register an I2C_Device_t on that bus with the chip's
 *          address and the speed it needs. The bus is reprogrammed
 *          whenever a different device is selected, so a slow sensor and
 *          a fast sensor can share the same two wires.
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

#define I2C_TIMEOUT_DEFAULT 1000U
#define I2C_TIMEOUT_SHORT 100U
#define I2C_TIMEOUT_LONG 5000U

#define I2C_ADDR_MIN 0x08U
#define I2C_ADDR_MAX 0x77U

/**
 * @brief Result of an I2C action
 */
typedef enum {
    I2C_OK = 0,       /* All good */
    I2C_ERROR,        /* Something went wrong */
    I2C_BUSY,         /* Bus is busy, try later */
    I2C_TIMEOUT,      /* Took too long, no answer */
    I2C_NACK,         /* Device said no (no ack) */
    I2C_INVALID_PARAM /* Bad setting given */
} I2C_StatusTypeDef;

/**
 * @brief Bus settings one device needs; mirrors I2C_InitTypeDef field by field
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
 * @brief Which peripheral and which pins make up a bus
 */
typedef struct {
    I2C_TypeDef *instance; /*!< I2C1..I2C3 */
    GPIO_TypeDef *sclPort; /*!< Clock line */
    uint16_t sclPin;
    GPIO_TypeDef *sdaPort; /*!< Data line */
    uint16_t sdaPin;
    uint8_t alternate; /*!< GPIO_AFx_I2Cx, or 0 to derive it from the instance */
} I2C_BusConfig_t;

struct I2C_Device;

/**
 * @brief One I2C peripheral, owned by the application
 */
typedef struct {
    I2C_HandleTypeDef hal;    /*!< HAL handle, programmed for whichever device is selected */
    I2C_BusConfig_t config;   /*!< Wiring this bus was opened with */
    struct I2C_Device *owner; /*!< Device the peripheral is currently programmed for */
    bool ready;               /*!< Set once the pins and clock are up */
} I2C_Bus_t;

/**
 * @brief One sensor or chip on a bus, owned by the driver that talks to it
 */
typedef struct I2C_Device {
    I2C_Bus_t *bus;           /*!< Bus the chip is wired to */
    uint16_t address;         /*!< Address of the chip on the bus (already shifted left) */
    I2C_ConfigTypeDef config; /*!< Bus speed and settings for this chip */
    bool ready;               /*!< Set once registered on a bus */
} I2C_Device_t;

#ifdef __cplusplus
}
#endif

#endif /* I2C_TYPES_H */
