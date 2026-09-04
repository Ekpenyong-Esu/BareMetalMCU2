/**
 * @file ts_io.h
 * @brief STMPE811 register access over I2C
 */

#ifndef TS_IO_H
#define TS_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_types.h"

/**
 * @brief Register the STMPE811 on the application's bus
 * @param hts Touchscreen handle that will own the device record
 * @param bus Open bus the controller is wired to
 * @param address 8-bit I2C address of the controller
 * @return TS_StatusTypeDef TS_INVALID_PARAM when the bus is not usable
 * @note  Must run before any register access
 */
TS_StatusTypeDef TS_IO_DeviceInit(TS_HandleTypeDef *hts, I2C_Bus_t *bus, uint8_t address);

/**
 * @brief Probe the bus for the STMPE811
 * @param hts Touchscreen handle
 * @param trials Number of address attempts
 * @param timeout Timeout per attempt in ms
 * @return TS_StatusTypeDef TS_DEVICE_NOT_FOUND when nothing answers
 */
TS_StatusTypeDef TS_IO_IsDeviceReady(TS_HandleTypeDef *hts, uint32_t trials, uint32_t timeout);

/**
 * @brief Read one 8-bit register
 * @param hts Touchscreen handle
 * @param reg Register address
 * @param data Destination
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_ReadRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data);

/**
 * @brief Read a block of registers
 * @param hts Touchscreen handle
 * @param reg Starting register address
 * @param data Destination buffer
 * @param size Number of bytes to read
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_ReadRegisterMulti(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data,
                                      uint16_t size);

/**
 * @brief Write one 8-bit register
 * @param hts Touchscreen handle
 * @param reg Register address
 * @param data Value to write
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_WriteRegister(TS_HandleTypeDef *hts, uint8_t reg, uint8_t data);

/**
 * @brief Read one 16-bit big-endian register
 * @param hts Touchscreen handle
 * @param reg Register address
 * @param data Destination
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_ReadRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t *data);

/**
 * @brief Write one 16-bit big-endian register
 * @param hts Touchscreen handle
 * @param reg Register address
 * @param data Value to write
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_WriteRegister16(TS_HandleTypeDef *hts, uint8_t reg, uint16_t data);

/**
 * @brief Reset and release the touch FIFO, mandatory after every read
 * @param hts Touchscreen handle
 */
void TS_ResetFifo(TS_HandleTypeDef *hts);

#ifdef __cplusplus
}
#endif

#endif /* TS_IO_H */
