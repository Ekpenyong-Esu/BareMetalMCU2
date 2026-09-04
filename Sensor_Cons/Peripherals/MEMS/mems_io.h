/**
 ******************************************************************************
 * @file    mems_io.h
 * @brief   Register access over SPI for the L3GD20
 ******************************************************************************
 */

#ifndef MEMS_IO_H
#define MEMS_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mems_types.h"

/**
 * @note These run before MEMS_Init() completes, so they only require a SPI
 *       handle - not a fully initialised driver.
 */
MEMS_StatusTypeDef MEMS_ReadRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr, uint8_t *data,
                                      uint8_t length);
MEMS_StatusTypeDef MEMS_WriteRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                       const uint8_t *data, uint8_t length);
MEMS_StatusTypeDef MEMS_ReadRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t *data);
MEMS_StatusTypeDef MEMS_WriteRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t data);

/**
 * @brief Read-modify-write the bits selected by @p mask.
 */
MEMS_StatusTypeDef MEMS_UpdateRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t mask,
                                       uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* MEMS_IO_H */
