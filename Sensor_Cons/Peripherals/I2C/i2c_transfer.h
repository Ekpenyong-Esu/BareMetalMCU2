/**
  ******************************************************************************
  * @file    i2c_transfer.h
  * @brief   Master and memory transfer operations
  ******************************************************************************
  */

#ifndef I2C_TRANSFER_H
#define I2C_TRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_types.h"

/* Blocking send of Size bytes to a slave at DevAddress. */
I2C_StatusTypeDef I2C_Master_Transmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/* Blocking read of Size bytes from a slave at DevAddress. */
I2C_StatusTypeDef I2C_Master_Receive(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/* Blocking write then read in one transaction (repeated start, no stop between). */
I2C_StatusTypeDef I2C_Master_TransmitReceive(uint16_t DevAddress,
                                             uint8_t *pTxData, uint16_t TxSize,
                                             uint8_t *pRxData, uint16_t RxSize,
                                             uint32_t Timeout);

/* Writes Size bytes starting at a register/memory address on the slave. */
I2C_StatusTypeDef I2C_Mem_Write(uint16_t DevAddress, uint16_t MemAddress,
                               uint16_t MemAddSize, uint8_t *pData,
                               uint16_t Size, uint32_t Timeout);

/* Reads Size bytes starting at a register/memory address on the slave. */
I2C_StatusTypeDef I2C_Mem_Read(uint16_t DevAddress, uint16_t MemAddress,
                              uint16_t MemAddSize, uint8_t *pData,
                              uint16_t Size, uint32_t Timeout);

/* Probes for a slave by attempting Trials address acknowledgements. */
I2C_StatusTypeDef I2C_IsDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

/* Probes every address on the bus, writing responders into pDevices. */
uint8_t I2C_ScanBus(uint8_t *pDevices, uint8_t MaxDevices, uint32_t Timeout);

/* Raw HAL error flags from the last failed transfer. */
uint32_t I2C_GetError(void);

/* Human-readable name for a status code, for logging. */
const char *I2C_GetStatusString(I2C_StatusTypeDef status);

/**
 * @brief Free a bus left stuck by an aborted transfer, then re-initialise it.
 */
void I2C_RecoverOnError(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_TRANSFER_H */
