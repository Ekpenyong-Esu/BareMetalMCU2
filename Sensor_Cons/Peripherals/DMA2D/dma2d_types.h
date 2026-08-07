/**
 * @file dma2d_types.h
 * @brief Shared vocabulary for the DMA2D (Chrom-Art Accelerator) driver
 *
 * @details
 * Declares the constants, configuration structures, callback signatures and the
 * device record used by every DMA2D module. It contains no behaviour, so the
 * validation, colour, transfer and event modules can all depend on it without
 * depending on each other.
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

/** @brief Runtime counters and last known peripheral state */
typedef struct {
    bool initialized;                      /**< DMA2D initialization status */
    uint32_t last_error;                   /**< Last error code from HAL */
    uint32_t transfer_count;               /**< Number of successful transfers */
    uint32_t error_count;                  /**< Number of errors occurred */
    uint32_t state;                        /**< Current DMA2D state */
    uint32_t total_bytes_transferred;      /**< Total bytes transferred */
} DMA2D_Status;

/** @brief Peripheral configuration */
typedef struct {
    uint32_t mode;                        /**< Operating mode (R2M, M2M, etc.) */
    uint32_t color_mode;                  /**< Output color format */
    uint32_t output_offset;               /**< Output line offset (pixels to skip per line) */
    uint32_t red_value;                   /**< Red component for R2M mode (0-255) */
    uint32_t green_value;                 /**< Green component for R2M mode (0-255) */
    uint32_t blue_value;                  /**< Blue component for R2M mode (0-255) */
    uint32_t alpha_value;                 /**< Alpha component for R2M mode (0-255) */
} DMA2D_Config;

/** @brief Input layer configuration */
typedef struct {
    uint32_t input_color_mode;            /**< Input color format */
    uint32_t input_alpha_mode;            /**< Alpha mode for input */
    uint32_t input_alpha;                 /**< Alpha value for input (0-255) */
    uint32_t input_offset;                /**< Input line offset (pixels to skip per line) */
} DMA2D_LayerConfig;

/** @brief Rectangle for area operations */
typedef struct {
    uint32_t x;                           /**< X coordinate of top-left corner */
    uint32_t y;                           /**< Y coordinate of top-left corner */
    uint32_t width;                       /**< Rectangle width in pixels */
    uint32_t height;                      /**< Rectangle height in pixels */
} DMA2D_Rectangle;

/**
 * @brief Everything the driver owns
 * @details Keeping the HAL handle and the bookkeeping together means the whole
 *          driver state is one object, and no module needs a global symbol.
 */
typedef struct {
    DMA2D_HandleTypeDef hal;              /**< HAL handle for the single DMA2D unit */
    DMA2D_Status status;                  /**< Initialization flag and counters */
} DMA2D_Device;

/* Callback signatures -------------------------------------------------------*/

typedef void (*DMA2D_TransferCompleteCallback)(DMA2D_HandleTypeDef *hdma2d);
typedef void (*DMA2D_TransferErrorCallback)(DMA2D_HandleTypeDef *hdma2d);
typedef void (*DMA2D_TransferProgressCallback)(DMA2D_HandleTypeDef *hdma2d, uint32_t progress);

#endif /* DMA2D_TYPES_H */
