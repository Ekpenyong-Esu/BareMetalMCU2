/**
 * @file iwdg.c
 * @brief Independent watchdog initialization
 */

#include "iwdg_core.h"
#include "iwdg_timing.h"
#include "log.h"

#include <string.h>

static IWDG_HandleTypeDef s_hiwdg;
static bool s_initialized = false;

/**
 * @brief   Map a HAL status onto a driver status
 */
static IWDG_StatusTypeDef IWDG_ConvertHALStatus(HAL_StatusTypeDef halStatus) {
    switch (halStatus) {
        case HAL_OK:
            return IWDG_OK;
        case HAL_TIMEOUT:
            return IWDG_TIMEOUT;
        case HAL_BUSY:
            return IWDG_ERROR;
        case HAL_ERROR:
            return IWDG_ERROR;
        default:
            return IWDG_ERROR;
    }
}

/**
 * @brief   Program the peripheral and record the outcome
 * @note    HAL_IWDG_Init starts the counter, so a failure here still leaves a
 *          running watchdog; the flag is only raised once the HAL reports
 *          success so callers never refresh a handle that was not programmed.
 */
static IWDG_StatusTypeDef IWDG_Apply(uint32_t prescaler, uint32_t reload) {
    HAL_StatusTypeDef halStatus = HAL_OK;
    uint32_t divider = 0;

    if (IWDG_GetPrescalerDivider(prescaler, &divider) != IWDG_OK) {
        log_error("IWDG: rejected unknown prescaler 0x%08lX", (unsigned long)prescaler);
        return IWDG_INVALID_PARAM;
    }

    if (reload > IWDG_RELOAD_MAX) {
        log_error("IWDG: reload %lu exceeds %u", (unsigned long)reload, IWDG_RELOAD_MAX);
        return IWDG_INVALID_PARAM;
    }

    memset(&s_hiwdg, 0, sizeof(s_hiwdg));
    s_initialized = false;

    s_hiwdg.Instance = IWDG;
    s_hiwdg.Init.Prescaler = prescaler;
    s_hiwdg.Init.Reload = reload;

    halStatus = HAL_IWDG_Init(&s_hiwdg);
    if (halStatus != HAL_OK) {
        log_error("IWDG: HAL_IWDG_Init failed (%d)", (int)halStatus);
        return IWDG_ConvertHALStatus(halStatus);
    }

    s_initialized = true;

    uint32_t timeout_ms = 0U;
    (void)IWDG_CalculateTimeout(prescaler, reload, &timeout_ms);
    log_info("IWDG: started, nominal period %lu ms", (unsigned long)timeout_ms);

    return IWDG_OK;
}

IWDG_StatusTypeDef IWDG_Init(void) {
    log_debug("IWDG: initializing with default configuration");
    return IWDG_Apply(IWDG_DEFAULT_PRESCALER, IWDG_DEFAULT_RELOAD);
}

IWDG_StatusTypeDef IWDG_Init_Custom(const IWDG_ConfigTypeDef *config) {
    if (config == NULL) {
        return IWDG_INVALID_PARAM;
    }

    log_debug("IWDG: initializing with custom configuration");
    return IWDG_Apply(config->Prescaler, config->Reload);
}

IWDG_StatusTypeDef IWDG_Init_TimeoutMs(uint32_t timeout_ms) {
    uint32_t prescaler = 0U;
    uint32_t reload = 0U;
    IWDG_StatusTypeDef status = IWDG_OK;

    status = IWDG_CalculatePrescalerReload(timeout_ms, &prescaler, &reload);
    if (status != IWDG_OK) {
        log_error("IWDG: timeout %lu ms is out of range", (unsigned long)timeout_ms);
        return status;
    }

    log_debug("IWDG: initializing with a %lu ms timeout", (unsigned long)timeout_ms);
    return IWDG_Apply(prescaler, reload);
}

bool IWDG_IsInitialized(void) {
    return s_initialized;
}

IWDG_HandleTypeDef *IWDG_GetHandle(void) {
    return s_initialized ? &s_hiwdg : NULL;
}

IWDG_StatusTypeDef IWDG_GetConfig(IWDG_ConfigTypeDef *config) {
    if (config == NULL) {
        return IWDG_INVALID_PARAM;
    }

    if (!s_initialized) {
        return IWDG_NOT_READY;
    }

    config->Prescaler = s_hiwdg.Init.Prescaler;
    config->Reload = s_hiwdg.Init.Reload;

    return IWDG_OK;
}

IWDG_StatusTypeDef IWDG_GetTimeoutMs(uint32_t *timeout_ms) {
    if (timeout_ms == NULL) {
        return IWDG_INVALID_PARAM;
    }

    if (!s_initialized) {
        return IWDG_NOT_READY;
    }

    return IWDG_CalculateTimeout(s_hiwdg.Init.Prescaler, s_hiwdg.Init.Reload, timeout_ms);
}

const char *IWDG_GetStatusString(IWDG_StatusTypeDef status) {
    switch (status) {
        case IWDG_OK:
            return "IWDG_OK";
        case IWDG_ERROR:
            return "IWDG_ERROR";
        case IWDG_TIMEOUT:
            return "IWDG_TIMEOUT";
        case IWDG_INVALID_PARAM:
            return "IWDG_INVALID_PARAM";
        case IWDG_NOT_READY:
            return "IWDG_NOT_READY";
        case IWDG_NOT_SUPPORTED:
            return "IWDG_NOT_SUPPORTED";
        default:
            return "UNKNOWN_STATUS";
    }
}
