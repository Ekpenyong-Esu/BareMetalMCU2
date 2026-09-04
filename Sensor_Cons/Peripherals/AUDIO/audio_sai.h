/**
 ******************************************************************************
 * @file    audio_sai.h
 * @brief   SAI transport backend for the audio subsystem
 * @details Exposes nothing but its operations table; every function that
 *          implements the table is private to audio_sai.c. The block and
 *          pins are read from the handle's config.
 ******************************************************************************
 */

#ifndef AUDIO_SAI_H
#define AUDIO_SAI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_types.h"

/** Operations table for AUDIO_INTERFACE_SAI. */
extern const AudioIfOps_t AudioSaiOps;

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_SAI_H */
