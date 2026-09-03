/**
  ******************************************************************************
  * @file    can_baudrate.c
  * @brief   CAN bit-timing solver implementation
  *
  * This module derives bxCAN bit timing from the live APB1 clock:
  * - Solver searches every legal bit length (8..25 tq) for an exact
  *   PCLK1 / (baud * tq) prescaler, picking the split closest to 87.5%
  * - CAN_SplitBit() places the sample point inside a bit of tq quanta
  * - Custom timing path copies raw register values when baud_rate == 0
  *
  * Key Design Points:
  * - No fixed 14-tq assumption — 14 only divides 42 MHz exactly, so a table
  *   baked for one clock tree fails on 36/45/48 MHz boards
  * - Sample point target is 87.5% (CiA recommendation) in permille
  * - SJW is capped at BS2 and at 4 (hardware limit)
  */

#include "can_baudrate.h"
#include "stm32f4xx_hal.h"

/* Private constants ---------------------------------------------------------*/

/*
 * bxCAN splits a bit into 1 sync quantum + BS1 + BS2, with BS1 1..16 tq and
 * BS2 1..8 tq. Bits shorter than 8 tq leave too little room to place the
 * sample point, so the search stops there.
 */
#define CAN_TQ_MIN                  8U    /*!< Shortest bit the solver considers */
#define CAN_TQ_MAX                  25U   /*!< Longest bit (1 + 16 + 8) */
#define CAN_TSEG1_MAX               16U   /*!< BS1 register limit */
#define CAN_TSEG2_MAX               8U    /*!< BS2 register limit */
#define CAN_SJW_MAX                 4U    /*!< SJW register limit */

#define CAN_PRESCALER_MIN           1U
#define CAN_PRESCALER_MAX           1024U

/** @brief Target sample point (CiA recommends 87.5%), in permille */
#define CAN_SAMPLE_POINT_PERMILLE   875U

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Place the sample point inside a bit of @p tq quanta
 *
 * Computes BS1/BS2 so the sample point lands near 87.5% of the bit,
 * clamped to the BS1/BS2 register limits.
 *
 * @param tq Total quanta per bit (8..25)
 * @param tseg1 Output BS1 length (quanta, 1..16)
 * @param tseg2 Output BS2 length (quanta, 1..8)
 * @param sample_permille Output actual sample point in permille
 * @retval bool true when the split satisfies BS1/BS2 limits, false otherwise
 */
static bool CAN_SplitBit(uint32_t tq, uint32_t *tseg1, uint32_t *tseg2,
                         uint32_t *sample_permille)
{
    /* Quanta up to and including the sample point, i.e. sync + BS1. */
    uint32_t sampled = ((tq * CAN_SAMPLE_POINT_PERMILLE) + 500U) / 1000U;
    uint32_t s1;
    uint32_t s2;

    if (sampled < 2U) {
        sampled = 2U;
    }
    if (sampled > (tq - 1U)) {
        sampled = tq - 1U;
    }

    s1 = sampled - 1U;
    if (s1 > CAN_TSEG1_MAX) {
        s1 = CAN_TSEG1_MAX;
    }

    s2 = tq - 1U - s1;
    if (s2 < 1U || s2 > CAN_TSEG2_MAX) {
        return false;
    }

    *tseg1 = s1;
    *tseg2 = s2;
    *sample_permille = ((1U + s1) * 1000U) / tq;

    return true;
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Derive and apply bit-timing for a baud rate
 * @param hcan HAL handle whose Init timing fields will be filled
 * @param config Bus config carrying baud_rate or raw timing
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR when no exact
 *         prescaler exists or parameters are out of range
 */
HAL_StatusTypeDef CAN_ApplyBaudRate(CAN_HandleTypeDef *hcan, const CAN_Config *config)
{
    uint32_t pclk1 = 0U;
    uint32_t best_prescaler = 0U;
    uint32_t best_tseg1 = 0U;
    uint32_t best_tseg2 = 0U;
    uint32_t best_error = UINT32_MAX;
    uint32_t sjw;

    if (hcan == NULL || config == NULL) {
        return HAL_ERROR;
    }

    /* Custom timing: the caller supplies raw register values. */
    if (config->baud_rate == 0U) {
        if (config->prescaler < CAN_PRESCALER_MIN || config->prescaler > CAN_PRESCALER_MAX) {
            return HAL_ERROR;
        }

        hcan->Init.Prescaler     = config->prescaler;
        hcan->Init.SyncJumpWidth = config->sync_jump_width;
        hcan->Init.TimeSeg1      = config->time_segment_1;
        hcan->Init.TimeSeg2      = config->time_segment_2;

        return HAL_OK;
    }

    /* Derive the timing from the live APB1 frequency rather than a table baked
       for one clock setup. Search every legal bit length instead of fixing it
       at 14 tq: 14 divides 42 MHz exactly but not 36, 45 or 48 MHz, so a fixed
       length makes init fail outright on most other boards. */
    pclk1 = HAL_RCC_GetPCLK1Freq();

    for (uint32_t tq = CAN_TQ_MAX; tq >= CAN_TQ_MIN; tq--) {
        uint32_t divider = config->baud_rate * tq;
        uint32_t prescaler;
        uint32_t tseg1;
        uint32_t tseg2;
        uint32_t sample_point;
        uint32_t error;

        if (divider == 0U || (pclk1 % divider) != 0U) {
            continue;
        }

        prescaler = pclk1 / divider;
        if (prescaler < CAN_PRESCALER_MIN || prescaler > CAN_PRESCALER_MAX) {
            continue;
        }

        if (!CAN_SplitBit(tq, &tseg1, &tseg2, &sample_point)) {
            continue;
        }

        error = (sample_point > CAN_SAMPLE_POINT_PERMILLE)
                    ? (sample_point - CAN_SAMPLE_POINT_PERMILLE)
                    : (CAN_SAMPLE_POINT_PERMILLE - sample_point);

        if (error < best_error) {
            best_error     = error;
            best_prescaler = prescaler;
            best_tseg1     = tseg1;
            best_tseg2     = tseg2;
        }
    }

    if (best_prescaler == 0U) {
        return HAL_ERROR;
    }

    /* Resynchronisation may not extend past BS2, and the hardware caps it at 4. */
    sjw = (best_tseg2 < CAN_SJW_MAX) ? best_tseg2 : CAN_SJW_MAX;

    hcan->Init.Prescaler     = best_prescaler;
    hcan->Init.TimeSeg1      = (best_tseg1 - 1U) << CAN_BTR_TS1_Pos;
    hcan->Init.TimeSeg2      = (best_tseg2 - 1U) << CAN_BTR_TS2_Pos;
    hcan->Init.SyncJumpWidth = (sjw - 1U) << CAN_BTR_SJW_Pos;

    return HAL_OK;
}
