/**
 * @file usb_host_state.c
 * @brief Host state machine pump, connection state and device identity
 */

#include "usb_host_state.h"
#include "usb_core.h"
#include "usb_events.h"

static ApplicationTypeDef s_lastState = APPLICATION_IDLE;

USB_StatusTypeDef USB_Host_Process(void)
{
    ApplicationTypeDef state;

    if (!USB_IsInitialized()) {
        return USB_STATUS_NOT_READY;
    }

    USBH_Process(&hUsbHostHS);

    /* USBH_UserProcess only records the state, so the edge is detected here. */
    state = Appli_state;
    if (state != s_lastState) {
        s_lastState = state;

        if (state == APPLICATION_READY) {
            USB_ConnectCallback();
        } else if (state == APPLICATION_DISCONNECT) {
            USB_DisconnectCallback();
        }
    }

    return USB_STATUS_OK;
}

USB_HostStateTypeDef USB_Host_GetState(void)
{
    switch (Appli_state) {
        case APPLICATION_IDLE:       return USB_HOST_IDLE;
        case APPLICATION_START:      return USB_HOST_DEVICE_ATTACHED;
        case APPLICATION_READY:      return USB_HOST_CLASS_ACTIVE;
        case APPLICATION_DISCONNECT: return USB_HOST_WAIT_FOR_ATTACHMENT;
        default:                     return USB_HOST_ERROR_STATE;
    }
}

bool USB_Host_IsConnected(void)
{
    /* APPLICATION_IDLE means nothing has ever attached, so testing only for
       "not disconnected" would report a connection from reset. */
    return (Appli_state == APPLICATION_START) || (Appli_state == APPLICATION_READY);
}

bool USB_Host_IsReady(void)
{
    return (Appli_state == APPLICATION_READY);
}

uint16_t USB_GetConnectedDeviceVID(void)
{
    if (!USB_Host_IsReady()) {
        return 0U;
    }

    return hUsbHostHS.device.DevDesc.idVendor;
}

uint16_t USB_GetConnectedDevicePID(void)
{
    if (!USB_Host_IsReady()) {
        return 0U;
    }

    return hUsbHostHS.device.DevDesc.idProduct;
}
