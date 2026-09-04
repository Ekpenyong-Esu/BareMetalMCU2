/**
 ******************************************************************************
 * @file    mic_pdm.c
 * @brief   PDM bitstream to PCM decimation
 ******************************************************************************
 */

#include "mic_pdm.h"

MIC_StatusTypeDef MIC_PDMToPCM(const uint32_t *pdm_buffer, int16_t *pcm_buffer, uint32_t length) {
    if (pdm_buffer == NULL || pcm_buffer == NULL || length == 0) {
        return MIC_INVALID_PARAM;
    }

    const uint32_t decimation = MIC_DECIMATION_FACTOR;

    for (uint32_t i = 0; i < length; i++) {
        int32_t accumulator = 0;

        /* Sum +1/-1 over one decimation window; the density of set bits is the sample */
        for (uint32_t j = 0; j < decimation; j++) {
            uint32_t bit_index = (i * decimation) + j;
            uint32_t pdm_word = pdm_buffer[bit_index / MIC_PDM_WORD_SIZE];

            if (pdm_word & (1UL << (bit_index % MIC_PDM_WORD_SIZE))) {
                accumulator++;
            }
            else {
                accumulator--;
            }
        }

        pcm_buffer[i] = (int16_t)((accumulator * MIC_PCM_MAX_VALUE) / (int32_t)decimation);
    }

    return MIC_OK;
}
