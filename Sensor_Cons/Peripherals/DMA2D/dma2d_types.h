/**
 * @file    dma2d_types.h
 * @brief   Shared types for the DMA2D graphics helper
 * @details This file holds the basic types and settings for DMA2D.
 *
 * How it works (in simple words):
 * - DMA2D is a helper that draws and moves pictures very fast.
 * - It can fill a rectangle with one color, copy an image, or blend two images.
 * - It works without the CPU, so the screen updates quickly.
 * - You give it a color, a size, and where to draw.
 */

#ifndef DMA2D_TYPES_H
#define DMA2D_TYPES_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal_dma2d.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Limits --------------------------------------------------------------------*/

/** @brief Default timeout for DMA2D operations (milliseconds) */
#define DMA2D_DEFAULT_TIMEOUT        1000U

/** @brief Maximum supported width for DMA2D operations */
#define DMA2D_MAX_WIDTH              2048U

/** @brief Maximum supported height for DMA2D operations */
#define DMA2D_MAX_HEIGHT             2048U

/** @brief Maximum supported line offset */
#define DMA2D_MAX_OFFSET             2047U

/* Output colour formats -----------------------------------------------------*/

#define DMA2D_FORMAT_ARGB8888        DMA2D_OUTPUT_ARGB8888
#define DMA2D_FORMAT_RGB888          DMA2D_OUTPUT_RGB888
#define DMA2D_FORMAT_RGB565          DMA2D_OUTPUT_RGB565
#define DMA2D_FORMAT_ARGB1555        DMA2D_OUTPUT_ARGB1555
#define DMA2D_FORMAT_ARGB4444        DMA2D_OUTPUT_ARGB4444

/* Operating modes -----------------------------------------------------------*/

#define DMA2D_MODE_R2M               DMA2D_R2M          /**< Register to Memory */
#define DMA2D_MODE_M2M               DMA2D_M2M          /**< Memory to Memory */
#define DMA2D_MODE_M2M_PFC           DMA2D_M2M_PFC      /**< Memory to Memory with PFC */
#define DMA2D_MODE_M2M_BLEND         DMA2D_M2M_BLEND    /**< Memory to Memory with Blending */

/* Alpha modes ---------------------------------------------------------------*/

#define DMA2D_ALPHA_NO_MODIF         DMA2D_NO_MODIF_ALPHA   /**< No alpha modification */
#define DMA2D_ALPHA_REPLACE          DMA2D_REPLACE_ALPHA    /**< Replace alpha */
#define DMA2D_ALPHA_COMBINE          DMA2D_COMBINE_ALPHA    /**< Combine alpha */

/* Peripheral states ---------------------------------------------------------*/

#define DMA2D_STATE_RESET            HAL_DMA2D_STATE_RESET     /**< Not initialized */
#define DMA2D_STATE_READY            HAL_DMA2D_STATE_READY     /**< Initialized and ready */
#define DMA2D_STATE_BUSY             HAL_DMA2D_STATE_BUSY      /**< Transfer in progress */
#define DMA2D_STATE_TIMEOUT          HAL_DMA2D_STATE_TIMEOUT   /**< Timeout occurred */
#define DMA2D_STATE_ERROR            HAL_DMA2D_STATE_ERROR     /**< Error occurred */

/* Predefined colours (ARGB8888) ---------------------------------------------*/

#define DMA2D_COLOR_RED              0xFFFF0000U    /**< Pure red */
#define DMA2D_COLOR_GREEN            0xFF00FF00U    /**< Pure green */
#define DMA2D_COLOR_BLUE             0xFF0000FFU    /**< Pure blue */
#define DMA2D_COLOR_WHITE            0xFFFFFFFFU    /**< White */
#define DMA2D_COLOR_BLACK            0xFF000000U    /**< Black */
#define DMA2D_COLOR_YELLOW           0xFFFFFF00U    /**< Yellow */
#define DMA2D_COLOR_CYAN             0xFF00FFFFU    /**< Cyan */
#define DMA2D_COLOR_MAGENTA          0xFFFF00FFU    /**< Magenta */
#define DMA2D_COLOR_TRANSPARENT      0x00000000U    /**< Fully transparent */

/* Structures ----------------------------------------------------------------*/

/** @brief Keeps track of how DMA2D is doing */
typedef struct {
    bool initialized;                      /**< True if ready to use */
    uint32_t last_error;                   /**< Last error code */
    uint32_t transfer_count;               /**< How many draws worked */
    uint32_t error_count;                  /**< How many draws failed */
    uint32_t state;                        /**< What it is doing now */
    uint32_t total_bytes_transferred;      /**< Total bytes moved so far */
} DMA2D_Status;

/** @brief Settings for how to draw */
typedef struct {
    uint32_t mode;                        /**< What to do (fill color, copy, blend) */
    uint32_t color_mode;                  /**< Color type for the output */
    uint32_t output_offset;               /**< Extra pixels to skip at end of each line */
    uint32_t red_value;                   /**< Red amount (0-255) for fill mode */
    uint32_t green_value;                 /**< Green amount (0-255) for fill mode */
    uint32_t blue_value;                  /**< Blue amount (0-255) for fill mode */
    uint32_t alpha_value;                 /**< See-through amount (0-255) for fill mode */
} DMA2D_Config;

/** @brief Settings for one picture layer */
typedef struct {
    uint32_t input_color_mode;            /**< Color type of this layer */
    uint32_t input_alpha_mode;            /**< How to handle see-through */
    uint32_t input_alpha;                 /**< See-through amount (0-255) */
    uint32_t input_offset;                /**< Extra pixels to skip at end of each line */
} DMA2D_LayerConfig;

/** @brief A box on the screen */
typedef struct {
    uint32_t x;                           /**< Left side of the box */
    uint32_t y;                           /**< Top side of the box */
    uint32_t width;                       /**< How wide the box is */
    uint32_t height;                      /**< How tall the box is */
} DMA2D_Rectangle;

/**
 * @brief Everything the driver keeps in one place
 * @details Holds the low-level handle and the counters.
 */
typedef struct {
    DMA2D_HandleTypeDef hal;              /**< Low-level handle for DMA2D */
    DMA2D_Status status;                  /**< Status and counters */
} DMA2D_Device;

/* Callback signatures -------------------------------------------------------*/

typedef void (*DMA2D_TransferCompleteCallback)(DMA2D_HandleTypeDef *hdma2d);
typedef void (*DMA2D_TransferErrorCallback)(DMA2D_HandleTypeDef *hdma2d);
typedef void (*DMA2D_TransferProgressCallback)(DMA2D_HandleTypeDef *hdma2d, uint32_t progress);

#endif /* DMA2D_TYPES_H */
