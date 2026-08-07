/**
 * @file usb_core.h
 * @brief USB stack lifetime and operating mode
 */

#ifndef USB_CORE_H
#define USB_CORE_H

#include "usb_types.h"

/**
 * @brief Initialize the USB stack in the configured mode
 * @note The host library is started as part of initialization; USB_Start is
 *       only needed after an explicit USB_Stop.
 */
USB_StatusTypeDef USB_Init(const USB_ConfigTypeDef *config);

/**
 * @brief Deinitialize the USB stack
 */
USB_StatusTypeDef USB_DeInit(void);

/**
 * @brief Start the host library
 */
USB_StatusTypeDef USB_Start(void);

/**
 * @brief Stop the host library
 */
USB_StatusTypeDef USB_Stop(void);

/**
 * @brief Whether USB_Init has completed successfully
 */
bool USB_IsInitialized(void);

/**
 * @brief Initialize the host library directly
 */
USB_StatusTypeDef USB_Host_Init(void);

/**
 * @brief Deinitialize the host library directly
 */
USB_StatusTypeDef USB_Host_DeInit(void);

/**
 * @brief Select the operating mode; only USB_MODE_HOST is supported
 */
USB_StatusTypeDef USB_SetOperationMode(USB_OperationModeTypeDef mode);

/**
 * @brief Current operating mode
 */
USB_OperationModeTypeDef USB_GetOperationMode(void);

/**
 * @brief Map a USB host library status onto the driver status
 */
USB_StatusTypeDef USB_ConvertHostStatus(USBH_StatusTypeDef status);

#endif /* USB_CORE_H */
