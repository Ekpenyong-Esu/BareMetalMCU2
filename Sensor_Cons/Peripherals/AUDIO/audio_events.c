/**
  ******************************************************************************
  * @file    audio_events.c
  * @brief   Interrupt-context handlers for the audio subsystem
  * @details Deliberately has no header: HAL weak callbacks are overridden by
  *          definition, and the vector entry point is declared in audio_core.h
  *          because Core owns the vector table. Keeping all interrupt context
  *          in one file makes it obvious which code runs at ISR priority.
  ******************************************************************************
  */

#include "audio_core.h"
#include "audio_buffer.h"

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Retire the half-buffer the DMA has just finished streaming
 * @param   dev Device that owns the transfer
 * @retval  None
 */
static void Audio_OnTransferComplete(AudioDevice_t* dev)
{
    Audio_BufferAdvanceRead(dev);

    uint32_t frameSize = Audio_BufferFrameSize(dev);
    if (frameSize != 0U) {
        dev->stats.SampleCount += (dev->output.Size / 2U) / frameSize;
    }
}

/**
 * @brief   Record an unrecoverable transfer fault
 * @param   dev Device that owns the transfer
 * @retval  None
 */
static void Audio_OnTransferError(AudioDevice_t* dev)
{
    dev->state = AUDIO_STATE_ERROR;
    dev->stats.SyncErrors++;
}

/* Interrupt entry points ----------------------------------------------------*/

void AUDIO_IRQHandler(void)
{
    AudioDevice_t* dev = AUDIO_Device();

    if (dev->dma.Instance != NULL) {
        HAL_DMA_IRQHandler(&dev->dma);
    }
}

/* HAL transfer callbacks - only act on handles owned by this driver */

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef* hsai)
{
    AudioDevice_t* dev = AUDIO_Device();

    if (hsai == &dev->sai) {
        Audio_OnTransferComplete(dev);
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef* hsai)
{
    AudioDevice_t* dev = AUDIO_Device();

    if (hsai == &dev->sai) {
        Audio_OnTransferError(dev);
    }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef* hi2s)
{
    AudioDevice_t* dev = AUDIO_Device();

    if (hi2s == &dev->i2s) {
        Audio_OnTransferComplete(dev);
    }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef* hi2s)
{
    AudioDevice_t* dev = AUDIO_Device();

    if (hi2s == &dev->i2s) {
        Audio_OnTransferError(dev);
    }
}
