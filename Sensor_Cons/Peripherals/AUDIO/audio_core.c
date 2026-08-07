/**
  ******************************************************************************
  * @file    audio_core.c
  * @brief   Audio subsystem lifecycle and transport control
  ******************************************************************************
  */

#include "audio_core.h"
#include "audio_buffer.h"
#include "audio_codec.h"
#include "audio_dma.h"
#include "audio_i2s.h"
#include "audio_sai.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/

/** The board carries a single codec, hence a single device record. */
static AudioDevice_t s_device = {
    .volume = AUDIO_VOLUME_DEFAULT,
    .state  = AUDIO_STATE_RESET,
};

/** Sample rate lookup, indexed by AUDIO_FreqTypeDef. */
static const uint32_t s_sampleRateHz[] = {
    [AUDIO_FREQ_8K]  = 8000U,
    [AUDIO_FREQ_11K] = 11025U,
    [AUDIO_FREQ_16K] = 16000U,
    [AUDIO_FREQ_22K] = 22050U,
    [AUDIO_FREQ_32K] = 32000U,
    [AUDIO_FREQ_44K] = 44100U,
    [AUDIO_FREQ_48K] = 48000U,
    [AUDIO_FREQ_96K] = 96000U,
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Resolve the transport backend for an interface selection
 * @details This is the only place in the audio subsystem that inspects
 *          AUDIO_InterfaceTypeDef.
 * @param   interface Requested transport
 * @retval  const AudioIfOps_t* Operations table, or NULL if unsupported
 */
static const AudioIfOps_t* Audio_OpsFor(AUDIO_InterfaceTypeDef interface)
{
    switch (interface) {
        case AUDIO_INTERFACE_SAI: return &AudioSaiOps;
        case AUDIO_INTERFACE_I2S: return &AudioI2sOps;
        default:                  return NULL;
    }
}

/* Public functions ----------------------------------------------------------*/

uint32_t Audio_SampleRateHz(AUDIO_FreqTypeDef rate)
{
    if ((uint32_t)rate >= (sizeof(s_sampleRateHz) / sizeof(s_sampleRateHz[0]))) {
        return s_sampleRateHz[AUDIO_FREQ_44K];
    }
    return s_sampleRateHz[rate];
}

AudioDevice_t* AUDIO_Device(void)
{
    return &s_device;
}

AUDIO_StatusTypeDef AUDIO_Init(void)
{
    const AUDIO_ConfigTypeDef defaultConfig = {
        .Interface = AUDIO_INTERFACE_SAI,
        .SampleRate = AUDIO_FREQ_44K,
        .BitDepth = AUDIO_FORMAT_16BIT,
        .Channels = AUDIO_CHANNEL_STEREO,
        .BufferSize = AUDIO_BUFFER_SIZE_DEFAULT,
        .EnableDMA = true
    };

    return AUDIO_Init_Custom(&defaultConfig);
}

AUDIO_StatusTypeDef AUDIO_Init_Custom(const AUDIO_ConfigTypeDef* config)
{
    AudioDevice_t* dev = &s_device;

    if (config == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    const AudioIfOps_t* ops = Audio_OpsFor(config->Interface);
    if (ops == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    dev->ops = ops;
    dev->config = *config;
    memset(&dev->stats, 0, sizeof(dev->stats));

    AUDIO_StatusTypeDef status = Audio_BufferInit(dev);
    if (status != AUDIO_OK) {
        return status;
    }

    /* The transport must exist before the DMA can be linked to it. */
    status = ops->init(dev);
    if (status != AUDIO_OK) {
        return status;
    }

    if (dev->config.EnableDMA) {
        status = Audio_DmaInit(dev);
        if (status != AUDIO_OK) {
            return status;
        }
    }

    status = Audio_CodecInit(dev->volume);
    if (status != AUDIO_OK) {
        return status;
    }

    dev->state = AUDIO_STATE_READY;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_DeInit(void)
{
    AudioDevice_t* dev = &s_device;

    if (dev->ops == NULL) {
        return AUDIO_NOT_READY;
    }

    (void)AUDIO_Stop();

    dev->ops->deinit(dev);

    if (dev->config.EnableDMA) {
        Audio_DmaDeInit(dev);
    }

    Audio_BufferRelease(dev);

    dev->state = AUDIO_STATE_RESET;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_CodecInit(void)
{
    return Audio_CodecInit(s_device.volume);
}

AUDIO_StatusTypeDef AUDIO_Play(void)
{
    AudioDevice_t* dev = &s_device;

    AUDIO_StatusTypeDef status = Audio_CheckReady(dev);
    if (status != AUDIO_OK) {
        return status;
    }
    if (dev->state != AUDIO_STATE_READY && dev->state != AUDIO_STATE_PAUSED) {
        return AUDIO_NOT_READY;
    }

    status = dev->ops->start(dev);
    if (status != AUDIO_OK) {
        return status;
    }

    dev->state = AUDIO_STATE_PLAYING;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_Stop(void)
{
    AudioDevice_t* dev = &s_device;

    AUDIO_StatusTypeDef status = Audio_CheckReady(dev);
    if (status != AUDIO_OK) {
        return status;
    }
    if (dev->state != AUDIO_STATE_PLAYING && dev->state != AUDIO_STATE_PAUSED) {
        return AUDIO_NOT_READY;
    }

    dev->ops->stop(dev);
    Audio_BufferReset(dev);

    dev->state = AUDIO_STATE_READY;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_Pause(void)
{
    AudioDevice_t* dev = &s_device;

    AUDIO_StatusTypeDef status = Audio_CheckReady(dev);
    if (status != AUDIO_OK) {
        return status;
    }
    if (dev->state != AUDIO_STATE_PLAYING) {
        return AUDIO_NOT_READY;
    }

    dev->ops->pause(dev);

    dev->state = AUDIO_STATE_PAUSED;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_Resume(void)
{
    AudioDevice_t* dev = &s_device;

    AUDIO_StatusTypeDef status = Audio_CheckReady(dev);
    if (status != AUDIO_OK) {
        return status;
    }
    if (dev->state != AUDIO_STATE_PAUSED) {
        return AUDIO_NOT_READY;
    }

    dev->ops->resume(dev);

    dev->state = AUDIO_STATE_PLAYING;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_WriteBuffer(const uint8_t* data, uint32_t size)
{
    return Audio_BufferWrite(&s_device, data, size);
}

AUDIO_StatusTypeDef AUDIO_SetVolume(uint8_t volume)
{
    if (volume > AUDIO_VOLUME_MAX) {
        volume = AUDIO_VOLUME_MAX;
    }

    AUDIO_StatusTypeDef status = Audio_CodecSetVolume(volume);
    if (status != AUDIO_OK) {
        return status;
    }

    s_device.volume = volume;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_GetVolume(uint8_t* volume)
{
    if (volume == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    *volume = s_device.volume;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_SetMute(bool mute)
{
    AUDIO_StatusTypeDef status = Audio_CodecSetMute(mute);
    if (status != AUDIO_OK) {
        return status;
    }

    s_device.muted = mute;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_GetMute(bool* mute)
{
    if (mute == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    *mute = s_device.muted;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_GetStatus(void)
{
    return (s_device.state == AUDIO_STATE_ERROR) ? AUDIO_ERROR : AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_GetStatistics(AUDIO_StatsTypeDef* stats)
{
    if (stats == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    *stats = s_device.stats;
    return AUDIO_OK;
}

const char* AUDIO_GetStatusString(AUDIO_StatusTypeDef status)
{
    switch (status) {
        case AUDIO_OK:              return "OK";
        case AUDIO_ERROR:           return "Error";
        case AUDIO_BUSY:            return "Busy";
        case AUDIO_TIMEOUT:         return "Timeout";
        case AUDIO_INVALID_PARAM:   return "Invalid Parameter";
        case AUDIO_NOT_READY:       return "Not Ready";
        case AUDIO_OVERFLOW:        return "Buffer Overflow";
        case AUDIO_UNDERFLOW:       return "Buffer Underflow";
        default:                    return "Unknown";
    }
}
