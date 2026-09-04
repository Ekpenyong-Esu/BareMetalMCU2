/**
 * @file usb_host_state.c
 * @brief Host state machine pump, connection state and device identity
 */

#include "usb_host_state.h"
#include "usb_core.h"
#include "usb_events.h"

USB_StatusTypeDef USB_Host_Process(USB_Handle_t *husb) {
    ApplicationTypeDef state = APPLICATION_IDLE;

    if (!USB_IsInitialized(husb)) {
        return USB_STATUS_NOT_READY;
    }

    USBH_Process(husb->host);

    /* USBH_UserProcess only records the state, so the edge is detected here. */
    state = *husb->appliState;
    if (state != husb->lastState) {
        husb->lastState = state;

        if (state == APPLICATION_READY) {
            USB_ConnectCallback(husb);
        }
        else if (state == APPLICATION_DISCONNECT) {
            USB_DisconnectCallback(husb);
        }
    }

    return USB_STATUS_OK;
}

USB_HostStateTypeDef USB_Host_GetState(const USB_Handle_t *husb) {
    if (!USB_IsInitialized(husb)) {
        return USB_HOST_ERROR_STATE;
    }

    switch (*husb->appliState) {
        case APPLICATION_IDLE:
            return USB_HOST_IDLE;
        case APPLICATION_START:
            return USB_HOST_DEVICE_ATTACHED;
        case APPLICATION_READY:
            return USB_HOST_CLASS_ACTIVE;
        case APPLICATION_DISCONNECT:
            return USB_HOST_WAIT_FOR_ATTACHMENT;
        default:
            return USB_HOST_ERROR_STATE;
    }
}

bool USB_Host_IsConnected(const USB_Handle_t *husb) {
    if (!USB_IsInitialized(husb)) {
        return false;
    }

    /* APPLICATION_IDLE means nothing has ever attached, so testing only for
       "not disconnected" would report a connection from reset. */
    return (*husb->appliState == APPLICATION_START) || (*husb->appliState == APPLICATION_READY);
}

bool USB_Host_IsReady(const USB_Handle_t *husb) {
    return USB_IsInitialized(husb) && (*husb->appliState == APPLICATION_READY);
}

uint16_t USB_GetConnectedDeviceVID(const USB_Handle_t *husb) {
    if (!USB_Host_IsReady(husb)) {
        return 0U;
    }

    return husb->host->device.DevDesc.idVendor;
}

uint16_t USB_GetConnectedDevicePID(const USB_Handle_t *husb) {
    if (!USB_Host_IsReady(husb)) {
        return 0U;
    }

    return husb->host->device.DevDesc.idProduct;
}
