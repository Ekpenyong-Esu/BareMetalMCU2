/**
 ******************************************************************************
 * @file    mic_pdm.h
 * @brief   PDM bitstream to PCM decimation
 ******************************************************************************
 */

#ifndef MIC_PDM_H
#define MIC_PDM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mic_types.h"

/**
 * @brief Decimate a PDM bitstream into signed 16-bit PCM.
 * @param length Number of PCM samples to produce.
 */
MIC_StatusTypeDef MIC_PDMToPCM(const uint32_t *pdm_buffer, int16_t *pcm_buffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* MIC_PDM_H */
