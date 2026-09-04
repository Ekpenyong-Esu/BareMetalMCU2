/**
 * @file wwdg.c
 * @brief Window Watchdog lifetime, handle ownership and reset-source reporting
 */

#include "wwdg_core.h"
#include "wwdg_ewi.h"
#include "wwdg_timing.h"
#include "log.h"

static WWDG_HandleTypeDef s_hwwdg;
static bool s_initialized = false;

static WWDG_StatusTypeDef WWDG_ValidateConfig(const WWDG_ConfigTypeDef *config) {
    uint32_t divider = 0;

    if (config->Counter < WWDG_COUNTER_MIN || config->Counter > WWDG_COUNTER_MAX) {
        return WWDG_INVALID_PARAM;
    }

    /* Window == Counter is legal and means "refresh allowed immediately". */
    if (config->Window < WWDG_WINDOW_MIN || config->Window > config->Counter) {
        return WWDG_INVALID_PARAM;
    }

    if (config->EWIMode != WWDG_EWI_ENABLE && config->EWIMode != WWDG_EWI_DISABLE) {
        return WWDG_INVALID_PARAM;
    }

    return WWDG_GetPrescalerDivider(config->Prescaler, &divider);
}

WWDG_StatusTypeDef WWDG_GetDefaultConfig(WWDG_ConfigTypeDef *config) {
    if (config == NULL) {
        return WWDG_INVALID_PARAM;
    }

    config->Prescaler = WWDG_DEFAULT_PRESCALER;
    config->Window = WWDG_DEFAULT_WINDOW;
    config->Counter = WWDG_DEFAULT_COUNTER;
    config->EWIMode = WWDG_EWI_DISABLE;

    return WWDG_OK;
}

WWDG_StatusTypeDef WWDG_Init_Custom(const WWDG_ConfigTypeDef *config) {
    WWDG_StatusTypeDef status = WWDG_OK;

    if (config == NULL) {
        return WWDG_INVALID_PARAM;
    }

    if (s_initialized) {
        return WWDG_OK;
    }

    status = WWDG_ValidateConfig(config);
    if (status != WWDG_OK) {
        log_error("WWDG: rejected configuration");
        return status;
    }

    __HAL_RCC_WWDG_CLK_ENABLE();

    s_hwwdg.Instance = WWDG;
    s_hwwdg.Init.Prescaler = config->Prescaler;
    s_hwwdg.Init.Window = config->Window;
    s_hwwdg.Init.Counter = config->Counter;
    s_hwwdg.Init.EWIMode = config->EWIMode;

    if (HAL_WWDG_Init(&s_hwwdg) != HAL_OK) {
        __HAL_RCC_WWDG_CLK_DISABLE();
        s_hwwdg.Instance = NULL;
        log_error("WWDG: HAL init failed");
        return WWDG_ERROR;
    }

    s_initialized = true;

    log_debug("WWDG: armed, counter 0x%02lX window 0x%02lX", (unsigned long)config->Counter,
              (unsigned long)config->Window);

    return WWDG_OK;
}

WWDG_StatusTypeDef WWDG_Init(void) {
    WWDG_ConfigTypeDef config;

    (void)WWDG_GetDefaultConfig(&config);

    return WWDG_Init_Custom(&config);
}

WWDG_StatusTypeDef WWDG_DeInit(void) {
    if (!s_initialized) {
        return WWDG_OK;
    }

    (void)WWDG_DisableEWI();

    /* The APB1 clock stays on: gating it freezes the counter mid-window and it
       would resume from that stale value the next time the clock is enabled. */
    s_hwwdg.Instance = NULL;
    s_initialized = false;

    log_warning("WWDG: driver released, hardware watchdog still running");

    return WWDG_NOT_SUPPORTED;
}

bool WWDG_IsInitialized(void) {
    return s_initialized;
}

WWDG_HandleTypeDef *WWDG_GetHandle(void) {
    return s_initialized ? &s_hwwdg : NULL;
}

bool WWDG_WasResetSource(void) {
    return (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET);
}

void WWDG_ClearResetFlag(void) {
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

const char *WWDG_GetStatusString(WWDG_StatusTypeDef status) {
    switch (status) {
        case WWDG_OK:
            return "WWDG_OK";
        case WWDG_ERROR:
            return "WWDG_ERROR";
        case WWDG_TIMEOUT:
            return "WWDG_TIMEOUT";
        case WWDG_INVALID_PARAM:
            return "WWDG_INVALID_PARAM";
        case WWDG_WINDOW_ERROR:
            return "WWDG_WINDOW_ERROR";
        case WWDG_NOT_READY:
            return "WWDG_NOT_READY";
        case WWDG_NOT_SUPPORTED:
            return "WWDG_NOT_SUPPORTED";
        default:
            return "UNKNOWN_STATUS";
    }
}
