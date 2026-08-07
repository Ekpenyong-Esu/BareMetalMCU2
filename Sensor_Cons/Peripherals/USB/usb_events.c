/**
 * @file usb_events.c
 * @brief Default (empty) implementations of the USB application hooks
 */

#include "usb_events.h"
#include "log.h"

__weak void USB_ConnectCallback(void)
{
}

__weak void USB_DisconnectCallback(void)
{
}

__weak void USB_DataReceivedCallback(const uint8_t *data, uint16_t length)
{
    UNUSED(data);
    UNUSED(length);
}

__weak void USB_TransmitCompleteCallback(void)
{
}

/* Halting here would take the whole system down for a recoverable USB fault. */
__weak void USB_ErrorHandler(uint32_t error_code)
{
    log_error("USB: error %lu", (unsigned long)error_code);
}
