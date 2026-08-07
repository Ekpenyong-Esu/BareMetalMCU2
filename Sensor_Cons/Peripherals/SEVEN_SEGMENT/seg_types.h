/**
 * @file    seg_types.h
 * @brief   Shared vocabulary for the seven-segment driver
 * @details Declares the status codes, pin/configuration structures, the backend
 *          operations table and the display handle. Every other translation
 *          unit in the module depends on this header and nothing else.
 */

#ifndef SEG_TYPES_H
#define SEG_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/

#define SEG_MAX_DIGITS                  8       /**< Maximum supported digits */
#define SEG_DEFAULT_MULTIPLEX_DELAY_US  2000    /**< 2ms default multiplex delay */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Backend that physically drives the display
 */
typedef enum {
    SEG_DRIVER_GPIO = 0,    /**< Direct GPIO control (no driver IC) */
    SEG_DRIVER_HT1621       /**< HT1621 LCD driver IC */
} SegDriverType_t;

/**
 * @brief Segment polarity (GPIO backend only)
 */
typedef enum {
    SEG_COMMON_CATHODE = 0, /**< Segments are active HIGH */
    SEG_COMMON_ANODE        /**< Segments are active LOW */
} SegPolarity_t;

/**
 * @brief Operation status
 */
typedef enum {
    SEG_OK = 0,             /**< Operation successful */
    SEG_ERROR,              /**< General error */
    SEG_INVALID_PARAM,      /**< Invalid parameter */
    SEG_NOT_INITIALIZED,    /**< Driver not initialized */
    SEG_BUSY                /**< Driver busy */
} SegStatus_t;

/**
 * @brief Segment identifiers (standard 7-segment + decimal point)
 *
 *      --A--
 *     |     |
 *     F     B
 *     |     |
 *      --G--
 *     |     |
 *     E     C
 *     |     |
 *      --D--  .DP
 */
typedef enum {
    SEG_A = 0,              /**< Top segment */
    SEG_B,                  /**< Top-right segment */
    SEG_C,                  /**< Bottom-right segment */
    SEG_D,                  /**< Bottom segment */
    SEG_E,                  /**< Bottom-left segment */
    SEG_F,                  /**< Top-left segment */
    SEG_G,                  /**< Middle segment */
    SEG_DP,                 /**< Decimal point */
    SEG_COUNT               /**< Total segment count */
} SegmentId_t;

/**
 * @brief A single GPIO pin
 */
typedef struct {
    GPIO_TypeDef* port;     /**< GPIO port */
    uint16_t pin;           /**< GPIO pin */
} SegGpioPin_t;

/**
 * @brief Configuration for the direct-GPIO backend
 */
typedef struct {
    SegGpioPin_t segments[SEG_COUNT];   /**< Segment pins (A-G + DP) */
    SegGpioPin_t* digits;               /**< Digit select pins (array) */
    uint8_t digitCount;                 /**< Number of digits */
    SegPolarity_t polarity;             /**< Common cathode or anode */
    bool digitActiveHigh;               /**< Digit select active level */
} SegGpioConfig_t;

/**
 * @brief HT1621 bit-bang pins
 */
typedef struct {
    GPIO_TypeDef* csPort;   /**< Chip select port */
    uint16_t csPin;         /**< Chip select pin */
    GPIO_TypeDef* wrPort;   /**< Write clock port */
    uint16_t wrPin;         /**< Write clock pin */
    GPIO_TypeDef* dataPort; /**< Data port */
    uint16_t dataPin;       /**< Data pin */
} SegHT1621Pins_t;

/**
 * @brief Configuration for the HT1621 backend
 */
typedef struct {
    SegHT1621Pins_t pins;           /**< HT1621 GPIO pins */
    uint8_t digitCount;             /**< Number of digits */
    uint8_t bias;                   /**< LCD bias (2 or 3) */
    uint8_t commons;                /**< Number of commons (2, 3, or 4) */
} SegHT1621Config_t;

/**
 * @brief Display configuration
 */
typedef struct {
    SegDriverType_t driverType;     /**< Backend selection */
    union {
        SegGpioConfig_t gpio;       /**< GPIO backend configuration */
        SegHT1621Config_t ht1621;   /**< HT1621 backend configuration */
    } config;
    uint16_t multiplexDelayUs;      /**< Multiplex delay (GPIO backend only) */
    bool leadingZeros;              /**< Pad numeric output with zeros */
} SegDisplayConfig_t;

typedef struct SegDisplayHandle SegDisplayHandle_t;

/**
 * @brief Operations a display backend must provide
 * @details Resolved once during Seg_Init. Everything above this table works
 *          purely against the pattern buffer and never inspects driverType.
 */
typedef struct {
    /** @brief Bring the backend up and leave the display blank */
    SegStatus_t (*init)(SegDisplayHandle_t* handle);
    /** @brief Turn the display on */
    void (*enable)(SegDisplayHandle_t* handle);
    /** @brief Turn the display off */
    void (*disable)(SegDisplayHandle_t* handle);
    /** @brief Push the whole pattern buffer to the hardware */
    void (*commit)(SegDisplayHandle_t* handle);
    /** @brief Light one digit and advance; NULL when the backend is not multiplexed */
    void (*multiplexStep)(SegDisplayHandle_t* handle);
} SegDriverOps_t;

/**
 * @brief Display handle
 */
struct SegDisplayHandle {
    SegDisplayConfig_t config;              /**< Copy of the configuration */
    const SegDriverOps_t* ops;              /**< Resolved backend */
    uint8_t displayBuffer[SEG_MAX_DIGITS];  /**< One segment pattern per digit */
    uint8_t digitCount;                     /**< Resolved once, backend agnostic */
    uint8_t currentDigit;                   /**< Multiplex cursor */
    bool initialized;                       /**< Initialization flag */
    bool enabled;                           /**< Display enabled flag */
};

/* Exported inline helpers ---------------------------------------------------*/

/**
 * @brief   Common entry guard shared by every public entry point
 * @param   handle Handle to validate
 * @retval  SEG_OK when the handle is usable
 */
static inline SegStatus_t Seg_CheckReady(const SegDisplayHandle_t* handle)
{
    if (handle == NULL) {
        return SEG_INVALID_PARAM;
    }
    if (!handle->initialized) {
        return SEG_NOT_INITIALIZED;
    }
    return SEG_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* SEG_TYPES_H */
