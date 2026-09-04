/**
 ******************************************************************************
 * @file    mems_types.h
 * @brief   Shared vocabulary for the L3GD20 MEMS gyroscope driver
 ******************************************************************************
 */

#ifndef MEMS_TYPES_H
#define MEMS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "spi_core.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MEMS_SPI_TIMEOUT 1000U
#define MEMS_MAX_RETRIES 3U

/* Temperature offset used when converting the raw OUT_TEMP byte to degrees C. */
#define MEMS_TEMPERATURE_OFFSET 25.0f

/* Raw shift window a healthy gyro must land in during self-test. */
#define MEMS_SELF_TEST_DIFF_MIN 100
#define MEMS_SELF_TEST_DIFF_MAX 1000

typedef enum {
    MEMS_OK = 0,
    MEMS_ERROR,
    MEMS_BUSY,
    MEMS_TIMEOUT,
    MEMS_INVALID_PARAM,
    MEMS_NOT_INITIALIZED,
    MEMS_COMMUNICATION_ERROR,
    MEMS_DEVICE_NOT_FOUND
} MEMS_StatusTypeDef;

typedef enum {
    MEMS_GYRO_ODR_95Hz = 0,
    MEMS_GYRO_ODR_190Hz,
    MEMS_GYRO_ODR_380Hz,
    MEMS_GYRO_ODR_760Hz,
    MEMS_GYRO_ODR_COUNT
} MEMS_GyroODRTypeDef;

typedef enum {
    MEMS_GYRO_FULLSCALE_250 = 0,
    MEMS_GYRO_FULLSCALE_500,
    MEMS_GYRO_FULLSCALE_2000,
    MEMS_GYRO_FULLSCALE_COUNT
} MEMS_GyroFullScaleTypeDef;

typedef enum {
    MEMS_GYRO_BANDWIDTH_1 = 0,
    MEMS_GYRO_BANDWIDTH_2,
    MEMS_GYRO_BANDWIDTH_3,
    MEMS_GYRO_BANDWIDTH_4,
    MEMS_GYRO_BANDWIDTH_COUNT
} MEMS_GyroBandwidthTypeDef;

typedef struct {
    MEMS_GyroODRTypeDef OutputDataRate;
    MEMS_GyroFullScaleTypeDef FullScale;
    MEMS_GyroBandwidthTypeDef Bandwidth;
    bool XAxisEnable;
    bool YAxisEnable;
    bool ZAxisEnable;
    bool PowerDownMode;
} MEMS_GyroConfigTypeDef;

typedef struct {
    bool InterruptEnable;  /**< Route interrupt generator to INT1 */
    bool BootStatusEnable; /**< Route boot status to INT1 */
    bool ActiveLevel;      /**< false = active high, true = active low */
    bool OutputType;       /**< false = push-pull, true = open drain */
    bool DataReadyEnable;  /**< Data ready on INT2 */
    bool WatermarkEnable;  /**< FIFO watermark on INT2 */
    bool OverrunEnable;    /**< FIFO overrun on INT2 */
    bool EmptyEnable;      /**< FIFO empty on INT2 */
} MEMS_InterruptConfigTypeDef;

typedef struct {
    int16_t X;
    int16_t Y;
    int16_t Z;
} MEMS_AxesRawTypeDef;

typedef struct {
    float X;
    float Y;
    float Z;
} MEMS_AxesTypeDef;

typedef struct {
    uint8_t WhoAmI;
    const char *DeviceName;
    bool IsPresent;
    uint8_t Version;
} MEMS_DeviceInfoTypeDef;

/**
 * @brief Wiring the application gives the driver.
 * @note  The bus (SCK/MISO/MOSI) is opened by the application beforehand; the
 *        driver only owns its chip select and the interrupt inputs. A NULL
 *        INT port means that line is not connected.
 */
typedef struct {
    SPI_Bus_t *Bus;        /**< Open bus the gyro is wired to */
    GPIO_TypeDef *CS_Port; /**< Chip select, driven in software */
    uint16_t CS_Pin;
    GPIO_TypeDef *INT1_Port; /**< Interrupt-generator output, or NULL */
    uint16_t INT1_Pin;
    GPIO_TypeDef *INT2_Port; /**< Data-ready / FIFO output, or NULL */
    uint16_t INT2_Pin;
} MEMS_Config_t;

/**
 * @brief Driver handle.
 * @note  The bus may also carry other chips, so the settings this gyro needs
 *        travel with the device and are reapplied whenever it is selected
 *        rather than being inherited from whichever driver ran last.
 */
typedef struct {
    SPI_Device_t device; /**< Bus settings this gyro needs */
    GPIO_TypeDef *CS_Port;
    uint16_t CS_Pin;
    GPIO_TypeDef *INT1_Port; /**< NULL when not wired */
    uint16_t INT1_Pin;
    GPIO_TypeDef *INT2_Port; /**< NULL when not wired */
    uint16_t INT2_Pin;
    MEMS_GyroConfigTypeDef GyroConfig;
    bool IsInitialized;
    bool IsCalibrated;
    MEMS_AxesTypeDef CalibrationOffset;
} MEMS_HandleTypeDef;

/**
 * @brief Guard shared by every public entry point.
 */
static inline MEMS_StatusTypeDef MEMS_CheckReady(const MEMS_HandleTypeDef *hmems) {
    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }
    if (!hmems->IsInitialized || !SPI_DeviceIsReady(&hmems->device)) {
        return MEMS_NOT_INITIALIZED;
    }
    return MEMS_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* MEMS_TYPES_H */
