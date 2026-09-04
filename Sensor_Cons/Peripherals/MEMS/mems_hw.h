/**
 ******************************************************************************
 * @file    mems_hw.h
 * @brief   Chip select and interrupt inputs of one L3GD20
 ******************************************************************************
 */

#ifndef MEMS_HW_H
#define MEMS_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

/**
 * @brief Configure the chip-select output and whichever INT inputs are wired.
 * @note  Pins come from the handle, filled in by MEMS_Init() from the config;
 *        the SCK/MISO/MOSI lines belong to the bus and are not touched here.
 */
MEMS_StatusTypeDef MEMS_HW_InitGPIO(MEMS_HandleTypeDef *hmems);

void MEMS_CS_High(MEMS_HandleTypeDef *hmems);
void MEMS_CS_Low(MEMS_HandleTypeDef *hmems);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_HW_H */
