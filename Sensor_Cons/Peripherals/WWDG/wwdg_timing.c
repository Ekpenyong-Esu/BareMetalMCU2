/**
 * @file wwdg_timing.c
 * @brief Prescaler decoding and timeout arithmetic for the Window Watchdog
 */

#include "wwdg_timing.h"

#define WWDG_HZ_PER_MHZ 1000000U

WWDG_StatusTypeDef WWDG_GetPrescalerDivider(uint32_t prescaler, uint32_t *divider) {
    if (divider == NULL) {
        return WWDG_INVALID_PARAM;
    }

    switch (prescaler) {
        case WWDG_PRESCALER_1:
            *divider = 1U;
            break;
        case WWDG_PRESCALER_2:
            *divider = 2U;
            break;
        case WWDG_PRESCALER_4:
            *divider = 4U;
            break;
        case WWDG_PRESCALER_8:
            *divider = 8U;
            break;
        default:
            return WWDG_INVALID_PARAM;
    }

    return WWDG_OK;
}

WWDG_StatusTypeDef WWDG_CalculateTimeout(uint32_t prescaler, uint32_t counter, uint32_t window,
                                         uint32_t *minTimeout, uint32_t *maxTimeout) {
    WWDG_StatusTypeDef status = WWDG_OK;
    uint32_t divider = 0;
    uint32_t pclk1Mhz = 0;

    if (counter < WWDG_COUNTER_MIN || counter > WWDG_COUNTER_MAX || window < WWDG_WINDOW_MIN ||
        window > counter) {
        return WWDG_INVALID_PARAM;
    }

    status = WWDG_GetPrescalerDivider(prescaler, &divider);
    if (status != WWDG_OK) {
        return status;
    }

    pclk1Mhz = HAL_RCC_GetPCLK1Freq() / WWDG_HZ_PER_MHZ;
    if (pclk1Mhz == 0U) {
        return WWDG_ERROR;
    }

    /* Dividing the clock down to MHz first keeps the tick product inside 32 bits. */
    if (minTimeout != NULL) {
        *minTimeout = ((counter - window) * WWDG_TICKS_PER_COUNT * divider) / pclk1Mhz;
    }

    if (maxTimeout != NULL) {
        /* The reset fires as the counter rolls from WWDG_COUNTER_MIN down to 0x3F. */
        uint32_t ticksToReset = (counter - WWDG_COUNTER_MIN) + 1U;

        *maxTimeout = (ticksToReset * WWDG_TICKS_PER_COUNT * divider) / pclk1Mhz;
    }

    return WWDG_OK;
}
