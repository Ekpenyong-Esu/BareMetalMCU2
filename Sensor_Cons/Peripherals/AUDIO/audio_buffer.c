/**
  ******************************************************************************
  * @file    audio_buffer.c
  * @brief   Playback ring buffer implementation
  ******************************************************************************
  */

#include "audio_buffer.h"

/* Private variables ---------------------------------------------------------*/

/** Statically reserved playback memory; the driver never allocates. */
static uint8_t s_outputMemory[AUDIO_BUFFER_SIZE_DEFAULT];

/* Public functions ----------------------------------------------------------*/

AUDIO_StatusTypeDef Audio_BufferInit(AudioDevice_t* dev)
{
    if (dev == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    if (dev->config.BufferSize == 0U || dev->config.BufferSize > sizeof(s_outputMemory)) {
        return AUDIO_ERROR;
    }

    dev->output.Buffer = s_outputMemory;
    dev->output.Size = dev->config.BufferSize;
    Audio_BufferReset(dev);

    return AUDIO_OK;
}

void Audio_BufferRelease(AudioDevice_t* dev)
{
    if (dev == NULL) {
        return;
    }

    dev->output.Buffer = NULL;
    dev->output.Size = 0U;
    Audio_BufferReset(dev);
}

void Audio_BufferReset(AudioDevice_t* dev)
{
    if (dev == NULL) {
        return;
    }

    dev->output.ReadIndex = 0U;
    dev->output.WriteIndex = 0U;
    dev->output.IsEmpty = true;
    dev->output.IsFull = false;
}

uint32_t Audio_BufferFreeSpace(const AudioDevice_t* dev)
{
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

AUDIO_StatusTypeDef Audio_BufferWrite(AudioDevice_t* dev, const uint8_t* data, uint32_t size)
{
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

void Audio_BufferAdvanceRead(AudioDevice_t* dev)
{
    if (dev == NULL || dev->output.Buffer == NULL) {
        return;
    }

    dev->output.ReadIndex = (dev->output.ReadIndex + dev->output.Size / 2U) % dev->output.Size;

    if (dev->output.ReadIndex == dev->output.WriteIndex) {
        dev->output.IsEmpty = true;
    }
    dev->output.IsFull = false;
}

uint32_t Audio_BufferFrameSize(const AudioDevice_t* dev)
{
    if (dev == NULL) {
        return 0U;
    }

    uint32_t bytes = (dev->config.BitDepth == AUDIO_FORMAT_16BIT) ? 2U : 4U;
    if (dev->config.Channels == AUDIO_CHANNEL_STEREO) {
        bytes *= 2U;
    }
    return bytes;
}
