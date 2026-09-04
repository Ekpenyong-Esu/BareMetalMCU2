/**
 * @file eth_frame.c
 * @brief Ethernet frame transmission and reception
 */

#include "eth_frame.h"
#include "eth_buffers.h"
#include "eth_core.h"
#include "log.h"
#include <string.h>

#define ETH_BYTE_MASK 0xFFU

HAL_StatusTypeDef ETH_TransmitFrame(ETH_Handle_t *handle, const ETH_Frame_t *frame) {
    ETH_TxPacketConfigTypeDef txConfig;
    ETH_BufferTypeDef txBufferDesc;
    uint8_t *buffer = NULL;
    uint32_t frameLength = 0;

    if ((handle == NULL) || (frame == NULL)) {
        return HAL_ERROR;
    }

    if (!ETH_IsReady(handle)) {
        return HAL_ERROR;
    }

    if (frame->payloadLength > ETH_MAX_PAYLOAD_LEN) {
        log_error("ETH: payload of %lu bytes exceeds the maximum",
                  (unsigned long)frame->payloadLength);
        return HAL_ERROR;
    }

    if ((frame->payloadLength > 0U) && (frame->payload == NULL)) {
        return HAL_ERROR;
    }

    buffer = ETH_Buffers_GetTxBuffer();

    memcpy(&buffer[0], frame->destination, ETH_ADDR_LEN);
    memcpy(&buffer[ETH_ADDR_LEN], frame->source, ETH_ADDR_LEN);
    buffer[ETH_TYPE_OFFSET] = (uint8_t)((frame->type >> 8) & ETH_BYTE_MASK);
    buffer[ETH_TYPE_OFFSET + 1U] = (uint8_t)(frame->type & ETH_BYTE_MASK);

    if (frame->payloadLength > 0U) {
        memcpy(&buffer[ETH_PAYLOAD_OFFSET], frame->payload, frame->payloadLength);
    }

    frameLength = ETH_HEADER_LEN + frame->payloadLength;

    txBufferDesc.buffer = buffer;
    txBufferDesc.len = frameLength;
    txBufferDesc.next = NULL;

    memset(&txConfig, 0, sizeof(txConfig));
    txConfig.Attributes = ETH_TX_PACKETS_FEATURES_CRCPAD;
    txConfig.Length = frameLength;
    txConfig.TxBuffer = &txBufferDesc;
    txConfig.SrcAddrCtrl = ETH_SOURCEADDRESS_DISABLE;
    txConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

    return HAL_ETH_Transmit(&handle->heth, &txConfig, ETH_TX_TIMEOUT_MS);
}

HAL_StatusTypeDef ETH_ReceiveFrame(ETH_Handle_t *handle, ETH_Frame_t *frame) {
    uint8_t *packet = NULL;
    uint32_t frameLength = 0;

    if ((handle == NULL) || (frame == NULL)) {
        return HAL_ERROR;
    }

    if (!ETH_IsReady(handle)) {
        return HAL_ERROR;
    }

    if (HAL_ETH_ReadData(&handle->heth, (void **)&packet) != HAL_OK) {
        return HAL_ERROR;
    }

    if (packet == NULL) {
        return HAL_ERROR;
    }

    /* The DMA reports the length including the frame check sequence. */
    frameLength = handle->heth.RxDescList.RxDataLength;

    if (frameLength <= (ETH_HEADER_LEN + ETH_FCS_LEN)) {
        log_warning("ETH: runt frame of %lu bytes discarded", (unsigned long)frameLength);
        return HAL_ERROR;
    }

    frameLength -= ETH_FCS_LEN;

    if (frameLength > ETH_MAX_FRAME_LEN) {
        log_warning("ETH: oversized frame of %lu bytes discarded", (unsigned long)frameLength);
        return HAL_ERROR;
    }

    memcpy(frame->destination, &packet[0], ETH_ADDR_LEN);
    memcpy(frame->source, &packet[ETH_ADDR_LEN], ETH_ADDR_LEN);
    frame->type = (uint16_t)(((uint16_t)packet[ETH_TYPE_OFFSET] << 8) |
                             (uint16_t)packet[ETH_TYPE_OFFSET + 1U]);
    frame->payload = &packet[ETH_PAYLOAD_OFFSET];
    frame->payloadLength = frameLength - ETH_HEADER_LEN;

    return HAL_OK;
}
