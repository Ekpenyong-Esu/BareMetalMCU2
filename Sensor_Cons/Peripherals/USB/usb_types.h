/**
 * @file usb_types.h
 * @brief Shared vocabulary for the USB host driver
 */

#ifndef USB_TYPES_H
#define USB_TYPES_H

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_cdc.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief USB Mode enumeration
 * @note Only host mode is wired on the STM32F429I-DISC1.
 */
typedef enum {
    USB_MODE_HOST = 0
} USB_OperationModeTypeDef;

/**
 * @brief USB Status enumeration
 */
typedef enum {
    USB_STATUS_OK = 0,
    USB_STATUS_ERROR,
    USB_STATUS_BUSY,
    USB_STATUS_TIMEOUT,
    USB_STATUS_NOT_SUPPORTED,
    USB_STATUS_NOT_READY
} USB_StatusTypeDef;

/**
 * @brief USB Configuration structure
 */
typedef struct {
    USB_OperationModeTypeDef mode;
} USB_ConfigTypeDef;

/**
 * @brief USB Host Application State
 */
typedef enum {
    USB_HOST_IDLE = 0,
    USB_HOST_WAIT_FOR_ATTACHMENT,
    USB_HOST_DEVICE_ATTACHED,
    USB_HOST_CLASS_ACTIVE,
    USB_HOST_ERROR_STATE
} USB_HostStateTypeDef;

/** Staging buffer for CDC transmits, which are asynchronous */
#define USB_HOST_TX_BUFFER_SIZE             512U

/* Host core handle and application state owned by USB_HOST/App/usb_host.c */
extern USBH_HandleTypeDef hUsbHostHS;
extern ApplicationTypeDef Appli_state;

#endif /* USB_TYPES_H */
