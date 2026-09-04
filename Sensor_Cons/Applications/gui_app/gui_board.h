/**
 * @file    gui_board.h
 * @brief   Board wiring of the GUI application on the STM32F429I-DISC1
 * @details This is the only file in the application that knows which pins,
 *          buses and memory the display stack sits on. gui_board.c owns the
 *          bus objects and driver handles built from these choices; the LVGL
 *          ports and the low-power manager reach them through the accessors
 *          below rather than through driver singletons.
 *
 * Wiring (everything is on the board, nothing to connect):
 *
 *   ILI9341 panel, command path   SPI5: PF7 SCK, PF8 MISO, PF9 MOSI
 *                                 PC2 NCS, PD13 WRX (data/command)
 *                                 RST is tied to NRST, not software controlled
 *   ILI9341 panel, pixel path     LTDC RGB lines (HAL_LTDC_MspInit owns them)
 *                                 PK3 backlight enable
 *   STMPE811 touch controller     I2C3: PA8 SCL, PC9 SDA
 *                                 INT on PA15, serviced by EXTI15_10
 *   IS42S16400J SDRAM             FMC bank 2 at 0xD0000000, holds the framebuffer
 */

#ifndef GUI_BOARD_H
#define GUI_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "fmc_types.h"
#include "i2c_types.h"
#include "ili9341_types.h"
#include "ltdc_types.h"
#include "ts_types.h"
#include <stdbool.h>

/* ILI9341 command path ----------------------------------------------------*/

#define GUI_BOARD_LCD_SPI SPI5
#define GUI_BOARD_LCD_SCK_PORT GPIOF
#define GUI_BOARD_LCD_SCK_PIN GPIO_PIN_7
#define GUI_BOARD_LCD_MISO_PORT GPIOF
#define GUI_BOARD_LCD_MISO_PIN GPIO_PIN_8
#define GUI_BOARD_LCD_MOSI_PORT GPIOF
#define GUI_BOARD_LCD_MOSI_PIN GPIO_PIN_9
/* The gyroscope shares this bus; pull-downs keep the lines quiet while idle. */
#define GUI_BOARD_LCD_SPI_PULL GPIO_PULLDOWN

#define GUI_BOARD_LCD_CS_PORT GPIOC
#define GUI_BOARD_LCD_CS_PIN GPIO_PIN_2
#define GUI_BOARD_LCD_DC_PORT GPIOD
#define GUI_BOARD_LCD_DC_PIN GPIO_PIN_13
#define GUI_BOARD_LCD_RST_PORT NULL
#define GUI_BOARD_LCD_RST_PIN 0U

/* Backlight sits on GPIOK, which is outside the GPIO driver's A-I range. */
#define GUI_BOARD_LCD_BL_PORT GPIOK
#define GUI_BOARD_LCD_BL_PIN GPIO_PIN_3

/* STMPE811 touch controller -----------------------------------------------*/

#define GUI_BOARD_TOUCH_I2C I2C3
#define GUI_BOARD_TOUCH_SCL_PORT GPIOA
#define GUI_BOARD_TOUCH_SCL_PIN GPIO_PIN_8
#define GUI_BOARD_TOUCH_SDA_PORT GPIOC
#define GUI_BOARD_TOUCH_SDA_PIN GPIO_PIN_9
#define GUI_BOARD_TOUCH_INT_PORT GPIOA
#define GUI_BOARD_TOUCH_INT_PIN GPIO_PIN_15
/* Pin 15 lands on the shared EXTI15_10 vector in Core/Src/stm32f4xx_it.c. */
#define GUI_BOARD_TOUCH_INT_IRQn EXTI15_10_IRQn

/* Framebuffer ---------------------------------------------------------------*/

/* Plain ints, not unsigned: the LVGL port compares them with int32 areas. */
#define GUI_BOARD_DISPLAY_WIDTH 240
#define GUI_BOARD_DISPLAY_HEIGHT 320
#define GUI_BOARD_PIXEL_FORMAT LTDC_PIXEL_FORMAT_RGB565_ENUM
#define GUI_BOARD_BYTES_PER_PIXEL LTDC_BYTES_PER_PIXEL_RGB565
#define GUI_BOARD_SDRAM_BANK FMC_SDRAM_BANK2
#define GUI_BOARD_FRAMEBUFFER_ADDR SDRAM_DEVICE_ADDR
#define GUI_BOARD_FRAMEBUFFER_SIZE                                                                 \
    ((uint32_t)GUI_BOARD_DISPLAY_WIDTH * GUI_BOARD_DISPLAY_HEIGHT * GUI_BOARD_BYTES_PER_PIXEL)

/* Bring-up ------------------------------------------------------------------*/

/**
 * @brief  Bring up SDRAM, the panel, the LTDC and the touch controller, in that order
 * @retval false if the display path could not be started. A touch failure is
 *         logged but does not block the GUI; the touch handle then reports
 *         itself uninitialised.
 * @note   Runs once; later calls return the first result.
 */
bool GUI_Board_Init(void);

/**
 * @brief  Run the SDRAM initialisation sequence on FMC bank 2
 * @note   Also used by the low-power manager after it has cut FMC power.
 */
bool GUI_Board_SdramInit(void);

/**
 * @brief  Reopen SPI5, re-run the ILI9341 init sequence and reprogram the LTDC
 * @note   Used after a deep sleep, when the panel and controller registers
 *         are gone. The framebuffer in SDRAM must already be reachable.
 */
bool GUI_Board_DisplayInit(void);

/**
 * @brief  (Re)open I2C3 for the touch controller
 * @note   The STMPE811 keeps its configuration while the bus is closed, so
 *         reopening the bus is all the low-power manager needs to do.
 */
bool GUI_Board_TouchBusOpen(void);

/* Accessors -----------------------------------------------------------------*/

TS_HandleTypeDef *GUI_Board_Touch(void);
I2C_Bus_t *GUI_Board_TouchBus(void);
ILI9341_Handle_t *GUI_Board_Panel(void);
LTDC_HandleTypeDef *GUI_Board_Ltdc(void);
LTDC_Driver_t *GUI_Board_LtdcDriver(void);

#ifdef __cplusplus
}
#endif

#endif /* GUI_BOARD_H */
