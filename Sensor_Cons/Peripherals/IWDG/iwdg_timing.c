/**
 * @file iwdg_timing.c
 * @brief Prescaler, reload and timeout arithmetic
 */

#include "iwdg_timing.h"

static const uint32_t s_prescalers[] = {
    IWDG_PRESCALER_4, IWDG_PRESCALER_8, IWDG_PRESCALER_16, IWDG_PRESCALER_32,
    IWDG_PRESCALER_64, IWDG_PRESCALER_128, IWDG_PRESCALER_256
};

static const uint32_t s_dividers[] = { 4U, 8U, 16U, 32U, 64U, 128U, 256U };

#define IWDG_PRESCALER_COUNT   (sizeof(s_dividers) / sizeof(s_dividers[0]))

IWDG_StatusTypeDef IWDG_GetPrescalerDivider(uint32_t prescaler, uint32_t *divider)
{
    if (divider == NULL) {
        return IWDG_INVALID_PARAM;
    }

    for (uint32_t i = 0U; i < IWDG_PRESCALER_COUNT; i++) {
        if (s_prescalers[i] == prescaler) {
            *divider = s_dividers[i];
            return IWDG_OK;
        }
    }

    return IWDG_INVALID_PARAM;
}

IWDG_StatusTypeDef IWDG_CalculateTimeout(uint32_t prescaler, uint32_t reload,
                                         uint32_t *timeout_ms)
{
    uint32_t divider;
    IWDG_StatusTypeDef status;

    if (timeout_ms == NULL) {
        return IWDG_INVALID_PARAM;
    }

    if (reload > IWDG_RELOAD_MAX) {
        return IWDG_INVALID_PARAM;
    }

    status = IWDG_GetPrescalerDivider(prescaler, &divider);
    if (status != IWDG_OK) {
        return status;
    }

    /* Largest product is 4096 * 256 * 1000, which fits in 32 bits. */
    *timeout_ms = ((reload + 1U) * divider * IWDG_MSEC_PER_SEC) / IWDG_LSI_FREQ;

    return IWDG_OK;
}

IWDG_StatusTypeDef IWDG_CalculatePrescalerReload(uint32_t timeout_ms,
                                                 uint32_t *prescaler, uint32_t *reload)
{
    if ((prescaler == NULL) || (reload == NULL)) {
        return IWDG_INVALID_PARAM;
    }

    if ((timeout_ms == 0U) || (timeout_ms > IWDG_TIMEOUT_MAX_MS)) {
        return IWDG_INVALID_PARAM;
    }

    /* Smallest divider that fits gives the finest resolution. */
    for (uint32_t i = 0U; i < IWDG_PRESCALER_COUNT; i++) {
        uint32_t divisor = s_dividers[i] * IWDG_MSEC_PER_SEC;
        uint32_t ticks = ((timeout_ms * IWDG_LSI_FREQ) + divisor - 1U) / divisor;

        if ((ticks >= 1U) && ((ticks - 1U) <= IWDG_RELOAD_MAX)) {
            *prescaler = s_prescalers[i];
            *reload = ticks - 1U;
            return IWDG_OK;
        }
    }

    return IWDG_INVALID_PARAM;
}
