/**
 * @file    seg_font.h
 * @brief   Seven-segment glyph patterns
 * @details Pure lookup layer: maps values and characters to segment bit
 *          patterns. Knows nothing about handles, hardware or backends.
 */

#ifndef SEG_FONT_H
#define SEG_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @defgroup SEG_Patterns Segment bit patterns (bit0=A .. bit6=G, bit7=DP)
 * @{
 */
#define SEG_PATTERN_0 0x3F     /**< 0: A,B,C,D,E,F */
#define SEG_PATTERN_1 0x06     /**< 1: B,C */
#define SEG_PATTERN_2 0x5B     /**< 2: A,B,D,E,G */
#define SEG_PATTERN_3 0x4F     /**< 3: A,B,C,D,G */
#define SEG_PATTERN_4 0x66     /**< 4: B,C,F,G */
#define SEG_PATTERN_5 0x6D     /**< 5: A,C,D,F,G */
#define SEG_PATTERN_6 0x7D     /**< 6: A,C,D,E,F,G */
#define SEG_PATTERN_7 0x07     /**< 7: A,B,C */
#define SEG_PATTERN_8 0x7F     /**< 8: A,B,C,D,E,F,G */
#define SEG_PATTERN_9 0x6F     /**< 9: A,B,C,D,F,G */
#define SEG_PATTERN_A 0x77     /**< A: A,B,C,E,F,G */
#define SEG_PATTERN_B 0x7C     /**< b: C,D,E,F,G */
#define SEG_PATTERN_C 0x39     /**< C: A,D,E,F */
#define SEG_PATTERN_D 0x5E     /**< d: B,C,D,E,G */
#define SEG_PATTERN_E 0x79     /**< E: A,D,E,F,G */
#define SEG_PATTERN_F 0x71     /**< F: A,E,F,G */
#define SEG_PATTERN_MINUS 0x40 /**< -: G */
#define SEG_PATTERN_BLANK 0x00 /**< Blank */
#define SEG_PATTERN_DP 0x80    /**< Decimal point only */
/** @} */

/** Largest value Seg_GetPattern() can render: one hex digit */
#define SEG_DIGIT_VALUE_MAX 15U

/**
 * @brief   Get the segment pattern for a nibble value
 * @param   value Digit value (0-15); anything larger yields a blank
 * @retval  uint8_t Segment pattern
 */
uint8_t Seg_GetPattern(uint8_t value);

/**
 * @brief   Get the segment pattern for a character
 * @param   character Character (0-9, A-F, and a handful of renderable letters)
 * @retval  uint8_t Segment pattern, blank when the glyph is unavailable
 */
uint8_t Seg_CharToPattern(char character);

#ifdef __cplusplus
}
#endif

#endif /* SEG_FONT_H */
