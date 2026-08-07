/**
  ******************************************************************************
  * @file    ov7670_types.h
  * @brief   Shared vocabulary for the OV7670 camera driver
  ******************************************************************************
  */

#ifndef OV7670_TYPES_H
#define OV7670_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "i2c.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define OV7670_I2C_TIMEOUT            100U    /**< I2C timeout in ms */
#define OV7670_RESET_DELAY            100U    /**< Settle time after a soft reset, ms */

#define OV7670_MAX_WIDTH              640
#define OV7670_MAX_HEIGHT             480
#define OV7670_QVGA_WIDTH             320
#define OV7670_QVGA_HEIGHT            240
#define OV7670_QQVGA_WIDTH            160
#define OV7670_QQVGA_HEIGHT           120

/* Test patterns */
#define OV7670_TEST_PATTERN_NONE      0x00U
#define OV7670_TEST_PATTERN_1         0x01U
#define OV7670_TEST_PATTERN_2         0x02U
#define OV7670_TEST_PATTERN_BARS      0x03U

typedef enum {
    OV7670_OK = 0,
    OV7670_ERROR,
    OV7670_BUSY,
    OV7670_TIMEOUT,
    OV7670_INVALID_PARAM,
    OV7670_NOT_INITIALIZED,
    OV7670_I2C_ERROR,
    OV7670_INVALID_ID
} OV7670_StatusTypeDef;

typedef enum {
    OV7670_RES_QQVGA = 0,           /**< 160x120 */
    OV7670_RES_QVGA,                /**< 320x240 */
    OV7670_RES_VGA,                 /**< 640x480 */
    OV7670_RES_COUNT
} OV7670_ResolutionTypeDef;

typedef enum {
    OV7670_FMT_RGB565 = 0,
    OV7670_FMT_RGB555,
    OV7670_FMT_YUV422,
    OV7670_FMT_GRAYSCALE,
    OV7670_FMT_COUNT
} OV7670_FormatTypeDef;

typedef struct {
    OV7670_ResolutionTypeDef resolution;
    OV7670_FormatTypeDef format;
    uint8_t brightness;             /**< 0-255, 128 is neutral */
    uint8_t contrast;               /**< 0-255 */
    uint8_t saturation;             /**< 0-255 */
    bool flip_horizontal;
    bool flip_vertical;
    bool night_mode;
    uint8_t test_pattern;
} OV7670_Config_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;
    DCMI_HandleTypeDef *hdcmi;
    OV7670_Config_t config;
    bool initialized;
    uint16_t chip_id;
} OV7670_Handle_t;

/**
 * @brief Guard shared by every public entry point.
 */
static inline OV7670_StatusTypeDef OV7670_CheckReady(const OV7670_Handle_t *hov7670)
{
    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }
    if (!hov7670->initialized || hov7670->hi2c == NULL) {
        return OV7670_NOT_INITIALIZED;
    }
    return OV7670_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* OV7670_TYPES_H */
