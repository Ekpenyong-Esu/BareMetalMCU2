/**
 * @file usb_cdc.c
 * @brief CDC (virtual COM port) transfers over the USB host stack
 */

#include "usb_cdc.h"
#include "usb_core.h"
#include "usb_events.h"
#include "usb_host_state.h"
#include <string.h>

USB_StatusTypeDef USB_Host_CDC_Transmit(USB_Handle_t *husb, const uint8_t *data, uint16_t length) {
    USB_StatusTypeDef status = USB_STATUS_OK;

    if (data == NULL || length == 0U || length > USB_HOST_TX_BUFFER_SIZE) {
        return USB_STATUS_ERROR;
    }

    if (!USB_Host_IsReady(husb)) {
        return USB_STATUS_NOT_READY;
    }

    /* Overwriting the staging buffer would corrupt the transfer in flight. */
    if (husb->txBusy) {
        return USB_STATUS_BUSY;
    }

    memcpy(husb->txBuffer, data, length);

    status = USB_ConvertHostStatus(USBH_CDC_Transmit(husb->host, husb->txBuffer, length));
    if (status == USB_STATUS_OK) {
        husb->txBusy = true;
    }

    return status;
}

USB_StatusTypeDef USB_Host_CDC_Receive(USB_Handle_t *husb, uint8_t *data, uint16_t length) {
    USB_StatusTypeDef status = USB_STATUS_OK;

    if (data == NULL || length == 0U) {
        return USB_STATUS_ERROR;
    }

    if (!USB_Host_IsReady(husb)) {
        return USB_STATUS_NOT_READY;
    }

    status = USB_ConvertHostStatus(USBH_CDC_Receive(husb->host, data, length));
    if (status == USB_STATUS_OK) {
        husb->rxBuffer = data;
        husb->rxLength = length;
    }

    return status;
}

bool USB_Host_CDC_IsTransmitBusy(const USB_Handle_t *husb) {
    return (husb != NULL) && husb->txBusy;
}

USB_StatusTypeDef USB_Host_CDC_SetLineCoding(USB_Handle_t *husb,
                                             const CDC_LineCodingTypeDef *linecoding) {
    if (linecoding == NULL) {
        return USB_STATUS_ERROR;
    }

    if (!USB_Host_IsReady(husb)) {
        return USB_STATUS_NOT_READY;
    }

    /* USBH_CDC_SetLineCoding stores this pointer and applies it later. */
    husb->lineCoding = *linecoding;

    return USB_ConvertHostStatus(USBH_CDC_SetLineCoding(husb->host, &husb->lineCoding));
}

USB_StatusTypeDef USB_Host_CDC_GetLineCoding(USB_Handle_t *husb,
                                             CDC_LineCodingTypeDef *linecoding) {
    if (linecoding == NULL) {
        return USB_STATUS_ERROR;
    }

    if (!USB_Host_IsReady(husb)) {
        return USB_STATUS_NOT_READY;
    }

    return USB_ConvertHostStatus(USBH_CDC_GetLineCoding(husb->host, linecoding));
}

uint16_t USB_Host_CDC_GetLastReceivedDataSize(const USB_Handle_t *husb) {
    return (husb != NULL) ? husb->lastReceivedSize : 0U;
}

/* Host library callbacks ----------------------------------------------------*/

void USBH_CDC_TransmitCallback(USBH_HandleTypeDef *phost) {
    USB_Handle_t *husb = USB_Events_Resolve(phost);

    if (husb == NULL) {
        return;
    }

    husb->txBusy = false;
    USB_TransmitCompleteCallback(husb);
}

void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost) {
    USB_Handle_t *husb = USB_Events_Resolve(phost);
    uint16_t received = 0;

    if (husb == NULL) {
        return;
    }

    received = USBH_CDC_GetLastReceivedDataSize(phost);
    if (received > husb->rxLength) {
        received = husb->rxLength;
    }

    husb->lastReceivedSize = received;

    if (husb->rxBuffer != NULL) {
        USB_DataReceivedCallback(husb, husb->rxBuffer, received);
    }
}
