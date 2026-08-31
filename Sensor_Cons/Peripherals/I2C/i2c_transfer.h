/**
  ******************************************************************************
  * @file    i2c_transfer.h
  * @brief   Master and memory transfer operations
  * @details Every transfer names the device it targets. The device carries both
  *          the slave address and the bus settings, so selecting the bus and
  *          addressing the slave stay in step.
  ******************************************************************************
  */

#ifndef I2C_TRANSFER_H
#define I2C_TRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_types.h"

I2C_StatusTypeDef I2C_Master_Transmit(I2C_Device_t *device, uint8_t *pData,
                                      uint16_t Size, uint32_t Timeout);

I2C_StatusTypeDef I2C_Master_Receive(I2C_Device_t *device, uint8_t *pData,
                                     uint16_t Size, uint32_t Timeout);

I2C_StatusTypeDef I2C_Master_TransmitReceive(I2C_Device_t *device,
                                             uint8_t *pTxData, uint16_t TxSize,
                                             uint8_t *pRxData, uint16_t RxSize,
                                             uint32_t Timeout);

I2C_StatusTypeDef I2C_Mem_Write(I2C_Device_t *device, uint16_t MemAddress,
                                uint16_t MemAddSize, uint8_t *pData,
                                uint16_t Size, uint32_t Timeout);

I2C_StatusTypeDef I2C_Mem_Read(I2C_Device_t *device, uint16_t MemAddress,
                               uint16_t MemAddSize, uint8_t *pData,
                               uint16_t Size, uint32_t Timeout);

I2C_StatusTypeDef I2C_IsDeviceReady(I2C_Device_t *device, uint32_t Trials,
                                    uint32_t Timeout);

/**
 * @brief Probe every address on the bus
 * @param device Supplies the bus settings to scan with; its own address is
 *               ignored because the scan walks the whole range.
 */
uint8_t I2C_ScanBus(I2C_Device_t *device, uint8_t *pDevices, uint8_t MaxDevices,
                    uint32_t Timeout);

uint32_t I2C_GetError(void);

const char *I2C_GetStatusString(I2C_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* I2C_TRANSFER_H */
