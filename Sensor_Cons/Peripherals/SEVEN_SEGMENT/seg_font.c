/**
 * @file    seg_font.c
 * @brief   Seven-segment glyph lookup implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "seg_font.h"

#include <stddef.h>

/* Private types -------------------------------------------------------------*/

typedef struct {
    char ch;         /**< Source character */
    uint8_t pattern; /**< Segment pattern */
} SegGlyph_t;

/* Private constants ---------------------------------------------------------*/

/** @brief Nibble value to pattern, indexed directly by value */
static const uint8_t digitPatterns[16] = {
    SEG_PATTERN_0, SEG_PATTERN_1, SEG_PATTERN_2, SEG_PATTERN_3, SEG_PATTERN_4, SEG_PATTERN_5,
    SEG_PATTERN_6, SEG_PATTERN_7, SEG_PATTERN_8, SEG_PATTERN_9, SEG_PATTERN_A, SEG_PATTERN_B,
    SEG_PATTERN_C, SEG_PATTERN_D, SEG_PATTERN_E, SEG_PATTERN_F};

/** @brief Glyphs that exist in exactly one case, plus symbols */
static const SegGlyph_t exactGlyphs[] = {
    {'n', 0x54}, /* n: C,E,G */
    {'r', 0x50}, /* r: E,G */
    {'t', 0x78}, /* t: D,E,F,G */
    {'y', 0x6E}, /* y: B,C,D,F,G */
    {'-', SEG_PATTERN_MINUS},
    {'_', 0x08}, /* _: D */
    {' ', SEG_PATTERN_BLANK},
    {'.', SEG_PATTERN_DP},
};

/** @brief Glyphs rendered identically for upper and lower case */
static const SegGlyph_t foldedGlyphs[] = {
    {'A', SEG_PATTERN_A}, {'B', SEG_PATTERN_B}, {'C', SEG_PATTERN_C},
    {'D', SEG_PATTERN_D}, {'E', SEG_PATTERN_E}, {'F', SEG_PATTERN_F},
    {'G', SEG_PATTERN_9}, {'H', 0x76}, /* H: B,C,E,F,G */
    {'I', SEG_PATTERN_1}, {'L', 0x38}, /* L: D,E,F */
    {'O', SEG_PATTERN_0}, {'P', 0x73}, /* P: A,B,E,F,G */
    {'S', SEG_PATTERN_5}, {'U', 0x3E}, /* U: B,C,D,E,F */
};

/* Private functions ---------------------------------------------------------*/

static uint8_t Seg_LookupGlyph(const SegGlyph_t *table, size_t count, char character,
                               uint8_t fallback) {
    for (size_t i = 0; i < count; i++) {
        if (table[i].ch == character) {
            return table[i].pattern;
        }
    }
    return fallback;
}

/* Exported functions --------------------------------------------------------*/

uint8_t Seg_GetPattern(uint8_t value) {
    if (value > SEG_DIGIT_VALUE_MAX) {
        return SEG_PATTERN_BLANK;
    }
    return digitPatterns[value];
}

uint8_t Seg_CharToPattern(char character) {
    if (character >= '0' && character <= '9') {
        return digitPatterns[character - '0'];
    }

    uint8_t pattern = Seg_LookupGlyph(exactGlyphs, sizeof(exactGlyphs) / sizeof(exactGlyphs[0]),
                                      character, SEG_PATTERN_BLANK);
    if (pattern != SEG_PATTERN_BLANK) {
        return pattern;
    }

    char upper = (character >= 'a' && character <= 'z') ? (char)(character - 'a' + 'A') : character;
    return Seg_LookupGlyph(foldedGlyphs, sizeof(foldedGlyphs) / sizeof(foldedGlyphs[0]), upper,
                           SEG_PATTERN_BLANK);
}
