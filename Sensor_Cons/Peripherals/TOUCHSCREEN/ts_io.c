/**
 * @file ts_io.c
 * @brief STMPE811 register access over I2C
 */

#include "ts_io.h"
#include "ts_stmpe811.h"
#include "i2c.h"

#define TS_BYTE_MASK 0xFFU

TS_StatusTypeDef TS_IO_DeviceInit(TS_HandleTypeDef *hts, I2C_Bus_t *bus, uint8_t address) {
    if (hts == NULL || bus == NULL) {
        return TS_INVALID_PARAM;
    }

    const I2C_ConfigTypeDef config = I2C_ConfigDefault();

    /* The only way this fails is a bus the application never opened. */
    if (I2C_DeviceInit(&hts->Device, bus, address, &config) != I2C_OK) {
        return TS_INVALID_PARAM;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_IO_IsDeviceReady(TS_HandleTypeDef *hts, uint32_t trials, uint32_t timeout) {
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    if (I2C_IsDeviceReady(&hts->Device, trials, timeout) != I2C_OK) {
        return TS_DEVICE_NOT_FOUND;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_ReadRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data) {
    if (hts == NULL || data == NULL) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Read(&hts->Device, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_ReadRegisterMulti(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data,
                                      uint16_t size) {
    if (hts == NULL || data == NULL || size == 0) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Read(&hts->Device, reg, I2C_MEMADD_SIZE_8BIT, data, size, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_WriteRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t data) {
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Write(&hts->Device, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_ReadRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t *data) {
    if (hts == NULL || data == NULL) {
        return TS_INVALID_PARAM;
    }

    uint8_t buffer[2];
    if (I2C_Mem_Read(&hts->Device, reg, I2C_MEMADD_SIZE_8BIT, buffer, 2, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    *data = (uint16_t)(((uint16_t)buffer[0] << 8) | buffer[1]);

    return TS_OK;
}

TS_StatusTypeDef TS_WriteRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t data) {
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    uint8_t buffer[2] = {
        (uint8_t)(data >> 8),
        (uint8_t)(data & TS_BYTE_MASK),
    };

    if (I2C_Mem_Write(&hts->Device, reg, I2C_MEMADD_SIZE_8BIT, buffer, 2, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

void TS_ResetFifo(TS_HandleTypeDef *hts) {
    TS_WriteRegister(hts, STMPE811_REG_FIFO_STA, STMPE811_FIFO_RESET);
    TS_WriteRegister(hts, STMPE811_REG_FIFO_STA, STMPE811_FIFO_OPERATIONAL);
}
