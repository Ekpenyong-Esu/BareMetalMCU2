/**
  ******************************************************************************
  * @file    crc_core.c
  * @brief   Lifecycle, calculation and status for the CRC driver
  ******************************************************************************
  */

#include "crc_core.h"
#include "crc_sw.h"
#include "crc_events.h"
#include "log.h"
#include <string.h>

/** Words staged per hardware pass; bounds the stack cost of byte-wise input */
#define CRC_HW_CHUNK_WORDS          16U

#define CRC_BYTES_PER_WORD          4U

static CRC_HandleTypeDef s_hcrc;
static CRC_Status s_status = {0};
static CRC_Config s_config = {0};

static HAL_StatusTypeDef CRC_ValidateConfig(const CRC_Config *config)
{
    if (config->polynomial == 0U) {
        return HAL_ERROR;
    }

    if (config->input_format > CRC_FORMAT_32BIT || config->output_format > CRC_FORMAT_32BIT) {
        return HAL_ERROR;
    }

    /* The STM32F4 CRC unit is hard-wired to 0x04C11DB7 / 0xFFFFFFFF with no
       reflection. Accepting anything else in hardware mode would silently
       return a checksum the caller did not ask for. */
    if (config->method == CRC_METHOD_HARDWARE) {
        if (config->polynomial != CRC_DEFAULT_POLYNOMIAL ||
            config->init_value != CRC_DEFAULT_INIT_VALUE ||
            config->input_reverse || config->output_reverse) {
            log_error("CRC: hardware unit only supports the default polynomial and init value");
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef CRC_ConfigureHardware(void)
{
    s_hcrc.Instance = CRC;

    return HAL_CRC_Init(&s_hcrc);
}

/**
 * @brief Feed a byte buffer to the hardware unit through an aligned staging
 *        buffer, zero-padding a trailing partial word.
 * @note  Casting the caller's @c uint8_t* straight to @c uint32_t* would be an
 *        unaligned access, and dividing the size by four would drop the tail.
 */
static uint32_t CRC_HardwareBytes(const uint8_t *data, uint32_t size, bool reset)
{
    uint32_t buffer[CRC_HW_CHUNK_WORDS];
    uint32_t result = 0U;
    uint32_t offset = 0U;

    if (reset) {
        __HAL_CRC_DR_RESET(&s_hcrc);
    }

    while (offset < size) {
        uint32_t chunk = size - offset;

        if (chunk > sizeof(buffer)) {
            chunk = sizeof(buffer);
        }

        memset(buffer, 0, sizeof(buffer));
        memcpy(buffer, &data[offset], chunk);

        result = HAL_CRC_Accumulate(&s_hcrc, buffer,
                                    (chunk + CRC_BYTES_PER_WORD - 1U) / CRC_BYTES_PER_WORD);
        offset += chunk;
    }

    return result;
}

static void CRC_ApplyXor(uint32_t *crc)
{
    if (s_config.xor_output) {
        *crc ^= s_config.xor_value;
    }
}

static void CRC_RecordResult(uint32_t crc)
{
    s_status.last_calculated_crc = crc;
    s_status.calculation_count++;
}

static HAL_StatusTypeDef CRC_CheckInput(const void *data, uint32_t size,
                                        const uint32_t *crc, uint32_t maxSize)
{
    if (data == NULL || crc == NULL || size == 0U) {
        CRC_NotifyError(CRC_ERROR_INVALID_PARAM);
        return HAL_ERROR;
    }

    if (size > maxSize) {
        CRC_NotifyError(CRC_ERROR_DATA_SIZE);
        return HAL_ERROR;
    }

    if (!s_status.initialized) {
        CRC_NotifyError(CRC_ERROR_INVALID_PARAM);
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef CRC_Init(const CRC_Config *config)
{
    HAL_StatusTypeDef status;

    log_debug("CRC: Initializing CRC");

    if (config == NULL) {
        CRC_NotifyError(CRC_ERROR_INVALID_PARAM);
        return HAL_ERROR;
    }

    status = CRC_ValidateConfig(config);
    if (status != HAL_OK) {
        CRC_NotifyError(CRC_ERROR_INVALID_PARAM);
        return status;
    }

    memset(&s_status, 0, sizeof(s_status));
    memcpy(&s_config, config, sizeof(s_config));

    if (config->method == CRC_METHOD_HARDWARE) {
        status = CRC_ConfigureHardware();
        if (status != HAL_OK) {
            CRC_NotifyError(CRC_ERROR_HARDWARE);
            return status;
        }
    }

    s_status.initialized = true;
    s_status.current_method = config->method;

    log_debug("CRC: CRC initialized successfully");

    return HAL_OK;
}

HAL_StatusTypeDef CRC_DeInit(void)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (s_status.initialized && s_config.method == CRC_METHOD_HARDWARE) {
        status = HAL_CRC_DeInit(&s_hcrc);
        if (status != HAL_OK) {
            CRC_NotifyError(CRC_ERROR_HARDWARE);
            return status;
        }
    }

    memset(&s_status, 0, sizeof(s_status));
    memset(&s_config, 0, sizeof(s_config));

    return HAL_OK;
}

HAL_StatusTypeDef CRC_Calculate(const uint8_t *data, uint32_t size, uint32_t *crc)
{
    if (CRC_CheckInput(data, size, crc, CRC_MAX_DATA_SIZE) != HAL_OK) {
        return HAL_ERROR;
    }

    if (s_config.method == CRC_METHOD_HARDWARE) {
        *crc = CRC_HardwareBytes(data, size, true);
        s_status.hardware_usage_count++;
    } else {
        *crc = CRC_SoftwareFinalize(&s_config,
                                    CRC_SoftwareUpdate(&s_config, s_config.init_value, data, size));
        s_status.software_usage_count++;
    }

    CRC_ApplyXor(crc);
    CRC_RecordResult(*crc);
    CRC_NotifyComplete(*crc);

    return HAL_OK;
}

HAL_StatusTypeDef CRC_Calculate32(const uint32_t *data, uint32_t size, uint32_t *crc)
{
    if (CRC_CheckInput(data, size, crc, CRC_MAX_DATA_SIZE / CRC_BYTES_PER_WORD) != HAL_OK) {
        return HAL_ERROR;
    }

    if (s_config.method == CRC_METHOD_HARDWARE) {
        __HAL_CRC_DR_RESET(&s_hcrc);
        *crc = HAL_CRC_Accumulate(&s_hcrc, (uint32_t *)(uintptr_t)data, size);
        s_status.hardware_usage_count++;
    } else {
        *crc = CRC_SoftwareFinalize(&s_config,
                                    CRC_SoftwareUpdate32(&s_config, s_config.init_value, data, size));
        s_status.software_usage_count++;
    }

    CRC_ApplyXor(crc);
    CRC_RecordResult(*crc);
    CRC_NotifyComplete(*crc);

    return HAL_OK;
}

HAL_StatusTypeDef CRC_Accumulate(const uint8_t *data, uint32_t size, uint32_t *crc)
{
    if (CRC_CheckInput(data, size, crc, CRC_MAX_DATA_SIZE) != HAL_OK) {
        return HAL_ERROR;
    }

    if (s_config.method == CRC_METHOD_HARDWARE) {
        /* No reset: the hardware DR still holds the previous block's result. */
        *crc = CRC_HardwareBytes(data, size, false);
        s_status.hardware_usage_count++;
    } else {
        /* Continue the polynomial division from the caller's running value
           instead of XOR-ing two independent checksums together. */
        *crc = CRC_SoftwareUpdate(&s_config, *crc, data, size);
        s_status.software_usage_count++;
    }

    CRC_RecordResult(*crc);

    return HAL_OK;
}

HAL_StatusTypeDef CRC_Reset(void)
{
    if (!s_status.initialized) {
        return HAL_ERROR;
    }

    if (s_config.method == CRC_METHOD_HARDWARE) {
        __HAL_CRC_DR_RESET(&s_hcrc);
    }

    return HAL_OK;
}

HAL_StatusTypeDef CRC_GetStatus(CRC_Status *status)
{
    if (status == NULL) {
        return HAL_ERROR;
    }

    memcpy(status, &s_status, sizeof(CRC_Status));

    return HAL_OK;
}

HAL_StatusTypeDef CRC_SetPolynomial(uint32_t polynomial)
{
    if (!s_status.initialized || polynomial == 0U) {
        return HAL_ERROR;
    }

    if (s_config.method == CRC_METHOD_HARDWARE) {
        log_error("CRC: polynomial is fixed in hardware mode");
        return HAL_ERROR;
    }

    s_config.polynomial = polynomial;

    return HAL_OK;
}

HAL_StatusTypeDef CRC_SetInitValue(uint32_t init_value)
{
    if (!s_status.initialized) {
        return HAL_ERROR;
    }

    if (s_config.method == CRC_METHOD_HARDWARE) {
        log_error("CRC: init value is fixed in hardware mode");
        return HAL_ERROR;
    }

    s_config.init_value = init_value;

    return HAL_OK;
}

CRC_HandleTypeDef *CRC_GetHandle(void) { return &s_hcrc; }

void CRC_GetDefaultConfig(CRC_Config *config)
{
    if (config == NULL) {
        return;
    }

    config->method = CRC_METHOD_HARDWARE;
    config->polynomial = CRC_DEFAULT_POLYNOMIAL;
    config->init_value = CRC_DEFAULT_INIT_VALUE;
    config->input_reverse = false;
    config->output_reverse = false;
    config->input_format = CRC_FORMAT_32BIT;
    config->output_format = CRC_FORMAT_32BIT;
    config->xor_output = false;
    config->xor_value = 0U;
}
