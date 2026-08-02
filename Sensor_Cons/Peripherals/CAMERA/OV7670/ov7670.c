/**
  ******************************************************************************
  * @file    ov7670.c
  * @brief   OV7670 CMOS Camera Sensor Driver Implementation
  * @details This file provides the implementation of OV7670 camera functions
  *          using I2C interface for control and DCMI for video capture on STM32F429I-DISC1.
  * @version 1.0
  * @date    2026-01-20
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ov7670.h"
#include "i2c.h"
#include "stm32f4xx_hal_dcmi.h"
#include "gpio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup OV7670_Private_Defines Private Defines
 * @{
 */

/* OV7670 Register addresses */
#define OV7670_REG_GAIN               0x00    /**< Gain control */
#define OV7670_REG_BLUE               0x01    /**< Blue channel gain */
#define OV7670_REG_RED                0x02    /**< Red channel gain */
#define OV7670_REG_VREF               0x03    /**< Vertical reference */
#define OV7670_REG_COM1               0x04    /**< Common control 1 */
#define OV7670_REG_BAVE               0x05    /**< B channel average */
#define OV7670_REG_GbAVE              0x06    /**< Gb channel average */
#define OV7670_REG_AECHH              0x07    /**< AEC MS 5 bits */
#define OV7670_REG_RAVE               0x08    /**< R channel average */
#define OV7670_REG_COM2               0x09    /**< Common control 2 */
#define OV7670_REG_PID                0x0A    /**< Product ID MSB */
#define OV7670_REG_VER                0x0B    /**< Product ID LSB */
#define OV7670_REG_COM3               0x0C    /**< Common control 3 */
#define OV7670_REG_COM4               0x0D    /**< Common control 4 */
#define OV7670_REG_COM5               0x0E    /**< Common control 5 */
#define OV7670_REG_COM6               0x0F    /**< Common control 6 */
#define OV7670_REG_AECH               0x10    /**< AEC LSB */
#define OV7670_REG_CLKRC              0x11    /**< Clock rate control */
#define OV7670_REG_COM7               0x12    /**< Common control 7 */
#define OV7670_REG_COM8               0x13    /**< Common control 8 */
#define OV7670_REG_COM9               0x14    /**< Common control 9 */
#define OV7670_REG_COM10              0x15    /**< Common control 10 */
#define OV7670_REG_HSTART             0x17    /**< Horizontal start */
#define OV7670_REG_HSTOP              0x18    /**< Horizontal stop */
#define OV7670_REG_VSTART             0x19    /**< Vertical start */
#define OV7670_REG_VSTOP              0x1A    /**< Vertical stop */
#define OV7670_REG_PSHFT              0x1B    /**< Pixel shift */
#define OV7670_REG_MIDH               0x1C    /**< Manufacturer ID MSB */
#define OV7670_REG_MIDL               0x1D    /**< Manufacturer ID LSB */
#define OV7670_REG_MVFP               0x1E    /**< Mirror/VFlip */
#define OV7670_REG_LAEC               0x1F    /**< AEC LSB */
#define OV7670_REG_ADCCTR0            0x20    /**< ADC control 0 */
#define OV7670_REG_ADCCTR1            0x21    /**< ADC control 1 */
#define OV7670_REG_ADCCTR2            0x22    /**< ADC control 2 */
#define OV7670_REG_ADCCTR3            0x23    /**< ADC control 3 */
#define OV7670_REG_AEW                0x24    /**< AGC/AEC upper limit */
#define OV7670_REG_AEB                0x25    /**< AGC/AEC lower limit */
#define OV7670_REG_VPT                0x26    /**< AGC/AEC fast mode */
#define OV7670_REG_BBIAS              0x27    /**< B channel bias */
#define OV7670_REG_GbBIAS             0x28    /**< Gb channel bias */
#define OV7670_REG_EXHCH              0x2A    /**< Exposure value */
#define OV7670_REG_EXHCL              0x2B    /**< Exposure value */
#define OV7670_REG_RBIAS              0x2C    /**< R channel bias */
#define OV7670_REG_ADVFL              0x2D    /**< Insert dummy lines */
#define OV7670_REG_ADVFH              0x2E    /**< Insert dummy lines */
#define OV7670_REG_YAVE               0x2F    /**< Y/G channel average */
#define OV7670_REG_HSYST              0x30    /**< HSYNC rising edge delay */
#define OV7670_REG_HSYEN              0x31    /**< HSYNC falling edge delay */
#define OV7670_REG_HREF               0x32    /**< HREF control */
#define OV7670_REG_CHLF               0x33    /**< Array current control */
#define OV7670_REG_ARBLM              0x34    /**< Array reference control */
#define OV7670_REG_ADC                0x37    /**< ADC reference control */
#define OV7670_REG_ACOM               0x38    /**< ADC and analog common */
#define OV7670_REG_OFON               0x39    /**< ADC offset control */
#define OV7670_REG_TSLB               0x3A    /**< Line buffer test option */
#define OV7670_REG_COM11              0x3B    /**< Common control 11 */
#define OV7670_REG_COM12              0x3C    /**< Common control 12 */
#define OV7670_REG_COM13              0x3D    /**< Common control 13 */
#define OV7670_REG_COM14              0x3E    /**< Common control 14 */
#define OV7670_REG_EDGE               0x3F    /**< Edge enhancement */
#define OV7670_REG_COM15              0x40    /**< Common control 15 */
#define OV7670_REG_COM16              0x41    /**< Common control 16 */
#define OV7670_REG_COM17              0x42    /**< Common control 17 */
#define OV7670_REG_AWBC1              0x43    /**< AWB control 1 */
#define OV7670_REG_AWBC2              0x44    /**< AWB control 2 */
#define OV7670_REG_AWBC3              0x45    /**< AWB control 3 */
#define OV7670_REG_AWBC4              0x46    /**< AWB control 4 */
#define OV7670_REG_AWBC5              0x47    /**< AWB control 5 */
#define OV7670_REG_AWBC6              0x48    /**< AWB control 6 */
#define OV7670_REG_REG4B              0x4B    /**< UV average */
#define OV7670_REG_DNSTH              0x4C    /**< De-noise strength */
#define OV7670_REG_MTX1               0x4F    /**< Matrix coefficient 1 */
#define OV7670_REG_MTX2               0x50    /**< Matrix coefficient 2 */
#define OV7670_REG_MTX3               0x51    /**< Matrix coefficient 3 */
#define OV7670_REG_MTX4               0x52    /**< Matrix coefficient 4 */
#define OV7670_REG_MTX5               0x53    /**< Matrix coefficient 5 */
#define OV7670_REG_MTX6               0x54    /**< Matrix coefficient 6 */
#define OV7670_REG_BRIGHT             0x55    /**< Brightness */
#define OV7670_REG_CONTRAS            0x56    /**< Contrast */
#define OV7670_REG_CONTRAS_CENTER     0x57    /**< Contrast center */
#define OV7670_REG_MTXS               0x58    /**< Matrix coefficient sign */
#define OV7670_REG_LCC1               0x62    /**< Lens correction option 1 */
#define OV7670_REG_LCC2               0x63    /**< Lens correction option 2 */
#define OV7670_REG_LCC3               0x64    /**< Lens correction option 3 */
#define OV7670_REG_LCC4               0x65    /**< Lens correction option 4 */
#define OV7670_REG_LCC5               0x66    /**< Lens correction option 5 */
#define OV7670_REG_MANU               0x67    /**< Manual U value */
#define OV7670_REG_MANV               0x68    /**< Manual V value */
#define OV7670_REG_GFIX               0x69    /**< Fix gain control */
#define OV7670_REG_GGAIN              0x6A    /**< G channel AWB gain */
#define OV7670_REG_DBLV               0x6B    /**< PLL control */
#define OV7670_REG_AWBCTR3            0x6C    /**< AWB control 3 */
#define OV7670_REG_AWBCTR2            0x6D    /**< AWB control 2 */
#define OV7670_REG_AWBCTR1            0x6E    /**< AWB control 1 */
#define OV7670_REG_AWBCTR0            0x6F    /**< AWB control 0 */
#define OV7670_REG_SCALING_XSC        0x70    /**< Horizontal scaling */
#define OV7670_REG_SCALING_YSC        0x71    /**< Vertical scaling */
#define OV7670_REG_SCALING_DCWCTR     0x72    /**< DCW control */
#define OV7670_REG_SCALING_PCLK_DIV   0x73    /**< Clock divider */
#define OV7670_REG_REG74              0x74    /**< Digital gain control */
#define OV7670_REG_REG75              0x75    /**< Edge enhancement */
#define OV7670_REG_REG76              0x76    /**< Pixel correction */
#define OV7670_REG_REG77              0x77    /**< Pixel correction */
#define OV7670_REG_DBLC1              0x87    /**< Digital gain control */
#define OV7670_REG_AECHG              0x8D    /**< AEC algorithm selection */

