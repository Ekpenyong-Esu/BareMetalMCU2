/**
  ******************************************************************************
  * @file    mcp2515_transfer.c
  * @brief   Frame transmission and reception for the MCP2515
  ******************************************************************************
  */

#include "mcp2515_transfer.h"
#include "mcp2515_io.h"
#include "mcp2515_regs.h"

#include <string.h>

/* Private constants ---------------------------------------------------------*/

#define MCP2515_ID_STANDARD_MAX 0x7FFU
#define MCP2515_ID_EXTENDED_MAX 0x1FFFFFFFU

/** SIDH, SIDL, EID8, EID0, DLC then the payload, all written in one selection. */
#define MCP2515_FRAME_HEADER_LENGTH 5U

/* Private functions ---------------------------------------------------------*/

/** @brief Lay the id, the remote flag and the length out in the buffer registers */
static void MCP2515_EncodeHeader(const MCP2515_Frame_t *frame, uint8_t *header)
{
    if (frame->extended) {
        header[0] = (uint8_t)(frame->id >> 21);
        header[1] = (uint8_t)(((frame->id >> 13) & 0xE0U) | MCP2515_SIDL_EXIDE |
                              ((frame->id >> 16) & 0x03U));
        header[2] = (uint8_t)(frame->id >> 8);
        header[3] = (uint8_t)frame->id;
    } else {
        header[0] = (uint8_t)(frame->id >> 3);
        header[1] = (uint8_t)((frame->id & 0x07U) << 5);
        header[2] = 0U;
        header[3] = 0U;
    }

    header[4] = (uint8_t)(frame->length & MCP2515_DLC_MASK);
    if (frame->remote) {
        header[4] |= MCP2515_DLC_RTR;
    }
}

/** @brief Rebuild the id, the remote flag and the length from the buffer registers */
static void MCP2515_DecodeHeader(const uint8_t *header, MCP2515_Frame_t *frame)
{
    frame->extended = ((header[1] & MCP2515_SIDL_IDE) != 0U);

    if (frame->extended) {
        frame->id = ((uint32_t)header[0] << 21) |
                    (((uint32_t)header[1] & 0xE0U) << 13) |
                    (((uint32_t)header[1] & 0x03U) << 16) |
                    ((uint32_t)header[2] << 8) |
                    (uint32_t)header[3];
        /* Extended frames carry the remote flag in the length register. */
        frame->remote = ((header[4] & MCP2515_DLC_RTR) != 0U);
    } else {
        frame->id = ((uint32_t)header[0] << 3) | ((uint32_t)header[1] >> 5);
        /* Standard frames carry it beside the id instead. */
        frame->remote = ((header[1] & MCP2515_SIDL_SRR) != 0U);
    }

    frame->length = header[4] & MCP2515_DLC_MASK;
    if (frame->length > MCP2515_MAX_DATA_LENGTH) {
        frame->length = MCP2515_MAX_DATA_LENGTH;
    }
}

/** @brief Read one receive buffer and release it back to the controller */
static MCP2515_Status_t MCP2515_ReadRxBuffer(MCP2515_Handle_t *hmcp, uint8_t baseReg,
                                             uint8_t interruptFlag, MCP2515_Frame_t *frame)
{
    uint8_t header[MCP2515_FRAME_HEADER_LENGTH] = {0};
    MCP2515_Status_t status;

    status = MCP2515_ReadRegisters(hmcp, baseReg, header, sizeof(header));
    if (status != MCP2515_OK) {
        return status;
    }

    MCP2515_DecodeHeader(header, frame);
    memset(frame->data, 0, sizeof(frame->data));

    if (frame->length > 0U && !frame->remote) {
        status = MCP2515_ReadRegisters(hmcp, (uint8_t)(baseReg + MCP2515_FRAME_HEADER_LENGTH),
                                       frame->data, frame->length);
        if (status != MCP2515_OK) {
            return status;
        }
    }

    /* Until the flag is cleared the controller keeps reporting this frame. */
    return MCP2515_ModifyRegister(hmcp, MCP2515_REG_CANINTF, interruptFlag, 0U);
}

/* Exported functions --------------------------------------------------------*/

MCP2515_Status_t MCP2515_Transmit(MCP2515_Handle_t *hmcp, const MCP2515_Frame_t *frame)
{
    uint8_t payload[MCP2515_FRAME_HEADER_LENGTH + MCP2515_MAX_DATA_LENGTH] = {0};
    uint8_t control = 0U;
    uint8_t length;
    MCP2515_Status_t status;

    if (hmcp == NULL || frame == NULL) {
        return MCP2515_INVALID_PARAM;
    }
    if (!hmcp->initialized) {
        return MCP2515_NOT_INITIALIZED;
    }
    if (frame->length > MCP2515_MAX_DATA_LENGTH) {
        return MCP2515_INVALID_PARAM;
    }
    if (frame->id > (frame->extended ? MCP2515_ID_EXTENDED_MAX : MCP2515_ID_STANDARD_MAX)) {
        return MCP2515_INVALID_PARAM;
    }

    status = MCP2515_ReadRegister(hmcp, MCP2515_REG_TXB0CTRL, &control);
    if (status != MCP2515_OK) {
        return status;
    }
    if ((control & MCP2515_TXBCTRL_TXREQ) != 0U) {
        return MCP2515_TX_BUSY;
    }

    MCP2515_EncodeHeader(frame, payload);
    length = MCP2515_FRAME_HEADER_LENGTH;

    if (!frame->remote && frame->length > 0U) {
        memcpy(&payload[MCP2515_FRAME_HEADER_LENGTH], frame->data, frame->length);
        length = (uint8_t)(length + frame->length);
    }

    status = MCP2515_WriteRegisters(hmcp, MCP2515_REG_TXB0SIDH, payload, length);
    if (status != MCP2515_OK) {
        return status;
    }

    return MCP2515_Command(hmcp, MCP2515_CMD_RTS_TXB0);
}

MCP2515_Status_t MCP2515_Receive(MCP2515_Handle_t *hmcp, MCP2515_Frame_t *frame)
{
    uint8_t flags = 0U;
    MCP2515_Status_t status;

    if (hmcp == NULL || frame == NULL) {
        return MCP2515_INVALID_PARAM;
    }
    if (!hmcp->initialized) {
        return MCP2515_NOT_INITIALIZED;
    }

    status = MCP2515_ReadRegister(hmcp, MCP2515_REG_CANINTF, &flags);
    if (status != MCP2515_OK) {
        return status;
    }

    /* Buffer 0 first: buffer 1 only fills once 0 has overflowed, so it holds
       the newer frame. */
    if ((flags & MCP2515_CANINTF_RX0IF) != 0U) {
        return MCP2515_ReadRxBuffer(hmcp, MCP2515_REG_RXB0SIDH,
                                    MCP2515_CANINTF_RX0IF, frame);
    }

    if ((flags & MCP2515_CANINTF_RX1IF) != 0U) {
        return MCP2515_ReadRxBuffer(hmcp, MCP2515_REG_RXB1SIDH,
                                    MCP2515_CANINTF_RX1IF, frame);
    }

    return MCP2515_NO_MESSAGE;
}

bool MCP2515_IsMessagePending(MCP2515_Handle_t *hmcp)
{
    uint8_t flags = 0U;

    if (hmcp == NULL || !hmcp->initialized) {
        return false;
    }

    if (MCP2515_ReadRegister(hmcp, MCP2515_REG_CANINTF, &flags) != MCP2515_OK) {
        return false;
    }

    return (flags & (MCP2515_CANINTF_RX0IF | MCP2515_CANINTF_RX1IF)) != 0U;
}
