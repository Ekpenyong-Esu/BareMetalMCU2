/**
 * @file dma2d_color.c
 * @brief ARGB8888 colour helpers and diagnostic strings for the DMA2D driver
 */

#include "dma2d_color.h"

uint32_t DMA2D_MakeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    return ((uint32_t)alpha << 24) | ((uint32_t)red << 16) |
           ((uint32_t)green << 8) | (uint32_t)blue;
}

void DMA2D_GetColorComponents(uint32_t color, uint8_t *red, uint8_t *green,
                              uint8_t *blue, uint8_t *alpha)
{
    if (alpha != NULL) { *alpha = (uint8_t)((color >> 24) & 0xFFU); }
    if (red   != NULL) { *red   = (uint8_t)((color >> 16) & 0xFFU); }
    if (green != NULL) { *green = (uint8_t)((color >> 8)  & 0xFFU); }
    if (blue  != NULL) { *blue  = (uint8_t)(color & 0xFFU); }
}

const char* DMA2D_GetErrorString(HAL_StatusTypeDef error_code)
{
    switch (error_code) {
        case HAL_OK:       return "No error";
        case HAL_ERROR:    return "General error";
        case HAL_BUSY:     return "Peripheral busy";
        case HAL_TIMEOUT:  return "Timeout occurred";
        default:           return "Unknown error";
    }
}

const char* DMA2D_GetStateString(uint32_t state)
{
    switch (state) {
        case HAL_DMA2D_STATE_RESET:   return "Reset";
        case HAL_DMA2D_STATE_READY:   return "Ready";
        case HAL_DMA2D_STATE_BUSY:    return "Busy";
        case HAL_DMA2D_STATE_TIMEOUT: return "Timeout";
        case HAL_DMA2D_STATE_ERROR:   return "Error";
        default:                      return "Unknown";
    }
}