/* Timing constants */
#define OV7670_I2C_TIMEOUT            100     /**< I2C timeout in ms */
#define OV7670_RESET_DELAY            100     /**< Reset delay in ms */

/** @} */

/* Private types ------------------------------------------------------------*/

/**
 * @brief Register value structure
 */
typedef struct {
    uint8_t reg_addr;
    uint8_t value;
} OV7670_RegVal_t;

/* Private variables ---------------------------------------------------------*/

/* QVGA RGB565 configuration */
static const OV7670_RegVal_t OV7670_QVGA_RGB565[] = {
    {OV7670_REG_COM7, 0x14},     /* QVGA, RGB */
    {OV7670_REG_CLKRC, 0x01},    /* Clock divider */
    {OV7670_REG_TSLB, 0x04},     /* Line buffer test option */
    {OV7670_REG_COM10, 0x02},    /* PCLK does not toggle during horizontal blank */
    {OV7670_REG_HSTART, 0x16},   /* Horizontal start */
    {OV7670_REG_HSTOP, 0x04},    /* Horizontal stop */
    {OV7670_REG_HREF, 0x24},     /* HREF control */
    {OV7670_REG_VSTART, 0x02},   /* Vertical start */
    {OV7670_REG_VSTOP, 0x7a},    /* Vertical stop */
    {OV7670_REG_VREF, 0x0a},     /* Vertical reference */
    {OV7670_REG_COM3, 0x04},     /* Common control 3 */
    {OV7670_REG_COM14, 0x1a},    /* Common control 14 */
    {OV7670_REG_SCALING_XSC, 0x3a}, /* Horizontal scaling */
    {OV7670_REG_SCALING_YSC, 0x35}, /* Vertical scaling */
    {OV7670_REG_SCALING_DCWCTR, 0x11}, /* DCW control */
    {OV7670_REG_SCALING_PCLK_DIV, 0xf0}, /* Clock divider */
    {OV7670_REG_COM9, 0x2a},     /* Common control 9 */
    {OV7670_REG_COM16, 0x08},    /* Common control 16 */
    {OV7670_REG_COM17, 0x00},    /* Common control 17 */
    {OV7670_REG_COM1, 0x00},     /* Common control 1 */
    {OV7670_REG_COM2, 0x01},     /* Common control 2 */
    {OV7670_REG_COM15, 0xc0},    /* Common control 15 - RGB565 */
    {0xFF, 0xFF}                 /* End marker */
};

