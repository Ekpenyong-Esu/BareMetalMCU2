/**
 * @file usb_core.h
 * @brief USB stack lifetime and operating mode
 */

#ifndef USB_CORE_H
#define USB_CORE_H

#include "usb_types.h"

/**
 * @brief Bind a driver handle to an already-initialised host library handle
 * @param husb       Driver handle, owned by the caller
 * @param host       Host library handle after USBH_Init/RegisterClass/Start
 *                   (CubeMX's MX_USB_HOST_Init does all three)
 * @param appliState The ApplicationTypeDef the caller's USBH_UserProcess writes
 * @param config     Operating mode; only USB_MODE_HOST is supported
 * @note The library is not started here; USB_Start is only needed after an
 *       explicit USB_Stop.
 */
USB_StatusTypeDef USB_Init(USB_Handle_t *husb, USBH_HandleTypeDef *host,
                           ApplicationTypeDef *appliState, const USB_ConfigTypeDef *config);

/**
 * @brief Deinitialize the host library and release the handle
 */
USB_StatusTypeDef USB_DeInit(USB_Handle_t *husb);

/**
 * @brief Start the host library
 */
USB_StatusTypeDef USB_Start(USB_Handle_t *husb);

/**
 * @brief Stop the host library
 */
USB_StatusTypeDef USB_Stop(USB_Handle_t *husb);

/**
 * @brief Whether USB_Init has completed successfully
 */
bool USB_IsInitialized(const USB_Handle_t *husb);

/**
 * @brief Select the operating mode; only USB_MODE_HOST is supported
 */
USB_StatusTypeDef USB_SetOperationMode(USB_Handle_t *husb, USB_OperationModeTypeDef mode);

/**
 * @brief Current operating mode
 */
USB_OperationModeTypeDef USB_GetOperationMode(const USB_Handle_t *husb);

/**
 * @brief Map a USB host library status onto the driver status
 */
USB_StatusTypeDef USB_ConvertHostStatus(USBH_StatusTypeDef status);

#endif /* USB_CORE_H */
