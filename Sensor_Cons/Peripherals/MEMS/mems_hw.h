/**
  ******************************************************************************
  * @file    mems_hw.h
  * @brief   Board wiring for the on-board L3GD20
  ******************************************************************************
  */

#ifndef MEMS_HW_H
#define MEMS_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

/* On-board wiring: L3GD20 on SPI5, chip select driven in software. */
#define MEMS_CS_PIN                     GPIO_PIN_1
#define MEMS_CS_GPIO_PORT               GPIOC
#define MEMS_INT1_PIN                   GPIO_PIN_1
#define MEMS_INT1_GPIO_PORT             GPIOA
#define MEMS_INT2_PIN                   GPIO_PIN_2
#define MEMS_INT2_GPIO_PORT             GPIOA

#define MEMS_SPI                        SPI5
#define MEMS_SPI_CLK_ENABLE()           __HAL_RCC_SPI5_CLK_ENABLE()
#define MEMS_SPI_GPIO_PORT              GPIOF
#define MEMS_SPI_SCK_PIN                GPIO_PIN_7
#define MEMS_SPI_MISO_PIN               GPIO_PIN_8
#define MEMS_SPI_MOSI_PIN               GPIO_PIN_9

/**
 * @brief Configure chip select, the SPI5 pins and the two interrupt inputs.
 */
MEMS_StatusTypeDef MEMS_HW_InitGPIO(MEMS_HandleTypeDef *hmems);

/**
 * @brief Override the chip-select pin before MEMS_Init for an off-board device.
 */
void MEMS_SetCS(MEMS_HandleTypeDef *hmems, GPIO_TypeDef *csPort, uint16_t csPin);

void MEMS_CS_High(MEMS_HandleTypeDef *hmems);
void MEMS_CS_Low(MEMS_HandleTypeDef *hmems);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_HW_H */