/* VGA RGB565 configuration */
static const OV7670_RegVal_t OV7670_VGA_RGB565[] = {
    {OV7670_REG_COM7, 0x14},     /* VGA, RGB */
    {OV7670_REG_CLKRC, 0x01},    /* Clock divider */
    {OV7670_REG_TSLB, 0x04},     /* Line buffer test option */
    {OV7670_REG_COM10, 0x02},    /* PCLK does not toggle during horizontal blank */
    {OV7670_REG_HSTART, 0x13},   /* Horizontal start */
    {OV7670_REG_HSTOP, 0x01},    /* Horizontal stop */
    {OV7670_REG_HREF, 0xb6},     /* HREF control */
    {OV7670_REG_VSTART, 0x02},   /* Vertical start */
    {OV7670_REG_VSTOP, 0x7a},    /* Vertical stop */
    {OV7670_REG_VREF, 0x0a},     /* Vertical reference */
    {OV7670_REG_COM3, 0x00},     /* Common control 3 */
    {OV7670_REG_COM14, 0x00},    /* Common control 14 */
    {OV7670_REG_SCALING_XSC, 0x3a}, /* Horizontal scaling */
    {OV7670_REG_SCALING_YSC, 0x35}, /* Vertical scaling */
    {OV7670_REG_SCALING_DCWCTR, 0x11}, /* DCW control */
    {OV7670_REG_SCALING_PCLK_DIV, 0xf0}, /* Clock divider */
    {OV7670_REG_COM9, 0x2a},     /* Common control 9 */
    {OV7670_REG_COM16, 0x08},    /* Common control 16 */
    {OV7670_REG_COM17, 0x00},    /* Common control 17 */
    {OV7670_REG_COM1, 0x00},     /* Common control 1 */
    {OV7670_REG_COM2, 0x01},     /* Common control 2 */
    {OV7670_REG_COM15, 0xc0},    /* Common control 15 - RGB565 */
    {0xFF, 0xFF}                 /* End marker */
};

