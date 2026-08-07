/**
 * @file usb_events.h
 * @brief Application hooks raised by the USB host driver
 * @note  All four are weak; override them in application code.
 */

#ifndef USB_EVENTS_H
#define USB_EVENTS_H

#include "usb_types.h"

/**
 * @brief A device has finished enumeration and its class is active
 */
void USB_ConnectCallback(void);

/**
 * @brief The device has been detached
 */
void USB_DisconnectCallback(void);

/**
 * @brief A CDC receive transfer completed
 * @param data   Buffer passed to USB_Host_CDC_Receive
 * @param length Bytes actually received
 */
void USB_DataReceivedCallback(const uint8_t *data, uint16_t length);

/**
 * @brief A CDC transmit transfer completed and the staging buffer is free
 */
void USB_TransmitCompleteCallback(void);

/**
 * @brief Reported when a USB operation fails
 */
void USB_ErrorHandler(uint32_t error_code);

#endif /* USB_EVENTS_H */
