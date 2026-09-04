/**
 * @file    spi_types.h
 * @brief   Shared types for the SPI driver
 * @details SPI is a fast 4-wire bus. It talks to sensors, displays, and
 *          memory chips. One main device controls the clock. Data goes
 *          out and comes back at the same time.
 *
 *          The application owns one SPI_Bus_t per peripheral it wants to
 *          use (SPI1..SPI6) and says which pins carry it. Device drivers
 *          then register an SPI_Device_t on that bus with the settings the
 *          chip needs. The bus is reprogrammed whenever a different device
 *          is selected, so slow and fast chips can share the same wires.
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

#define SPI_TIMEOUT_DEFAULT 1000U
#define SPI_TIMEOUT_SHORT 100U
#define SPI_TIMEOUT_LONG 5000U

typedef enum { SPI_OK = 0, SPI_ERROR, SPI_BUSY, SPI_TIMEOUT, SPI_INVALID_PARAM } SPI_StatusTypeDef;

/** Bus settings one device needs; mirrors SPI_InitTypeDef field by field. */
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
 * @brief Which peripheral and which pins make up a bus
 * @note  A NULL port leaves that line unconfigured, for write-only devices
 *        that have no MISO, or for boards where the pin is set up elsewhere.
 */
typedef struct {
    SPI_TypeDef *instance; /*!< SPI1..SPI6 */
    GPIO_TypeDef *sckPort; /*!< Clock line */
    uint16_t sckPin;
    GPIO_TypeDef *misoPort; /*!< Data in, NULL if not wired */
    uint16_t misoPin;
    GPIO_TypeDef *mosiPort; /*!< Data out, NULL if not wired */
    uint16_t mosiPin;
    uint32_t pull;     /*!< GPIO_NOPULL (0), GPIO_PULLUP or GPIO_PULLDOWN */
    uint8_t alternate; /*!< GPIO_AFx_SPIx, or 0 to derive it from the instance */
} SPI_BusConfig_t;

struct SPI_Device;

/**
 * @brief One SPI peripheral, owned by the application
 */
typedef struct {
    SPI_HandleTypeDef hal;    /*!< HAL handle, programmed for whichever device is selected */
    SPI_BusConfig_t config;   /*!< Wiring this bus was opened with */
    struct SPI_Device *owner; /*!< Device the peripheral is currently programmed for */
    bool ready;               /*!< Set once the pins and clock are up */
} SPI_Bus_t;

/**
 * @brief One chip on a bus, owned by the driver that talks to it
 */
typedef struct SPI_Device {
    SPI_Bus_t *bus;           /*!< Bus the chip is wired to */
    SPI_ConfigTypeDef config; /*!< Bus settings this chip needs */
    bool ready;               /*!< Set once registered on a bus */
} SPI_Device_t;

#ifdef __cplusplus
}
#endif

#endif /* SPI_TYPES_H */