/* Private function prototypes -----------------------------------------------*/
static OV7670_StatusTypeDef OV7670_WriteReg(OV7670_Handle_t *hov7670, uint8_t reg, uint8_t value);
static OV7670_StatusTypeDef OV7670_ReadReg(OV7670_Handle_t *hov7670, uint8_t reg, uint8_t *value);
static OV7670_StatusTypeDef OV7670_WriteRegList(OV7670_Handle_t *hov7670, const OV7670_RegVal_t *reg_list);
static OV7670_StatusTypeDef OV7670_SetResolutionConfig(OV7670_Handle_t *hov7670, OV7670_ResolutionTypeDef resolution);
static OV7670_StatusTypeDef OV7670_SetFormatConfig(OV7670_Handle_t *hov7670, OV7670_FormatTypeDef format);
static void OV7670_DelayMs(uint32_t delay);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize OV7670 camera sensor
 * @details Configures I2C and DCMI interfaces and initializes the camera
 * @param   hov7670 Pointer to OV7670 handle
 * @param   hi2c I2C handle for camera control
 * @param   hdcmi DCMI handle for video capture
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_Init(OV7670_Handle_t *hov7670,
                                I2C_HandleTypeDef *hi2c,
                                DCMI_HandleTypeDef *hdcmi)
{
    OV7670_StatusTypeDef status = OV7670_OK;
    uint16_t chip_id = 0;

    if (hov7670 == NULL || hi2c == NULL || hdcmi == NULL) {
        return OV7670_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hov7670, 0, sizeof(OV7670_Handle_t));

    hov7670->hi2c = hi2c;
    hov7670->hdcmi = hdcmi;

    /* Default configuration */
    hov7670->config.resolution = OV7670_RES_QVGA;
    hov7670->config.format = OV7670_FMT_RGB565;
    hov7670->config.brightness = 128;
    hov7670->config.contrast = 128;
    hov7670->config.saturation = 128;
    hov7670->config.flip_horizontal = false;
    hov7670->config.flip_vertical = false;
    hov7670->config.night_mode = false;
    hov7670->config.test_pattern = OV7670_TEST_PATTERN_NONE;

    /* Check chip ID */
    status = OV7670_GetChipID(hov7670, &chip_id);
    if (status != OV7670_OK) {
        return status;
    }

    if (chip_id != OV7670_ID) {
        return OV7670_INVALID_ID;
    }

    hov7670->chip_id = chip_id;

    /* Reset camera */
    status = OV7670_Reset(hov7670);
    if (status != OV7670_OK) {
        return status;
    }

    /* Configure camera with default settings */
    status = OV7670_Config(hov7670, &hov7670->config);
    if (status != OV7670_OK) {
        return status;
    }

    hov7670->initialized = true;

    return OV7670_OK;
}

/**
 * @brief   Deinitialize OV7670 camera sensor
 * @details Releases resources and powers down the camera
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_DeInit(OV7670_Handle_t *hov7670)
{
    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Stop any ongoing capture */
    OV7670_StopCapture(hov7670);

    /* Power down camera */
    OV7670_WriteReg(hov7670, OV7670_REG_COM2, 0x10); /* Power down */

    hov7670->initialized = false;

    return OV7670_OK;
}

