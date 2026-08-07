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
TS_StatusTypeDef TS_ReadRegisterMulti(TS_HandleTypeDef *hts, uint8_t reg, uint8_t *data, uint16_t size);

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
