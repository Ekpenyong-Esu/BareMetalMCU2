/**
 * @file usb.c
 * @brief USB stack lifetime and operating mode
 */

#include "usb_core.h"
#include "usb_events.h"
#include "log.h"
#include <string.h>

USB_StatusTypeDef USB_ConvertHostStatus(USBH_StatusTypeDef status) {
    switch (status) {
        case USBH_OK:
            return USB_STATUS_OK;
        case USBH_BUSY:
            return USB_STATUS_BUSY;
        case USBH_NOT_SUPPORTED:
            return USB_STATUS_NOT_SUPPORTED;
        default:
            return USB_STATUS_ERROR;
    }
}

USB_StatusTypeDef USB_Init(USB_Handle_t *husb, USBH_HandleTypeDef *host,
                           ApplicationTypeDef *appliState, const USB_ConfigTypeDef *config) {
    if (husb == NULL || host == NULL || appliState == NULL || config == NULL) {
        return USB_STATUS_ERROR;
    }

    if (config->mode != USB_MODE_HOST) {
        return USB_STATUS_NOT_SUPPORTED;
    }

    if (husb->initialized) {
        return USB_STATUS_OK;
    }

    memset(husb, 0, sizeof(*husb));
    husb->host = host;
    husb->appliState = appliState;
    husb->mode = config->mode;
    /* Start from whatever the library already reported so a device attached
       before USB_Init does not produce a spurious connect edge. */
    husb->lastState = *appliState;

    if (USB_Events_Attach(husb) != USB_STATUS_OK) {
        return USB_STATUS_ERROR;
    }

    husb->initialized = true;

    log_debug("USB: host handle bound");

    return USB_STATUS_OK;
}

USB_StatusTypeDef USB_DeInit(USB_Handle_t *husb) {
    USB_StatusTypeDef status = USB_STATUS_ERROR;

    if (husb == NULL) {
        return USB_STATUS_ERROR;
    }

    if (!husb->initialized) {
        return USB_STATUS_OK;
    }

    status = USB_ConvertHostStatus(USBH_DeInit(husb->host));
    if (status != USB_STATUS_OK) {
        return status;
    }

    USB_Events_Detach(husb);
    husb->initialized = false;

    return USB_STATUS_OK;
}

bool USB_IsInitialized(const USB_Handle_t *husb) {
    return (husb != NULL) && husb->initialized;
}

USB_StatusTypeDef USB_Start(USB_Handle_t *husb) {
    if (!USB_IsInitialized(husb)) {
        return USB_STATUS_NOT_READY;
    }

    return USB_ConvertHostStatus(USBH_Start(husb->host));
}

USB_StatusTypeDef USB_Stop(USB_Handle_t *husb) {
    if (!USB_IsInitialized(husb)) {
        return USB_STATUS_NOT_READY;
    }

    return USB_ConvertHostStatus(USBH_Stop(husb->host));
}

USB_StatusTypeDef USB_SetOperationMode(USB_Handle_t *husb, USB_OperationModeTypeDef mode) {
    if (husb == NULL) {
        return USB_STATUS_ERROR;
    }

    /* Accepting an unsupported mode here would make every later call report
       NOT_SUPPORTED with no way back. */
    if (mode != USB_MODE_HOST) {
        return USB_STATUS_NOT_SUPPORTED;
    }

    husb->mode = mode;

    return USB_STATUS_OK;
}

USB_OperationModeTypeDef USB_GetOperationMode(const USB_Handle_t *husb) {
    return (husb != NULL) ? husb->mode : USB_MODE_HOST;
}