/**
 * @brief   Configure OV7670 camera parameters
 * @details Sets camera configuration options
 * @param   hov7670 Pointer to OV7670 handle
 * @param   config Pointer to configuration structure
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_Config(OV7670_Handle_t *hov7670, OV7670_Config_t *config)
{
    OV7670_StatusTypeDef status = OV7670_OK;

    if (hov7670 == NULL || config == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Set resolution */
    status = OV7670_SetResolution(hov7670, config->resolution);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set format */
    status = OV7670_SetFormat(hov7670, config->format);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set brightness */
    status = OV7670_SetBrightness(hov7670, config->brightness);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set contrast */
    status = OV7670_SetContrast(hov7670, config->contrast);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set saturation */
    status = OV7670_SetSaturation(hov7670, config->saturation);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set flip options */
    status = OV7670_SetFlipHorizontal(hov7670, config->flip_horizontal);
    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_SetFlipVertical(hov7670, config->flip_vertical);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set night mode */
    status = OV7670_SetNightMode(hov7670, config->night_mode);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set test pattern */
    status = OV7670_SetTestPattern(hov7670, config->test_pattern);
    if (status != OV7670_OK) {
        return status;
    }

    /* Copy configuration */
    memcpy(&hov7670->config, config, sizeof(OV7670_Config_t));

    return OV7670_OK;
}

/**
 * @brief   Reset OV7670 camera sensor
 * @details Performs software reset of the camera
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_Reset(OV7670_Handle_t *hov7670)
{
    OV7670_StatusTypeDef status;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    /* Software reset */
    status = OV7670_WriteReg(hov7670, OV7670_REG_COM7, 0x80);
    if (status != OV7670_OK) {
        return status;
    }

    /* Wait for reset to complete */
    OV7670_DelayMs(OV7670_RESET_DELAY);

    return OV7670_OK;
}

/**
 * @brief   Start image capture
 * @details Begins capturing image data to the specified buffer
 * @param   hov7670 Pointer to OV7670 handle
 * @param   buffer Destination buffer for image data
 * @param   length Buffer length in bytes
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_StartCapture(OV7670_Handle_t *hov7670,
                                        uint32_t *buffer, uint32_t length)
{
    if (hov7670 == NULL || buffer == NULL || length == 0) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Start DCMI capture */
    if (HAL_DCMI_Start_DMA(hov7670->hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)buffer, length) != HAL_OK) {
        return OV7670_ERROR;
    }

    return OV7670_OK;
}

/**
 * @brief   Stop image capture
 * @details Stops the current capture operation
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_StopCapture(OV7670_Handle_t *hov7670)
{
    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Stop DCMI capture */
    if (HAL_DCMI_Stop(hov7670->hdcmi) != HAL_OK) {
        return OV7670_ERROR;
    }

    return OV7670_OK;
}

/**
 * @brief   Check if capture is complete
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  bool True if capture complete, false otherwise
 */
bool OV7670_IsCaptureComplete(OV7670_Handle_t *hov7670)
{
    if (hov7670 == NULL || !hov7670->initialized) {
        return false;
    }

    /* Check DCMI state */
    return (HAL_DCMI_GetState(hov7670->hdcmi) == HAL_DCMI_STATE_READY);
}

/**
 * @brief   Set camera resolution
 * @param   hov7670 Pointer to OV7670 handle
 * @param   resolution New resolution
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetResolution(OV7670_Handle_t *hov7670,
                                         OV7670_ResolutionTypeDef resolution)
{
    OV7670_StatusTypeDef status;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    status = OV7670_SetResolutionConfig(hov7670, resolution);
    if (status == OV7670_OK) {
        hov7670->config.resolution = resolution;
    }

    return status;
}

/**
 * @brief   Set color format
 * @param   hov7670 Pointer to OV7670 handle
 * @param   format New color format
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetFormat(OV7670_Handle_t *hov7670,
                                     OV7670_FormatTypeDef format)
{
    OV7670_StatusTypeDef status;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    status = OV7670_SetFormatConfig(hov7670, format);
    if (status == OV7670_OK) {
        hov7670->config.format = format;
    }

    return status;
}

/**
 * @brief   Set brightness
 * @param   hov7670 Pointer to OV7670 handle
 * @param   brightness Brightness value (0-255)
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetBrightness(OV7670_Handle_t *hov7670, uint8_t brightness)
{
    OV7670_StatusTypeDef status;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Convert 0-255 to -128 to +127 */
    int8_t brt_val = (int8_t)(brightness - 128);

    status = OV7670_WriteReg(hov7670, OV7670_REG_BRIGHT, (uint8_t)brt_val);
    if (status == OV7670_OK) {
        hov7670->config.brightness = brightness;
    }

    return status;
}

