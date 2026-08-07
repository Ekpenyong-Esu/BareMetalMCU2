/**
 * @file ts_core.c
 * @brief Touchscreen lifecycle, controller bring-up and handle registry
 */

#include "ts_core.h"
#include "ts_io.h"
#include "ts_events.h"
#include "ts_calibration.h"
#include "ts_stmpe811.h"
#include "i2c.h"
#include "log.h"
#include <string.h>

/* Private constants ---------------------------------------------------------*/
#define TS_ADC_CTRL_12BIT               0x49U   /*!< BSP setting: 64 sample time, 12-bit, internal reference */
#define TS_ADC_CLOCK_3_25MHZ            0x01U   /*!< ADC_CTRL2: 3.25 MHz ADC clock */
#define TS_TSC_CFG_DEFAULT              0x9AU   /*!< 4-sample average, 500us detect delay, 500us settling */
#define TS_FIFO_THRESHOLD_SINGLE        0x01U   /*!< Interrupt after a single point */
#define TS_TSC_FRACT_XYZ_DEFAULT        0x01U   /*!< Z pressure: 7 fractional, 1 whole */
#define TS_TSC_DRIVE_50MA               0x01U   /*!< TSC pin driving capability */

/** One entry of the fixed controller start-up sequence. */
typedef struct {
    uint8_t reg;
    uint8_t value;
} TS_RegisterWrite_t;

/* Private data --------------------------------------------------------------*/

/* Applied in order once TSC and ADC are powered; see the STMPE811 datasheet. */
static const TS_RegisterWrite_t s_startupSequence[] = {
    {STMPE811_REG_ADC_CTRL2,     TS_ADC_CLOCK_3_25MHZ},
    {STMPE811_REG_TSC_CFG,       TS_TSC_CFG_DEFAULT},
    {STMPE811_REG_FIFO_TH,       TS_FIFO_THRESHOLD_SINGLE},
    {STMPE811_REG_FIFO_STA,      STMPE811_FIFO_RESET},
    {STMPE811_REG_FIFO_STA,      STMPE811_FIFO_OPERATIONAL},
    {STMPE811_REG_TSC_FRACT_XYZ, TS_TSC_FRACT_XYZ_DEFAULT},
    {STMPE811_REG_TSC_I_DRIVE,   TS_TSC_DRIVE_50MA},
    {STMPE811_REG_TSC_CTRL,      STMPE811_TSC_CTRL_EN},
    {STMPE811_REG_INT_STA,       STMPE811_INT_CLEAR_ALL},
};

static TS_HandleTypeDef *s_handle = NULL;   /*!< Handle the touch interrupt resolves to */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Adopt the shared I2C3 handle when the caller supplied an unconfigured one
 * @param hts Touchscreen handle
 */
static void TS_ResolveBus(TS_HandleTypeDef *hts)
{
    if (hts->hi2c->Instance == NULL) {
        I2C_Init();
        hts->hi2c = I2C_GetHandle();
    }
}

/**
 * @brief Confirm an STMPE811 is answering on the bus
 * @param hts Touchscreen handle
 * @return TS_StatusTypeDef TS_DEVICE_NOT_FOUND when the ID does not match
 */
static TS_StatusTypeDef TS_CheckDevice(TS_HandleTypeDef *hts)
{
    if (I2C_IsDeviceReady(STMPE811_I2C_ADDRESS, 3, 100) != I2C_OK) {
        log_debug("TS: Device not found on I2C bus");
        return TS_DEVICE_NOT_FOUND;
    }

    uint16_t deviceId = 0;
    if (TS_ReadRegister16(hts, STMPE811_REG_CHIP_ID, &deviceId) != TS_OK) {
        log_debug("TS: Failed to read device ID");
        return TS_DEVICE_NOT_FOUND;
    }

    if (deviceId != STMPE811_CHIP_ID) {
        log_debug("TS: Device ID mismatch");
        return TS_DEVICE_NOT_FOUND;
    }

    log_debug("TS: Device found, ID: 0x%04X", deviceId);
    return TS_OK;
}

/**
 * @brief Clear the given bits in a register, leaving the rest untouched
 * @param hts Touchscreen handle
 * @param reg Register address
 * @param mask Bits to clear
 * @return TS_StatusTypeDef Status of the operation
 */
