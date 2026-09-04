/**
 ******************************************************************************
 * @file    audio_core.c
 * @brief   Audio handle lifecycle, registry and transport control
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

/** Live handles; the only file-scope state, kept so HAL callbacks and the
    DMA vector can find the caller's handle from a HAL handle. */
static AUDIO_Handle_t *s_registry[AUDIO_MAX_HANDLES] = {NULL};

/** Sample rate lookup, indexed by AUDIO_FreqTypeDef. */
static const uint32_t s_sampleRateHz[] = {
    [AUDIO_FREQ_8K] = 8000U,   [AUDIO_FREQ_11K] = 11025U, [AUDIO_FREQ_16K] = 16000U,
    [AUDIO_FREQ_22K] = 22050U, [AUDIO_FREQ_32K] = 32000U, [AUDIO_FREQ_44K] = 44100U,
    [AUDIO_FREQ_48K] = 48000U, [AUDIO_FREQ_96K] = 96000U,
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Resolve the transport backend for an interface selection
 * @details This is the only place in the audio subsystem that inspects
 *          AUDIO_InterfaceTypeDef.
 * @param   interface Requested transport
 * @retval  const AudioIfOps_t* Operations table, or NULL if unsupported
 */
static const AudioIfOps_t *Audio_OpsFor(AUDIO_InterfaceTypeDef interface) {
    switch (interface) {
        case AUDIO_INTERFACE_SAI:
            return &AudioSaiOps;
        case AUDIO_INTERFACE_I2S:
            return &AudioI2sOps;
        default:
            return NULL;
    }
}

/**
 * @brief   Claim a registry slot for a handle
 * @retval  AUDIO_StatusTypeDef AUDIO_BUSY when every slot is taken or the
 *          handle is already registered
 */
static AUDIO_StatusTypeDef Audio_Register(AUDIO_Handle_t *dev) {
    int32_t freeSlot = -1;

    for (uint32_t i = 0U; i < AUDIO_MAX_HANDLES; i++) {
        if (s_registry[i] == dev) {
            return AUDIO_BUSY;
        }
        if (s_registry[i] == NULL && freeSlot < 0) {
            freeSlot = (int32_t)i;
        }
    }

    if (freeSlot < 0) {
        return AUDIO_BUSY;
    }

    s_registry[freeSlot] = dev;
    return AUDIO_OK;
}

/**
 * @brief   Release the registry slot of a handle, if it holds one
 */
static void Audio_Unregister(const AUDIO_Handle_t *dev) {
    for (uint32_t i = 0U; i < AUDIO_MAX_HANDLES; i++) {
        if (s_registry[i] == dev) {
            s_registry[i] = NULL;
        }
    }
}

/**
 * @brief   Reject configs that name no hardware before anything is touched
 * @details Transport-specific fields (instance, alternate) are checked by the
 *          backend that reads them.
 */
static AUDIO_StatusTypeDef Audio_ValidateConfig(const AUDIO_ConfigTypeDef *config) {
    if (!Audio_PinIsWired(&config->wsPin) || !Audio_PinIsWired(&config->ckPin) ||
        !Audio_PinIsWired(&config->sdPin)) {
        return AUDIO_INVALID_PARAM;
    }
    if (config->EnableDMA && config->dmaStream == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    return AUDIO_OK;
}

/* Public functions ----------------------------------------------------------*/

uint32_t Audio_SampleRateHz(AUDIO_FreqTypeDef rate) {
    if ((uint32_t)rate >= (sizeof(s_sampleRateHz) / sizeof(s_sampleRateHz[0]))) {
        return s_sampleRateHz[AUDIO_FREQ_44K];
    }
    return s_sampleRateHz[rate];
}

AUDIO_Handle_t *AUDIO_HandleAt(uint32_t slot) {
    return (slot < AUDIO_MAX_HANDLES) ? s_registry[slot] : NULL;
}

AUDIO_Handle_t *AUDIO_FromSai(const SAI_HandleTypeDef *hsai) {
    for (uint32_t i = 0U; i < AUDIO_MAX_HANDLES; i++) {
        if (s_registry[i] != NULL && &s_registry[i]->sai == hsai) {
            return s_registry[i];
        }
    }
    return NULL;
}

AUDIO_Handle_t *AUDIO_FromI2s(const I2S_HandleTypeDef *hi2s) {
    for (uint32_t i = 0U; i < AUDIO_MAX_HANDLES; i++) {
        if (s_registry[i] != NULL && &s_registry[i]->i2s == hi2s) {
            return s_registry[i];
        }
    }
    return NULL;
}

AUDIO_StatusTypeDef AUDIO_Init(AUDIO_Handle_t *dev, const AUDIO_ConfigTypeDef *config) {
    if (dev == NULL || config == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    const AudioIfOps_t *ops = Audio_OpsFor(config->Interface);
    if (ops == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    AUDIO_StatusTypeDef status = Audio_ValidateConfig(config);
    if (status != AUDIO_OK) {
        return status;
    }

    status = Audio_Register(dev);
    if (status != AUDIO_OK) {
        return status;
    }

    memset(dev, 0, sizeof(*dev));
    dev->ops = ops;
    dev->config = *config;
    dev->volume = AUDIO_VOLUME_DEFAULT;

    status = Audio_BufferInit(dev);
    if (status != AUDIO_OK) {
        Audio_Unregister(dev);
        return status;
    }

    /* The transport must exist before the DMA can be linked to it. */
    status = ops->init(dev);
    if (status != AUDIO_OK) {
        Audio_Unregister(dev);
        return status;
    }

    if (dev->config.EnableDMA) {
        status = Audio_DmaInit(dev);
        if (status != AUDIO_OK) {
            ops->deinit(dev);
            Audio_Unregister(dev);
            return status;
        }
    }

    /* A transport-only handle (no control bus) still plays; the codec is
       simply somebody else's problem. */
    if (dev->config.codecBus != NULL) {
        status = Audio_CodecInit(dev, dev->volume);
        if (status != AUDIO_OK) {
            Audio_DmaDeInit(dev);
            ops->deinit(dev);
            Audio_Unregister(dev);
            return status;
        }
    }

    dev->state = AUDIO_STATE_READY;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_DeInit(AUDIO_Handle_t *dev) {
    if (dev == NULL || dev->ops == NULL) {
        return AUDIO_NOT_READY;
    }

    (void)AUDIO_Stop(dev);

    dev->ops->deinit(dev);

    if (dev->config.EnableDMA) {
        Audio_DmaDeInit(dev);
    }

    Audio_BufferRelease(dev);
    Audio_Unregister(dev);

    dev->state = AUDIO_STATE_RESET;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_CodecInit(AUDIO_Handle_t *dev) {
    if (dev == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    if (dev->config.codecBus == NULL) {
        return AUDIO_NOT_READY;
    }
    return Audio_CodecInit(dev, dev->volume);
}

AUDIO_StatusTypeDef AUDIO_Play(AUDIO_Handle_t *dev) {
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

AUDIO_StatusTypeDef AUDIO_Stop(AUDIO_Handle_t *dev) {
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

AUDIO_StatusTypeDef AUDIO_Pause(AUDIO_Handle_t *dev) {
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

AUDIO_StatusTypeDef AUDIO_Resume(AUDIO_Handle_t *dev) {
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

AUDIO_StatusTypeDef AUDIO_WriteBuffer(AUDIO_Handle_t *dev, const uint8_t *data, uint32_t size) {
    return Audio_BufferWrite(dev, data, size);
}

AUDIO_StatusTypeDef AUDIO_SetVolume(AUDIO_Handle_t *dev, uint8_t volume) {
    if (dev == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    if (volume > AUDIO_VOLUME_MAX) {
        volume = AUDIO_VOLUME_MAX;
    }

    AUDIO_StatusTypeDef status = Audio_CodecSetVolume(dev, volume);
    if (status != AUDIO_OK) {
        return status;
    }

    dev->volume = volume;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_GetVolume(const AUDIO_Handle_t *dev, uint8_t *volume) {
    if (dev == NULL || volume == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    *volume = dev->volume;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_SetMute(AUDIO_Handle_t *dev, bool mute) {
    if (dev == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    AUDIO_StatusTypeDef status = Audio_CodecSetMute(dev, mute);
    if (status != AUDIO_OK) {
        return status;
    }

    dev->muted = mute;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_GetMute(const AUDIO_Handle_t *dev, bool *mute) {
    if (dev == NULL || mute == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    *mute = dev->muted;
    return AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_GetStatus(const AUDIO_Handle_t *dev) {
    if (dev == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    return (dev->state == AUDIO_STATE_ERROR) ? AUDIO_ERROR : AUDIO_OK;
}

AUDIO_StatusTypeDef AUDIO_GetStatistics(const AUDIO_Handle_t *dev, AUDIO_StatsTypeDef *stats) {
    if (dev == NULL || stats == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    *stats = dev->stats;
    return AUDIO_OK;
}

const char *AUDIO_GetStatusString(AUDIO_StatusTypeDef status) {
    switch (status) {
        case AUDIO_OK:
            return "OK";
        case AUDIO_ERROR:
            return "Error";
        case AUDIO_BUSY:
            return "Busy";
        case AUDIO_TIMEOUT:
            return "Timeout";
        case AUDIO_INVALID_PARAM:
            return "Invalid Parameter";
        case AUDIO_NOT_READY:
            return "Not Ready";
        case AUDIO_OVERFLOW:
            return "Buffer Overflow";
        case AUDIO_UNDERFLOW:
            return "Buffer Underflow";
        default:
            return "Unknown";
    }
}
