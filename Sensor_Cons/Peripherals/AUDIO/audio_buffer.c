/**
 ******************************************************************************
 * @file    audio_buffer.c
 * @brief   Playback ring buffer implementation
 ******************************************************************************
 */

#include "audio_buffer.h"

/* Public functions ----------------------------------------------------------*/

AUDIO_StatusTypeDef Audio_BufferInit(AUDIO_Handle_t *dev) {
    if (dev == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    if (dev->config.BufferSize == 0U || dev->config.BufferSize > sizeof(dev->outputMemory)) {
        return AUDIO_ERROR;
    }

    /* The memory lives in the handle so two devices never share a buffer. */
    dev->output.Buffer = dev->outputMemory;
    dev->output.Size = dev->config.BufferSize;
    Audio_BufferReset(dev);

    return AUDIO_OK;
}

void Audio_BufferRelease(AUDIO_Handle_t *dev) {
    if (dev == NULL) {
        return;
    }

    dev->output.Buffer = NULL;
    dev->output.Size = 0U;
    Audio_BufferReset(dev);
}

void Audio_BufferReset(AUDIO_Handle_t *dev) {
    if (dev == NULL) {
        return;
    }

    dev->output.ReadIndex = 0U;
    dev->output.WriteIndex = 0U;
    dev->output.IsEmpty = true;
    dev->output.IsFull = false;
}

uint32_t Audio_BufferFreeSpace(const AUDIO_Handle_t *dev) {
    if (dev == NULL || dev->output.Buffer == NULL) {
        return 0U;
    }

    /* Equal indices mean either full or empty, so the flag decides. */
    if (dev->output.IsFull) {
        return 0U;
    }
    if (dev->output.WriteIndex >= dev->output.ReadIndex) {
        return dev->output.Size - (dev->output.WriteIndex - dev->output.ReadIndex);
    }
    return dev->output.ReadIndex - dev->output.WriteIndex;
}

AUDIO_StatusTypeDef Audio_BufferWrite(AUDIO_Handle_t *dev, const uint8_t *data, uint32_t size) {
    if (dev == NULL || data == NULL || size == 0U || dev->output.Buffer == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    if (size > Audio_BufferFreeSpace(dev)) {
        dev->stats.BufferOverflows++;
        return AUDIO_OVERFLOW;
    }

    for (uint32_t written = 0U; written < size; written++) {
        dev->output.Buffer[dev->output.WriteIndex] = data[written];
        dev->output.WriteIndex = (dev->output.WriteIndex + 1U) % dev->output.Size;
    }

    dev->output.IsEmpty = false;
    if (dev->output.WriteIndex == dev->output.ReadIndex) {
        dev->output.IsFull = true;
    }

    return AUDIO_OK;
}

void Audio_BufferAdvanceRead(AUDIO_Handle_t *dev) {
    if (dev == NULL || dev->output.Buffer == NULL) {
        return;
    }

    dev->output.ReadIndex = (dev->output.ReadIndex + dev->output.Size / 2U) % dev->output.Size;

    if (dev->output.ReadIndex == dev->output.WriteIndex) {
        dev->output.IsEmpty = true;
    }
    dev->output.IsFull = false;
}

uint32_t Audio_BufferFrameSize(const AUDIO_Handle_t *dev) {
    if (dev == NULL) {
        return 0U;
    }

    uint32_t bytes = (dev->config.BitDepth == AUDIO_FORMAT_16BIT) ? 2U : 4U;
    if (dev->config.Channels == AUDIO_CHANNEL_STEREO) {
        bytes *= 2U;
    }
    return bytes;
}
