/**
 * @file usb.c
 * @brief USB stack lifetime and operating mode
 */

#include "usb_core.h"
#include "log.h"

static USB_OperationModeTypeDef s_mode = USB_MODE_HOST;
static bool s_initialized = false;

USB_StatusTypeDef USB_ConvertHostStatus(USBH_StatusTypeDef status)
{
    switch (status) {
        case USBH_OK:            return USB_STATUS_OK;
        case USBH_BUSY:          return USB_STATUS_BUSY;
        case USBH_NOT_SUPPORTED: return USB_STATUS_NOT_SUPPORTED;
        default:                 return USB_STATUS_ERROR;
    }
}

USB_StatusTypeDef USB_Host_Init(void)
{
    /* MX_USB_HOST_Init registers the CDC class and starts the library. */
    MX_USB_HOST_Init();

    return USB_STATUS_OK;
}

USB_StatusTypeDef USB_Host_DeInit(void)
{
    USBH_StatusTypeDef hostStatus = USBH_DeInit(&hUsbHostHS);

    return USB_ConvertHostStatus(hostStatus);
}

USB_StatusTypeDef USB_Init(const USB_ConfigTypeDef *config)
{
    USB_StatusTypeDef status;

    if (config == NULL) {
        return USB_STATUS_ERROR;
    }

    if (config->mode != USB_MODE_HOST) {
        return USB_STATUS_NOT_SUPPORTED;
    }

    if (s_initialized) {
        return USB_STATUS_OK;
    }

    status = USB_Host_Init();
    if (status != USB_STATUS_OK) {
        return status;
    }

    s_mode = config->mode;
    s_initialized = true;

    log_debug("USB: host stack initialized");

    return USB_STATUS_OK;
}

USB_StatusTypeDef USB_DeInit(void)
{
    USB_StatusTypeDef status;

    if (!s_initialized) {
        return USB_STATUS_OK;
    }

    status = USB_Host_DeInit();
    if (status != USB_STATUS_OK) {
        return status;
    }

    s_initialized = false;

    return USB_STATUS_OK;
}

bool USB_IsInitialized(void)
{
    return s_initialized;
}

USB_StatusTypeDef USB_Start(void)
{
    USBH_StatusTypeDef hostStatus = USBH_OK;

    if (!s_initialized) {
        return USB_STATUS_NOT_READY;
    }

    hostStatus = USBH_Start(&hUsbHostHS);
    return USB_ConvertHostStatus(hostStatus);
}

USB_StatusTypeDef USB_Stop(void)
{
    USBH_StatusTypeDef hostStatus = USBH_OK;

    if (!s_initialized) {
        return USB_STATUS_NOT_READY;
    }

    hostStatus = USBH_Stop(&hUsbHostHS);
    return USB_ConvertHostStatus(hostStatus);
}

USB_StatusTypeDef USB_SetOperationMode(USB_OperationModeTypeDef mode)
{
    /* Accepting an unsupported mode here would make every later call report
       NOT_SUPPORTED with no way back. */
    if (mode != USB_MODE_HOST) {
        return USB_STATUS_NOT_SUPPORTED;
    }

    s_mode = mode;

    return USB_STATUS_OK;
}

USB_OperationModeTypeDef USB_GetOperationMode(void)
{
    return s_mode;
}
