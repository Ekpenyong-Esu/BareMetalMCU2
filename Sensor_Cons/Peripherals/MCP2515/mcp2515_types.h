/**
  ******************************************************************************
  * @file    mcp2515_types.h
  * @brief   Shared vocabulary for the MCP2515 stand-alone CAN controller
  * @details The STM32F429I-DISC1 cannot use its own bxCAN: on LQFP144 every
  *          CAN-capable pin is spent on the LCD, the SDRAM or USB. This part
  *          carries the whole CAN engine and reaches the MCU over SPI, so it
  *          costs one chip-select pin instead of a peripheral's worth of pins.
  ******************************************************************************
  */

#ifndef MCP2515_TYPES_H
#define MCP2515_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spi_core.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

#define MCP2515_MAX_DATA_LENGTH     8U
#define MCP2515_SPI_TIMEOUT         100U    /*!< Milliseconds per register access */

/* Crystals fitted to the common breakout modules */
#define MCP2515_OSC_8MHZ            8000000U
#define MCP2515_OSC_16MHZ           16000000U

/* Bit rates the controller and the timing maths both support */
#define MCP2515_BAUD_1000KBPS       1000000U
#define MCP2515_BAUD_500KBPS        500000U
#define MCP2515_BAUD_250KBPS        250000U
#define MCP2515_BAUD_125KBPS        125000U
#define MCP2515_BAUD_100KBPS        100000U

/* Exported types ------------------------------------------------------------*/

typedef enum {
    MCP2515_OK = 0,
    MCP2515_ERROR,              /*!< Device did not respond as expected */
    MCP2515_INVALID_PARAM,
    MCP2515_NOT_INITIALIZED,
    MCP2515_TIMEOUT,            /*!< Mode change did not take effect */
    MCP2515_NO_MESSAGE,         /*!< Receive found nothing waiting */
    MCP2515_TX_BUSY,            /*!< Previous frame still queued */
    MCP2515_BITRATE_UNSUPPORTED /*!< Crystal cannot produce this bit rate exactly */
} MCP2515_Status_t;

/** Values are the CANCTRL REQOP field, so they can be written straight out. */
typedef enum {
    MCP2515_MODE_NORMAL      = 0x00U,
    MCP2515_MODE_SLEEP       = 0x20U,
    MCP2515_MODE_LOOPBACK    = 0x40U,  /*!< Self-test: no bus or transceiver needed */
    MCP2515_MODE_LISTEN_ONLY = 0x60U,  /*!< Receives without acknowledging */
    MCP2515_MODE_CONFIG      = 0x80U   /*!< Only mode in which bit timing may be written */
} MCP2515_Mode_t;

typedef struct {
    uint32_t id;                            /*!< 11-bit, or 29-bit when extended */
    bool     extended;
    bool     remote;                        /*!< Remote transmission request */
    uint8_t  length;                        /*!< 0..8 */
    uint8_t  data[MCP2515_MAX_DATA_LENGTH];
} MCP2515_Frame_t;

typedef struct {
    GPIO_TypeDef  *cs_port;         /*!< Chip-select port; the bus itself is shared */
    uint16_t       cs_pin;
    uint32_t       oscillator_hz;   /*!< Crystal on the module, not the MCU clock */
    uint32_t       baud_rate;
    MCP2515_Mode_t mode;            /*!< Mode to leave the device in after init */
} MCP2515_Config_t;

typedef struct {
    SPI_Device_t  device;       /*!< Bus settings travel with the device */
    GPIO_TypeDef *cs_port;
    uint16_t      cs_pin;
    bool          initialized;
} MCP2515_Handle_t;

#ifdef __cplusplus
}
#endif

#endif /* MCP2515_TYPES_H */
