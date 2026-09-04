/**
 * @file usb_events.c
 * @brief Default (empty) application hooks and host-library callback routing
 * @details The host library reports events with only its own handle, so a
 *          small table maps each attached USBH_HandleTypeDef back to the
 *          driver handle the application owns. One slot per OTG core.
 */

#include "usb_events.h"
#include "log.h"

static USB_Handle_t *s_hosts[USB_MAX_HOSTS] = {NULL};

/* Callback routing ----------------------------------------------------------*/

USB_StatusTypeDef USB_Events_Attach(USB_Handle_t *husb) {
    USB_Handle_t **freeSlot = NULL;

    if (husb == NULL || husb->host == NULL) {
        return USB_STATUS_ERROR;
    }

    for (uint32_t i = 0; i < USB_MAX_HOSTS; i++) {
        if (s_hosts[i] == husb) {
            return USB_STATUS_OK; /* Already attached */
        }
        if (s_hosts[i] == NULL && freeSlot == NULL) {
            freeSlot = &s_hosts[i];
        }
    }

    if (freeSlot == NULL) {
        return USB_STATUS_ERROR;
    }

    *freeSlot = husb;
    return USB_STATUS_OK;
}

void USB_Events_Detach(USB_Handle_t *husb) {
    for (uint32_t i = 0; i < USB_MAX_HOSTS; i++) {
        if (s_hosts[i] == husb) {
            s_hosts[i] = NULL;
        }
    }
}

USB_Handle_t *USB_Events_Resolve(const USBH_HandleTypeDef *phost) {
    for (uint32_t i = 0; i < USB_MAX_HOSTS; i++) {
        if (s_hosts[i] != NULL && s_hosts[i]->host == phost) {
            return s_hosts[i];
        }
    }
    return NULL;
}

/* Application hooks ---------------------------------------------------------*/

__weak void USB_ConnectCallback(USB_Handle_t *husb) {
    UNUSED(husb);
}

__weak void USB_DisconnectCallback(USB_Handle_t *husb) {
    UNUSED(husb);
}

__weak void USB_DataReceivedCallback(USB_Handle_t *husb, const uint8_t *data, uint16_t length) {
    UNUSED(husb);
    UNUSED(data);
    UNUSED(length);
}

__weak void USB_TransmitCompleteCallback(USB_Handle_t *husb) {
    UNUSED(husb);
}

/* Halting here would take the whole system down for a recoverable USB fault. */
__weak void USB_ErrorHandler(USB_Handle_t *husb, uint32_t error_code) {
    UNUSED(husb);
    log_error("USB: error %lu", (unsigned long)error_code);
}
