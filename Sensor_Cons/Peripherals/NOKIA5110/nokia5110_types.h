/**
  ******************************************************************************
  * @file    nokia5110_types.h
  * @brief   Shared vocabulary for the Nokia 5110 (PCD8544) LCD driver
  ******************************************************************************
  */

#ifndef NOKIA5110_TYPES_H
#define NOKIA5110_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/** @defgroup NOKIA5110_Display_Specifications Display Specifications
 * @{
 */
#define NOKIA5110_WIDTH          84      /*!< Display width in pixels */
#define NOKIA5110_HEIGHT         48      /*!< Display height in pixels */
#define NOKIA5110_ROWS           6       /*!< Number of rows (8 pixels each) */
#define NOKIA5110_ROW_HEIGHT     8       /*!< Pixels per buffer row */
/** @} */

/** @defgroup NOKIA5110_Commands LCD Commands
 * @{
 */
#define NOKIA5110_CMD_FUNCTION_SET     0x20  /*!< Function set */
#define NOKIA5110_CMD_DISPLAY_CONTROL  0x08  /*!< Display control (H=0) */
#define NOKIA5110_CMD_SET_Y_ADDR       0x40  /*!< Set Y address (H=0) */
#define NOKIA5110_CMD_SET_X_ADDR       0x80  /*!< Set X address (H=0) */
#define NOKIA5110_CMD_TEMP_CONTROL     0x04  /*!< Temperature control (H=1) */
#define NOKIA5110_CMD_BIAS_SYSTEM      0x10  /*!< Bias system (H=1) */
#define NOKIA5110_CMD_VOP              0x80  /*!< Vop / contrast (H=1) */

/* Function set options */
#define NOKIA5110_FUNCTION_H            0x01  /*!< Extended instruction set */
#define NOKIA5110_FUNCTION_V            0x02  /*!< Vertical addressing */
#define NOKIA5110_FUNCTION_PD           0x04  /*!< Power down mode */

/* Display control options */
#define NOKIA5110_DISPLAY_BLANK         0x00  /*!< Display blank */
#define NOKIA5110_DISPLAY_NORMAL        0x04  /*!< Normal mode */
#define NOKIA5110_DISPLAY_ALL_ON        0x01  /*!< All pixels on */
#define NOKIA5110_DISPLAY_INVERSE       0x05  /*!< Inverse video mode */

/* Temperature control coefficients */
#define NOKIA5110_TEMP_COEFF_0          0x00  /*!< TC0 */
#define NOKIA5110_TEMP_COEFF_1          0x01  /*!< TC1 */
#define NOKIA5110_TEMP_COEFF_2          0x02  /*!< TC2 */
#define NOKIA5110_TEMP_COEFF_3          0x03  /*!< TC3 */

/* Bias system values */
#define NOKIA5110_BIAS_1_100            0x00  /*!< 1:100 bias */
#define NOKIA5110_BIAS_1_80             0x01  /*!< 1:80 bias */
#define NOKIA5110_BIAS_1_65             0x02  /*!< 1:65 bias */
#define NOKIA5110_BIAS_1_48             0x03  /*!< 1:48 bias */
#define NOKIA5110_BIAS_1_40_1           0x04  /*!< 1:40/1:34 */
#define NOKIA5110_BIAS_1_24             0x05  /*!< 1:24 bias */
#define NOKIA5110_BIAS_1_18_1           0x06  /*!< 1:18/1:16 */
#define NOKIA5110_BIAS_1_10_1           0x07  /*!< 1:10/1:9 */
/** @} */

/** @defgroup NOKIA5110_Limits Parameter limits
 * @{
 */
#define NOKIA5110_CONTRAST_MAX          0x7F  /*!< Vop is 7 bits wide */
#define NOKIA5110_TEMP_COEFF_MAX        NOKIA5110_TEMP_COEFF_3
#define NOKIA5110_BIAS_MAX              NOKIA5110_BIAS_1_10_1
/** @} */

/**
 * @brief Nokia 5110 status enumeration
 */
typedef enum {
    NOKIA5110_OK = 0,              /*!< Operation completed successfully */
    NOKIA5110_ERROR,               /*!< General error occurred */
    NOKIA5110_TIMEOUT,             /*!< Operation timed out */
    NOKIA5110_INVALID_PARAM,       /*!< Invalid parameter provided */
    NOKIA5110_NOT_INITIALIZED      /*!< Device not initialized */
} NOKIA5110_StatusTypeDef;

/**
 * @brief Nokia 5110 display mode enumeration
 */
typedef enum {
    NOKIA5110_MODE_BLANK = 0,      /*!< Display blank */
    NOKIA5110_MODE_NORMAL,         /*!< Normal display mode */
    NOKIA5110_MODE_ALL_ON,         /*!< All pixels on */
    NOKIA5110_MODE_INVERSE         /*!< Inverse video mode */
} NOKIA5110_DisplayMode_t;

/**
 * @brief Nokia 5110 configuration structure
 */
typedef struct {
    uint8_t Contrast;              /*!< Display contrast (0-127) */
    uint8_t TemperatureCoeff;      /*!< Temperature coefficient (0-3) */
    uint8_t BiasSystem;            /*!< Bias system (0-7) */
    NOKIA5110_DisplayMode_t Mode;  /*!< Display mode */
} NOKIA5110_Config_t;

/**
 * @brief Nokia 5110 handle structure
 */
typedef struct {
    NOKIA5110_Config_t Config;     /*!< Configuration */
    bool IsInitialized;            /*!< Initialization status */
    uint8_t Buffer[NOKIA5110_ROWS][NOKIA5110_WIDTH]; /*!< Display buffer */
} NOKIA5110_Handle_t;

/** Reject a handle that is NULL or whose display was never brought up */
#define NOKIA5110_CHECK_HANDLE(hnok)                        \
    do {                                                    \
        if ((hnok) == NULL) {                               \
            return NOKIA5110_INVALID_PARAM;                 \
        }                                                   \
        if (!(hnok)->IsInitialized) {                       \
            return NOKIA5110_NOT_INITIALIZED;               \
        }                                                   \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* NOKIA5110_TYPES_H */
