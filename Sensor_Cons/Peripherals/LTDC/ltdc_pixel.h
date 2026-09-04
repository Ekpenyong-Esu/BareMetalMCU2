/**
 * @file ltdc_pixel.h
 * @brief Pixel format mapping, colour conversion and pixel writes
 * @details Pure functions over pixel data. Nothing here touches the LTDC
 *          peripheral, so the framebuffer and layer modules can share one
 *          definition of what a pixel is.
 */

#ifndef LTDC_PIXEL_H
#define LTDC_PIXEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ltdc_types.h"

/**
 * @brief Translate a driver pixel format to the HAL constant
 * @param format Driver pixel format
 * @return uint32_t HAL pixel format, ARGB8888 for unknown inputs
 */
uint32_t LTDC_PixelFormatToHAL(LTDC_PixelFormat_t format);

/**
 * @brief Storage size of one pixel in the given format
 * @param format Driver pixel format
 * @return uint32_t Bytes per pixel
 */
uint32_t LTDC_PixelSize(LTDC_PixelFormat_t format);

/**
 * @brief Write one pixel at a linear row-major index
 * @param fbBase Framebuffer base address as a byte pointer
 * @param index  Pixel index within the image
 * @param color  Colour in the layer's own format
 * @param format Pixel format of the framebuffer
 */
void LTDC_WritePixel(uint8_t *fbBase, uint32_t index, uint32_t color, LTDC_PixelFormat_t format);

/**
 * @brief Fill a whole framebuffer with one colour
 * @param fbBase     Framebuffer base address
 * @param pixelCount Number of pixels to write
 * @param color      Colour in the layer's own format
 * @param format     Pixel format of the framebuffer
 */
void LTDC_FillPixels(uint8_t *fbBase, uint32_t pixelCount, uint32_t color,
                     LTDC_PixelFormat_t format);

/**
 * @brief Convert a colour between two pixel formats
 * @param color Input color
 * @param fromFormat Source pixel format
 * @param toFormat Target pixel format
 * @return uint32_t Converted color
 */
uint32_t LTDC_ConvertColor(uint32_t color, LTDC_PixelFormat_t fromFormat,
                           LTDC_PixelFormat_t toFormat);

/**
 * @brief Convert RGB888 to RGB565
 * @param rgb888 24-bit RGB color
 * @return uint32_t 16-bit RGB565 color
 */
uint32_t LTDC_RGB888_To_RGB565(uint32_t rgb888);

/**
 * @brief Convert RGB565 to RGB888
 * @param rgb565 16-bit RGB565 color
 * @return uint32_t 24-bit RGB888 color
 */
uint32_t LTDC_RGB565_To_RGB888(uint16_t rgb565);

/**
 * @brief Convert ARGB8888 to RGB565
 * @param argb8888 32-bit ARGB color
 * @return uint32_t 16-bit RGB565 color
 */
uint32_t LTDC_ARGB8888_To_RGB565(uint32_t argb8888);

#ifdef __cplusplus
}
#endif

#endif /* LTDC_PIXEL_H */
