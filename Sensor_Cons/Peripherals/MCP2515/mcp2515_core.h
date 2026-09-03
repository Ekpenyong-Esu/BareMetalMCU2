/**
  ******************************************************************************
  * @file    mcp2515_core.h
  * @brief   Lifecycle, mode and bit timing for the MCP2515
  ******************************************************************************
  */

#ifndef MCP2515_CORE_H
#define MCP2515_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mcp2515_types.h"

/**
 * @brief   Bring up the controller and leave it in the requested mode
 * @param   hmcp Caller-owned handle
 * @param   config Chip-select pin, crystal, bit rate and target mode
 * @retval  MCP2515_Status_t Status of the operation
 * @note    Registers a device on the shared SPI bus, so the display and gyro
 *          on the same wires are unaffected.
 */
MCP2515_Status_t MCP2515_Init(MCP2515_Handle_t *hmcp, const MCP2515_Config_t *config);

/**
 * @brief   Put the device back in reset and release the handle
 * @param   hmcp Device handle
 * @retval  MCP2515_Status_t Status of the operation
 */
MCP2515_Status_t MCP2515_DeInit(MCP2515_Handle_t *hmcp);

/**
 * @brief   Request an operating mode and wait for the device to confirm it
 * @param   hmcp Device handle
 * @param   mode Mode to enter
 * @retval  MCP2515_Status_t MCP2515_TIMEOUT if the device never reports it
 */
MCP2515_Status_t MCP2515_SetMode(MCP2515_Handle_t *hmcp, MCP2515_Mode_t mode);

/**
 * @brief   Program the bit timing registers for a bit rate
 * @param   hmcp Device handle
 * @param   oscillator_hz Crystal fitted to the module
 * @param   baud_rate Wanted bit rate
 * @retval  MCP2515_Status_t MCP2515_BITRATE_UNSUPPORTED when no exact division exists
 * @note    Only writable in configuration mode; MCP2515_Init() handles that.
 */
MCP2515_Status_t MCP2515_SetBitRate(MCP2515_Handle_t *hmcp, uint32_t oscillator_hz,
                                    uint32_t baud_rate);

#ifdef __cplusplus
}
#endif

#endif /* MCP2515_CORE_H */
