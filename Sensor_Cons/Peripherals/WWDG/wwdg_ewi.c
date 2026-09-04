/**
 * @file wwdg_ewi.c
 * @brief Early wakeup interrupt handling for the Window Watchdog
 */

#include "wwdg_ewi.h"
#include "wwdg_core.h"

static WWDG_EWI_Callback_t s_ewiCallback = NULL;
static bool s_ewiEnabled = false;

WWDG_StatusTypeDef WWDG_RegisterEWICallback(WWDG_EWI_Callback_t callback) {
    s_ewiCallback = callback;

    return WWDG_OK;
}

WWDG_StatusTypeDef WWDG_EnableEWI(void) {
    WWDG_HandleTypeDef *handle = WWDG_GetHandle();

    if (handle == NULL) {
        return WWDG_NOT_READY;
    }

    if (s_ewiEnabled) {
        return WWDG_OK;
    }

    /* Drop a flag latched before the vector was unmasked. */
    __HAL_WWDG_CLEAR_FLAG(handle, WWDG_FLAG_EWIF);
    __HAL_WWDG_ENABLE_IT(handle, WWDG_IT_EWI);

    HAL_NVIC_SetPriority(WWDG_IRQn, WWDG_EWI_IRQ_PRIORITY, WWDG_EWI_IRQ_SUBPRIORITY);
    HAL_NVIC_EnableIRQ(WWDG_IRQn);

    s_ewiEnabled = true;

    return WWDG_OK;
}

WWDG_StatusTypeDef WWDG_DisableEWI(void) {
    HAL_NVIC_DisableIRQ(WWDG_IRQn);
    s_ewiEnabled = false;

    return WWDG_OK;
}

bool WWDG_IsEWIEnabled(void) {
    return s_ewiEnabled;
}

void WWDG_EWI_IRQHandler(void) {
    WWDG_HandleTypeDef *handle = WWDG_GetHandle();

    if (handle == NULL) {
        return;
    }

    HAL_WWDG_IRQHandler(handle);
}

void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg) {
    UNUSED(hwwdg);

    /* Deliberately does not refresh: kicking the watchdog here would make the
       reset unreachable and defeat the whole mechanism. */
    if (s_ewiCallback != NULL) {
        s_ewiCallback();
    }
}
