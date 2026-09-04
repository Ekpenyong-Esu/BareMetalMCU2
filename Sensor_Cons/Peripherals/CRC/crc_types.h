/**
 ******************************************************************************
 * @file    crc_types.h
 * @brief   Shared vocabulary for the CRC driver
 ******************************************************************************
 */

#ifndef CRC_TYPES_H
#define CRC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CRC_MAX_DATA_SIZE 1024U
#define CRC_DEFAULT_POLYNOMIAL 0x04C11DB7U
#define CRC_DEFAULT_INIT_VALUE 0xFFFFFFFFU

#define CRC_POLY_CRC32 0x04C11DB7U
#define CRC_POLY_CRC32C 0x1EDC6F41U
#define CRC_POLY_CRC16 0x8005U
#define CRC_POLY_CRC16_CCITT 0x1021U
#define CRC_POLY_CRC8 0x07U

typedef enum { CRC_FORMAT_8BIT = 0U, CRC_FORMAT_16BIT = 1U, CRC_FORMAT_32BIT = 2U } CRC_DataFormat;

typedef enum { CRC_METHOD_HARDWARE = 0U, CRC_METHOD_SOFTWARE = 1U } CRC_Method;

typedef enum {
    CRC_ERROR_NONE = 0,
    CRC_ERROR_INVALID_PARAM,
    CRC_ERROR_DATA_SIZE,
    CRC_ERROR_HARDWARE,
    CRC_ERROR_TIMEOUT
} CRC_ErrorType;

typedef struct {
    CRC_Method method;
    uint32_t polynomial;
    uint32_t init_value;
    bool input_reverse;
    bool output_reverse;
    CRC_DataFormat input_format;
    CRC_DataFormat output_format;
    bool xor_output;
    uint32_t xor_value;
} CRC_Config;

typedef struct {
    bool initialized;
    CRC_Method current_method;
    uint32_t last_calculated_crc;
    CRC_ErrorType last_error;
    uint32_t calculation_count;
    uint32_t hardware_usage_count;
    uint32_t software_usage_count;
} CRC_Status;

typedef void (*CRC_CompleteCallback)(uint32_t crc_value);
typedef void (*CRC_ErrorCallbackFn)(CRC_ErrorType error);

#ifdef __cplusplus
}
#endif

#endif /* CRC_TYPES_H */
