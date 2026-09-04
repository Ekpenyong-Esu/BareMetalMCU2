/**
 * @file    seg_format.h
 * @brief   Value-to-pattern rendering for seven-segment displays
 * @details Everything here writes segment patterns into the handle's buffer and
 *          then asks the core to push them. No hardware knowledge lives here.
 */

#ifndef SEG_FORMAT_H
#define SEG_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "seg_types.h"

/**
 * @brief   Show a single nibble value at one position
 * @param   handle Display handle
 * @param   position Digit position (0 = leftmost)
 * @param   value Value to display (0-15)
 * @param   showDp Also light the decimal point
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_SetDigit(SegDisplayHandle_t *handle, uint8_t position, uint8_t value, bool showDp);

/**
 * @brief   Write a raw segment pattern at one position
 * @param   handle Display handle
 * @param   position Digit position
 * @param   pattern Raw pattern (bits 0-6 = A-G, bit 7 = DP)
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_SetPattern(SegDisplayHandle_t *handle, uint8_t position, uint8_t pattern);

/**
 * @brief   Write a character at one position
 * @param   handle Display handle
 * @param   position Digit position
 * @param   character Character to display
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_SetChar(SegDisplayHandle_t *handle, uint8_t position, char character);

/**
 * @brief   Display a signed integer, right aligned
 * @param   handle Display handle
 * @param   value Value to display
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DisplayInt(SegDisplayHandle_t *handle, int32_t value);

/**
 * @brief   Display a float with a fixed number of decimals
 * @param   handle Display handle
 * @param   value Value to display
 * @param   decimals Number of decimal places
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DisplayFloat(SegDisplayHandle_t *handle, float value, uint8_t decimals);

/**
 * @brief   Display an unsigned value in hexadecimal, right aligned
 * @param   handle Display handle
 * @param   value Value to display
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DisplayHex(SegDisplayHandle_t *handle, uint32_t value);

/**
 * @brief   Display a string, left aligned
 * @param   handle Display handle
 * @param   str String to display; '.' attaches to the previous digit
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DisplayString(SegDisplayHandle_t *handle, const char *str);

/**
 * @brief   Light every segment on every digit
 * @param   handle Display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Test(SegDisplayHandle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* SEG_FORMAT_H */