/**
 * @brief   Set contrast
 * @param   hov7670 Pointer to OV7670 handle
 * @param   contrast Contrast value (0-255)
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetContrast(OV7670_Handle_t *hov7670, uint8_t contrast)
{
    OV7670_StatusTypeDef status;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Contrast is controlled by MTX registers */
    uint8_t contrast_val = contrast / 4; /* Scale to 0-63 */

    status = OV7670_WriteReg(hov7670, OV7670_REG_CONTRAS, contrast_val);
    if (status == OV7670_OK) {
        hov7670->config.contrast = contrast;
    }

    return status;
}

/**
 * @brief   Set saturation
 * @param   hov7670 Pointer to OV7670 handle
 * @param   saturation Saturation value (0-255)
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetSaturation(OV7670_Handle_t *hov7670, uint8_t saturation)
{
    OV7670_StatusTypeDef status;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Saturation is controlled by MTX registers */
    uint8_t sat_val = saturation / 4; /* Scale to 0-63 */

    /* Set UV saturation */
    status = OV7670_WriteReg(hov7670, OV7670_REG_MTX1, sat_val);
    if (status != OV7670_OK) return status;

    status = OV7670_WriteReg(hov7670, OV7670_REG_MTX4, sat_val);
    if (status == OV7670_OK) {
        hov7670->config.saturation = saturation;
    }

    return status;
}

/**
 * @brief   Enable/disable horizontal flip
 * @param   hov7670 Pointer to OV7670 handle
 * @param   enable True to enable flip, false to disable
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetFlipHorizontal(OV7670_Handle_t *hov7670, bool enable)
{
    OV7670_StatusTypeDef status;
    uint8_t reg_val;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Read current MVFP register */
    status = OV7670_ReadReg(hov7670, OV7670_REG_MVFP, &reg_val);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set/clear mirror bit */
    if (enable) {
        reg_val |= 0x20;  /* Set mirror bit */
    } else {
        reg_val &= ~0x20; /* Clear mirror bit */
    }

    status = OV7670_WriteReg(hov7670, OV7670_REG_MVFP, reg_val);
    if (status == OV7670_OK) {
        hov7670->config.flip_horizontal = enable;
    }

    return status;
}

/**
 * @brief   Enable/disable vertical flip
 * @param   hov7670 Pointer to OV7670 handle
 * @param   enable True to enable flip, false to disable
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetFlipVertical(OV7670_Handle_t *hov7670, bool enable)
{
    OV7670_StatusTypeDef status;
    uint8_t reg_val = 0;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Read current MVFP register */
    status = OV7670_ReadReg(hov7670, OV7670_REG_MVFP, &reg_val);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set/clear VFlip bit */
    if (enable) {
        reg_val |= 0x10;  /* Set VFlip bit */
    } else {
        reg_val &= ~0x10; /* Clear VFlip bit */
    }

    status = OV7670_WriteReg(hov7670, OV7670_REG_MVFP, reg_val);
    if (status == OV7670_OK) {
        hov7670->config.flip_vertical = enable;
    }

    return status;
}