static TS_StatusTypeDef TS_ClearRegisterBits(TS_HandleTypeDef *hts, uint8_t reg, uint8_t mask)
{
    uint8_t value = 0;
    if (TS_ReadRegister(hts, reg, &value) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    value &= (uint8_t)~mask;
    if (TS_WriteRegister(hts, reg, value) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    return TS_OK;
}

/**
 * @brief Power up the touch blocks and apply the start-up register sequence
 * @param hts Touchscreen handle
 * @return TS_StatusTypeDef Status of the operation
 * @note Sequence based on ST BSP STMPE811_TS_Init()
 */
static TS_StatusTypeDef TS_ConfigureController(TS_HandleTypeDef *hts)
{
    TS_Reset(hts);

    /* Powering blocks up means clearing their OFF bits. */
    if (TS_ClearRegisterBits(hts, STMPE811_REG_SYS_CTRL2, STMPE811_SYS_CTRL2_GPIO_OFF) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }
    if (TS_ClearRegisterBits(hts, STMPE811_REG_IO_AF, STMPE811_TOUCH_IO_ALL) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }
    if (TS_ClearRegisterBits(hts, STMPE811_REG_SYS_CTRL2,
                             STMPE811_SYS_CTRL2_TSC_OFF | STMPE811_SYS_CTRL2_ADC_OFF) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }

    if (TS_WriteRegister(hts, STMPE811_REG_ADC_CTRL1, TS_ADC_CTRL_12BIT) != TS_OK) {
        return TS_COMMUNICATION_ERROR;
    }
    HAL_Delay(2);   /* ADC needs to stabilise before its clock is selected */

    for (size_t i = 0; i < (sizeof(s_startupSequence) / sizeof(s_startupSequence[0])); i++) {
        if (TS_WriteRegister(hts, s_startupSequence[i].reg, s_startupSequence[i].value) != TS_OK) {
            log_error("TS: Failed to write STMPE811 register 0x%02X", s_startupSequence[i].reg);
            return TS_COMMUNICATION_ERROR;
        }
    }

    HAL_Delay(2);
    return TS_OK;
}

/* Public functions ----------------------------------------------------------*/

TS_HandleTypeDef* TS_GetHandle(void)
{
    return s_handle;
}

TS_StatusTypeDef TS_Init(TS_HandleTypeDef *hts, I2C_HandleTypeDef *hi2c)
{
    if (hts == NULL || hi2c == NULL) {
        return TS_INVALID_PARAM;
    }

    memset(hts, 0, sizeof(TS_HandleTypeDef));
    hts->hi2c = hi2c;
    TS_GetDefaultCalibration(&hts->Calibration);
    s_handle = hts;

    TS_ResolveBus(hts);

    TS_StatusTypeDef status = TS_CheckDevice(hts);
    if (status != TS_OK) {
        return status;
    }

    TS_ConfigTypeDef defaultConfig = TS_GetDefaultConfig();
    status = TS_Configure(hts, &defaultConfig);
    if (status != TS_OK) {
        return status;
    }

    status = TS_ConfigureController(hts);
    if (status != TS_OK) {
        return status;
    }

    TS_EnableInterrupt(hts, hts->Config.InterruptEnable);
    if (hts->Config.InterruptEnable) {
        TS_ITConfig(hts);
    }

    hts->IsInitialized = true;
    return TS_OK;
}

TS_StatusTypeDef TS_DeInit(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    TS_EnableInterrupt(hts, false);
    TS_WriteRegister(hts, STMPE811_REG_SYS_CTRL2, STMPE811_SYS_CTRL2_TSC_OFF);

    hts->IsInitialized = false;
    if (s_handle == hts) {
        s_handle = NULL;
    }

    return TS_OK;
}

TS_StatusTypeDef TS_Configure(TS_HandleTypeDef *hts, TS_ConfigTypeDef *config)
{
    if (hts == NULL || config == NULL) {
        return TS_INVALID_PARAM;
    }

    hts->Config = *config;
    return TS_OK;
}

TS_StatusTypeDef TS_Reset(TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    TS_WriteRegister(hts, STMPE811_REG_SYS_CTRL1, STMPE811_SYS_CTRL1_RESET);
    HAL_Delay(5);
    TS_WriteRegister(hts, STMPE811_REG_SYS_CTRL1, 0x00);
    HAL_Delay(2);

    return TS_OK;
}

TS_ConfigTypeDef TS_GetDefaultConfig(void)
{
    TS_ConfigTypeDef config = {
        .InterruptEnable = true
    };

    return config;
}

TS_StatusTypeDef TS_RegisterCallbacks(TS_HandleTypeDef *hts,
                                      void (*touch_callback)(void),
                                      void (*release_callback)(void),
                                      void (*gesture_callback)(TS_GestureTypeDef))
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }

    hts->TouchCallback = touch_callback;
    hts->ReleaseCallback = release_callback;
    hts->GestureCallback = gesture_callback;

    return TS_OK;
}
