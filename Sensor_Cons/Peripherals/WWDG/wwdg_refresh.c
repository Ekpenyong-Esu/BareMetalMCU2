/**
 * @file wwdg_refresh.c
 * @brief Counter refresh and window state of the Window Watchdog
 */

#include "wwdg_refresh.h"
#include "wwdg_core.h"

uint32_t WWDG_GetCounter(void) {
    const WWDG_HandleTypeDef *handle = WWDG_GetHandle();

    if (handle == NULL) {
        return 0U;
    }

    return (handle->Instance->CR & WWDG_CR_T);
}

uint32_t WWDG_GetWindow(void) {
    const WWDG_HandleTypeDef *handle = WWDG_GetHandle();

    if (handle == NULL) {
        return 0U;
    }

    return (handle->Instance->CFR & WWDG_CFR_W);
}

bool WWDG_IsInWindow(void) {
    uint32_t counter = 0;

    if (!WWDG_IsInitialized()) {
        return false;
    }

    counter = WWDG_GetCounter();

    /* A reset is generated when the counter is written while it is still above
       the window value; below WWDG_COUNTER_MIN the reset has already happened. */
    return (counter >= WWDG_COUNTER_MIN) && (counter <= WWDG_GetWindow());
}

WWDG_StatusTypeDef WWDG_Refresh(void) {
    WWDG_HandleTypeDef *handle = WWDG_GetHandle();

    if (handle == NULL) {
        return WWDG_NOT_READY;
    }

    if (!WWDG_IsInWindow()) {
        return WWDG_WINDOW_ERROR;
    }

    return (HAL_WWDG_Refresh(handle) == HAL_OK) ? WWDG_OK : WWDG_ERROR;
}

WWDG_StatusTypeDef WWDG_RefreshWithCounter(uint32_t counter) {
    WWDG_HandleTypeDef *handle = WWDG_GetHandle();

    if (counter < WWDG_COUNTER_MIN || counter > WWDG_COUNTER_MAX) {
        return WWDG_INVALID_PARAM;
    }

    if (handle == NULL) {
        return WWDG_NOT_READY;
    }

    if (counter < WWDG_GetWindow()) {
        /* The next refresh could never fall inside the window again. */
        return WWDG_INVALID_PARAM;
    }

    if (!WWDG_IsInWindow()) {
        return WWDG_WINDOW_ERROR;
    }

    /* Stored in the handle so every later WWDG_Refresh() reloads the same value. */
    handle->Init.Counter = counter;

    return (HAL_WWDG_Refresh(handle) == HAL_OK) ? WWDG_OK : WWDG_ERROR;
}
