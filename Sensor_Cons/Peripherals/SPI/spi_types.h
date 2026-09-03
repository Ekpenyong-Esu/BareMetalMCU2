/**
  * @file    spi_types.h
  * @brief   Shared types for the SPI driver
  * @details SPI is a fast 4-wire bus. It talks to sensors, displays, and
  *          memory chips. One main device controls the clock. Data goes
  *          out and comes back at the same time. This file holds the
  *          common settings and status codes used by the driver.
  */

#ifndef SPI_TYPES_H
#define SPI_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SPI_TIMEOUT_DEFAULT     1000U
#define SPI_TIMEOUT_SHORT       100U
#define SPI_TIMEOUT_LONG        5000U

typedef enum {
    SPI_OK = 0,
    SPI_ERROR,
    SPI_BUSY,
    SPI_TIMEOUT,
    SPI_INVALID_PARAM
} SPI_StatusTypeDef;

typedef struct {
    uint32_t Mode;
    uint32_t Direction;
    uint32_t DataSize;
    uint32_t CLKPolarity;
    uint32_t CLKPhase;
    uint32_t NSS;
    uint32_t BaudRatePrescaler;
    uint32_t FirstBit;
    uint32_t TIMode;
    uint32_t CRCCalculation;
    uint32_t CRCPolynomial;
} SPI_ConfigTypeDef;

/**
 * @brief One device on the shared bus, owned by the driver that talks to it
 * @note  The board has a single usable SPI bus but several devices on it, so
 *        the settings belong to the device rather than to the peripheral.
 */
typedef struct {
    SPI_ConfigTypeDef config;   /*!< Bus settings this device needs */
    bool              ready;    /*!< Set once the config has been accepted */
} SPI_Device_t;

#ifdef __cplusplus
}
#endif

#endif /* SPI_TYPES_H */
