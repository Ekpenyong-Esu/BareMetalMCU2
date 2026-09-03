/**
  ******************************************************************************
  * @file    can_filter.c
  * @brief   CAN filter bank configuration implementation
  *
  * This module packs driver filter configs into bxCAN register layout:
  * - CAN_PackFilter32: 32-bit scale — one filter per bank
  * - CAN_PackFilter16: 16-bit scale — two filters per bank (mirrored)
  * - CAN_ConfigFilter: Validate, pack and program via HAL_CAN_ConfigFilter()
  *
  * Key Design Points:
  * - Filter bank count and slave split come from the handle's config, not
  *   a compile-time constant (varies single- vs dual-CAN parts)
  * - 16-bit banks hold two independent filters; this API mirrors one pair
  * - IDs are shifted into register position — raw values match nothing
  */

#include "can_filter.h"
#include "can_core.h"

/* Private constants ---------------------------------------------------------*/

/*
 * bxCAN filter register layout, 32-bit scale:
 *   bits 31..21  STID[10:0]
 *   bits 20..3   EXID[17:0]
 *   bit  2       IDE
 *   bit  1       RTR
 *   bit  0       reserved
 *
 * 16-bit scale (each half-register):
 *   bits 15..5   STID[10:0]
 *   bit  4       RTR
 *   bit  3       IDE
 *   bits 2..0    EXID[17:15]
 */
#define CAN_FILTER32_STID_SHIFT      21U   /*!< Standard ID shift in 32-bit filter */
#define CAN_FILTER32_EXID_SHIFT      3U    /*!< Extended ID shift in 32-bit filter */
#define CAN_FILTER32_IDE_BIT         (1UL << 2) /*!< IDE flag in 32-bit filter */

#define CAN_FILTER16_STID_SHIFT      5U    /*!< Standard ID shift in 16-bit filter */
#define CAN_FILTER16_IDE_BIT         (1U << 3) /*!< IDE flag in 16-bit filter */
#define CAN_FILTER16_EXID_SHIFT      15U   /*!< Extended ID high bits shift */
#define CAN_FILTER16_EXID_MASK       0x07U /*!< Extended ID high bits mask (3 bits) */

#define CAN_ID_HIGH_SHIFT            16U     /*!< Split 32-bit filter word into halves */
#define CAN_ID_LOW_MASK              0xFFFFU /*!< Low half mask */

#define CAN_STD_ID_MAX               0x7FFU       /*!< 11-bit standard ID limit */
#define CAN_EXT_ID_MAX               0x1FFFFFFFU /*!< 29-bit extended ID limit */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Pack an identifier into 32-bit filter register layout
 * @param id Identifier (11-bit standard or 29-bit extended)
 * @param frame_type Standard vs extended
 * @retval uint32_t Packed filter word
 */
static uint32_t CAN_PackFilter32(uint32_t id, CAN_FrameType frame_type)
{
    if (frame_type == CAN_FRAME_EXTENDED) {
        return (id << CAN_FILTER32_EXID_SHIFT) | CAN_FILTER32_IDE_BIT;
    }

    return id << CAN_FILTER32_STID_SHIFT;
}

/**
 * @brief Pack an identifier into 16-bit filter register layout
 * @param id Identifier (11-bit standard or 29-bit extended)
 * @param frame_type Standard vs extended
 * @retval uint16_t Packed filter half-word
 */
static uint16_t CAN_PackFilter16(uint32_t id, CAN_FrameType frame_type)
{
    if (frame_type == CAN_FRAME_EXTENDED) {
        return (uint16_t)(((id >> CAN_FILTER16_EXID_SHIFT) & CAN_FILTER16_EXID_MASK) |
                          CAN_FILTER16_IDE_BIT);
    }

    return (uint16_t)((id & CAN_STD_ID_MAX) << CAN_FILTER16_STID_SHIFT);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Program one filter bank
 * @param hcan Handle (must be initialized)
 * @param filter_config Filter to program
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid params
 */

HAL_StatusTypeDef CAN_ConfigFilter(CAN_Handle_t *hcan, const CAN_FilterConfig *filter_config)
{
    CAN_FilterTypeDef filter = {0};
    uint32_t idMax = 0U;

    if (hcan == NULL || filter_config == NULL ||
        filter_config->filter_bank >= hcan->config.filter_bank_count) {
        return HAL_ERROR;
    }

    if (filter_config->fifo > 1U) {
        return HAL_ERROR;
    }

    idMax = (filter_config->frame_type == CAN_FRAME_EXTENDED) ? CAN_EXT_ID_MAX : CAN_STD_ID_MAX;
    if (filter_config->id > idMax || filter_config->mask > idMax) {
        return HAL_ERROR;
    }

    filter.FilterBank  = filter_config->filter_bank;
    filter.FilterMode  = filter_config->mode;
    filter.FilterScale = filter_config->scale;

    /* The ID has to be shifted into its register position; storing the raw
       value split across the two halves matches no CAN identifier format and
       makes the filter reject everything. */
    if (filter_config->scale == CAN_FILTER_SCALE_32BIT) {
        uint32_t id   = CAN_PackFilter32(filter_config->id, filter_config->frame_type);
        uint32_t mask = CAN_PackFilter32(filter_config->mask, filter_config->frame_type);

        filter.FilterIdHigh     = (uint16_t)(id >> CAN_ID_HIGH_SHIFT);
        filter.FilterIdLow      = (uint16_t)(id & CAN_ID_LOW_MASK);
        filter.FilterMaskIdHigh = (uint16_t)(mask >> CAN_ID_HIGH_SHIFT);
        filter.FilterMaskIdLow  = (uint16_t)(mask & CAN_ID_LOW_MASK);
    } else {
        /* A 16-bit bank holds two independent filters; this API carries one
           id/mask pair, so the same pair is written to both halves. */
        uint16_t id   = CAN_PackFilter16(filter_config->id, filter_config->frame_type);
        uint16_t mask = CAN_PackFilter16(filter_config->mask, filter_config->frame_type);

        filter.FilterIdHigh     = id;
        filter.FilterIdLow      = id;
        filter.FilterMaskIdHigh = mask;
        filter.FilterMaskIdLow  = mask;
    }

    filter.FilterFIFOAssignment = (filter_config->fifo == 0U) ? CAN_RX_FIFO0 : CAN_RX_FIFO1;
    filter.FilterActivation     = filter_config->enable ? ENABLE : DISABLE;

    /* Leaving this at 0 hands every bank to CAN2, so CAN1 would never accept a
       frame. Banks below the split belong to CAN1. */
    filter.SlaveStartFilterBank = hcan->config.slave_start_bank;

    return HAL_CAN_ConfigFilter(&hcan->hal, &filter);
}
