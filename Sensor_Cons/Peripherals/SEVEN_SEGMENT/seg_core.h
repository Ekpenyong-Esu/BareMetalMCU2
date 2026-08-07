/**
 * @file    seg_core.h
 * @brief   Seven-segment display lifecycle and output control
 * @details Owns the handle: initialization, enable/disable, and the two
 *          refresh entry points. Backend selection happens here and nowhere
 *          else, so no function above this layer inspects the driver type.
 */

#ifndef SEG_CORE_H
#define SEG_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "seg_types.h"

/**
 * @brief   Initialize a seven-segment display
 * @param   handle Handle to initialize
 * @param   config Configuration to copy into the handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Init(SegDisplayHandle_t* handle, const SegDisplayConfig_t* config);

/**
 * @brief   Release a display and blank its output
 * @param   handle Display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_DeInit(SegDisplayHandle_t* handle);

/**
 * @brief   Enable display output
 * @param   handle Display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Enable(SegDisplayHandle_t* handle);

/**
 * @brief   Disable display output
 * @param   handle Display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Disable(SegDisplayHandle_t* handle);

/**
 * @brief   Blank every digit
 * @param   handle Display handle
 * @retval  SegStatus_t Operation status
 */
SegStatus_t Seg_Clear(SegDisplayHandle_t* handle);

/**
 * @brief   Advance the multiplex cursor by one digit
 * @param   handle Display handle
 * @retval  SegStatus_t Operation status
 * @note    Only multiplexed backends do work here; call it from a timer at
 *          >= 100Hz * digitCount for a flicker-free display. Backends that
 *          hold the whole frame themselves ignore it.
 */
SegStatus_t Seg_Update(SegDisplayHandle_t* handle);

/**
 * @brief   Push the pattern buffer to the hardware
 * @param   handle Display handle
 * @retval  SegStatus_t Operation status
 * @note    Called automatically by every function that changes the buffer.
 */
SegStatus_t Seg_Refresh(SegDisplayHandle_t* handle);

/**
 * @brief   Number of digits the display was configured with
 * @param   handle Display handle
 * @retval  uint8_t Digit count, zero if the handle is unusable
 */
uint8_t Seg_GetDigitCount(const SegDisplayHandle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* SEG_CORE_H */
