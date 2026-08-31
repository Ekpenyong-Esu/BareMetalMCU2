/**
 * @file    ili9341_core.h
 * @brief   ILI9341 panel lifecycle and display controls
 */

#ifndef ILI9341_CORE_H
#define ILI9341_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "ili9341_types.h"


/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize the board transport and configure the panel
 * @return bool False if the transport or any step of the init sequence failed,
 *         which leaves the panel in an undefined state.
 */
bool ili9341_Init(void);

/** @brief Read the controller identifier. */
uint16_t ili9341_ReadID(void);

/**
 * @brief Enable panel output
 * @return bool False if the command could not be sent
 */
bool ili9341_DisplayOn(void);

/**
 * @brief Disable panel output
 * @return bool False if the command could not be sent
 */
bool ili9341_DisplayOff(void);

/**
 * @brief Put the panel into sleep mode
 * @return bool False if the command could not be sent
 */
bool ili9341_SleepIn(void);

/**
 * @brief Wake the panel from sleep mode
 * @return bool False if the command could not be sent
 */
bool ili9341_SleepOut(void);

/** @brief Return the panel width in pixels. */
uint16_t ili9341_GetLcdPixelWidth(void);

/** @brief Return the panel height in pixels. */
uint16_t ili9341_GetLcdPixelHeight(void);

/**
 * @brief Configure the board pins and SPI transport for the panel
 * @return bool False if the panel could not claim its slot on the bus
 */
bool ILI9341_MspInit(void);

/** @brief Return panel-specific board resources to their idle state. */
void ILI9341_MspDeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_CORE_H */
