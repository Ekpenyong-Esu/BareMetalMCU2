/**
 ******************************************************************************
 * @file    mcp2515_transfer.h
 * @brief   Frame transmission and reception for the MCP2515
 ******************************************************************************
 */

#ifndef MCP2515_TRANSFER_H
#define MCP2515_TRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mcp2515_types.h"

/**
 * @brief   Queue a frame in transmit buffer 0
 * @param   hmcp Device handle
 * @param   frame Frame to send
 * @retval  MCP2515_Status_t MCP2515_TX_BUSY if the previous frame is still queued
 * @note    Returns as soon as the frame is handed over; it is on the wire only
 *          once the controller wins arbitration.
 */
MCP2515_Status_t MCP2515_Transmit(MCP2515_Handle_t *hmcp, const MCP2515_Frame_t *frame);

/**
 * @brief   Take the oldest received frame, if any
 * @param   hmcp Device handle
 * @param   frame Destination
 * @retval  MCP2515_Status_t MCP2515_NO_MESSAGE when nothing is waiting
 */
MCP2515_Status_t MCP2515_Receive(MCP2515_Handle_t *hmcp, MCP2515_Frame_t *frame);

/**
 * @brief   Whether a frame is waiting to be read
 * @param   hmcp Device handle
 * @retval  bool true when MCP2515_Receive() would return a frame
 */
bool MCP2515_IsMessagePending(MCP2515_Handle_t *hmcp);

#ifdef __cplusplus
}
#endif

#endif /* MCP2515_TRANSFER_H */
