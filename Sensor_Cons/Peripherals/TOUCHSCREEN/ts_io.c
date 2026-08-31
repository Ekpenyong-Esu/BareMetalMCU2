/**
 * @file ts_io.c
 * @brief STMPE811 register access over I2C
 */

#include "ts_io.h"
#include "ts_stmpe811.h"
#include "i2c.h"

#define TS_BYTE_MASK    0xFFU

/* The STMPE811 sits at a fixed address, so one device record serves the whole
   transport layer. */
static I2C_Device_t s_device;

void TS_IO_BusInit(void)
{
    const I2C_ConfigTypeDef config = I2C_ConfigDefault();

    I2C_DeviceInit(&s_device, STMPE811_I2C_ADDRESS, &config);
}

TS_StatusTypeDef TS_IO_IsDeviceReady(uint32_t trials, uint32_t timeout)
{
    if (I2C_IsDeviceReady(&s_device, trials, timeout) != I2C_OK) {
        return TS_DEVICE_NOT_FOUND;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_ReadRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data)
{
    if (hts == NULL || data == NULL) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Read(&s_device, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_ReadRegisterMulti(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data, uint16_t size)
{
    if (hts == NULL || data == NULL || size == 0) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Read(&s_device, reg, I2C_MEMADD_SIZE_8BIT, data, size, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_WriteRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t data)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    if (I2C_Mem_Write(&s_device, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_ReadRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t *data)
{
    if (hts == NULL || data == NULL) {
        return TS_INVALID_PARAM;
    }

    uint8_t buffer[2];
    if (I2C_Mem_Read(&s_device, reg, I2C_MEMADD_SIZE_8BIT, buffer, 2, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    *data = (uint16_t)(((uint16_t)buffer[0] << 8) | buffer[1]);

    return TS_OK;
}

TS_StatusTypeDef TS_WriteRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t data)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    uint8_t buffer[2] = {
        (uint8_t)(data >> 8),
        (uint8_t)(data & TS_BYTE_MASK),
    };

    if (I2C_Mem_Write(&s_device, reg, I2C_MEMADD_SIZE_8BIT, buffer, 2, TS_TIMEOUT) != I2C_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

void TS_ResetFifo(TS_HandleTypeDef *hts)
{
    TS_WriteRegister(hts, STMPE811_REG_FIFO_STA, STMPE811_FIFO_RESET);
    TS_WriteRegister(hts, STMPE811_REG_FIFO_STA, STMPE811_FIFO_OPERATIONAL);
}
