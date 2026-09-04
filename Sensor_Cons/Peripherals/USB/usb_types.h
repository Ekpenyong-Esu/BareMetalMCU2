/**
 * @file usb_types.h
 * @brief Shared vocabulary for the USB host driver
 * @details The host library handle and the CubeMX application-state variable
 *          belong to the application (USB_HOST/App/usb_host.c); the driver
 *          only borrows pointers to them through USB_Handle_t.
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
 * @note Only host mode is implemented by this façade.
 */
typedef enum { USB_MODE_HOST = 0 } USB_OperationModeTypeDef;

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
#define USB_HOST_TX_BUFFER_SIZE 512U

/** OTG cores on the STM32F4 (HS and FS), so at most this many handles at once */
#define USB_MAX_HOSTS 2U

/**
 * @brief Driver handle: one per host core, owned by the application
 * @note The class library keeps the buffer pointers it is given and consumes
 *       them asynchronously, so the transmit staging buffer and line coding
 *       live here rather than on any caller's stack.
 */
typedef struct {
    USBH_HandleTypeDef *host;       /*!< Host library handle, initialised by the application */
    ApplicationTypeDef *appliState; /*!< State written by the application's USBH_UserProcess */
    USB_OperationModeTypeDef mode;  /*!< Operating mode */
    ApplicationTypeDef lastState;   /*!< Last state seen by USB_Host_Process, for edge detection */
    bool initialized;               /*!< USB_Init completed */
    uint8_t txBuffer[USB_HOST_TX_BUFFER_SIZE]; /*!< CDC transmit staging buffer */
    volatile bool txBusy;                      /*!< A transmit is in flight */
    CDC_LineCodingTypeDef lineCoding;          /*!< Pending line coding for the class */
    uint8_t *rxBuffer;                         /*!< Caller buffer armed by USB_Host_CDC_Receive */
    uint16_t rxLength;                         /*!< Size of rxBuffer */
    uint16_t lastReceivedSize;                 /*!< Bytes delivered by the last receive */
} USB_Handle_t;

#endif /* USB_TYPES_H */