/**
 * @brief   Enable/disable night mode
 * @param   hov7670 Pointer to OV7670 handle
 * @param   enable True to enable night mode, false to disable
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetNightMode(OV7670_Handle_t *hov7670, bool enable)
{
    OV7670_StatusTypeDef status;
    uint8_t reg_val = 0;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Read current COM11 register */
    status = OV7670_ReadReg(hov7670, OV7670_REG_COM11, &reg_val);
    if (status != OV7670_OK) {
        return status;
    }

    /* Set/clear night mode bit */
    if (enable) {
        reg_val |= 0x80;  /* Set night mode bit */
    } else {
        reg_val &= ~0x80; /* Clear night mode bit */
    }

    status = OV7670_WriteReg(hov7670, OV7670_REG_COM11, reg_val);
    if (status == OV7670_OK) {
        hov7670->config.night_mode = enable;
    }

    return status;
}

/**
 * @brief   Set test pattern
 * @param   hov7670 Pointer to OV7670 handle
 * @param   pattern Test pattern mode
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_SetTestPattern(OV7670_Handle_t *hov7670, uint8_t pattern)
{
    OV7670_StatusTypeDef status;
    uint8_t reg_val;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    /* Read current SCALING_XSC register */
    status = OV7670_ReadReg(hov7670, OV7670_REG_SCALING_XSC, &reg_val);
    if (status != OV7670_OK) {
        return status;
    }

    /* Clear test pattern bits */
    reg_val &= ~0x0A;

    /* Set test pattern */
    switch (pattern) {
        case OV7670_TEST_PATTERN_1:
            reg_val |= 0x02;
            break;
        case OV7670_TEST_PATTERN_2:
            reg_val |= 0x08;
            break;
        case OV7670_TEST_PATTERN_BARS:
            reg_val |= 0x0A;
            break;
        case OV7670_TEST_PATTERN_NONE:
        default:
            /* No test pattern */
            break;
    }

    status = OV7670_WriteReg(hov7670, OV7670_REG_SCALING_XSC, reg_val);
    if (status == OV7670_OK) {
        hov7670->config.test_pattern = pattern;
    }

    return status;
}

/**
 * @brief   Get camera chip ID
 * @param   hov7670 Pointer to OV7670 handle
 * @param   chip_id Pointer to store chip ID
 * @retval  OV7670_StatusTypeDef Operation status
 */
OV7670_StatusTypeDef OV7670_GetChipID(OV7670_Handle_t *hov7670, uint16_t *chip_id)
{
    OV7670_StatusTypeDef status;
    uint8_t pid, ver;

    if (hov7670 == NULL || chip_id == NULL) {
        return OV7670_INVALID_PARAM;
    }

    /* Read PID and VER registers */
    status = OV7670_ReadReg(hov7670, OV7670_REG_PID, &pid);
    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_ReadReg(hov7670, OV7670_REG_VER, &ver);
    if (status != OV7670_OK) {
        return status;
    }

    *chip_id = ((uint16_t)pid << 8) | ver;

    return OV7670_OK;
}

/**
 * @brief   Get current camera status
 * @param   hov7670 Pointer to OV7670 handle
 * @retval  OV7670_StatusTypeDef Current status
 */
