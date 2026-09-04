/**
 ******************************************************************************
 * @file    audio_events.c
 * @brief   Interrupt-context handlers for the audio subsystem
 * @details Deliberately has no header: HAL weak callbacks are overridden by
 *          definition, and the vector entry point is declared in audio_core.h
 *          because Core owns the vector table. Keeping all interrupt context
 *          in one file makes it obvious which code runs at ISR priority.
 *
 *          HAL handles carry no parent pointer, so every callback resolves
 *          the owning handle through the registry in audio_core.c.
 ******************************************************************************
 */

#include "audio_core.h"
#include "audio_buffer.h"

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Retire the half-buffer the DMA has just finished streaming
 * @param   dev Handle that owns the transfer
 * @retval  None
 */
static void Audio_OnTransferComplete(AUDIO_Handle_t *dev) {
    Audio_BufferAdvanceRead(dev);

    uint32_t frameSize = Audio_BufferFrameSize(dev);
    if (frameSize != 0U) {
        dev->stats.SampleCount += (dev->output.Size / 2U) / frameSize;
    }
}

/**
 * @brief   Record an unrecoverable transfer fault
 * @param   dev Handle that owns the transfer
 * @retval  None
 */
static void Audio_OnTransferError(AUDIO_Handle_t *dev) {
    dev->state = AUDIO_STATE_ERROR;
    dev->stats.SyncErrors++;
}

/* Interrupt entry points ----------------------------------------------------*/

void AUDIO_IRQHandler(void) {
    /* One vector may serve any registered stream; HAL_DMA_IRQHandler only
       acts on the flags its own stream has raised. */
    for (uint32_t i = 0U; i < AUDIO_MAX_HANDLES; i++) {
        AUDIO_Handle_t *dev = AUDIO_HandleAt(i);

        if (dev != NULL && dev->dma.Instance != NULL) {
            HAL_DMA_IRQHandler(&dev->dma);
        }
    }
}

/* HAL transfer callbacks - only act on handles owned by this driver */

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    AUDIO_Handle_t *dev = AUDIO_FromSai(hsai);

    if (dev != NULL) {
        Audio_OnTransferComplete(dev);
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai) {
    AUDIO_Handle_t *dev = AUDIO_FromSai(hsai);

    if (dev != NULL) {
        Audio_OnTransferError(dev);
    }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
    AUDIO_Handle_t *dev = AUDIO_FromI2s(hi2s);

    if (dev != NULL) {
        Audio_OnTransferComplete(dev);
    }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s) {
    AUDIO_Handle_t *dev = AUDIO_FromI2s(hi2s);

    if (dev != NULL) {
        Audio_OnTransferError(dev);
    }
}
