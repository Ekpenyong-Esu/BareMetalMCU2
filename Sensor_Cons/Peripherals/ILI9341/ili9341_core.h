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
 * @brief Bring up the panel on the bus and control lines the caller names
 * @param hili   Caller-owned handle
 * @param config Bus (already opened with SPI_BusInit) and CS/DC/RST pins
 * @return bool False if a required pointer is NULL, the panel could not claim
 *         its slot on the bus, or a step of the init sequence failed, which
 *         leaves the panel in an undefined state.
 */
bool ILI9341_Init(ILI9341_Handle_t *hili, const ILI9341_Config_t *config);

/**
 * @brief Return the control lines to idle and forget the panel
 * @note  Leaves the bus open; it belongs to the application.
 */
void ILI9341_DeInit(ILI9341_Handle_t *hili);

/** @brief Read the controller identifier; zero if the handle is not initialized. */
uint16_t ILI9341_ReadID(ILI9341_Handle_t *hili);

/**
 * @brief Enable panel output
 * @return bool False if the command could not be sent
 */
bool ILI9341_DisplayOn(ILI9341_Handle_t *hili);

/**
 * @brief Disable panel output
 * @return bool False if the command could not be sent
 */
bool ILI9341_DisplayOff(ILI9341_Handle_t *hili);

/**
 * @brief Put the panel into sleep mode
 * @return bool False if the command could not be sent
 */
bool ILI9341_SleepIn(ILI9341_Handle_t *hili);

/**
 * @brief Wake the panel from sleep mode
 * @return bool False if the command could not be sent
 */
bool ILI9341_SleepOut(ILI9341_Handle_t *hili);

/** @brief Return the panel width in pixels. */
uint16_t ILI9341_GetLcdPixelWidth(void);

/** @brief Return the panel height in pixels. */
uint16_t ILI9341_GetLcdPixelHeight(void);

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_CORE_H */
