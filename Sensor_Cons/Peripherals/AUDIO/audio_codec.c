/**
 ******************************************************************************
 * @file    audio_codec.c
 * @brief   CS43L22 codec control implementation
 ******************************************************************************
 */

#include "audio_codec.h"
#include "gpio.h"
#include "i2c.h"

/* Private defines -----------------------------------------------------------*/

/**
 * @brief Control interface (Cirrus Logic CS43L22)
 * @note  Control is I2C, audio data is I2S/SAI. This is the 8-bit write
 *        address of the part with AD0 tied low (7-bit 0x4A); a board that
 *        straps AD0 high passes 0x96 in the config instead.
 */
#define AUDIO_CODEC_I2C_ADDRESS_DEFAULT 0x94U
#define AUDIO_CODEC_I2C_TIMEOUT I2C_TIMEOUT_DEFAULT

#define AUDIO_CODEC_RESET_DELAY_MS 10U

#define AUDIO_CODEC_CHIP_ID 0xE0U      /* CS43L22 part number field */
#define AUDIO_CODEC_CHIP_ID_MASK 0xF8U /* Upper 5 bits hold the part number */

#define AUDIO_VOLUME_SCALE 100U            /* Volume percentage scale */
#define AUDIO_CODEC_VOLUME_FULL_SCALE 255U /* Master volume register spans 8 bits */

/**
 * @brief Master volume code of the quietest setting
 * @note  The master volume register is a wrapping 256-step ramp: 0x19 is the
 *        quietest (-51.5 dB), it climbs to 0xFF (-0.5 dB), then 0x00 (0 dB) up
 *        to 0x18 (+12 dB). Adding this offset with 8-bit wraparound therefore
 *        turns a linear 0..255 level into a monotonic register code.
 */
#define AUDIO_CODEC_VOLUME_ZERO_CODE 0x19U

/**
 * @brief Register values applied during playback configuration
 */
#define AUDIO_CODEC_POWER_DOWN 0x01U       /* Power control 1: powered down */
#define AUDIO_CODEC_POWER_UP 0x9EU         /* Power control 1: powered up */
#define AUDIO_CODEC_OUTPUT_HEADPHONE 0xAFU /* Power control 2: headphone on, speaker off */
#define AUDIO_CODEC_CLOCK_AUTODETECT 0x81U /* Clocking control: auto-detect MCLK ratio */
#define AUDIO_CODEC_INTERFACE_I2S 0x04U    /* Interface control 1: I2S slave, 16-bit */
#define AUDIO_CODEC_ANALOG_ZC_SR 0x00U     /* No zero-cross / soft-ramp on the analog path */
#define AUDIO_CODEC_HEADPHONE_MUTE 0xC0U   /* Playback control 2: mute both HP channels */
#define AUDIO_CODEC_HEADPHONE_UNMUTE 0x00U

/* Private types -------------------------------------------------------------*/

/**
 * @brief CS43L22 register addresses
 */
typedef enum {
    CODEC_REG_ID = 0x01,
    CODEC_REG_POWER_CTL1 = 0x02,
    CODEC_REG_POWER_CTL2 = 0x04,
    CODEC_REG_CLOCKING_CTL = 0x05,
    CODEC_REG_INTERFACE_CTL1 = 0x06,
    CODEC_REG_ANALOG_ZC_SR = 0x0A,
    CODEC_REG_PLAYBACK_CTL2 = 0x0F,
    CODEC_REG_MASTER_VOL_A = 0x20,
    CODEC_REG_MASTER_VOL_B = 0x21
} AUDIO_CodecRegTypeDef;

/** A single register write in a configuration sequence. */
typedef struct {
    uint8_t reg;
    uint8_t value;
} AudioCodecWrite_t;

/* Private variables ---------------------------------------------------------*/

