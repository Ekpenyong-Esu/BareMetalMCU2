/**
  ******************************************************************************
  * @file    i2c_transfer.c
  * @brief   Master and memory transfer operations
  ******************************************************************************
  */

#include "i2c_transfer.h"
#include "i2c_core.h"
#include "gpio.h"
#include "main.h"
#include "log.h"

/** Enough edges for a slave holding SDA to finish the byte it is clocking out */
#define I2C_RECOVERY_CLOCK_PULSES   9U

static void I2C_BusRecovery(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = I2C3_SCL_Pin;
    GPIO_Driver_Pin_Init(I2C3_SCL_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = I2C3_SDA_Pin;
    GPIO_Driver_Pin_Init(I2C3_SDA_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_SET);
    HAL_Delay(1);

    if (HAL_GPIO_ReadPin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin) == GPIO_PIN_RESET) {
        for (uint8_t i = 0; i < I2C_RECOVERY_CLOCK_PULSES; i++) {
            HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_RESET);
            HAL_Delay(1);
            HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET);
            HAL_Delay(1);
        }
    }

    /* STOP: SDA low -> SCL high -> SDA high */
    HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(I2C3_SCL_GPIO_Port, I2C3_SCL_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(I2C3_SDA_GPIO_Port, I2C3_SDA_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
}

/**
 * @brief Turn a failed transfer into a status, recovering the bus if needed.
 * @note  An absent device answers with a NACK; that is an expected reply, not a
 *        stuck bus, so it must not trigger the recovery sequence.
 */
static I2C_StatusTypeDef I2C_HandleFailure(HAL_StatusTypeDef halStatus)
{
    if ((HAL_I2C_GetError(I2C_GetHandle()) & HAL_I2C_ERROR_AF) != 0U) {
        return I2C_NACK;
    }

    /* Releasing the bus drops its owner, so the next transfer reprograms it
       for whichever device asks first. */
    if (I2C_BusDeInit() != I2C_OK) {
        log_error("I2C: error recovery failed to release the bus");
    }

    I2C_BusRecovery();

    return I2C_ConvertHALStatus(halStatus);
}

I2C_StatusTypeDef I2C_Master_Transmit(I2C_Device_t *device, uint8_t *pData,
                                      uint16_t Size, uint32_t Timeout)
{
    I2C_StatusTypeDef status = I2C_OK;
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    halStatus = HAL_I2C_Master_Transmit(I2C_GetHandle(), device->address,
                                        pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Master_Receive(I2C_Device_t *device, uint8_t *pData,
                                     uint16_t Size, uint32_t Timeout)
{
    I2C_StatusTypeDef status = I2C_OK;
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    halStatus = HAL_I2C_Master_Receive(I2C_GetHandle(), device->address,
                                       pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Master_TransmitReceive(I2C_Device_t *device,
                                             uint8_t *pTxData, uint16_t TxSize,
                                             uint8_t *pRxData, uint16_t RxSize,
                                             uint32_t Timeout)
{
    I2C_StatusTypeDef status = I2C_OK;
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (pTxData == NULL || pRxData == NULL || TxSize == 0 || RxSize == 0) {
        return I2C_INVALID_PARAM;
    }

    status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    halStatus = HAL_I2C_Master_Transmit(I2C_GetHandle(), device->address,
                                        pTxData, TxSize, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    halStatus = HAL_I2C_Master_Receive(I2C_GetHandle(), device->address,
                                       pRxData, RxSize, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Mem_Write(I2C_Device_t *device, uint16_t MemAddress,
                                uint16_t MemAddSize, uint8_t *pData,
                                uint16_t Size, uint32_t Timeout)
{
    I2C_StatusTypeDef status = I2C_OK;
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    halStatus = HAL_I2C_Mem_Write(I2C_GetHandle(), device->address, MemAddress,
                                  MemAddSize, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Mem_Read(I2C_Device_t *device, uint16_t MemAddress,
                               uint16_t MemAddSize, uint8_t *pData,
                               uint16_t Size, uint32_t Timeout)
{
    I2C_StatusTypeDef status = I2C_OK;
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    halStatus = HAL_I2C_Mem_Read(I2C_GetHandle(), device->address, MemAddress,
                                 MemAddSize, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_IsDeviceReady(I2C_Device_t *device, uint32_t Trials,
                                    uint32_t Timeout)
{
    I2C_StatusTypeDef status = I2C_Select(device);
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (status != I2C_OK) {
        return status;
    }

    halStatus = HAL_I2C_IsDeviceReady(I2C_GetHandle(), device->address,
                                      Trials, Timeout);
    if (halStatus != HAL_OK &&
        (HAL_I2C_GetError(I2C_GetHandle()) & HAL_I2C_ERROR_AF) != 0U) {
        return I2C_NACK;
    }

    return I2C_ConvertHALStatus(halStatus);
}

uint8_t I2C_ScanBus(I2C_Device_t *device, uint8_t *pDevices, uint8_t MaxDevices,
                    uint32_t Timeout)
{
    uint8_t deviceCount = 0;

    if (pDevices == NULL || MaxDevices == 0) {
        return 0;
    }

    if (I2C_Select(device) != I2C_OK) {
        return 0;
    }

    for (uint8_t addr = I2C_ADDR_MIN; addr <= I2C_ADDR_MAX && deviceCount < MaxDevices; addr++) {
        uint16_t devAddr = (uint16_t)addr << 1;

        if (HAL_I2C_IsDeviceReady(I2C_GetHandle(), devAddr, 1, Timeout) == HAL_OK) {
            pDevices[deviceCount++] = addr;
        }
    }

    return deviceCount;
}

uint32_t I2C_GetError(void)
{
    return HAL_I2C_GetError(I2C_GetHandle());
}

const char *I2C_GetStatusString(I2C_StatusTypeDef status)
{
    switch (status) {
        case I2C_OK:           return "Operation completed successfully";
        case I2C_ERROR:        return "General error occurred";
        case I2C_BUSY:         return "I2C bus is busy";
        case I2C_TIMEOUT:      return "Operation timed out";
        case I2C_NACK:         return "No acknowledge received";
        case I2C_INVALID_PARAM: return "Invalid parameter provided";
        default:               return "Unknown status";
    }
}
