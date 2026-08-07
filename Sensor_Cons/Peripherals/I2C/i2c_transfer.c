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

    I2C_RecoverOnError();

    return I2C_ConvertHALStatus(halStatus);
}

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

void I2C_RecoverOnError(void)
{
    I2C_DeInit();
    I2C_BusRecovery();

    if (I2C_Reinit() != I2C_OK) {
        log_error("I2C: bus recovery failed to re-initialize I2C3");
    }
}

I2C_StatusTypeDef I2C_Master_Transmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    halStatus = HAL_I2C_Master_Transmit(I2C_GetHandle(), DevAddress, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Master_Receive(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    halStatus = HAL_I2C_Master_Receive(I2C_GetHandle(), DevAddress, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Master_TransmitReceive(uint16_t DevAddress,
                                             uint8_t *pTxData, uint16_t TxSize,
                                             uint8_t *pRxData, uint16_t RxSize,
                                             uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pTxData == NULL || pRxData == NULL || TxSize == 0 || RxSize == 0) {
        return I2C_INVALID_PARAM;
    }

    halStatus = HAL_I2C_Master_Transmit(I2C_GetHandle(), DevAddress, pTxData, TxSize, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    halStatus = HAL_I2C_Master_Receive(I2C_GetHandle(), DevAddress, pRxData, RxSize, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Mem_Write(uint16_t DevAddress, uint16_t MemAddress,
                               uint16_t MemAddSize, uint8_t *pData,
                               uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    halStatus = HAL_I2C_Mem_Write(I2C_GetHandle(), DevAddress, MemAddress,
                                  MemAddSize, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Mem_Read(uint16_t DevAddress, uint16_t MemAddress,
                              uint16_t MemAddSize, uint8_t *pData,
                              uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus;

    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    halStatus = HAL_I2C_Mem_Read(I2C_GetHandle(), DevAddress, MemAddress,
                                 MemAddSize, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_IsDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
    HAL_StatusTypeDef halStatus = HAL_I2C_IsDeviceReady(I2C_GetHandle(), DevAddress,
                                                        Trials, Timeout);

    if (halStatus != HAL_OK &&
        (HAL_I2C_GetError(I2C_GetHandle()) & HAL_I2C_ERROR_AF) != 0U) {
        return I2C_NACK;
    }

    return I2C_ConvertHALStatus(halStatus);
}

uint8_t I2C_ScanBus(uint8_t *pDevices, uint8_t MaxDevices, uint32_t Timeout)
{
    uint8_t deviceCount = 0;

    if (pDevices == NULL || MaxDevices == 0) {
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
