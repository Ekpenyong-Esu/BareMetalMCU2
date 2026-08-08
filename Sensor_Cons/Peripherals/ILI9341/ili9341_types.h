/**
 * @file    ili9341_types.h
 * @brief   Shared ILI9341 constants and board defaults
 */

#ifndef ILI9341_TYPES_H
#define ILI9341_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"

#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

#define ILI9341_LCD_PIXEL_WIDTH   ((uint16_t)240)
#define ILI9341_LCD_PIXEL_HEIGHT  ((uint16_t)320)

#ifndef ILI9341_WRX_PIN
#define ILI9341_WRX_PIN           GPIO_PIN_13
#define ILI9341_WRX_PORT          GPIOD
#endif

#ifndef ILI9341_CS_PIN
#define ILI9341_CS_PIN            GPIO_PIN_2
#define ILI9341_CS_PORT           GPIOC
#endif

#define ILI9341_SWRESET           0x01U
#define ILI9341_SLEEP_IN          0x10U
#define ILI9341_SLEEP_OUT         0x11U
#define ILI9341_DISPLAY_ON        0x29U
#define ILI9341_DISPLAY_OFF       0x28U
#define ILI9341_COLUMN_ADDR       0x2AU
#define ILI9341_PAGE_ADDR         0x2BU
#define ILI9341_GRAM              0x2CU
#define ILI9341_MAC               0x36U
#define ILI9341_PIXEL_FORMAT      0x3AU
#define ILI9341_GAMMA             0x26U
#define ILI9341_PGAMMA            0xE0U
#define ILI9341_NGAMMA            0xE1U
#define ILI9341_POWERA            0xCBU
#define ILI9341_POWERB            0xCFU
#define ILI9341_DTCA              0xE8U
#define ILI9341_DTCB              0xEAU
#define ILI9341_POWER_SEQ         0xEDU
#define ILI9341_3GAMMA_EN         0xF2U
#define ILI9341_INTERFACE         0xF6U
#define ILI9341_PRC               0xF7U
#define ILI9341_FRC               0xB1U
#define ILI9341_DFC               0xB6U
#define ILI9341_RGB_INTERFACE     0xB0U
#define ILI9341_POWER1            0xC0U
#define ILI9341_POWER2            0xC1U
#define ILI9341_VCOM1             0xC5U
#define ILI9341_VCOM2             0xC7U
#define ILI9341_POWER_ON_SEQUENCE 0xCAU

#define ILI9341_READ_ID4          0xD3U
#define ILI9341_READ_ID4_SIZE     3U
#define ILI9341_WORD_MASK         0xFFFFU
#define ILI9341_INIT_DELAY_MS     200U
#define ILI9341_WAKE_DELAY_MS     200U
#define ILI9341_SLEEP_DELAY_MS    5U
#define ILI9341_COL_END           0xEFU
#define ILI9341_PAGE_END          0x3FU

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_TYPES_H */
