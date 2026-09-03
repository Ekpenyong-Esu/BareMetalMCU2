/**
 * @file    ili9341_types.h
 * @brief   Shared types and settings for ILI9341 display
 * @details ILI9341 is a small color screen (240x320 pixels). This file holds
 *          screen size, pin settings, and command codes for the display.
 *          Each command code tells the screen to do one thing, like wake up
 *          or draw pixels.
 */

#ifndef ILI9341_TYPES_H
#define ILI9341_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"

#include <stdint.h>

/* Screen size -------------------------------------------------------------*/

#define ILI9341_LCD_PIXEL_WIDTH   ((uint16_t)240) /* Screen width in pixels */
#define ILI9341_LCD_PIXEL_HEIGHT  ((uint16_t)320) /* Screen height in pixels */

/* Pins to talk to the screen ----------------------------------------------*/

#ifndef ILI9341_WRX_PIN
#define ILI9341_WRX_PIN           GPIO_PIN_13 /* Data/command pin */
#define ILI9341_WRX_PORT          GPIOD       /* Port for data/command pin */
#endif

#ifndef ILI9341_CS_PIN
#define ILI9341_CS_PIN            GPIO_PIN_2  /* Chip select pin */
#define ILI9341_CS_PORT           GPIOC       /* Port for chip select */
#endif

/* Commands sent to the screen - each tells the screen to do one job ------*/

#define ILI9341_SWRESET           0x01U /* Soft reset - restart the screen */
#define ILI9341_SLEEP_IN          0x10U /* Go to sleep - save power */
#define ILI9341_SLEEP_OUT         0x11U /* Wake up from sleep */
#define ILI9341_DISPLAY_ON        0x29U /* Turn display on */
#define ILI9341_DISPLAY_OFF       0x28U /* Turn display off */
#define ILI9341_COLUMN_ADDR       0x2AU /* Set left and right edge to draw */
#define ILI9341_PAGE_ADDR         0x2BU /* Set top and bottom edge to draw */
#define ILI9341_GRAM              0x2CU /* Write pixel colors */
#define ILI9341_MAC               0x36U /* Set screen rotation and flip */
#define ILI9341_PIXEL_FORMAT      0x3AU /* Set color depth (16-bit etc) */
#define ILI9341_GAMMA             0x26U /* Gamma setting */
#define ILI9341_PGAMMA            0xE0U /* Bright color correction */
#define ILI9341_NGAMMA            0xE1U /* Dark color correction */
#define ILI9341_POWERA            0xCBU /* Power setting A */
#define ILI9341_POWERB            0xCFU /* Power setting B */
#define ILI9341_DTCA              0xE8U /* Timing setting A */
#define ILI9341_DTCB              0xEAU /* Timing setting B */
#define ILI9341_POWER_SEQ         0xEDU /* Power on order */
#define ILI9341_3GAMMA_EN         0xF2U /* Enable 3 gamma */
#define ILI9341_INTERFACE         0xF6U /* Interface setting */
#define ILI9341_PRC               0xF7U /* Pump ratio control */
#define ILI9341_FRC               0xB1U /* Frame rate control */
#define ILI9341_DFC               0xB6U /* Display function control */
#define ILI9341_RGB_INTERFACE     0xB0U /* RGB interface setting */
#define ILI9341_POWER1            0xC0U /* Power control 1 */
#define ILI9341_POWER2            0xC1U /* Power control 2 */
#define ILI9341_VCOM1             0xC5U /* Voltage for colors 1 */
#define ILI9341_VCOM2             0xC7U /* Voltage for colors 2 */
#define ILI9341_POWER_ON_SEQUENCE 0xCAU /* Power on steps */

/* Other helpers -----------------------------------------------------------*/

#define ILI9341_READ_ID4          0xD3U   /* Read screen ID */
#define ILI9341_READ_ID4_SIZE     3U      /* ID is 3 bytes long */
#define ILI9341_WORD_MASK         0xFFFFU /* Keeps only 16 bits */
#define ILI9341_INIT_DELAY_MS     200U    /* Wait after init (ms) */
#define ILI9341_WAKE_DELAY_MS     200U    /* Wait after wake up (ms) */
#define ILI9341_SLEEP_DELAY_MS    5U      /* Wait before sleep (ms) */
#define ILI9341_COL_END           0xEFU   /* Last column (239) */
#define ILI9341_PAGE_END          0x3FU   /* Last page helper value */

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_TYPES_H */
