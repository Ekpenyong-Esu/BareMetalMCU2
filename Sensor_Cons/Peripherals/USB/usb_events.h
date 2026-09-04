/**
 * @file usb_events.h
 * @brief Application hooks raised by the USB host driver, and the routing of
 *        host-library callbacks back to the driver handle that owns them
 * @note  The application hooks are weak; override them in application code.
 */

#ifndef USB_EVENTS_H
#define USB_EVENTS_H

#include "usb_types.h"

/* Application hooks ---------------------------------------------------------*/

/**
 * @brief A device has finished enumeration and its class is active
 */
void USB_ConnectCallback(USB_Handle_t *husb);

/**
 * @brief The device has been detached
 */
void USB_DisconnectCallback(USB_Handle_t *husb);

/**
 * @brief A CDC receive transfer completed
 * @param data   Buffer passed to USB_Host_CDC_Receive
 * @param length Bytes actually received
 */
void USB_DataReceivedCallback(USB_Handle_t *husb, const uint8_t *data, uint16_t length);

/**
 * @brief A CDC transmit transfer completed and the staging buffer is free
 */
void USB_TransmitCompleteCallback(USB_Handle_t *husb);

/**
 * @brief Reported when a USB operation fails
 */
void USB_ErrorHandler(USB_Handle_t *husb, uint32_t error_code);

/* Callback routing ----------------------------------------------------------*/

/**
 * @brief Route host-library callbacks for husb->host to this handle
 * @return USB_STATUS_ERROR when every host slot is taken
 */
USB_StatusTypeDef USB_Events_Attach(USB_Handle_t *husb);

/**
 * @brief Stop routing callbacks to a handle; ignored if it is not attached
 */
void USB_Events_Detach(USB_Handle_t *husb);

/**
 * @brief Find the driver handle attached to a host-library handle
 * @return NULL when the host is not attached
 */
USB_Handle_t *USB_Events_Resolve(const USBH_HandleTypeDef *phost);

#endif /* USB_EVENTS_H */
