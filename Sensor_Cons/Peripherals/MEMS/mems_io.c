/**
  ******************************************************************************
  * @file    mems_io.c
  * @brief   Register access over SPI for the L3GD20
  ******************************************************************************
  */

#include "mems_io.h"
#include "mems_hw.h"
#include "mems_l3gd20.h"

/**
 * @brief Frame one transfer: assert CS, send the command byte, move the payload.
 * @note  CS is released on every path so a failed transfer cannot leave the bus
 *        selected.
 */
static MEMS_StatusTypeDef MEMS_Transfer(MEMS_HandleTypeDef *hmems, uint8_t command,
                                        uint8_t *rxData, const uint8_t *txData,
                                        uint8_t length)
{
    HAL_StatusTypeDef hal_status;

    MEMS_CS_Low(hmems);

    hal_status = HAL_SPI_Transmit(hmems->hspi, &command, 1, MEMS_SPI_TIMEOUT);
    if (hal_status == HAL_OK) {
        if (rxData != NULL) {
            hal_status = HAL_SPI_Receive(hmems->hspi, rxData, length, MEMS_SPI_TIMEOUT);
        } else {
            hal_status = HAL_SPI_Transmit(hmems->hspi, (uint8_t *)txData, length, MEMS_SPI_TIMEOUT);
        }
    }

    MEMS_CS_High(hmems);

    return (hal_status == HAL_OK) ? MEMS_OK : MEMS_COMMUNICATION_ERROR;
}

static uint8_t MEMS_CommandByte(uint8_t addr, uint8_t length, bool read)
{
    uint8_t command = addr;

    if (read) {
        command |= L3GD20_READ_CMD;
    }
    if (length > 1U) {
        command |= L3GD20_MULTIPLEBYTE_CMD;
    }

    return command;
}

MEMS_StatusTypeDef MEMS_ReadRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                      uint8_t *data, uint8_t length)
{
    if (hmems == NULL || data == NULL || length == 0U) {
        return MEMS_INVALID_PARAM;
    }
    if (hmems->hspi == NULL) {
        return MEMS_NOT_INITIALIZED;
    }

    return MEMS_Transfer(hmems, MEMS_CommandByte(start_addr, length, true), data, NULL, length);
}

MEMS_StatusTypeDef MEMS_WriteRegisters(MEMS_HandleTypeDef *hmems, uint8_t start_addr,
                                       const uint8_t *data, uint8_t length)
{
    if (hmems == NULL || data == NULL || length == 0U) {
        return MEMS_INVALID_PARAM;
    }
    if (hmems->hspi == NULL) {
        return MEMS_NOT_INITIALIZED;
    }

    return MEMS_Transfer(hmems, MEMS_CommandByte(start_addr, length, false), NULL, data, length);
}

MEMS_StatusTypeDef MEMS_ReadRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t *data)
{
    return MEMS_ReadRegisters(hmems, addr, data, 1U);
}

MEMS_StatusTypeDef MEMS_WriteRegister(MEMS_HandleTypeDef *hmems, uint8_t addr, uint8_t data)
{
    return MEMS_WriteRegisters(hmems, addr, &data, 1U);
}

MEMS_StatusTypeDef MEMS_UpdateRegister(MEMS_HandleTypeDef *hmems, uint8_t addr,
                                       uint8_t mask, uint8_t value)
{
    MEMS_StatusTypeDef status;
    uint8_t reg = 0U;

    status = MEMS_ReadRegister(hmems, addr, &reg);
    if (status != MEMS_OK) {
        return status;
    }

    reg = (uint8_t)((reg & (uint8_t)~mask) | (value & mask));

    return MEMS_WriteRegister(hmems, addr, reg);
}
