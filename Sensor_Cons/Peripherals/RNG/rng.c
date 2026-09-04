/**
 * @file rng.c
 * @brief RNG peripheral lifetime, health and status reporting
 */

#include "rng_core.h"
#include "log.h"

static RNG_HandleTypeDef s_hrng;
static bool s_initialized = false;

/**
 * @brief Discard one word so a stale or suspect sample is not handed out
 */
static void RNG_FlushWord(void) {
    uint32_t discarded = 0;

    (void)HAL_RNG_GenerateRandomNumber(&s_hrng, &discarded);
    (void)discarded;
}

RNG_StatusTypeDef RNG_Init(void) {
    log_debug("RNG: Initializing Random Number Generator");

    if (s_initialized) {
        return RNG_OK;
    }

    __HAL_RCC_RNG_CLK_ENABLE();

    s_hrng.Instance = RNG;

    if (HAL_RNG_Init(&s_hrng) != HAL_OK) {
        __HAL_RCC_RNG_CLK_DISABLE();
        return RNG_ERROR;
    }

    s_initialized = true;

    RNG_StatusTypeDef status = RNG_GetErrorStatus();

    if (status != RNG_OK) {
        const char *statusStr = RNG_GetStatusString(status);
        log_error("RNG: %s at startup", statusStr);
        (void)RNG_DeInit();
        return status;
    }

    log_debug("RNG: Random Number Generator initialized successfully");

    return RNG_OK;
}

RNG_StatusTypeDef RNG_DeInit(void) {
    if (!s_initialized) {
        return RNG_OK;
    }

    HAL_StatusTypeDef halStatus = HAL_RNG_DeInit(&s_hrng);

    __HAL_RCC_RNG_CLK_DISABLE();
    s_initialized = false;

    return (halStatus == HAL_OK) ? RNG_OK : RNG_ERROR;
}

bool RNG_IsInitialized(void) {
    return s_initialized;
}

RNG_HandleTypeDef *RNG_GetHandle(void) {
    return s_initialized ? &s_hrng : NULL;
}

bool RNG_IsReady(void) {
    /* Reading SR before HAL_RNG_Init would dereference a NULL Instance. */
    if (!s_initialized) {
        return false;
    }

    return (__HAL_RNG_GET_FLAG(&s_hrng, RNG_FLAG_DRDY) != 0U);
}

RNG_StatusTypeDef RNG_GetErrorStatus(void) {
    if (!s_initialized) {
        return RNG_NOT_READY;
    }

    /* A seed error invalidates the data register; a clock error does not, but
       it means the 48 MHz source is out of spec. */
    if (__HAL_RNG_GET_FLAG(&s_hrng, RNG_FLAG_SECS)) {
        return RNG_SEED_ERROR;
    }

    if (__HAL_RNG_GET_FLAG(&s_hrng, RNG_FLAG_CECS)) {
        return RNG_CLOCK_ERROR;
    }

    return RNG_OK;
}

bool RNG_HasError(void) {
    RNG_StatusTypeDef status = RNG_GetErrorStatus();

    return (status == RNG_SEED_ERROR) || (status == RNG_CLOCK_ERROR);
}

RNG_StatusTypeDef RNG_ClearErrors(void) {
    if (!s_initialized) {
        return RNG_NOT_READY;
    }

    /* SECS and CECS are read-only mirrors; only the SEIS/CEIS interrupt status
       bits are writable, and __HAL_RNG_CLEAR_FLAG is an empty macro. */
    s_hrng.Instance->SR &= ~(RNG_SR_SEIS | RNG_SR_CEIS);

    /* The reference manual requires an RNGEN off/on cycle plus a discarded
       word before the generator can be trusted again. */
    __HAL_RNG_DISABLE(&s_hrng);
    __HAL_RNG_ENABLE(&s_hrng);
    RNG_FlushWord();

    return RNG_GetErrorStatus();
}

const char *RNG_GetStatusString(RNG_StatusTypeDef status) {
    switch (status) {
        case RNG_OK:
            return "RNG_OK";
        case RNG_ERROR:
            return "RNG_ERROR";
        case RNG_TIMEOUT:
            return "RNG_TIMEOUT";
        case RNG_CLOCK_ERROR:
            return "RNG_CLOCK_ERROR";
        case RNG_SEED_ERROR:
            return "RNG_SEED_ERROR";
        case RNG_BUSY:
            return "RNG_BUSY";
        case RNG_NOT_READY:
            return "RNG_NOT_READY";
        default:
            return "UNKNOWN_STATUS";
    }
}
