/**
 * @file usb_host_state.h
 * @brief Host state machine pump, connection state and device identity
 */

#ifndef USB_HOST_STATE_H
#define USB_HOST_STATE_H

#include "usb_types.h"

/**
 * @brief Run one iteration of the host state machine
 * @note Must be called periodically; connect and disconnect events are raised
 *       from here.
 */
USB_StatusTypeDef USB_Host_Process(void);

/**
 * @brief Current host state
 */
USB_HostStateTypeDef USB_Host_GetState(void);

/**
 * @brief Whether a device is attached (enumerated or still enumerating)
 */
bool USB_Host_IsConnected(void);

/**
 * @brief Whether the class driver is active and transfers are possible
 */
bool USB_Host_IsReady(void);

/**
 * @brief Vendor ID of the attached device, 0 when none is ready
 */
uint16_t USB_GetConnectedDeviceVID(void);

/**
 * @brief Product ID of the attached device, 0 when none is ready
 */
uint16_t USB_GetConnectedDevicePID(void);

#endif /* USB_HOST_STATE_H */
