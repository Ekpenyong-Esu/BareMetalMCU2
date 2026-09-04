/**
 ******************************************************************************
 * @file    mcp2515_io.h
 * @brief   Register-level access to the MCP2515 over the shared SPI bus
 ******************************************************************************
 */

#ifndef MCP2515_IO_H
#define MCP2515_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mcp2515_types.h"

/**
 * @brief   Read one register
 * @param   hmcp Device handle
 * @param   reg Register address
 * @param   value Destination
 * @retval  MCP2515_Status_t Status of the operation
 */
MCP2515_Status_t MCP2515_ReadRegister(MCP2515_Handle_t *hmcp, uint8_t reg, uint8_t *value);

/**
 * @brief   Read consecutive registers in one selection
 * @param   hmcp Device handle
 * @param   reg First register address
 * @param   data Destination buffer
 * @param   length Number of registers
 * @retval  MCP2515_Status_t Status of the operation
 */
MCP2515_Status_t MCP2515_ReadRegisters(MCP2515_Handle_t *hmcp, uint8_t reg, uint8_t *data,
                                       uint8_t length);

/**
 * @brief   Write one register
 * @param   hmcp Device handle
 * @param   reg Register address
 * @param   value Value to write
 * @retval  MCP2515_Status_t Status of the operation
 */
MCP2515_Status_t MCP2515_WriteRegister(MCP2515_Handle_t *hmcp, uint8_t reg, uint8_t value);

/**
 * @brief   Write consecutive registers in one selection
 * @param   hmcp Device handle
 * @param   reg First register address
 * @param   data Values to write
 * @param   length Number of registers
 * @retval  MCP2515_Status_t Status of the operation
 */
MCP2515_Status_t MCP2515_WriteRegisters(MCP2515_Handle_t *hmcp, uint8_t reg, const uint8_t *data,
                                        uint8_t length);

/**
 * @brief   Change only the masked bits of a register
 * @param   hmcp Device handle
 * @param   reg Register address
 * @param   mask Bits to change
 * @param   value New values for the masked bits
 * @retval  MCP2515_Status_t Status of the operation
 * @note    Not every register supports this; the datasheet lists the ones that
 *          do, and the control and interrupt registers used here are among them.
 */
MCP2515_Status_t MCP2515_ModifyRegister(MCP2515_Handle_t *hmcp, uint8_t reg, uint8_t mask,
                                        uint8_t value);

/**
 * @brief   Issue the reset instruction, leaving the device in configuration mode
 * @param   hmcp Device handle
 * @retval  MCP2515_Status_t Status of the operation
 */
MCP2515_Status_t MCP2515_ResetDevice(MCP2515_Handle_t *hmcp);

/**
 * @brief   Send a single-byte instruction that carries no payload
 * @param   hmcp Device handle
 * @param   command Instruction byte
 * @retval  MCP2515_Status_t Status of the operation
 */
MCP2515_Status_t MCP2515_Command(MCP2515_Handle_t *hmcp, uint8_t command);

#ifdef __cplusplus
}
#endif

#endif /* MCP2515_IO_H */
