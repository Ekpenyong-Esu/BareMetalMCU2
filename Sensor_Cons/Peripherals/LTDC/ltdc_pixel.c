/**
 * @file ltdc_pixel.c
 * @brief Pixel format mapping, colour conversion and pixel writes
 */

#include "ltdc_pixel.h"

/* Private defines -----------------------------------------------------------*/

/* RGB565 channel layout: 5 bits red, 6 bits green, 5 bits blue. */
#define LTDC_RGB565_RED_SHIFT 11  /*!< Bit position of the red channel */
#define LTDC_RGB565_GREEN_SHIFT 5 /*!< Bit position of the green channel */
#define LTDC_RGB565_RED_MASK 0x1FU
#define LTDC_RGB565_GREEN_MASK 0x3FU
#define LTDC_RGB565_BLUE_MASK 0x1FU

/* Private types -------------------------------------------------------------*/

/** One row of the pixel format table. */
typedef struct {
    LTDC_PixelFormat_t format; /*!< Driver format */
    uint32_t halFormat;        /*!< Equivalent HAL constant */
    uint8_t bytesPerPixel;     /*!< Storage size of one pixel */
} LTDC_PixelFormatEntry_t;

/* Private data --------------------------------------------------------------*/

static const LTDC_PixelFormatEntry_t s_pixelFormats[] = {
    {LTDC_PIXEL_FORMAT_ARGB8888_ENUM, LTDC_PIXEL_FORMAT_ARGB8888, LTDC_BYTES_PER_PIXEL_ARGB8888},
    {LTDC_PIXEL_FORMAT_RGB888_ENUM, LTDC_PIXEL_FORMAT_RGB888, LTDC_BYTES_PER_PIXEL_RGB888},
    {LTDC_PIXEL_FORMAT_RGB565_ENUM, LTDC_PIXEL_FORMAT_RGB565, LTDC_BYTES_PER_PIXEL_RGB565},
    {LTDC_PIXEL_FORMAT_ARGB1555_ENUM, LTDC_PIXEL_FORMAT_ARGB1555, 2},
    {LTDC_PIXEL_FORMAT_ARGB4444_ENUM, LTDC_PIXEL_FORMAT_ARGB4444, 2},
    {LTDC_PIXEL_FORMAT_L8_ENUM, LTDC_PIXEL_FORMAT_L8, 1},
    {LTDC_PIXEL_FORMAT_AL44_ENUM, LTDC_PIXEL_FORMAT_AL44, 1},
    {LTDC_PIXEL_FORMAT_AL88_ENUM, LTDC_PIXEL_FORMAT_AL88, 2},
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Look a driver format up in the format table
 * @param format Driver pixel format
 * @return const LTDC_PixelFormatEntry_t* Matching row, or NULL when unknown
 */
static const LTDC_PixelFormatEntry_t *LTDC_FindPixelFormat(LTDC_PixelFormat_t format) {
    for (uint32_t i = 0; i < (sizeof(s_pixelFormats) / sizeof(s_pixelFormats[0])); i++) {
        if (s_pixelFormats[i].format == format) {
            return &s_pixelFormats[i];
        }
    }
    return NULL;
}

/* Public functions ----------------------------------------------------------*/

uint32_t LTDC_PixelFormatToHAL(LTDC_PixelFormat_t format) {
    const LTDC_PixelFormatEntry_t *entry = LTDC_FindPixelFormat(format);
    return (entry != NULL) ? entry->halFormat : LTDC_PIXEL_FORMAT_ARGB8888;
}

uint32_t LTDC_PixelSize(LTDC_PixelFormat_t format) {
    const LTDC_PixelFormatEntry_t *entry = LTDC_FindPixelFormat(format);
    return (entry != NULL) ? entry->bytesPerPixel : LTDC_BYTES_PER_PIXEL_ARGB8888;
}

void LTDC_WritePixel(uint8_t *fbBase, uint32_t index, uint32_t color, LTDC_PixelFormat_t format) {
    switch (format) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            ((uint16_t *)(void *)fbBase)[index] = (uint16_t)color;
            break;

        case LTDC_PIXEL_FORMAT_RGB888_ENUM: {
            /* Packed 24-bit; rejected at layer configuration time but kept for completeness. */
            uint8_t *pixel = fbBase + (index * LTDC_BYTES_PER_PIXEL_RGB888);
            pixel[0] = (uint8_t)(color & LTDC_CHANNEL_MASK);                       /* Blue */
            pixel[1] = (uint8_t)((color >> LTDC_GREEN_SHIFT) & LTDC_CHANNEL_MASK); /* Green */
            pixel[2] = (uint8_t)((color >> LTDC_RED_SHIFT) & LTDC_CHANNEL_MASK);   /* Red */
            break;
        }

        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
        default:
            ((uint32_t *)(void *)fbBase)[index] = color;
            break;
    }
}

void LTDC_FillPixels(uint8_t *fbBase, uint32_t pixelCount, uint32_t color,
                     LTDC_PixelFormat_t format) {
    for (uint32_t i = 0; i < pixelCount; i++) {
        LTDC_WritePixel(fbBase, i, color, format);
    }
}

uint32_t LTDC_ConvertColor(uint32_t color, LTDC_PixelFormat_t fromFormat,
                           LTDC_PixelFormat_t toFormat) {
    if (fromFormat == toFormat) {
        return color;
    }

    /* RGB888 is the intermediate representation for every supported conversion. */
    uint32_t rgb888 = 0;
    switch (fromFormat) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            rgb888 = LTDC_RGB565_To_RGB888((uint16_t)color);
            break;
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
            rgb888 = color & LTDC_RGB888_MASK;
            break;
        default:
            rgb888 = color;
            break;
    }

    switch (toFormat) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            return LTDC_RGB888_To_RGB565(rgb888);
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
            return rgb888 | LTDC_ALPHA_OPAQUE_MASK;
        default:
            return rgb888;
    }
}

uint32_t LTDC_RGB888_To_RGB565(uint32_t rgb888) {
    uint32_t red = (rgb888 >> LTDC_RED_SHIFT) & LTDC_CHANNEL_MASK;
    uint32_t green = (rgb888 >> LTDC_GREEN_SHIFT) & LTDC_CHANNEL_MASK;
    uint32_t blue = rgb888 & LTDC_CHANNEL_MASK;

    /* Drop the low bits of each 8-bit channel to fit the 5/6/5 layout. */
    return ((red >> 3) << LTDC_RGB565_RED_SHIFT) | ((green >> 2) << LTDC_RGB565_GREEN_SHIFT) |
           (blue >> 3);
}

uint32_t LTDC_RGB565_To_RGB888(uint16_t rgb565) {
    uint32_t red = (rgb565 >> LTDC_RGB565_RED_SHIFT) & LTDC_RGB565_RED_MASK;
    uint32_t green = (rgb565 >> LTDC_RGB565_GREEN_SHIFT) & LTDC_RGB565_GREEN_MASK;
    uint32_t blue = rgb565 & LTDC_RGB565_BLUE_MASK;

    /* Replicate the high bits into the low ones so full-scale stays full-scale. */
    red = (red << 3) | (red >> 2);
    green = (green << 2) | (green >> 4);
    blue = (blue << 3) | (blue >> 2);

    return (red << LTDC_RED_SHIFT) | (green << LTDC_GREEN_SHIFT) | blue;
}

uint32_t LTDC_ARGB8888_To_RGB565(uint32_t argb8888) {
    return LTDC_RGB888_To_RGB565(argb8888 & LTDC_RGB888_MASK);
}