OV7670_StatusTypeDef OV7670_GetStatus(OV7670_Handle_t *hov7670)
{
    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (!hov7670->initialized) {
        return OV7670_NOT_INITIALIZED;
    }

    return OV7670_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Write register value
 * @param   hov7670 Pointer to OV7670 handle
 * @param   reg Register address
 * @param   value Value to write
 * @retval  OV7670_StatusTypeDef Operation status
 */
static OV7670_StatusTypeDef OV7670_WriteReg(OV7670_Handle_t *hov7670, uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_I2C_Mem_Write(hov7670->hi2c, OV7670_I2C_ADDRESS, reg,
                                  I2C_MEMADD_SIZE_8BIT, &value, 1, OV7670_I2C_TIMEOUT);

    if (hal_status != HAL_OK) {
        return OV7670_I2C_ERROR;
    }

    return OV7670_OK;
}

/**
 * @brief   Read register value
 * @param   hov7670 Pointer to OV7670 handle
 * @param   reg Register address
 * @param   value Pointer to store read value
 * @retval  OV7670_StatusTypeDef Operation status
 */
static OV7670_StatusTypeDef OV7670_ReadReg(OV7670_Handle_t *hov7670, uint8_t reg, uint8_t *value)
{
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_I2C_Mem_Read(hov7670->hi2c, OV7670_I2C_ADDRESS, reg,
                                 I2C_MEMADD_SIZE_8BIT, value, 1, OV7670_I2C_TIMEOUT);

    if (hal_status != HAL_OK) {
        return OV7670_I2C_ERROR;
    }

    return OV7670_OK;
}

/**
 * @brief   Write register list
 * @param   hov7670 Pointer to OV7670 handle
 * @param   reg_list Register list to write
 * @retval  OV7670_StatusTypeDef Operation status
 */
static OV7670_StatusTypeDef OV7670_WriteRegList(OV7670_Handle_t *hov7670, const OV7670_RegVal_t *reg_list)
{
    OV7670_StatusTypeDef status;

    while (reg_list->reg_addr != 0xFF) {
        status = OV7670_WriteReg(hov7670, reg_list->reg_addr, reg_list->value);
        if (status != OV7670_OK) {
            return status;
        }
        reg_list++;
    }

    return OV7670_OK;
}

/**
 * @brief   Set resolution configuration
 * @param   hov7670 Pointer to OV7670 handle
 * @param   resolution Resolution to set
 * @retval  OV7670_StatusTypeDef Operation status
 */
static OV7670_StatusTypeDef OV7670_SetResolutionConfig(OV7670_Handle_t *hov7670, OV7670_ResolutionTypeDef resolution)
{
    OV7670_StatusTypeDef status;

    switch (resolution) {
        case OV7670_RES_QQVGA:
            /* QQVGA 160x120 - not directly supported, use QVGA with scaling */
            status = OV7670_WriteRegList(hov7670, OV7670_QVGA_RGB565);
            if (status != OV7670_OK) return status;
            /* Enable additional scaling for QQVGA */
            status = OV7670_WriteReg(hov7670, OV7670_REG_SCALING_DCWCTR, 0x33);
            break;

        case OV7670_RES_QVGA:
            status = OV7670_WriteRegList(hov7670, OV7670_QVGA_RGB565);
            break;

        case OV7670_RES_VGA:
            status = OV7670_WriteRegList(hov7670, OV7670_VGA_RGB565);
            break;

        default:
            return OV7670_INVALID_PARAM;
    }

    return status;
}

/**
 * @brief   Set format configuration
 * @param   hov7670 Pointer to OV7670 handle
 * @param   format Format to set
 * @retval  OV7670_StatusTypeDef Operation status
 */
static OV7670_StatusTypeDef OV7670_SetFormatConfig(OV7670_Handle_t *hov7670, OV7670_FormatTypeDef format)
{
    uint8_t com7_val, com15_val;

    /* Read current COM7 register */
    if (OV7670_ReadReg(hov7670, OV7670_REG_COM7, &com7_val) != OV7670_OK) {
        return OV7670_I2C_ERROR;
    }

    /* Clear format bits in COM7 */
    com7_val &= ~0x05;

    switch (format) {
        case OV7670_FMT_RGB565:
            com15_val = 0xC0;  /* RGB565 */
            break;

        case OV7670_FMT_RGB555:
            com15_val = 0xD0;  /* RGB555 */
            break;

        case OV7670_FMT_YUV422:
            com7_val |= 0x00;  /* YUV */
            com15_val = 0x00;  /* YUV422 */
            break;

        case OV7670_FMT_GRAYSCALE:
            com7_val |= 0x04;  /* Bayer RAW */
            com15_val = 0x10;  /* Raw Bayer */
            break;

        default:
            return OV7670_INVALID_PARAM;
    }

    /* Write COM7 register */
    if (OV7670_WriteReg(hov7670, OV7670_REG_COM7, com7_val) != OV7670_OK) {
        return OV7670_I2C_ERROR;
    }

    /* Write COM15 register */
    if (OV7670_WriteReg(hov7670, OV7670_REG_COM15, com15_val) != OV7670_OK) {
        return OV7670_I2C_ERROR;
    }

    return OV7670_OK;
}

/**
 * @brief   Delay in milliseconds
 * @param   delay Delay time in milliseconds
 */
static void OV7670_DelayMs(uint32_t delay)
{
    HAL_Delay(delay);
}
