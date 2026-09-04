/**
 * @file dma2d_color.c
 * @brief ARGB8888 colour helpers and diagnostic strings for the DMA2D driver
 */

#include "dma2d_color.h"

uint32_t DMA2D_MakeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    return ((uint32_t)alpha << DMA2D_ALPHA_SHIFT) | ((uint32_t)red << DMA2D_RED_SHIFT) |
           ((uint32_t)green << DMA2D_GREEN_SHIFT) | (uint32_t)blue;
}

void DMA2D_GetColorComponents(uint32_t color, uint8_t *red, uint8_t *green, uint8_t *blue,
                              uint8_t *alpha) {
    if (alpha != NULL) {
        *alpha = (uint8_t)((color >> DMA2D_ALPHA_SHIFT) & DMA2D_CHANNEL_MASK);
    }
    if (red != NULL) {
        *red = (uint8_t)((color >> DMA2D_RED_SHIFT) & DMA2D_CHANNEL_MASK);
    }
    if (green != NULL) {
        *green = (uint8_t)((color >> DMA2D_GREEN_SHIFT) & DMA2D_CHANNEL_MASK);
    }
    if (blue != NULL) {
        *blue = (uint8_t)(color & DMA2D_CHANNEL_MASK);
    }
}

const char *DMA2D_GetErrorString(HAL_StatusTypeDef error_code) {
    switch (error_code) {
        case HAL_OK:
            return "No error";
        case HAL_ERROR:
            return "General error";
        case HAL_BUSY:
            return "Peripheral busy";
        case HAL_TIMEOUT:
            return "Timeout occurred";
        default:
            return "Unknown error";
    }
}

const char *DMA2D_GetStateString(uint32_t state) {
    switch (state) {
        case HAL_DMA2D_STATE_RESET:
            return "Reset";
        case HAL_DMA2D_STATE_READY:
            return "Ready";
        case HAL_DMA2D_STATE_BUSY:
            return "Busy";
        case HAL_DMA2D_STATE_TIMEOUT:
            return "Timeout";
        case HAL_DMA2D_STATE_ERROR:
            return "Error";
        default:
            return "Unknown";
    }
}
