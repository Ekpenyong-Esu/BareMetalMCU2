/**
 ******************************************************************************
 * @file    can_filter.h
 * @brief   CAN filter bank configuration
 * @details Packs driver filter configs into bxCAN register layout and
 *          programs them via HAL_CAN_ConfigFilter(). Handles both 32-bit
 *          and 16-bit scale, standard and extended identifiers.
 *
 * CAN Filter Responsibilities:
 * - CAN_ConfigFilter: Validate, pack and program one filter bank
 * - Packing helpers: CAN_PackFilter32 / CAN_PackFilter16 (in can_filter.c)
 * - Filter bank count and slave split are config-driven (not compile-time)
 *
 * Filter Bank Layout (bxCAN):
 * - 32-bit scale: one filter per bank — STID[10:0] at bits 31..21,
 *   EXID[17:0] at bits 20..3, IDE at bit 2, RTR at bit 1
 * - 16-bit scale: two filters per bank — STID at bits 15..5, IDE at bit 3,
 *   EXID[17:15] at bits 2..0; this driver mirrors one id/mask pair to both halves
 ******************************************************************************
 */

#ifndef CAN_FILTER_H
#define CAN_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/**
 * @brief   Program one filter bank
 *
 * Validates the bank index against the handle's filter_bank_count, checks
 * FIFO and identifier range, packs the id/mask into register layout, and
 * calls HAL_CAN_ConfigFilter().
 *
 * @param   hcan Handle (must be initialized)
 * @param   filter_config Filter to program
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid params
 */
HAL_StatusTypeDef CAN_ConfigFilter(CAN_Handle_t *hcan, const CAN_FilterConfig *filter_config);

#ifdef __cplusplus
}
#endif

#endif /* CAN_FILTER_H */
