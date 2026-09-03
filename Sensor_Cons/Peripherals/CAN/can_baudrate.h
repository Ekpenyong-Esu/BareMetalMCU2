/**
  ******************************************************************************
  * @file    can_baudrate.h
  * @brief   CAN bit-timing solver
  * @details Derives prescaler, BS1, BS2 and SJW from the live APB1 clock so
  *          the same presets work on any clock tree. Replaces the former
  *          fixed-14-tq table that only divided 42 MHz exactly.
  *
  * CAN Bit Timing:
  * - One bit = 1 sync quantum + BS1 (1..16 tq) + BS2 (1..8 tq)
  * - Sample point placed near 87.5% (CiA recommendation) via CAN_SplitBit()
  * - Solver searches every legal bit length (8..25 tq) for an exact
  *   PCLK1 / (baud * tq) prescaler, picking the split closest to 87.5%
  * - Custom timing: when baud_rate == 0 the caller supplies raw register values
  ******************************************************************************
  */

#ifndef CAN_BAUDRATE_H
#define CAN_BAUDRATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/**
 * @brief   Derive and apply bit-timing for a baud rate
 *
 * When config->baud_rate is non-zero the solver searches all legal bit
 * lengths for an exact prescaler at the current PCLK1 and writes the best
 * BS1/BS2/SJW. When baud_rate == 0 the raw prescaler/sync_jump_width/
 * time_segment_* fields are copied verbatim.
 *
 * @param   hcan HAL handle whose Init timing fields will be filled
 * @param   config Bus config carrying baud_rate or raw timing
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR when no exact
 *          prescaler exists or parameters are out of range
 * @note    Only fills the timing fields; does not call HAL_CAN_Init.
 */
HAL_StatusTypeDef CAN_ApplyBaudRate(CAN_HandleTypeDef *hcan, const CAN_Config *config);

#ifdef __cplusplus
}
#endif

#endif /* CAN_BAUDRATE_H */
