/**
  ******************************************************************************
  * @file    qspi_hw.h
  * @brief   Board wiring and SPI transport bring-up for the serial NOR flash
  ******************************************************************************
  */

#ifndef QSPI_HW_H
#define QSPI_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

/* SPI1 on GPIOB, chip select driven in software. */
#define QSPI_SCK_PIN                    GPIO_PIN_3    /* PB3 - SPI1_SCK */
#define QSPI_MISO_PIN                   GPIO_PIN_4    /* PB4 - SPI1_MISO */
#define QSPI_MOSI_PIN                   GPIO_PIN_5    /* PB5 - SPI1_MOSI */
#define QSPI_NCS_PIN                    GPIO_PIN_6    /* PB6 - chip select */
#define QSPI_GPIO_PORT                  GPIOB

QSPI_StatusTypeDef QSPI_HW_InitGPIO(void);
QSPI_StatusTypeDef QSPI_HW_InitSPI(QSPI_HandleStructTypeDef *hqspi);
void QSPI_HW_DeInitSPI(QSPI_HandleStructTypeDef *hqspi);

/**
 * @brief Assert (true) or release (false) the flash chip select.
 */
void QSPI_ChipSelect(bool select);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_HW_H */
