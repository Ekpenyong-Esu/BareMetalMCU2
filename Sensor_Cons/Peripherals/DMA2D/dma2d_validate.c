/**
 * @file dma2d_validate.c
 * @brief Parameter validation for the DMA2D driver
 */

#include "dma2d_validate.h"
#include "log.h"

/* Private data --------------------------------------------------------------*/

/** Accepted operating modes. */
static const uint32_t s_validModes[] = {DMA2D_MODE_R2M, DMA2D_MODE_M2M, DMA2D_MODE_M2M_PFC,
                                        DMA2D_MODE_M2M_BLEND};

/** Accepted output colour formats. */
static const uint32_t s_validOutputFormats[] = {DMA2D_FORMAT_ARGB8888, DMA2D_FORMAT_RGB888,
                                                DMA2D_FORMAT_RGB565, DMA2D_FORMAT_ARGB1555,
                                                DMA2D_FORMAT_ARGB4444};

/** Accepted input colour formats. */
static const uint32_t s_validInputFormats[] = {DMA2D_INPUT_ARGB8888, DMA2D_INPUT_RGB888,
                                               DMA2D_INPUT_RGB565, DMA2D_INPUT_ARGB1555,
                                               DMA2D_INPUT_ARGB4444};

/** Accepted alpha modes. */
static const uint32_t s_validAlphaModes[] = {DMA2D_ALPHA_NO_MODIF, DMA2D_ALPHA_REPLACE,
                                             DMA2D_ALPHA_COMBINE};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Test membership of a value in a table of accepted values
 * @param value  Value to look for
 * @param table  Table of accepted values
 * @param count  Number of entries in @p table
 * @return true when @p value appears in @p table
 */
static bool DMA2D_IsOneOf(uint32_t value, const uint32_t *table, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (table[i] == value) {
            return true;
        }
    }
    return false;
}

/** Convenience wrapper so call sites do not repeat the element count. */
#define DMA2D_IS_ONE_OF(value, table)                                                              \
    DMA2D_IsOneOf((value), (table), sizeof(table) / sizeof((table)[0]))

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef DMA2D_ValidateConfig(const DMA2D_Config *config) {
    if (config == NULL) {
        log_error("DMA2D configuration is NULL");
        return HAL_ERROR;
    }

    if (!DMA2D_IS_ONE_OF(config->mode, s_validModes)) {
        log_error("Invalid DMA2D mode: %lu", (unsigned long)config->mode);
        return HAL_ERROR;
    }

    if (!DMA2D_IS_ONE_OF(config->color_mode, s_validOutputFormats)) {
        log_error("Invalid DMA2D color mode: %lu", (unsigned long)config->color_mode);
        return HAL_ERROR;
    }

    if (config->output_offset > DMA2D_MAX_OFFSET) {
        log_error("Invalid DMA2D output offset: %lu", (unsigned long)config->output_offset);
        return HAL_ERROR;
    }

    /* The colour components are only used to build the register-to-memory colour. */
    if (config->mode == DMA2D_MODE_R2M) {
        if (config->red_value > DMA2D_CHANNEL_MAX || config->green_value > DMA2D_CHANNEL_MAX ||
            config->blue_value > DMA2D_CHANNEL_MAX || config->alpha_value > DMA2D_CHANNEL_MAX) {
            log_error("Invalid DMA2D color components: R=%lu, G=%lu, B=%lu, A=%lu",
                      (unsigned long)config->red_value, (unsigned long)config->green_value,
                      (unsigned long)config->blue_value, (unsigned long)config->alpha_value);
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

HAL_StatusTypeDef DMA2D_ValidateLayerConfig(const DMA2D_LayerConfig *layer_config) {
    if (layer_config == NULL) {
        log_error("DMA2D layer configuration is NULL");
        return HAL_ERROR;
    }

    if (!DMA2D_IS_ONE_OF(layer_config->input_color_mode, s_validInputFormats)) {
        log_error("Invalid DMA2D input color mode: %lu",
                  (unsigned long)layer_config->input_color_mode);
        return HAL_ERROR;
    }

    if (!DMA2D_IS_ONE_OF(layer_config->input_alpha_mode, s_validAlphaModes)) {
        log_error("Invalid DMA2D alpha mode: %lu", (unsigned long)layer_config->input_alpha_mode);
        return HAL_ERROR;
    }

    if (layer_config->input_alpha > DMA2D_CHANNEL_MAX) {
        log_error("Invalid DMA2D alpha value: %lu", (unsigned long)layer_config->input_alpha);
        return HAL_ERROR;
    }

    if (layer_config->input_offset > DMA2D_MAX_OFFSET) {
        log_error("Invalid DMA2D input offset: %lu", (unsigned long)layer_config->input_offset);
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef DMA2D_ValidateLayer(uint32_t layer) {
    if (layer != DMA2D_FOREGROUND_LAYER && layer != DMA2D_BACKGROUND_LAYER) {
        log_error("Invalid DMA2D layer: %lu", (unsigned long)layer);
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef DMA2D_ValidateTransfer(uint32_t mode, const uint32_t *pSrc, const uint32_t *pDst,
                                         uint32_t width, uint32_t height) {
    if (pDst == NULL) {
        log_error("DMA2D destination buffer is NULL");
        return HAL_ERROR;
    }

    if (width == 0U || width > DMA2D_MAX_WIDTH) {
        log_error("Invalid DMA2D width: %lu", (unsigned long)width);
        return HAL_ERROR;
    }

    if (height == 0U || height > DMA2D_MAX_HEIGHT) {
        log_error("Invalid DMA2D height: %lu", (unsigned long)height);
        return HAL_ERROR;
    }

    /* Every mode except register-to-memory reads from a source buffer. */
    if (mode != DMA2D_R2M && pSrc == NULL) {
        log_error("DMA2D source buffer is NULL for M2M operation");
        return HAL_ERROR;
    }

    return HAL_OK;
}
