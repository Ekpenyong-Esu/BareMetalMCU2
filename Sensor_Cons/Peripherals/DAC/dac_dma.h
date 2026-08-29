/**
 * @file dac_dma.h
 * @brief Streams a sample buffer to the DAC over DMA
 *
 * The DAC raises one DMA request per conversion trigger, so the transfer rate
 * is set by whatever drives the trigger (a timer TRGO) and not by the CPU.
 * In circular mode the buffer repeats forever with no software involvement.
 *
 * Only DAC_CHANNEL_1 is supported, which is the only channel wired on the
 * Discovery board. It is served by DMA1 Stream 5, Channel 7.
 */

#ifndef DAC_DMA_H
#define DAC_DMA_H

#include "dac_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start streaming @p samples to the DAC, repeating forever.
 * @param samples 12-bit right-aligned codes; must stay valid until stopped.
 * @param length  Number of entries in @p samples.
 */
HAL_StatusTypeDef DAC_StartCircularDMA(DAC_HandleStruct *hdac, uint32_t channel,
                                       uint32_t *samples, uint32_t length);

/** Stop an in-progress DMA stream. Leaves the last value on the output. */
HAL_StatusTypeDef DAC_StopDMA(DAC_HandleStruct *hdac, uint32_t channel);

/** Service the DAC's DMA stream. Call only from DMA1_Stream5_IRQHandler(). */
void DAC_DMA_IrqHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* DAC_DMA_H */
