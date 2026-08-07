/**
 * @file usb_cdc.c
 * @brief CDC (virtual COM port) transfers over the USB host stack
 */

#include "usb_cdc.h"
#include "usb_core.h"
#include "usb_events.h"
#include "usb_host_state.h"
#include <string.h>

/* The class keeps the pointers it is given and consumes them asynchronously,
   so none of these may be caller-owned temporaries. */
static uint8_t s_txBuffer[USB_HOST_TX_BUFFER_SIZE];
static CDC_LineCodingTypeDef s_lineCoding;
static uint8_t *s_rxBuffer = NULL;
static uint16_t s_rxLength = 0;
static uint16_t s_lastReceivedSize = 0;
static volatile bool s_txBusy = false;

USB_StatusTypeDef USB_Host_CDC_Transmit(const uint8_t *data, uint16_t length)
{
    USB_StatusTypeDef status;

    if (data == NULL || length == 0U || length > USB_HOST_TX_BUFFER_SIZE) {
        return USB_STATUS_ERROR;
    }

    if (!USB_Host_IsReady()) {
        return USB_STATUS_NOT_READY;
    }

    /* Overwriting the staging buffer would corrupt the transfer in flight. */
    if (s_txBusy) {
        return USB_STATUS_BUSY;
    }

    memcpy(s_txBuffer, data, length);

    status = USB_ConvertHostStatus(USBH_CDC_Transmit(&hUsbHostHS, s_txBuffer, length));
    if (status == USB_STATUS_OK) {
        s_txBusy = true;
    }

    return status;
}

USB_StatusTypeDef USB_Host_CDC_Receive(uint8_t *data, uint16_t length)
{
    USB_StatusTypeDef status;

    if (data == NULL || length == 0U) {
        return USB_STATUS_ERROR;
    }

    if (!USB_Host_IsReady()) {
        return USB_STATUS_NOT_READY;
    }

    status = USB_ConvertHostStatus(USBH_CDC_Receive(&hUsbHostHS, data, length));
    if (status == USB_STATUS_OK) {
        s_rxBuffer = data;
        s_rxLength = length;
    }

    return status;
}

bool USB_Host_CDC_IsTransmitBusy(void)
{
    return s_txBusy;
}

USB_StatusTypeDef USB_Host_CDC_SetLineCoding(const CDC_LineCodingTypeDef *linecoding)
{
    if (linecoding == NULL) {
        return USB_STATUS_ERROR;
    }

    if (!USB_Host_IsReady()) {
        return USB_STATUS_NOT_READY;
    }

    /* USBH_CDC_SetLineCoding stores this pointer and applies it later. */
    s_lineCoding = *linecoding;

    return USB_ConvertHostStatus(USBH_CDC_SetLineCoding(&hUsbHostHS, &s_lineCoding));
}

USB_StatusTypeDef USB_Host_CDC_GetLineCoding(CDC_LineCodingTypeDef *linecoding)
{
    if (linecoding == NULL) {
        return USB_STATUS_ERROR;
    }

    if (!USB_Host_IsReady()) {
        return USB_STATUS_NOT_READY;
    }

    return USB_ConvertHostStatus(USBH_CDC_GetLineCoding(&hUsbHostHS, linecoding));
}

uint16_t USB_Host_CDC_GetLastReceivedDataSize(void)
{
    return s_lastReceivedSize;
}

void USBH_CDC_TransmitCallback(USBH_HandleTypeDef *phost)
{
    UNUSED(phost);

    s_txBusy = false;
    USB_TransmitCompleteCallback();
}

void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
    uint16_t received = USBH_CDC_GetLastReceivedDataSize(phost);

    if (received > s_rxLength) {
        received = s_rxLength;
    }

    s_lastReceivedSize = received;

    if (s_rxBuffer != NULL) {
        USB_DataReceivedCallback(s_rxBuffer, received);
    }
}
