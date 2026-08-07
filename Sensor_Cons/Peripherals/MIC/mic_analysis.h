/**
  ******************************************************************************
  * @file    mic_analysis.h
  * @brief   Frequency-domain analysis of the captured PCM buffer
  ******************************************************************************
  */

#ifndef MIC_ANALYSIS_H
#define MIC_ANALYSIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mic_types.h"

/**
 * @brief Fill magnitude spectrum, spectral centroid and zero-crossing rate.
 * @note  Requires a captured buffer; returns MIC_BUSY until one is ready.
 */
MIC_StatusTypeDef MIC_PerformAudioAnalysis(MIC_HandleTypeDef *hmic, MIC_AudioAnalysisTypeDef *analysis);

#ifdef __cplusplus
}
#endif

#endif /* MIC_ANALYSIS_H */
