/**
 * @file    seg_format.c
 * @brief   Value-to-pattern rendering implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "seg_format.h"

#include "seg_core.h"
#include "seg_font.h"

#include <string.h>

/* Private define ------------------------------------------------------------*/

/** Radix of the decimal and hexadecimal renderers */
#define SEG_BASE_DECIMAL 10
#define SEG_BASE_HEX 16

/** Added before truncating so the scaled float rounds to nearest */
#define SEG_ROUND_HALF 0.5f

/* Private functions ---------------------------------------------------------*/

static void Seg_BlankBuffer(SegDisplayHandle_t *handle) {
    memset(handle->displayBuffer, SEG_PATTERN_BLANK, handle->digitCount);
}

/**
 * @brief   Render @p value right to left starting at @p startPos
 * @retval  int8_t Position immediately left of the most significant digit,
 *          or -1 when the buffer ran out
 */
static int8_t Seg_RenderNumber(SegDisplayHandle_t *handle, uint32_t value, uint32_t base,
                               int8_t startPos) {
    int8_t pos = startPos;

    if (value == 0U) {
        handle->displayBuffer[pos] = SEG_PATTERN_0;
        return (int8_t)(pos - 1);
    }

    while (value > 0U && pos >= 0) {
        handle->displayBuffer[pos] = Seg_GetPattern((uint8_t)(value % base));
        value /= base;
        pos--;
    }

    return pos;
}

/** @brief Replace blanks with zeros from @p from up to the last digit */
static void Seg_PadLeadingZeros(SegDisplayHandle_t *handle, int8_t from) {
    for (int8_t i = from; i < (int8_t)handle->digitCount - 1; i++) {
        if (handle->displayBuffer[i] == SEG_PATTERN_BLANK) {
            handle->displayBuffer[i] = SEG_PATTERN_0;
        }
    }
}

/* Exported functions --------------------------------------------------------*/

SegStatus_t Seg_SetDigit(SegDisplayHandle_t *handle, uint8_t position, uint8_t value, bool showDp) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    if (position >= handle->digitCount || value > SEG_DIGIT_VALUE_MAX) {
        return SEG_INVALID_PARAM;
    }

    uint8_t pattern = Seg_GetPattern(value);
    if (showDp) {
        pattern |= SEG_PATTERN_DP;
    }
    handle->displayBuffer[position] = pattern;

    return Seg_Refresh(handle);
}

SegStatus_t Seg_SetPattern(SegDisplayHandle_t *handle, uint8_t position, uint8_t pattern) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    if (position >= handle->digitCount) {
        return SEG_INVALID_PARAM;
    }

    handle->displayBuffer[position] = pattern;

    return Seg_Refresh(handle);
}

SegStatus_t Seg_SetChar(SegDisplayHandle_t *handle, uint8_t position, char character) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    if (position >= handle->digitCount) {
        return SEG_INVALID_PARAM;
    }

    handle->displayBuffer[position] = Seg_CharToPattern(character);

    return Seg_Refresh(handle);
}

SegStatus_t Seg_DisplayInt(SegDisplayHandle_t *handle, int32_t value) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    bool negative = (value < 0);
    uint32_t magnitude = negative ? (uint32_t)(-(int64_t)value) : (uint32_t)value;

    Seg_BlankBuffer(handle);

    int8_t pos =
        Seg_RenderNumber(handle, magnitude, SEG_BASE_DECIMAL, (int8_t)(handle->digitCount - 1));

    if (negative && pos >= 0) {
        handle->displayBuffer[pos] = SEG_PATTERN_MINUS;
    }

    if (handle->config.leadingZeros) {
        Seg_PadLeadingZeros(handle, negative ? 1 : 0);
    }

    return Seg_Refresh(handle);
}

SegStatus_t Seg_DisplayHex(SegDisplayHandle_t *handle, uint32_t value) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    Seg_BlankBuffer(handle);

    (void)Seg_RenderNumber(handle, value, SEG_BASE_HEX, (int8_t)(handle->digitCount - 1));

    if (handle->config.leadingZeros) {
        Seg_PadLeadingZeros(handle, 0);
    }

    return Seg_Refresh(handle);
}

SegStatus_t Seg_DisplayFloat(SegDisplayHandle_t *handle, float value, uint8_t decimals) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    if (decimals >= handle->digitCount) {
        decimals = (uint8_t)(handle->digitCount - 1);
    }

    bool negative = (value < 0.0f);
    if (negative) {
        value = -value;
    }

    int32_t multiplier = 1;
    for (uint8_t i = 0; i < decimals; i++) {
        multiplier *= SEG_BASE_DECIMAL;
    }
    int32_t scaled = (int32_t)(value * (float)multiplier + SEG_ROUND_HALF);

    Seg_BlankBuffer(handle);

    /* Render at least as far as the decimal point so trailing zeros survive */
    int8_t decimalPos = (int8_t)(handle->digitCount - 1 - decimals);
    int8_t pos = (int8_t)(handle->digitCount - 1);

    for (int8_t i = pos; i >= 0 && (scaled > 0 || i >= decimalPos); i--) {
        handle->displayBuffer[i] = Seg_GetPattern((uint8_t)(scaled % SEG_BASE_DECIMAL));
        scaled /= SEG_BASE_DECIMAL;
        if (i == decimalPos && decimals > 0U) {
            handle->displayBuffer[i] |= SEG_PATTERN_DP;
        }
        pos = (int8_t)(i - 1);
    }

    if (negative && pos >= 0) {
        handle->displayBuffer[pos] = SEG_PATTERN_MINUS;
    }

    return Seg_Refresh(handle);
}

SegStatus_t Seg_DisplayString(SegDisplayHandle_t *handle, const char *str) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    if (str == NULL) {
        return SEG_INVALID_PARAM;
    }

    Seg_BlankBuffer(handle);

    uint8_t pos = 0;
    while (*str != '\0' && pos < handle->digitCount) {
        if (*str == '.') {
            if (pos > 0) {
                handle->displayBuffer[pos - 1] |= SEG_PATTERN_DP;
            }
        }
        else {
            handle->displayBuffer[pos] = Seg_CharToPattern(*str);
            pos++;
        }
        str++;
    }

    return Seg_Refresh(handle);
}

SegStatus_t Seg_Test(SegDisplayHandle_t *handle) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    memset(handle->displayBuffer, SEG_PATTERN_8 | SEG_PATTERN_DP, handle->digitCount);

    return Seg_Refresh(handle);
}
