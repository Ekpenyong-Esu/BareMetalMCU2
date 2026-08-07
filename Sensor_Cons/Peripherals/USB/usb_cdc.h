/**
 * @file usb_cdc.h
 * @brief CDC (virtual COM port) transfers over the USB host stack
 */

#ifndef USB_CDC_H
#define USB_CDC_H

#include "usb_types.h"

/**
 * @brief Queue a CDC transmit
 * @note Asynchronous: the data is staged internally and
 *       USB_TransmitCompleteCallback fires when the buffer is free again.
 */
USB_StatusTypeDef USB_Host_CDC_Transmit(const uint8_t *data, uint16_t length);

/**
 * @brief Arm a CDC receive into a caller-owned buffer
 * @note The buffer must stay valid until USB_DataReceivedCallback fires.
 */
USB_StatusTypeDef USB_Host_CDC_Receive(uint8_t *data, uint16_t length);

/**
 * @brief Whether a transmit is still in flight
 */
bool USB_Host_CDC_IsTransmitBusy(void);

/**
 * @brief Apply a line coding to the attached device
 */
USB_StatusTypeDef USB_Host_CDC_SetLineCoding(const CDC_LineCodingTypeDef *linecoding);

/**
 * @brief Read back the device's line coding
 */
USB_StatusTypeDef USB_Host_CDC_GetLineCoding(CDC_LineCodingTypeDef *linecoding);

/**
 * @brief Bytes delivered by the most recent receive
 */
uint16_t USB_Host_CDC_GetLastReceivedDataSize(void);

#endif /* USB_CDC_H */
