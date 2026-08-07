/**
 * @file ir_decode.h
 * @brief Protocol decoding of a captured IR pulse train
 */

#ifndef IR_DECODE_H
#define IR_DECODE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ir_types.h"

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Decode the captured pulse train into handle->rxFrame
 * @note  When the configured protocol is IR_PROTOCOL_CUSTOM every supported
 *        protocol is tried in turn.
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL_OK when a frame was recognised
 */
HAL_StatusTypeDef IR_DecodeFrame(IR_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* IR_DECODE_H */
