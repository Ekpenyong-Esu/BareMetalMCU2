/**
 * @file ltdc_pixel.c
 * @brief Pixel format mapping, colour conversion and pixel writes
 */

#include "ltdc_pixel.h"

/* Private types -------------------------------------------------------------*/

/** One row of the pixel format table. */
typedef struct {
    LTDC_PixelFormat_t format;   /*!< Driver format */
    uint32_t halFormat;          /*!< Equivalent HAL constant */
    uint8_t bytesPerPixel;       /*!< Storage size of one pixel */
} LTDC_PixelFormatEntry_t;

/* Private data --------------------------------------------------------------*/

static const LTDC_PixelFormatEntry_t s_pixelFormats[] = {
    {LTDC_PIXEL_FORMAT_ARGB8888_ENUM, LTDC_PIXEL_FORMAT_ARGB8888, LTDC_BYTES_PER_PIXEL_ARGB8888},
    {LTDC_PIXEL_FORMAT_RGB888_ENUM,   LTDC_PIXEL_FORMAT_RGB888,   LTDC_BYTES_PER_PIXEL_RGB888},
    {LTDC_PIXEL_FORMAT_RGB565_ENUM,   LTDC_PIXEL_FORMAT_RGB565,   LTDC_BYTES_PER_PIXEL_RGB565},
    {LTDC_PIXEL_FORMAT_ARGB1555_ENUM, LTDC_PIXEL_FORMAT_ARGB1555, 2},
    {LTDC_PIXEL_FORMAT_ARGB4444_ENUM, LTDC_PIXEL_FORMAT_ARGB4444, 2},
    {LTDC_PIXEL_FORMAT_L8_ENUM,       LTDC_PIXEL_FORMAT_L8,       1},
    {LTDC_PIXEL_FORMAT_AL44_ENUM,     LTDC_PIXEL_FORMAT_AL44,     1},
    {LTDC_PIXEL_FORMAT_AL88_ENUM,     LTDC_PIXEL_FORMAT_AL88,     2},
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Look a driver format up in the format table
 * @param format Driver pixel format
 * @return const LTDC_PixelFormatEntry_t* Matching row, or NULL when unknown
 */
static const LTDC_PixelFormatEntry_t* LTDC_FindPixelFormat(LTDC_PixelFormat_t format)
{
    for (uint32_t i = 0; i < (sizeof(s_pixelFormats) / sizeof(s_pixelFormats[0])); i++) {
        if (s_pixelFormats[i].format == format) {
            return &s_pixelFormats[i];
        }
    }
    return NULL;
}

/* Public functions ----------------------------------------------------------*/

uint32_t LTDC_PixelFormatToHAL(LTDC_PixelFormat_t format)
{
    const LTDC_PixelFormatEntry_t *entry = LTDC_FindPixelFormat(format);
    return (entry != NULL) ? entry->halFormat : LTDC_PIXEL_FORMAT_ARGB8888;
}

uint32_t LTDC_PixelSize(LTDC_PixelFormat_t format)
{
    const LTDC_PixelFormatEntry_t *entry = LTDC_FindPixelFormat(format);
    return (entry != NULL) ? entry->bytesPerPixel : LTDC_BYTES_PER_PIXEL_ARGB8888;
}

void LTDC_WritePixel(uint8_t *fbBase, uint32_t index, uint32_t color, LTDC_PixelFormat_t format)
{
    switch (format) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            ((uint16_t *)(void *)fbBase)[index] = (uint16_t)color;
            break;

        case LTDC_PIXEL_FORMAT_RGB888_ENUM: {
            /* Packed 24-bit; rejected at layer configuration time but kept for completeness. */
            uint8_t *p = fbBase + (index * LTDC_BYTES_PER_PIXEL_RGB888);
            p[0] = (uint8_t)(color & 0xFFU);          /* Blue */
            p[1] = (uint8_t)((color >> 8) & 0xFFU);   /* Green */
            p[2] = (uint8_t)((color >> 16) & 0xFFU);  /* Red */
            break;
        }

        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
        default:
            ((uint32_t *)(void *)fbBase)[index] = color;
            break;
    }
}

void LTDC_FillPixels(uint8_t *fbBase, uint32_t pixelCount, uint32_t color, LTDC_PixelFormat_t format)
{
    for (uint32_t i = 0; i < pixelCount; i++) {
        LTDC_WritePixel(fbBase, i, color, format);
    }
}

uint32_t LTDC_ConvertColor(uint32_t color, LTDC_PixelFormat_t fromFormat, LTDC_PixelFormat_t toFormat)
{
    if (fromFormat == toFormat) {
        return color;
    }

    /* RGB888 is the intermediate representation for every supported conversion. */
    uint32_t rgb888;
    switch (fromFormat) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            rgb888 = LTDC_RGB565_To_RGB888((uint16_t)color);
            break;
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
            rgb888 = color & 0x00FFFFFFU;
            break;
        default:
            rgb888 = color;
            break;
    }

    switch (toFormat) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            return LTDC_RGB888_To_RGB565(rgb888);
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
            return rgb888 | 0xFF000000U;
        default:
            return rgb888;
    }
}

uint32_t LTDC_RGB888_To_RGB565(uint32_t rgb888)
{
    uint32_t r = (rgb888 >> 16) & 0xFFU;
    uint32_t g = (rgb888 >> 8) & 0xFFU;
    uint32_t b = rgb888 & 0xFFU;

    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

uint32_t LTDC_RGB565_To_RGB888(uint16_t rgb565)
{
    uint32_t r = (rgb565 >> 11) & 0x1FU;
    uint32_t g = (rgb565 >> 5) & 0x3FU;
    uint32_t b = rgb565 & 0x1FU;

    /* Replicate the high bits into the low ones so full-scale stays full-scale. */
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    return (r << 16) | (g << 8) | b;
}

uint32_t LTDC_ARGB8888_To_RGB565(uint32_t argb8888)
{
    return LTDC_RGB888_To_RGB565(argb8888 & 0x00FFFFFFU);
}