/** Register sequence that puts the codec into 16-bit I2S playback mode. */
static const AudioCodecWrite_t s_playbackSetup[] = {
    {CODEC_REG_POWER_CTL1, AUDIO_CODEC_POWER_DOWN},
    {CODEC_REG_POWER_CTL2, AUDIO_CODEC_OUTPUT_HEADPHONE},
    {CODEC_REG_CLOCKING_CTL, AUDIO_CODEC_CLOCK_AUTODETECT},
    {CODEC_REG_INTERFACE_CTL1, AUDIO_CODEC_INTERFACE_I2S},
    {CODEC_REG_ANALOG_ZC_SR, AUDIO_CODEC_ANALOG_ZC_SR},
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Write one codec register over I2C
 * @param   dev   Handle that owns the codec device
 * @param   reg   Register address
 * @param   value Value to write
 * @retval  AUDIO_StatusTypeDef Operation status
 */
static AUDIO_StatusTypeDef Audio_CodecWrite(AUDIO_Handle_t *dev, uint8_t reg, uint8_t value) {
    if (I2C_Mem_Write(&dev->codec, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, AUDIO_CODEC_I2C_TIMEOUT) !=
        I2C_OK) {
        return AUDIO_ERROR;
    }
    return AUDIO_OK;
}

/**
 * @brief   Read one codec register over I2C
 * @param   dev   Handle that owns the codec device
 * @param   reg   Register address
 * @param   value Destination for the register contents
 * @retval  AUDIO_StatusTypeDef Operation status
 */
static AUDIO_StatusTypeDef Audio_CodecRead(AUDIO_Handle_t *dev, uint8_t reg, uint8_t *value) {
    if (value == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    if (I2C_Mem_Read(&dev->codec, reg, I2C_MEMADD_SIZE_8BIT, value, 1, AUDIO_CODEC_I2C_TIMEOUT) !=
        I2C_OK) {
        return AUDIO_ERROR;
    }
    return AUDIO_OK;
}

/**
 * @brief   Drive the codec reset line low and then high
 * @details RESET is active low and must be released before the codec answers
 *          on I2C. Boards that tie RESET high leave the pin unwired.
 * @param   reset Reset line from the config
 * @retval  None
 */
static void Audio_CodecReleaseReset(const AUDIO_Pin_t *reset) {
    if (!Audio_PinIsWired(reset)) {
        return;
    }

    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin = reset->pin;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    (void)GPIO_Driver_Pin_Init(reset->port, &gpioInit);

    HAL_GPIO_WritePin(reset->port, reset->pin, GPIO_PIN_RESET);
    HAL_Delay(AUDIO_CODEC_RESET_DELAY_MS);
    HAL_GPIO_WritePin(reset->port, reset->pin, GPIO_PIN_SET);
    HAL_Delay(AUDIO_CODEC_RESET_DELAY_MS);
}

/**
 * @brief   Confirm that the device on the control bus is a CS43L22
 * @param   dev Handle that owns the codec device
 * @retval  AUDIO_StatusTypeDef AUDIO_OK when the part number matches
 */
static AUDIO_StatusTypeDef Audio_CodecVerifyId(AUDIO_Handle_t *dev) {
    uint8_t chipId = 0;

    AUDIO_StatusTypeDef status = Audio_CodecRead(dev, CODEC_REG_ID, &chipId);
    if (status != AUDIO_OK) {
        return status;
    }

    return ((chipId & AUDIO_CODEC_CHIP_ID_MASK) == AUDIO_CODEC_CHIP_ID) ? AUDIO_OK : AUDIO_ERROR;
}

/**
 * @brief   Guard for volume and mute: the codec must have been registered
 */
static AUDIO_StatusTypeDef Audio_CodecCheck(const AUDIO_Handle_t *dev) {
    if (dev == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    if (!I2C_DeviceIsReady(&dev->codec)) {
        return AUDIO_NOT_READY;
    }
    return AUDIO_OK;
}

/* Public functions ----------------------------------------------------------*/

AUDIO_StatusTypeDef Audio_CodecInit(AUDIO_Handle_t *dev, uint8_t volume) {
    if (dev == NULL || dev->config.codecBus == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    const I2C_ConfigTypeDef busConfig = I2C_ConfigDefault();
    uint16_t address = (dev->config.codecAddress != 0U) ? dev->config.codecAddress
                                                        : AUDIO_CODEC_I2C_ADDRESS_DEFAULT;

    Audio_CodecReleaseReset(&dev->config.codecResetPin);

    /* The control bus is shared; registering only records what the codec needs
       and the bus is programmed on the first transfer. */
    if (I2C_DeviceInit(&dev->codec, dev->config.codecBus, address, &busConfig) != I2C_OK) {
        return AUDIO_ERROR;
    }

    AUDIO_StatusTypeDef status = Audio_CodecVerifyId(dev);
    if (status != AUDIO_OK) {
        return status;
    }

    for (uint32_t i = 0; i < (sizeof(s_playbackSetup) / sizeof(s_playbackSetup[0])); i++) {
        status = Audio_CodecWrite(dev, s_playbackSetup[i].reg, s_playbackSetup[i].value);
        if (status != AUDIO_OK) {
            return status;
        }
    }

    status = Audio_CodecSetVolume(dev, volume);
    if (status != AUDIO_OK) {
        return status;
    }

    return Audio_CodecWrite(dev, CODEC_REG_POWER_CTL1, AUDIO_CODEC_POWER_UP);
}

AUDIO_StatusTypeDef Audio_CodecSetVolume(AUDIO_Handle_t *dev, uint8_t volume) {
    AUDIO_StatusTypeDef status = Audio_CodecCheck(dev);
    if (status != AUDIO_OK) {
        return status;
    }

    if (volume > AUDIO_VOLUME_MAX) {
        volume = AUDIO_VOLUME_MAX;
    }

    uint8_t level =
        (uint8_t)(((uint32_t)volume * AUDIO_CODEC_VOLUME_FULL_SCALE) / AUDIO_VOLUME_SCALE);
    uint8_t regValue = (uint8_t)(level + AUDIO_CODEC_VOLUME_ZERO_CODE);

    status = Audio_CodecWrite(dev, CODEC_REG_MASTER_VOL_A, regValue);
    if (status != AUDIO_OK) {
        return status;
    }

    return Audio_CodecWrite(dev, CODEC_REG_MASTER_VOL_B, regValue);
}

AUDIO_StatusTypeDef Audio_CodecSetMute(AUDIO_Handle_t *dev, bool mute) {
    AUDIO_StatusTypeDef status = Audio_CodecCheck(dev);
    if (status != AUDIO_OK) {
        return status;
    }

    uint8_t regValue = mute ? AUDIO_CODEC_HEADPHONE_MUTE : AUDIO_CODEC_HEADPHONE_UNMUTE;

    return Audio_CodecWrite(dev, CODEC_REG_PLAYBACK_CTL2, regValue);
}
