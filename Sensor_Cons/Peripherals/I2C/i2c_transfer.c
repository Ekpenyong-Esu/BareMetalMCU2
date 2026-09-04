/**
 ******************************************************************************
 * @file    i2c_transfer.c
 * @brief   Master and memory transfer operations
 ******************************************************************************
 */

#include "i2c_transfer.h"
#include "i2c_core.h"
#include "gpio.h"
#include "log.h"

/** Enough edges for a slave holding SDA to finish the byte it is clocking out */
#define I2C_RECOVERY_CLOCK_PULSES 9U

/**
 * @brief Bit-bang the bus free, then hand the pins back to the peripheral
 * @note  Drives the wires as plain open-drain outputs, so it works whatever
 *        state the peripheral was left in.
 */
static void I2C_BusRecovery(I2C_Bus_t *bus) {
    GPIO_TypeDef *sclPort = bus->config.sclPort;
    GPIO_TypeDef *sdaPort = bus->config.sdaPort;
    uint16_t sclPin = bus->config.sclPin;
    uint16_t sdaPin = bus->config.sdaPin;
    GPIO_InitTypeDef init = {0};

    init.Mode = GPIO_MODE_OUTPUT_OD;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_LOW;

    init.Pin = sclPin;
    GPIO_Driver_Pin_Init(sclPort, &init);

    init.Pin = sdaPin;
    GPIO_Driver_Pin_Init(sdaPort, &init);

    HAL_GPIO_WritePin(sclPort, sclPin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(sdaPort, sdaPin, GPIO_PIN_SET);
    HAL_Delay(1);

    if (HAL_GPIO_ReadPin(sdaPort, sdaPin) == GPIO_PIN_RESET) {
        for (uint8_t i = 0; i < I2C_RECOVERY_CLOCK_PULSES; i++) {
            HAL_GPIO_WritePin(sclPort, sclPin, GPIO_PIN_RESET);
            HAL_Delay(1);
            HAL_GPIO_WritePin(sclPort, sclPin, GPIO_PIN_SET);
            HAL_Delay(1);
        }
    }

    /* STOP: SDA low -> SCL high -> SDA high */
    HAL_GPIO_WritePin(sdaPort, sdaPin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(sclPort, sclPin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(sdaPort, sdaPin, GPIO_PIN_SET);
    HAL_Delay(1);

    I2C_BusInitPins(bus);
}

/**
 * @brief Turn a failed transfer into a status, recovering the bus if needed.
 * @note  An absent device answers with a NACK; that is an expected reply, not a
 *        stuck bus, so it must not trigger the recovery sequence.
 */
static I2C_StatusTypeDef I2C_HandleFailure(I2C_Device_t *device, HAL_StatusTypeDef halStatus) {
    I2C_Bus_t *bus = device->bus;

    if ((HAL_I2C_GetError(&bus->hal) & HAL_I2C_ERROR_AF) != 0U) {
        return I2C_NACK;
    }

    /* Resetting the peripheral drops its owner, so the next transfer
       reprograms it for whichever device asks first. */
    if (HAL_I2C_DeInit(&bus->hal) != HAL_OK) {
        log_error("I2C: error recovery failed to reset the peripheral");
    }
    bus->owner = NULL;

    I2C_BusRecovery(bus);

    return I2C_ConvertHALStatus(halStatus);
}

I2C_StatusTypeDef I2C_Master_Transmit(I2C_Device_t *device, uint8_t *pData, uint16_t Size,
                                      uint32_t Timeout) {
    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    I2C_StatusTypeDef status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus =
        HAL_I2C_Master_Transmit(&device->bus->hal, device->address, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(device, halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Master_Receive(I2C_Device_t *device, uint8_t *pData, uint16_t Size,
                                     uint32_t Timeout) {
    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    I2C_StatusTypeDef status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus =
        HAL_I2C_Master_Receive(&device->bus->hal, device->address, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(device, halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Master_TransmitReceive(I2C_Device_t *device, uint8_t *pTxData,
                                             uint16_t TxSize, uint8_t *pRxData, uint16_t RxSize,
                                             uint32_t Timeout) {
    if (pTxData == NULL || pRxData == NULL || TxSize == 0 || RxSize == 0) {
        return I2C_INVALID_PARAM;
    }

    I2C_StatusTypeDef status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus =
        HAL_I2C_Master_Transmit(&device->bus->hal, device->address, pTxData, TxSize, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(device, halStatus);
    }

    halStatus =
        HAL_I2C_Master_Receive(&device->bus->hal, device->address, pRxData, RxSize, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(device, halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Mem_Write(I2C_Device_t *device, uint16_t MemAddress, uint16_t MemAddSize,
                                uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    I2C_StatusTypeDef status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_Mem_Write(&device->bus->hal, device->address, MemAddress,
                                                    MemAddSize, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(device, halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Mem_Read(I2C_Device_t *device, uint16_t MemAddress, uint16_t MemAddSize,
                               uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    if (pData == NULL || Size == 0) {
        return I2C_INVALID_PARAM;
    }

    I2C_StatusTypeDef status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_I2C_Mem_Read(&device->bus->hal, device->address, MemAddress,
                                                   MemAddSize, pData, Size, Timeout);
    if (halStatus != HAL_OK) {
        return I2C_HandleFailure(device, halStatus);
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_IsDeviceReady(I2C_Device_t *device, uint32_t Trials, uint32_t Timeout) {
    I2C_StatusTypeDef status = I2C_Select(device);
    if (status != I2C_OK) {
        return status;
    }

    HAL_StatusTypeDef halStatus =
        HAL_I2C_IsDeviceReady(&device->bus->hal, device->address, Trials, Timeout);
    if (halStatus != HAL_OK && (HAL_I2C_GetError(&device->bus->hal) & HAL_I2C_ERROR_AF) != 0U) {
        return I2C_NACK;
    }

    return I2C_ConvertHALStatus(halStatus);
}

uint8_t I2C_ScanBus(I2C_Device_t *device, uint8_t *pDevices, uint8_t MaxDevices, uint32_t Timeout) {
    uint8_t deviceCount = 0;

    if (pDevices == NULL || MaxDevices == 0) {
        return 0;
    }

    if (I2C_Select(device) != I2C_OK) {
        return 0;
    }

    for (uint8_t addr = I2C_ADDR_MIN; addr <= I2C_ADDR_MAX && deviceCount < MaxDevices; addr++) {
        uint16_t devAddr = (uint16_t)addr << 1;

        if (HAL_I2C_IsDeviceReady(&device->bus->hal, devAddr, 1, Timeout) == HAL_OK) {
            pDevices[deviceCount++] = addr;
        }
    }

    return deviceCount;
}

uint32_t I2C_GetError(const I2C_Device_t *device) {
    I2C_HandleTypeDef *hal = I2C_GetHandle(device);

    return (hal != NULL) ? HAL_I2C_GetError(hal) : HAL_I2C_ERROR_NONE;
}

const char *I2C_GetStatusString(I2C_StatusTypeDef status) {
    switch (status) {
        case I2C_OK:
            return "Operation completed successfully";
        case I2C_ERROR:
            return "General error occurred";
        case I2C_BUSY:
            return "I2C bus is busy";
        case I2C_TIMEOUT:
            return "Operation timed out";
        case I2C_NACK:
            return "No acknowledge received";
        case I2C_INVALID_PARAM:
            return "Invalid parameter provided";
        default:
            return "Unknown status";
    }
}
